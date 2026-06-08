#include "geom.h"

void InitGeom(GEOM* pgeom)
{
	pgeom->aedge.clear();
	pgeom->cpos = 0;
	pgeom->apos.clear();
	pgeom->csurf = 0;
	pgeom->asurf.clear();
	pgeom->cedge = 0;
}

void ReadGeom(GEOM *pgeom, CBinaryInputStream *pbis)
{
    pgeom->sRadius = pbis->F32Read();

    pgeom->cpos = pbis->U16Read();
    pgeom->apos.resize(pgeom->cpos);

    for (int i = 0; i < pgeom->cpos; ++i)
        pgeom->apos[i] = pbis->ReadVector();

    pgeom->csurf = pbis->U16Read();

    pgeom->asurf.clear();
    pgeom->asurf.resize(pgeom->csurf);

    pgeom->mpisurfposCenter.resize(pgeom->csurf);
    pgeom->mpisurfsRadius.resize(pgeom->csurf);

    pgeom->cedge = pbis->U16Read();

    pgeom->aedge.clear();
    pgeom->aedge.resize(pgeom->cedge);

    pgeom->indices.clear();

    int edgeIndex = 0;

    for (int isurf = 0; isurf < pgeom->csurf; ++isurf)
    {
        SURF &surf = pgeom->asurf[isurf];

        uint16_t ipos0 = pbis->U16Read();
        uint16_t ipos1 = pbis->U16Read();
        uint16_t ipos2 = pbis->U16Read();

        glm::vec3& p0 = pgeom->apos[ipos0];
        glm::vec3& p1 = pgeom->apos[ipos1];
        glm::vec3& p2 = pgeom->apos[ipos2];

        glm::vec3 dpos0 = p1 - p0;
        glm::vec3 dpos1 = p2 - p0;

        surf.normal = glm::normalize(glm::cross(dpos0, dpos1));
        surf.ipos = ipos0;
        surf.gDot = glm::dot(surf.normal, p0);

        pgeom->mpisurfsRadius[isurf] = pbis->F32Read();
        pgeom->mpisurfposCenter[isurf] = pbis->ReadVector();

        uint8_t cedgeSurf = pbis->U8Read();

        EDGE *prevEdge = nullptr;

        for (int iedge = 0; iedge < cedgeSurf; ++iedge)
        {
            EDGE& edge = pgeom->aedge[edgeIndex];

            edge.aipos[0]  = pbis->U16Read();
            edge.aisurf[0] = static_cast<int16_t>(isurf);

            edge.aipos[1]  = pbis->U16Read();
            edge.aisurf[1] = pbis->U16Read();

            if (iedge == 0)
                surf.pedge = &edge;
            else
                prevEdge->pedgeNext = &edge;

            SURF& otherSurf = pgeom->asurf[edge.aisurf[1]];

            edge.pedgeOtherNext = otherSurf.pedgeOther;
            otherSurf.pedgeOther = &edge;

            prevEdge = &edge;
            ++edgeIndex;

            // Optional: useful if you are rendering wireframe edges
            pgeom->indices.push_back(edge.aipos[0]);
            pgeom->indices.push_back(edge.aipos[1]);
        }
    }

    if (!pgeom->apos.empty())
    {
        glGenVertexArrays(1, &pgeom->VAO);
        glBindVertexArray(pgeom->VAO);

        glGenBuffers(1, &pgeom->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, pgeom->VBO);
        glBufferData(GL_ARRAY_BUFFER, pgeom->apos.size() * sizeof(glm::vec3), pgeom->apos.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &pgeom->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pgeom->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pgeom->indices.size() * sizeof(uint16_t), pgeom->indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
}

void CloneGeom(GEOM* pgeomSrc, glm::mat4* pdmat, GEOM* pgeomDst)
{
    pgeomDst->sRadius = pgeomSrc->sRadius;

    pgeomDst->cpos = pgeomSrc->cpos;
    pgeomDst->csurf = pgeomSrc->csurf;
    pgeomDst->cedge = pgeomSrc->cedge;

    pgeomDst->apos = pgeomSrc->apos;
    pgeomDst->asurf = pgeomSrc->asurf;
    pgeomDst->mpisurfposCenter = pgeomSrc->mpisurfposCenter;
    pgeomDst->mpisurfsRadius = pgeomSrc->mpisurfsRadius;
    pgeomDst->aedge = pgeomSrc->aedge;
    pgeomDst->indices = pgeomSrc->indices;

    pgeomDst->VAO = 0;
    pgeomDst->VBO = 0;
    pgeomDst->EBO = 0;

    if (pdmat != nullptr)
    {
        for (glm::vec3& pos : pgeomDst->apos)
            pos = glm::vec3((*pdmat) * glm::vec4(pos, 1.0f));

        for (glm::vec3& center : pgeomDst->mpisurfposCenter)
            center = glm::vec3((*pdmat) * glm::vec4(center, 1.0f));

        for (size_t i = 0; i < pgeomDst->asurf.size(); ++i)
        {
            glm::vec3 normal = pgeomDst->asurf[i].normal;
            normal = glm::normalize(glm::mat3(*pdmat) * normal);

            pgeomDst->asurf[i].normal = normal;

            uint16_t ipos = pgeomDst->asurf[i].ipos;
            pgeomDst->asurf[i].gDot = glm::dot(normal, pgeomDst->apos[ipos]);
        }
    }

    if (pgeomDst->cpos != 0)
    {
        glGenVertexArrays(1, &pgeomDst->VAO);
        glBindVertexArray(pgeomDst->VAO);

        glGenBuffers(1, &pgeomDst->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, pgeomDst->VBO);
        glBufferData(GL_ARRAY_BUFFER, pgeomDst->apos.size() * sizeof(glm::vec3), pgeomDst->apos.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &pgeomDst->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pgeomDst->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pgeomDst->indices.size() * sizeof(uint16_t), pgeomDst->indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
}

void UpdateGeomWorld(GEOM* pgeomLocal, GEOM* pgeomWorld, glm::vec3& pos, glm::mat3& mat)
{
    // If local/world share the same position buffer, nothing to update.
    if (pgeomLocal->apos == pgeomWorld->apos)
        return;

    // Transform vertex positions: worldPos = mat * localPos + pos
    for (int i = 0; i < pgeomLocal->cpos; ++i)
    {
        const glm::vec3 localPos = pgeomLocal->apos[i];
        pgeomWorld->apos[i] = mat * localPos + pos;
    }

    // Transform surface normals and surface centers
    for (int i = 0; i < pgeomLocal->csurf; ++i)
    {
        SURF &localSurf = pgeomLocal->asurf[i];
        SURF &worldSurf = pgeomWorld->asurf[i];

        // Normals get rotated only, no translation.
        glm::vec3 worldNormal = mat * localSurf.normal;

        worldSurf.normal = worldNormal;

        // gDot appears to be dot(normal, world position at surf ipos)
        int posIndex = static_cast<unsigned short>(worldSurf.ipos);
        worldSurf.gDot = glm::dot(worldNormal, pgeomWorld->apos[posIndex]);

        // Transform surface center position
        glm::vec3 localCenter = pgeomLocal->mpisurfposCenter[i];

        pgeomWorld->mpisurfposCenter[i] = mat * localCenter + pos;
    }
}