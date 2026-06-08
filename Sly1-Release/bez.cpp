#include "bez.h"
#include "crv.h"

float SBezierPosLength(float dtSeg, float tSeg, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1)
{
    std::vector <glm::vec3> apos(20);

    TesselateBezier(dtSeg, 0.0f, tSeg, ppos0, pv0, ppos1, pv1, 20, apos.data());

    return SMeasureApos(20, apos.data(), nullptr);
}

void TesselateBezier(float dtSeg, float tStart, float tEnd, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, int cpos, glm::vec3* apos)
{
    float step = (tEnd - tStart) / float(cpos - 1);

    for (int i = 0; i < cpos; i++)
    {
        EvaluateBezierPos(dtSeg, tStart, 1.0f, ppos0, pv0, ppos1, pv1, &apos[i], nullptr, nullptr);
        tStart += step;
    }
}

void EvaluateBezierPos(float dtSeg, float tSeg, float svt, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, glm::vec3* ppos, glm::vec3* pv, glm::vec3* pdv)
{
    float t = tSeg / dtSeg;
    float u = 1.0f - t;

    if (ppos)
    {
        float t2 = t * t;
        float t3 = t2 * t;
        float u2 = u * u;
        float u3 = u2 * u;

        float a = u3 + 3.0f * t * u2;
        float b = t * u2 * dtSeg;
        float c = -(t2 * u) * dtSeg;
        float d = t3 + 3.0f * t2 * u;

        *ppos = (*ppos0) * a + (*pv0) * b + (*pv1) * c + (*ppos1) * d;
    }

    if (pv)
    {
        glm::vec3 dpos = (*ppos1) - (*ppos0);

        float a = (t * (6.0f - t * 6.0f)) / dtSeg;
        float b = 1.0f - t * (4.0f - t * 3.0f);
        float c = t * (t * 3.0f - 2.0f);

        *pv = (dpos * a + (*pv0) * b + (*pv1) * c) * svt;
    }

    if (pdv)
    {
        glm::vec3 dpos = (*ppos1) - (*ppos0);

        float a = (6.0f - t * 12.0f) / dtSeg;
        float b = t * 6.0f - 4.0f;
        float c = t * 6.0f - 2.0f;

        *pdv = (dpos * a + (*pv0) * b + (*pv1) * c) * ((svt * svt) / dtSeg);
    }
}
