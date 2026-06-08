#include "glob.h"
#include "wr.h"

void LoadGlobsetFromBrx(GLOBSET* pglobset, ALO* palo, CBinaryInputStream* pbis)
{
    pglobset->cpsaa = 0;

    byte fRelight = pbis->U8Read();
    pbis->U8Read();
    pglobset->cbnd = pbis->U8Read();

    pglobset->mpibndoid.resize(pglobset->cbnd);

    for (int i = 0; i < pglobset->cbnd; i++)
        pglobset->mpibndoid[i] = (OID)pbis->S16Read();

    pglobset->cpose = pbis->U8Read();
    pglobset->agPoses.resize(pglobset->cpose);

    for (int i = 0; i < pglobset->cpose; i++)
        pglobset->agPoses[i] = pbis->F32Read();

    // Loading number of submodels for model
    pglobset->cglob = pbis->U16Read();

    pglobset->aglob.resize(pglobset->cglob);
    pglobset->aglobi.resize(pglobset->cglob);

    int fInstanceGlob = 0;
    int instanceIndex = 0;

    // Loading each submodel for a model
    for (int i = 0; i < pglobset->cglob; i++)
    {
        uint16_t globPropertys = pbis->U16Read();

        if ((globPropertys & 0x0001) == 0)
        {
            fInstanceGlob = 0;

            pglobset->aglob[i].sMRD = 1.0e10f;
            pglobset->aglob[i].sCelBorderMRD = 2000.0;
            pglobset->aglob[i].gZOrder = std::numeric_limits<float>::max();
            pglobset->aglob[i].uFog = 1.0;
            pglobset->aglob[i].rSubglobRadius = 1.0;
            pglobset->aglob[i].fDynamic = fRelight;
            pglobset->aglob[i].fTransluscentSort = 0;
            pglobset->aglobi[i].uAlpha = 1.0;
        }
        else
        {
            fInstanceGlob = globPropertys & 1;
            instanceIndex = pbis->S16Read();

            pglobset->aglob[i].VAO = pglobset->aglob[instanceIndex].VAO;
            pglobset->aglob[i].VBO = pglobset->aglob[instanceIndex].VBO;
            pglobset->aglob[i].EBO = pglobset->aglob[instanceIndex].EBO;
            pglobset->aglob[i].posCenter = pglobset->aglob[instanceIndex].posCenter;
            pglobset->aglob[i].sRadius = pglobset->aglob[instanceIndex].sRadius;
            pglobset->aglob[i].rp = pglobset->aglob[instanceIndex].rp;
            pglobset->aglob[i].sMRD = pglobset->aglob[instanceIndex].sMRD;
            pglobset->aglob[i].sCelBorderMRD = pglobset->aglob[instanceIndex].sCelBorderMRD;
            pglobset->aglob[i].gZOrder = pglobset->aglob[instanceIndex].gZOrder;
            pglobset->aglob[i].uFog = pglobset->aglob[instanceIndex].uFog;
            pglobset->aglob[i].fDynamic = pglobset->aglob[instanceIndex].fDynamic;
            pglobset->aglob[i].fTransluscentSort = pglobset->aglob[instanceIndex].fTransluscentSort;
            pglobset->aglob[i].fThreeWay = pglobset->aglob[instanceIndex].fThreeWay;
            pglobset->aglob[i].trlk = pglobset->aglob[instanceIndex].trlk;
            pglobset->aglob[i].grfshd = pglobset->aglob[instanceIndex].grfshd;
            pglobset->aglobi[i] = pglobset->aglobi[instanceIndex];

            glm::mat4 instanceModelMatrix = pbis->ReadMatrix4();

            std::shared_ptr <glm::mat4> mat = std::make_shared <glm::mat4>(instanceModelMatrix);
            pglobset->aglob[i].pdmat = mat;
        }

        if ((globPropertys & 2) != 0)
            pglobset->aglobi[i].grfzon = pbis->U32Read();

        if ((globPropertys & 0x200) != 0)
            pglobset->aglob[i].rSubglobRadius = pbis->F32Read();

        if (globPropertys & 0x04)
        {
            float gZOrder = pbis->F32Read();

            if (gZOrder == std::numeric_limits<float>::max())
                pglobset->aglob[i].gZOrder = gZOrder;
            else
                pglobset->aglob[i].gZOrder = gZOrder * std::abs(gZOrder);
        }

        if ((globPropertys & 8) != 0)
            pglobset->aglob[i].uFog = pbis->F32Read();

        if (globPropertys & 0x10)
        {
            const float v = pbis->F32Read();
            pglobset->aglob[i].sMRD = (v == std::numeric_limits<float>::max()) ? 1.0e10f : v;
        }

        if (globPropertys & 0x20)
        {
            float sCelBorderMRD = pbis->F32Read();
            pglobset->aglob[i].sCelBorderMRD = (sCelBorderMRD == std::numeric_limits<float>::max()) ? 2000.0f : sCelBorderMRD;
        }

        // Clamp afterward
        pglobset->aglob[i].sCelBorderMRD = std::min(pglobset->aglob[i].sCelBorderMRD, pglobset->aglob[i].sMRD);

        if ((globPropertys & 0x40) != 0)
        {
            pglobset->aglob[i].psaa = PsaaLoadFromBrx(pbis);

            if (pglobset->aglob[i].psaa != nullptr)
                pglobset->cpsaa++;
        }

        if ((globPropertys & 0x80) != 0)
        {
            GLEAM gleam{};
            gleam.normal = pbis->ReadVector();

            gleam.clqc.g0 = pbis->F32Read();
            gleam.clqc.g1 = pbis->F32Read();
            gleam.clqc.g2 = pbis->F32Read();
            gleam.clqc.g3 = pbis->F32Read();

            std::shared_ptr <GLEAM> pgleam = std::make_shared <GLEAM>(gleam);
            pglobset->aglob[i].gleam = pgleam;
        }

        if ((globPropertys & 0x100) != 0)
        {
            auto wrbgPtr = std::make_shared<WRBG>();
            WRBG& wrbg = *wrbgPtr;

            wrbg.palo = palo;
            wrbg.pglob = &pglobset->aglob[i];

            wrbg.oid = (OID)pbis->S16Read();
            wrbg.weki.wek = (WEK)pbis->S8Read();

            if (wrbg.weki.wek != WEK_Nil)
            {
                wrbg.weki.sInner = pbis->F32Read();
                wrbg.weki.uInner = pbis->F32Read();
                wrbg.weki.sOuter = pbis->F32Read();
                wrbg.weki.uOuter = pbis->F32Read();
                wrbg.weki.dmat   = pbis->ReadMatrix4();
            }

            // preserve original (char) cast behavior:
            wrbg.cmat  = (int8_t)pbis->U8Read();
            wrbg.fDpos = (int8_t)pbis->U8Read();
            wrbg.fDuv  = (int8_t)pbis->U8Read();

            if (wrbg.fDpos == 1 && wrbg.fDuv == 0)
                wrbg.warpType = WARP_POS;
            else if (wrbg.fDpos == 0 && wrbg.fDuv == 1)
                wrbg.warpType = WARP_UV;
            else if (wrbg.fDpos == 1 && wrbg.fDuv == 1)
                wrbg.warpType = WARP_BOTH;

            pglobset->aglob[i].pwrbg = wrbgPtr;
            wrbg.pwrbgNextGlobset    = pglobset->pwrbgFirst;
            pglobset->pwrbgFirst     = wrbgPtr;
        }

        pglobset->aglob[i].posCenter = pbis->ReadVector();
        pglobset->aglob[i].sRadius   = pbis->F32Read();
        pglobset->aglob[i].oid       = (OID)pbis->S16Read();
        pglobset->aglob[i].rtck      = (RTCK)pbis->U8Read();
        pglobset->aglob[i].rp        = (RP)pbis->U8Read();
        pglobset->aglob[i].grfglob   = pbis->U8Read();

        if (fInstanceGlob == 0)
        {
            int fProjVolume = 0;
            // Number of submodels
            // std::cout << "Model Start: " << std::hex << file.tellg()<<"\n";
            pglobset->aglob[i].csubglob = pbis->U16Read();
            pglobset->aglob[i].asubglob.resize(pglobset->aglob[i].csubglob);

            for (int a = 0; a < pglobset->aglob[i].csubglob; a++)
            {
                // Loading submodel origin
                pglobset->aglob[i].asubglob[a].posCenter = pbis->ReadVector();
                pglobset->aglob[i].asubglob[a].sRadius = pbis->F32Read();

                //std::cout << std::dec << "Vertex Count: " << (uint32_t)vertexCount << "\n";
                uint32_t vertexCount = pbis->U8Read();
                //std::cout << std::dec << "Rotations Count: " << (uint32_t)rotationsCount << "\n";
                uint32_t normalCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Color Count: " << (uint32_t)vertexColorCount << "\n";
                uint32_t vertexColorCount = pbis->U8Read();
                //std::cout << std::dec << "Texcoords Count: " << (uint32_t)texCoordCount << "\n";
                uint32_t texcoordCount = pbis->U8Read();
                //std::cout << std::dec << "Index Count: " << (uint32_t)indexCount << "\n";
                uint32_t indexCount = pbis->U8Read();

                std::vector <glm::vec3> vertexes;
                vertexes.resize(vertexCount);

                std::vector <glm::vec3>normals;
                normals.resize(normalCount);

                std::vector <glm::vec4> vertexColors;
                vertexColors.resize(vertexColorCount);

                std::vector <glm::vec2> texcoords;
                texcoords.resize(texcoordCount);

                std::vector <VTXFLG> indexes;
                indexes.resize(indexCount);

                pbis->Align(0x4);

                //std::cout << "Vertices: " << std::hex << pbis->file.tellg() << "\n";
                for (int b = 0; b < vertexCount; b++)
                    vertexes[b] = pbis->ReadVector();

                //std::cout << "Normals: " << std::hex << pbis->file.tellg() << "\n";
                for (int c = 0; c < normalCount; c++)
                    normals[c] = pbis->ReadVector();

                //std::cout << "Vertex Colors: " << std::hex << pbis->file.tellg() << "\n";
                for (int d = 0; d < vertexColorCount; d++)
                {
                    vertexColors[d].r = (pbis->U8Read() * 2.0f) / 0x1FE;
                    vertexColors[d].g = (pbis->U8Read() * 2.0f) / 0x1FE;
                    vertexColors[d].b = (pbis->U8Read() * 2.0f) / 0x1FE;
                    vertexColors[d].a = (pbis->U8Read() * 2.0f) / 0x1FE;
                }

                //std::cout << "Texcoords: " << std::hex << pbis->file.tellg() << "\n";
                for (int e = 0; e < texcoordCount; e++)
                    texcoords[e] = pbis->ReadVector2();

                //std::cout << "Indexes: " << std::hex << pbis->file.tellg() << "\n\n";
                for (int f = 0; f < indexCount; f++)
                {
                    indexes[f].ipos    = pbis->U8Read();
                    indexes[f].inormal = pbis->U8Read();
                    indexes[f].iuv     = pbis->U8Read();
                    indexes[f].bMisc   = pbis->U8Read();
                }

                // Loading texture property 
                pglobset->aglob[i].asubglob[a].shdID = pbis->U16Read();
                pglobset->aglob[i].asubglob[a].pshd  = &g_ashd[pglobset->aglob[i].asubglob[a].shdID];

                auto& glob = pglobset->aglob[i];
                auto* shd  = glob.asubglob[a].pshd;

                if (!glob.fTransluscentSort && shd)
                {
                    const uint32_t g = (uint32_t)shd->grfshd;

                    if (g == 2)
                    {
                        if (glob.rp == RP_Background ||
                            glob.rp == RP_Cutout ||
                            glob.rp == RP_CutoutAfterProjVolume ||
                            glob.rp == RP_Translucent)
                        {
                            glob.fTransluscentSort = 1;
                        }
                    }
                    else if (g == 6)
                    {
                        if (glob.rp == RP_Translucent)
                            glob.fTransluscentSort = 1;
                    }
                }

                if (glob.rp == RP_ProjVolume)
                    glob.grfshd = glob.asubglob[a].pshd->grfshd;

                pglobset->aglob[i].asubglob[a].unSelfIllum = static_cast<uint16_t>((pbis->U8Read() * 0x7FA6) / 0xFF);

                pglobset->aglob[i].asubglob[a].cibnd = pbis->U8Read();
                pglobset->aglob[i].asubglob[a].aibnd.resize(pglobset->aglob[i].asubglob[a].cibnd);

                for (int g = 0; g < pglobset->aglob[i].asubglob[a].cibnd; g++)
                    pglobset->aglob[i].asubglob[a].aibnd[g] = pbis->U8Read();

                int weightCount = vertexCount * pglobset->aglob[i].asubglob[a].cibnd;

                std::vector <float> agWeights;
                agWeights.resize(weightCount);

                for (int i = 0; i < weightCount; i++)
                    agWeights[i] = pbis->F32Read();

                std::vector <glm::vec3> posfPose;
                std::vector <glm::vec3> normalfPose;
                SUBPOSEF subposef;

                if (pglobset->cpose != 0)
                {
                    uint16_t posfPosesCount = pbis->U16Read();
                    posfPose.resize(posfPosesCount);

                    for (int g = 0; g < posfPosesCount; g++)
                        posfPose[g] = pbis->ReadVector();

                    uint16_t normalfPoseCount = pbis->U16Read();
                    normalfPose.resize(normalfPoseCount);

                    for (int h = 0; h < normalfPoseCount; h++)
                        normalfPose[h] = pbis->ReadVector();

                    subposef.aiposf.resize(indexCount);
                    subposef.ainormalf.resize(indexCount);

                    for (int j = 0; j < pglobset->cpose; j++)
                    {
                        for (int a = 0; a < indexCount; a++)
                            subposef.aiposf[a] = pbis->U16Read();

                        for (int b = 0; b < indexCount; b++)
                            subposef.ainormalf[b] = pbis->U16Read();
                    }
                }

                BuildSubGlob(&pglobset->aglob[i], &pglobset->aglob[i].asubglob[a], pglobset->aglob[i].asubglob[a].pshd, vertexes, normals, vertexColors, texcoords, indexes, &subposef, posfPose, normalfPose, agWeights, pglobset->aglob[i].fDynamic);
            }

            if (pglobset->aglob[i].asubglob.size() > 0)
            {
                auto& glob = pglobset->aglob[i];

                // 1) Count totals
                size_t totalVerts = 0;
                size_t totalIndices = 0; // flattened (tri*3)

                for (auto& s : glob.asubglob)
                {
                    totalVerts   += s.vertices.size();
                    totalIndices += s.indices.size() * 3;
                }

                // 2) Allocate final CPU arrays once (no insert() / no reallocation)
                std::vector <VERTICE>  packedVerts(totalVerts);
                std::vector <uint32_t> packedIdx(totalIndices);

                VERTICE*  vptr = packedVerts.data();
                uint32_t* iptr = packedIdx.data();

                uint32_t vOffset = 0;
                uint32_t iOffset = 0;

                // 3) Pack each subglob into the big arrays + record slice offsets
                for (auto& s : glob.asubglob)
                {
                    s.baseVertex = (int32_t)vOffset;
                    s.firstIndex = (uint32_t)iOffset;
                    s.indexCount = (uint32_t)(s.indices.size() * 3);

                    // Copy vertices
                    if (!s.vertices.empty())
                    {
                        memcpy(vptr, s.vertices.data(), s.vertices.size() * sizeof(VERTICE));
                        vptr += s.vertices.size();
                        vOffset += (uint32_t)s.vertices.size();
                    }

                    for (auto& tri : s.indices)
                    {
                        iptr[0] = (uint32_t)tri.v1;
                        iptr[1] = (uint32_t)tri.v2;
                        iptr[2] = (uint32_t)tri.v3;
                        iptr += 3;
                    }

                    iOffset += (uint32_t)(s.indices.size() * 3);
                }

                // 4) Create ONE VAO/VBO/EBO for the glob and upload once
                glGenVertexArrays(1, &glob.VAO);
                glBindVertexArray(glob.VAO);

                glGenBuffers(1, &glob.VBO);
                glBindBuffer(GL_ARRAY_BUFFER, glob.VBO);
                glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(packedVerts.size() * sizeof(VERTICE)), packedVerts.data(), GL_STATIC_DRAW);

                glGenBuffers(1, &glob.EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glob.EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(packedIdx.size() * sizeof(uint32_t)), packedIdx.data(), GL_STATIC_DRAW);

                // 5) Vertex attributes ONCE (your exact layout)
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, pos));
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, normal));
                glEnableVertexAttribArray(1);

                glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, color));
                glEnableVertexAttribArray(2);

                glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, uv));
                glEnableVertexAttribArray(3);

                glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, sizeof(VERTICE), (void*)offsetof(VERTICE, boneIndices));
                glEnableVertexAttribArray(4);

                glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, boneWeights));
                glEnableVertexAttribArray(5);

                glBindVertexArray(0);

                if (glob.pwrbg && glob.pwrbg->cmat > 0)
                {
                    if (!glob.pwarpGlob)
                        glob.pwarpGlob = std::make_shared<WRBGLOB_GL>();

                    WRBGLOB_GL& w = *glob.pwarpGlob;

                    w.vertexCount = (int)packedVerts.size();
                    w.basePos.resize((size_t)w.vertexCount);

                    for (int v = 0; v < w.vertexCount; ++v)
                        w.basePos[v] = glm::vec4(packedVerts[v].pos, 1.0f);

                    // don't allocate state yet here (because WR->cmat isn’t known until ApplyWrGlob)
                    // but you CAN create the SSBO name now:
                    if (w.ssboState == 0)
                        glGenBuffers(1, &w.ssboState);
                }

                if (glob.fThreeWay == 1 && glob.fDynamic == 0)
                {
                    if (glob.pwarpGlob == nullptr)
                    {
                        glob.trlk = TRLK_Relight;

                        glGenBuffers(1, &glob.ssboCachedMaterial);
                        glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.ssboCachedMaterial);
                        glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)(totalVerts * sizeof(MATERIAL)), nullptr, GL_STATIC_DRAW);
                        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
                    }
                }

                SetRpCount(&pglobset->aglob[i], pglobset->aglob[i].fTransluscentSort);
                numRo++;
            }

            pglobset->aglob[i].csubcel = pbis->U16Read();
            pglobset->aglob[i].asubcel.resize(pglobset->aglob[i].csubcel);
            std::vector <glm::vec4> mergedEdges;

            for (int k = 0; k < pglobset->aglob[i].csubcel; k++)
            {
                SUBCEL subcel;

                byte aposfCount = pbis->U8Read();

                std::vector <glm::vec3> aposf;
                aposf.resize(aposfCount);

                for (int a = 0; a < aposfCount; a++)
                    aposf[a] = pbis->ReadVector();

                byte ctwef = pbis->U8Read();

                std::vector <TWEF> atwef;
                atwef.resize(ctwef);

                for (int b = 0; b < ctwef; b++)
                {
                    atwef[b].aipos0 = (uint32_t)pbis->U8Read();
                    atwef[b].aipos1 = (uint32_t)pbis->U8Read();
                    atwef[b].aipos2 = (uint32_t)pbis->U8Read();
                    atwef[b].aipos3 = (uint32_t)pbis->U8Read();
                }

                int cibnd = pbis->U8Read();

                std::vector <int> aibnd;
                aibnd.resize(cibnd);

                for (int c = 0; c < cibnd; c++)
                    aibnd[c] = pbis->U8Read();

                int weightsCelCount = cibnd * aposfCount;

                std::vector <float> weightsCel;
                weightsCel.resize(weightsCelCount);

                for (int d = 0; d < weightsCelCount; d++)
                    weightsCel[d] = pbis->F32Read();

                std::vector <SUBPOSEF> subposef;
                std::vector <glm::vec3> aposfPoses;

                if (pglobset->cpose != 0)
                {
                    uint16_t aposfPosesCount = pbis->U16Read();
                    aposfPoses.resize(aposfPosesCount);

                    for (int e = 0; e < aposfPosesCount; e++)
                        aposfPoses[e] = pbis->ReadVector();

                    subposef.resize(pglobset->cpose);

                    for (int f = 0; f < pglobset->cpose; f++)
                    {
                        subposef[f].aiposf.resize(aposfCount);

                        for (int a = 0; a < aposfCount; a++)
                            subposef[f].aiposf[a] = pbis->U16Read();
                    }
                }

                BuildSubcel(pglobset, &pglobset->aglob[i], &subcel, aposfCount, aposf, ctwef, atwef, subposef, aposfPoses, weightsCel, mergedEdges);
                pglobset->aglob[i].asubcel[k] = subcel;
            }

            if (pglobset->aglob[i].edgeCount > 0)
            {
                glGenBuffers(1, &pglobset->aglob[i].edgeSSBO);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, pglobset->aglob[i].edgeSSBO);
                glBufferData(GL_SHADER_STORAGE_BUFFER, mergedEdges.size() * sizeof(glm::vec4), mergedEdges.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                SetRpCount(&pglobset->aglob[i], 0);
                numRoCel++;
            }
        }
        else
        {
            pglobset->aglob[i].csubglob = pglobset->aglob[instanceIndex].csubglob;
            pglobset->aglob[i].asubglob = pglobset->aglob[instanceIndex].asubglob;

            pglobset->aglob[i].csubcel = pglobset->aglob[instanceIndex].csubcel;
            pglobset->aglob[i].asubcel = pglobset->aglob[instanceIndex].asubcel;

            if (pglobset->aglob[i].fThreeWay == 1 && pglobset->aglob[i].fDynamic == 0)
            {
                uint32_t totalVerts = 0;
                for (int a = 0; a < pglobset->aglob[i].csubglob; a++)
                    totalVerts += pglobset->aglob[i].asubglob[a].vertices.size();

                glGenBuffers(1, &pglobset->aglob[i].ssboCachedMaterial);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, pglobset->aglob[i].ssboCachedMaterial);
                glBufferData(GL_SHADER_STORAGE_BUFFER, totalVerts * sizeof(MATERIAL), nullptr, GL_STATIC_DRAW);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }

            // after you've copied VAO/VBO/EBO/etc from instanceIndex:
            GLOB& inst = pglobset->aglob[i];
            GLOB& base = pglobset->aglob[instanceIndex];

            if (inst.pwrbg && inst.pwrbg->cmat > 0)
            {
                // base must have built warp basePos already (it was non-instanced and packed)
                if (!inst.pwarpGlob)
                    inst.pwarpGlob = std::make_shared <WRBGLOB_GL>();

                WRBGLOB_GL& wInst = *inst.pwarpGlob;
                WRBGLOB_GL& wBase = *base.pwarpGlob;

                // share geometry-side data
                wInst.vertexCount = wBase.vertexCount;
                wInst.basePos = wBase.basePos;     // shares the vector (copy) — see note below
                // If you want true sharing without copy, store basePos in a shared_ptr (see Option C)

                // unique SSBO for this instance's state
                if (wInst.ssboState == 0)
                    glGenBuffers(1, &wInst.ssboState);
            }

            if (pglobset->aglob[i].asubglob.size() > 0)
            {
                SetRpCount(&pglobset->aglob[i], pglobset->aglob[i].fTransluscentSort);
                numRo++;
            }

            if (pglobset->aglob[i].edgeCount > 0)
            {
                SetRpCount(&pglobset->aglob[i], 0);
                numRoCel++;
            }
        }
    }

    BuildGlobsetSaaArray(pglobset);
}

void BuildSubGlob(GLOB* pglob, SUBGLOB* psubglob, SHD* pshd, std::vector <glm::vec3>& positions, std::vector <glm::vec3>& normals, std::vector <glm::vec4>& colors, std::vector <glm::vec2>& texcoords, std::vector <VTXFLG>& indexes, SUBPOSEF* subposef, std::vector <glm::vec3>& aposfPoses, std::vector <glm::vec3>& anormalfPoses, std::vector <float>& agWeights, int fDynamic)
{
    if (pshd->shdk == SHDK_ThreeWay)
        pglob->fThreeWay = 1;

    psubglob->vertices.resize(indexes.size());

    for (int i = 0; i < indexes.size(); i++)
    {
        psubglob->vertices[i].pos = positions[indexes[i].ipos];

        if (indexes[i].inormal == 0xFF)
            psubglob->vertices[i].normal = glm::vec3(0.0);
        else
            psubglob->vertices[i].normal = normals[indexes[i].inormal];

        if (pshd->shdk == SHDK_ProjectedVolume)
        {
            if ((indexes[i].bMisc & 0x7F) == 0x7F)
                psubglob->vertices[i].color = pshd->rgbaVolume;
            else
                psubglob->vertices[i].color = colors[indexes[i].bMisc & 0x7F] * pshd->rgbaVolume;
        }
        else
        {
            if ((indexes[i].bMisc & 0x7F) == 0x7F)
                psubglob->vertices[i].color = pshd->rgba;
            else
                psubglob->vertices[i].color = colors[indexes[i].bMisc & 0x7F] * pshd->rgba;
        }

        if (indexes[i].iuv == 0xFF)
            psubglob->vertices[i].uv = glm::vec2{ 0.0 };
        else
            psubglob->vertices[i].uv = texcoords[indexes[i].iuv];
    }

    const int maxInfluences = 4; // Assume 4 for GPU skinning
    const int cibnd = psubglob->cibnd; // number of influences per vertex
    const std::vector<int>& aibnd = psubglob->aibnd;
    const int vertexCount = indexes.size();

    for (int i = 0; i < vertexCount; i++)
    {
        int ipos = indexes[i].ipos;
        glm::uvec4 boneIDs(0);
        glm::vec4 weights(0.0f);

        for (int j = 0; j < cibnd && j < maxInfluences; j++)
        {
            int weightIndex = ipos * cibnd + j;

            if (weightIndex < agWeights.size())
            {
                weights[j] = agWeights[weightIndex];

                // aibnd maps influence slot j to real bone ID
                if (j < aibnd.size())
                    boneIDs[j] = aibnd[j];
            }
        }

        // Normalize weights
        float totalWeight = glm::compAdd(weights);
        if (totalWeight > 0.0f)
            weights /= totalWeight;

        psubglob->vertices[i].boneIndices = boneIDs;
        psubglob->vertices[i].boneWeights = weights;
    }

    uint32_t idx = 0;
    for (int i = 2; i < indexes.size(); i++)
    {
        if (!(indexes[idx + 2].bMisc & 0x80))
        {
            if (i % 2 == 0)
            {
                INDICE indice{};

                indice.v1 = idx + 0;
                indice.v2 = idx + 1;
                indice.v3 = idx + 2;

                psubglob->indices.push_back(indice);
            }
            else
            {
                INDICE indice{};

                indice.v1 = idx + 0;
                indice.v2 = idx + 2;
                indice.v3 = idx + 1;

                psubglob->indices.push_back(indice);
            }
        }

        idx++;
    }

    SAI* uvSai = nullptr;
    bool usesUvAnim = false;

    SAI* sai = nullptr;

    if (pglob->psaa != nullptr)
        sai = pglob->psaa->pvtsaa->pfnPsaiFromSaaShd(pglob->psaa, pshd);

    if (!sai && pshd->psaa)
        sai = pshd->psaa->pvtsaa->pfnPsaiFromSaaShd(pshd->psaa, pshd);

    psubglob->uvSai = sai;
    psubglob->usesUvAnim = (sai && (sai->grfsai & 0x2));
}

void BuildSubcel(GLOBSET *pglobset, GLOB *pglob, SUBCEL *psubcel, int cposf, std::vector <glm::vec3> &aposf, int ctwef, std::vector <TWEF>& atwef, std::vector <SUBPOSEF>& asubposef, std::vector <glm::vec3>& aposfPoses, std::vector <float>& agWeights, std::vector <glm::vec4>& totalEdges)
{
    // Keep only what you still need:
    pglob->edgeCount += ctwef;

    // Reserve extra space in the merged buffer (4 vec4 per edge)
    totalEdges.reserve(totalEdges.size() + (size_t)ctwef * 4);

    auto getP = [&](uint32_t idx) -> const glm::vec3& {return aposf[idx];};

    for (int i = 0; i < ctwef; ++i)
    {
        const uint32_t iOppA = atwef[i].aipos0;
        const uint32_t iE0   = atwef[i].aipos1;
        const uint32_t iE1   = atwef[i].aipos2;
        const uint32_t iOppB = atwef[i].aipos3;

        const glm::vec3 E0 = getP(iE0);
        const glm::vec3 E1 = getP(iE1);
        const glm::vec3 OA = getP(iOppA);
        const glm::vec3 OB = getP(iOppB);

        // same layout as before, but appended to the merged buffer
        totalEdges.emplace_back(E0, 1.0f);
        totalEdges.emplace_back(E1, 1.0f);
        totalEdges.emplace_back(OA, 1.0f);
        totalEdges.emplace_back(OB, 1.0f);
    }
}

void BuildGlobsetSaaArray(GLOBSET* pglobset)
{
    pglobset->apsaa.resize(pglobset->cpsaa);

    for (int i = 0, saaIndex = 0; i < pglobset->cglob; i++)
    {
        SAA* psaa = pglobset->aglob[i].psaa;

        if (psaa != nullptr)
            pglobset->apsaa[saaIndex++] = psaa;
    }
}

void PostGlobsetLoad(GLOBSET* pglobset, ALO* palo)
{
    for (int i = 0; i < pglobset->aglob.size(); i++)
    {
        GLOB* glob = &pglobset->aglob[i];

        // If this glob has an SAA, call its post-load hook.
        SAA *saa = glob->psaa;
        
        if (saa != nullptr)
            saa->pvtsaa->pfnPostSaaLoad(saa);
    }

    for (WRBG* wrbg = pglobset->pwrbgFirst.get(); wrbg; wrbg = wrbg->pwrbgNextGlobset.get())
    {
        WR* pwr = (WR*)PloFindSwObject(palo->psw, 0x104, wrbg->oid, palo);
        if (!pwr) continue;

        ApplyWrGlob(pwr, palo, wrbg->pglob);
    }
}

void UpdateGlobset(GLOBSET* pglobset, ALO* palo, float dt)
{
    for (int i = 0; i < pglobset->apsaa.size(); i++)
    {
        SAA* saa = pglobset->apsaa[i];
        if (!saa)
            continue;

        if (!FUpdatableSaa(saa))
            continue;

        auto* vtsaa = saa->pvtscroller;
        if (!vtsaa || !vtsaa->pfnUpdateScroller)
            continue;

        vtsaa->pfnUpdateScroller((SCROLLER*)saa, dt);
    }

    // 2) Update WR matrices (driven by WRBG list)
    for (WRBG* wrbg = pglobset->pwrbgFirst.get(); wrbg != nullptr; wrbg = wrbg->pwrbgNextGlobset.get())
    {
        WR* pwr = wrbg->pwr;

        if (!pwr) continue;

        UpdateWrMatrixes(pwr);
    }
}

int  g_fogType = 1;
bool g_fRenderModels = true;
bool g_fRenderCollision = false;
bool g_fRenderCelBorders = true;
bool g_fBsp = false;
float g_uAlpha = 1.0;
SMP s_smpFade = { 2.0, 0.0, 0.1 };
SMP g_smpAlphaFade = { 2.0, 0.0, 0.1 };
glm::vec4 g_rgbaCel = glm::vec4(16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f, 1.0);