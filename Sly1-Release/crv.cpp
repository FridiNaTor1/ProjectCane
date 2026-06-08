#include "crv.h"

std::shared_ptr <CRV> PcrvNew(CRVK crvk)
{
	std::shared_ptr <CRV> pcrv;

	if (crvk == CRVK_Linear)
	{
		CRVL crvl{};
		crvl.pvtcrvl = &g_vtcrvl;
		pcrv = std::make_shared <CRVL>(crvl);
	}

	else
	{
		if (crvk == CRVK_Cubic)
		{
			CRVC crvc{};
			crvc.pvtcrvc = &g_vtcrvc;
			pcrv = std::make_shared <CRVC>(crvc);
		}
	}

	if (pcrv != nullptr)
		pcrv->crvk = crvk;

	return pcrv;
}

void LoadCrvlFromBrx(CRVL *pcrvl, CBinaryInputStream *pbis)
{
	pcrvl->fClosed = pbis->U8Read();
	pcrvl->ccv = pbis->U8Read();
	
	pcrvl->mpicvu.resize(pcrvl->ccv);
	pcrvl->mpicvs.resize(pcrvl->ccv);
	pcrvl->mpicvpos.resize(pcrvl->ccv);

	for (int i = 0; i < pcrvl->ccv; i++)
	{
		pcrvl->mpicvu[i] = pbis->F32Read();
		pcrvl->mpicvpos[i] = pbis->ReadVector();
	}

	pcrvl->pvtcrvl->pfnMeasureCrvl(pcrvl);
}

void ConvertCrvl(CRVL* pcrvl, glm::mat4* pmatSrc, glm::mat4* pmatDst)
{
	ConvertApos(pcrvl->ccv, pcrvl->mpicvpos.data(), *pmatSrc, *pmatDst);
}

void ConvertApos(int cpos, glm::vec3* apos, glm::mat4& pmatSrc, glm::mat4& pmatDst)
{
	glm::mat4 dmat4;

	CalculateDmat4(pmatDst, pmatSrc, dmat4);

	for (int i = 0; i < cpos; ++i)
	{
		const glm::vec4 pos = dmat4 * glm::vec4(apos[i], 1.0f);
		apos[i] = glm::vec3(pos);
	}
}

void MeasureCrvl(CRVL *pcrvl)
{
	SMeasureApos(pcrvl->ccv, pcrvl->mpicvpos.data(), pcrvl->mpicvs.data());
}

float SMeasureApos(int cpos, glm::vec3 *apos, float *mpiposs)
{
	float sTotal = 0.0f;

	if (mpiposs != nullptr)
		mpiposs[0] = 0.0f;

	if (cpos <= 1)
		return 0.0f;

	for (int i = 1; i < cpos; ++i)
	{
		glm::vec3 dpos = apos[i] - apos[i - 1];

		sTotal += glm::length(dpos);

		if (mpiposs != nullptr)
			mpiposs[i] = sTotal;
	}

	return sTotal;
}

void LoadCrvcFromBrx(CRVC *pcrvc, CBinaryInputStream *pbis)
{
	pcrvc->fClosed = pbis->U8Read();
	pcrvc->ccv = pbis->U8Read();
	
	pcrvc->mpicvu.resize(pcrvc->ccv);
	pcrvc->mpicvs.resize(pcrvc->ccv);
	pcrvc->mpicvpos.resize(pcrvc->ccv);
	pcrvc->mpicvdposIn.resize(pcrvc->ccv);
	pcrvc->mpicvdposOut.resize(pcrvc->ccv);


	for (int i = 0; i < pcrvc->ccv; i++)
	{
		pcrvc->mpicvu[i] = pbis->F32Read();
		pcrvc->mpicvpos[i] = pbis->ReadVector();
		pcrvc->mpicvdposIn[i] = pbis->ReadVector();
		pcrvc->mpicvdposOut[i] = pbis->ReadVector();
	}

	pcrvc->pvtcrvc->pfnMeasureCrvc(pcrvc);

	InvalidateCrvcCache(pcrvc);
}

void ConvertCrvc(CRVC* pcrvc, glm::mat4& pmatSrc, glm::mat4& pmatDst)
{
	glm::mat4 dmat4;

	CalculateDmat4(pmatDst, pmatSrc, dmat4);

	const glm::mat3 rot = glm::mat3(dmat4);

	for (int i = 0; i < pcrvc->ccv; ++i)
	{
		// Position gets full 4x4 transform, w = 1
		pcrvc->mpicvpos[i] = glm::vec3(dmat4 * glm::vec4(pcrvc->mpicvpos[i], 1.0f));

		// Handles/deltas get rotation/scale only, no translation
		pcrvc->mpicvdposIn[i]  = rot * pcrvc->mpicvdposIn[i];
		pcrvc->mpicvdposOut[i] = rot * pcrvc->mpicvdposOut[i];
	}

	InvalidateCrvcCache(pcrvc);
}

void InvalidateCrvcCache(CRVC* pcrvc)
{
	pcrvc->icvCache = -1;
}

void MeasureCrvc(CRVC* pcrvc)
{
	pcrvc->mpicvs[0] = 0.0f;

	int cseg = pcrvc->ccv - 1;

	for (int i = 0; i < cseg - 1; i++)
	{
		float dtSeg  = pcrvc->mpicvu[i + 1] - pcrvc->mpicvu[i];
		float length = SBezierPosLength(dtSeg, dtSeg, &pcrvc->mpicvpos[i], &pcrvc->mpicvdposOut[i], &pcrvc->mpicvpos[i + 1], &pcrvc->mpicvdposIn[i + 1]);

		pcrvc->mpicvs[i + 1] = pcrvc->mpicvs[i] + length;
	}
}