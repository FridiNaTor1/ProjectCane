#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct ExtractedLevel
{
    std::string name;
    std::filesystem::path path;
};

struct IsoExtractionResult
{
    bool ok = false;
    bool usedCache = false;
    std::string message;
    std::filesystem::path cacheDirectory;
    std::vector<ExtractedLevel> levels;
};

const std::vector<std::string>& Sly1RetailLevelNames();
IsoExtractionResult EnsureSly1RetailIsoExtracted(const std::filesystem::path& isoPath);
