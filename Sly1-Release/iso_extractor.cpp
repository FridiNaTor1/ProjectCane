#include "iso_extractor.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

namespace
{
const std::vector<std::string> kLevelNames = {
    "Paris.brx",
    "The Hideout.brx",
    "A Stealthy Approach.brx",
    "Prowling the Grounds.brx",
    "High Class Heist.brx",
    "Into the Machine.brx",
    "A Cunning Disguise.brx",
    "The Fire Down Below.brx",
    "Treasure in the Depths.brx",
    "The Gunboat Graveyard.brx",
    "The Eye of the Storm.brx",
    "A Rocky Start.brx",
    "Muggshot's Turf.brx",
    "Boneyard Casino.brx",
    "Murray's Big Gamble.brx",
    "At the Dog Track.brx",
    "Two to Tango.brx",
    "Straight to the Top.brx",
    "Back Alley Heist.brx",
    "Last Call.brx",
    "The Dread Swamp Path.brx",
    "The Swamp's Dark Center.brx",
    "The Lair of the Beast.brx",
    "A Grave Undertaking.brx",
    "Piranha Lake.brx",
    "Descent into Danger.brx",
    "A Ghastly Voyage.brx",
    "Down Home Cooking.brx",
    "A Deadly Dance.brx",
    "A Perilous Ascent.brx",
    "Inside the Stronghold.brx",
    "Flaming Temple of Flame.brx",
    "The Unseen Foe.brx",
    "The King of the Hill.brx",
    "Rapid Fire Assault.brx",
    "Duel by the Dragon.brx",
    "A Desperate Race.brx",
    "Flame Fu!.brx",
    "A Hazardous Path.brx",
    "Burning Rubber.brx",
    "A Daring Rescue.brx",
    "Bentley Comes Through.brx",
    "A Temporary Truce.brx",
    "Sinking Peril.brx",
    "A Strange Reunion.brx",
};

std::filesystem::path UserCacheRoot()
{
    if (const char* xdgCacheHome = std::getenv("XDG_CACHE_HOME"))
    {
        if (*xdgCacheHome)
            return std::filesystem::path(xdgCacheHome) / "projectcane";
    }

    if (const char* home = std::getenv("HOME"))
    {
        if (*home)
            return std::filesystem::path(home) / ".cache" / "projectcane";
    }

    return std::filesystem::current_path() / ".projectcane-cache";
}

std::string SanitizePathComponent(std::string value)
{
    for (char& c : value)
    {
        const unsigned char uc = static_cast<unsigned char>(c);
        if (!std::isalnum(uc) && c != '-' && c != '_')
            c = '_';
    }
    return value;
}

std::filesystem::path CacheDirectoryForIso(const std::filesystem::path& isoPath)
{
    std::error_code ec;
    const auto absolute = std::filesystem::absolute(isoPath, ec);
    const auto canonicalish = ec ? isoPath : absolute;
    const auto size = std::filesystem::file_size(isoPath, ec);

    std::uint64_t stamp = 0;
    const auto mtime = std::filesystem::last_write_time(isoPath, ec);
    if (!ec)
        stamp = static_cast<std::uint64_t>(mtime.time_since_epoch().count());

    std::ostringstream id;
    id << SanitizePathComponent(canonicalish.stem().string()) << "_" << size << "_" << stamp;
    return UserCacheRoot() / id.str();
}

bool CacheLooksComplete(const std::filesystem::path& cacheDirectory)
{
    std::error_code ec;
    for (const auto& levelName : kLevelNames)
    {
        const auto path = cacheDirectory / levelName;
        if (!std::filesystem::is_regular_file(path, ec) || std::filesystem::file_size(path, ec) == 0)
            return false;
    }
    return true;
}

std::vector<ExtractedLevel> BuildLevelList(const std::filesystem::path& cacheDirectory)
{
    std::vector<ExtractedLevel> levels;
    levels.reserve(kLevelNames.size());

    for (const auto& levelName : kLevelNames)
        levels.push_back({levelName, cacheDirectory / levelName});

    return levels;
}

void Decompress(std::vector<char>& fileBuffer, std::uint64_t size, std::vector<unsigned char>& outputData)
{
    std::vector<char> outputDataWindow;
    outputDataWindow.resize(0x4000);

    const std::uint32_t actualOutputDataSize = static_cast<std::uint32_t>(10 * size);
    std::vector<char> actualOutputData;
    actualOutputData.resize(actualOutputDataSize);

    const std::uint64_t inputSize = size;
    std::uint64_t inputPos = 0;
    std::uint64_t outputPos = 0;

    unsigned char bits = 0;
    unsigned short src = 0;
    short ssize = 0;
    short offset = 0;
    unsigned long k = 0;

    while (inputPos < inputSize)
    {
        bits = static_cast<unsigned char>(fileBuffer[inputPos++]);
        if (inputPos >= inputSize)
            break;

        for (int i = 0; i < 8; i++)
        {
            src = static_cast<unsigned char>(fileBuffer[inputPos++]);
            if (inputPos >= inputSize)
                break;

            if (bits & 1)
            {
                outputDataWindow[outputPos++] = static_cast<char>(src);
                if (outputPos >= 0x2000)
                {
                    outputPos &= 0x1fff;
                    std::memcpy(&actualOutputData[0] + (k++ * 0x2000), &outputDataWindow[0], 0x2000);
                }
            }
            else
            {
                src |= (static_cast<unsigned short>(fileBuffer[inputPos++]) << 8);
                ssize = (src >> 13) + 2;
                offset = src & 0x1FFF;
                while (ssize >= 0)
                {
                    --ssize;
                    outputDataWindow[outputPos++] = outputDataWindow[offset];
                    if (outputPos >= 0x2000)
                    {
                        outputPos &= 0x1fff;
                        std::memcpy(&actualOutputData[0] + (k++ * 0x2000), &outputDataWindow[0], 0x2000);
                    }
                    offset = (offset + 1) & 0x1FFF;
                }
            }
            bits >>= 1;
        }
    }

    std::memcpy(&actualOutputData[0] + (k++ * 0x2000), &outputDataWindow[0], outputPos);

    const size_t outputSize = k * 0x2000 + outputPos;
    outputData.resize(outputSize);
    std::memcpy(&outputData[0], &actualOutputData[0], outputSize);
}

bool ReadU32(std::ifstream& stream, std::uint32_t& value)
{
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return stream.good();
}
}

const std::vector<std::string>& Sly1RetailLevelNames()
{
    return kLevelNames;
}

IsoExtractionResult EnsureSly1RetailIsoExtracted(const std::filesystem::path& isoPath)
{
    IsoExtractionResult result;
    result.cacheDirectory = CacheDirectoryForIso(isoPath);

    std::error_code ec;
    if (!std::filesystem::is_regular_file(isoPath, ec))
    {
        result.message = "ISO path does not point to a regular file.";
        return result;
    }

    if (CacheLooksComplete(result.cacheDirectory))
    {
        result.ok = true;
        result.usedCache = true;
        result.message = "Using cached extracted maps.";
        result.levels = BuildLevelList(result.cacheDirectory);
        return result;
    }

    std::ifstream iso(isoPath, std::ios::binary);
    if (!iso)
    {
        result.message = "Failed to open ISO.";
        return result;
    }

    std::string region(4, '\0');
    iso.seekg(0x828BD, std::ios::beg);
    iso.read(&region[0], 4);
    if (region != "SCUS")
    {
        result.message = "Invalid ISO. Only retail NTSC Sly 1 ISOs are supported.";
        return result;
    }

    std::filesystem::create_directories(result.cacheDirectory, ec);
    if (ec)
    {
        result.message = "Failed to create cache directory: " + ec.message();
        return result;
    }

    iso.seekg(0x1D2B14, std::ios::beg);
    std::vector<char> fileBuffer;

    for (size_t i = 0; i < kLevelNames.size(); i++)
    {
        iso.seekg(0x8, std::ios::cur);

        std::uint32_t temp0 = 0;
        std::uint32_t temp1 = 0;
        std::uint32_t temp2 = 0;
        std::uint32_t temp3 = 0;
        std::uint32_t temp4 = 0;
        std::uint32_t temp5 = 0;
        std::uint32_t temp6 = 0;
        std::uint32_t temp7 = 0;

        if (!ReadU32(iso, temp0) || !ReadU32(iso, temp1) || !ReadU32(iso, temp2) || !ReadU32(iso, temp3) ||
            !ReadU32(iso, temp4) || !ReadU32(iso, temp5) || !ReadU32(iso, temp6) || !ReadU32(iso, temp7))
        {
            result.message = "Unexpected end of ISO while reading the level table.";
            return result;
        }

        const long size = static_cast<long>(temp1 ^ temp7);
        long sectorOffset = static_cast<long>(temp0 ^ temp5);
        sectorOffset *= 0x800;

        fileBuffer.resize(size);
        iso.seekg(4, std::ios::cur);
        const auto nextFileTable = iso.tellg();
        iso.seekg(sectorOffset, std::ios::beg);
        iso.read(&fileBuffer[0], size);
        if (!iso)
        {
            result.message = "Unexpected end of ISO while extracting " + kLevelNames[i] + ".";
            return result;
        }

        std::vector<unsigned char> outputData;
        Decompress(fileBuffer, size, outputData);

        const auto outputPath = result.cacheDirectory / kLevelNames[i];
        std::ofstream output(outputPath, std::ios::binary | std::ios::out);
        if (!output)
        {
            result.message = "Failed to write " + outputPath.string() + ".";
            return result;
        }

        output.write(reinterpret_cast<const char*>(&outputData[0]), static_cast<std::streamsize>(outputData.size()));
        iso.seekg(nextFileTable, std::ios::beg);
    }

    result.ok = true;
    result.usedCache = false;
    result.message = "Extracted maps to cache.";
    result.levels = BuildLevelList(result.cacheDirectory);
    return result;
}
