#include "sw.h"
#include "debug.h"
#include "render.h"

SW* NewSw()
{
	return new SW{};
}

void InitSw(SW* psw)
{
	InitLo(psw);
	psw->apaloRemerge.resize(4096);
	InitDl(&psw->dlChild, offsetof(LO, dleChild));
	InitDl(&psw->dlMRD, offsetof(ALO, dleMRD));
	InitDl(&psw->dlMRDRealClock, offsetof(ALO, dleMRD));
	InitDl(&psw->dlBusy, offsetof(ALO, dleBusy));
	InitSwBusySoDl(psw);
	InitSwRootDl(psw);
	InitSwAsegaDl(psw);
	InitSwAsegaRealClockDl(psw);
	InitSwAsegaPending(psw);
	InitSwSmaDl(psw);
	InitDl(&psw->dlAmb, 0x1C + 0x60); // GOTTA COME BACK TO THIS
	InitDl(&psw->dlExc, 0x1C + 4); // GOTTA COME BACK TO THIS
	InitSwLightDl(psw);
	InitSwShadowDl(psw);
	InitDl(&psw->dlExplste, 100);// GOTTA COME BACK TO THIS
	InitSwProxyDl(psw);
	InitSwFlyDl(psw);
	InitSwDprizeDl(psw);
	InitSwRatDl(psw);
	InitSwRatholeDl(psw);
	InitSwDartFreeDl(psw);
	InitSwSpireDl(psw);
	InitSwRailDl(psw);
	InitSwLandingDl(psw);
	InitSwLasenDl(psw);
	InitSwBlipgDl(psw);
	InitSwBlipgFreeDl(psw);
	InitSwFaderDl(psw);
	InitSwRealClockFader(psw);
	InitSwCrfodDl(psw);
	InitSwShapeDl(psw);
	InitSwPathzoneDl(psw);

	psw->rDarkenSmooth = 1.0;

	psw->rgbaSky.bRed   = 16 / 255;
	psw->rgbaSky.bGreen = 16 / 255;
	psw->rgbaSky.bBlue  = 40 / 255;
	psw->rgbaSky.bAlpha = 80 * 2;

	psw->lsmDefault.uShadow = 50.0;
	psw->musid = MUSID_Nil;
	psw->ibnk = -1;
	psw->cisi = 0;
	psw->cpsl = 0;
	psw->rDarken = 1.0;
}

int GetSwSize()
{
	return sizeof(SW);
}

void InitSwDlHash(SW* psw)
{
	for (int i = 0; i < 512; i++)
		InitDl(&psw->adlHash[i], offsetof(LO, dleOid));
}

void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis)
{
	std::cout << "\nLoading World...\n";
	// Setting difficulty for world
	OnDifficultyWorldPreLoad(&g_difficulty);
	// Loading unknown debug flag from file
	pbis->U8Read();
	// Loading index sound bank from file
	psw->ibnk = pbis->S16Read();
	pbis->U32Read();
	// Making new camera object for world
	g_pcm = (CM*)PloNew(CID_CM, psw, nullptr, OID__CAMERA, -1);

	for (int i = 0; i < 39; i++)
	{
		LO *blipg = PloNew(CID_BLIPG, psw, nullptr, OID_blip_group, -1);
		blipg->pvtlo->pfnRemoveLo(blipg);
	}
	
	// Loads all splice events from binary file
	LoadSwSpliceFromBrx(psw, pbis);
	LoadOptionsFromBrx(psw, pbis);
	pbis->file.seekg(0x20, std::ios::cur);
	pbis->file.seekg(pbis->S16Read(), std::ios::cur);
	pbis->file.seekg(0x20, std::ios::cur);
	// Loads all textures and shader data from file
	LoadShadersFromBrx(pbis);
	// Loads all the scene world objects from the binary file
	LoadSwObjectsFromBrx(psw, nullptr, pbis);

	if (FIsDlEmpty(&psw->dlLight) == true)
		CreateSwDefaultLights(psw);

	rgbaSky.r = psw->rgbaSky.bRed   / 255.0;
	rgbaSky.g = psw->rgbaSky.bGreen / 255.0;
	rgbaSky.b = psw->rgbaSky.bBlue  / 255.0;
	rgbaSky.a = (psw->rgbaSky.bAlpha / 255.0) * 2;

	// Aligns binary stream to texture data
	pbis->Align(0x10);
	std::cout << "Loading Textures...\n";
	// Loads textures from binary file
	LoadTexturesFromBrx(pbis);
	psw->lsmDefault.uShadow  *= 0.003921569;
	psw->lsmDefault.uMidtone *= 0.003921569;

	if (psw->dlChild.ploFirst != nullptr)
	{
		ALO* currentLo = psw->dlChild.paloFirst;

		while (currentLo != nullptr)
		{
			ALO* lo = currentLo;

			if (lo->pvtlo && lo->pvtlo->pfnBindLo)
				lo->pvtalo->pfnBindAlo(currentLo);

			currentLo = currentLo->dleChild.paloNext;
		}
	}

	DLI dlBusyWalker{};

	dlBusyWalker.m_ibDle = psw->dlChild.ibDle;
	dlBusyWalker.m_pdliNext = s_pdliFirst;
	dlBusyWalker.m_pdl = &psw->dlChild;

	LO* currentObject = psw->dlChild.ploFirst;

	// Only valid if we have a first element
	dlBusyWalker.m_ppv = currentObject ? (void**)((uintptr_t)currentObject + dlBusyWalker.m_ibDle) : nullptr;

	s_pdliFirst = &dlBusyWalker;

	while (currentObject != nullptr)
	{
		if (currentObject->pvtlo->pfnPostLoLoad)
			currentObject->pvtlo->pfnPostLoLoad(currentObject); 
		
		currentObject = (ALO*)*dlBusyWalker.m_ppv;

		// Guard before computing next pointer-field address
		dlBusyWalker.m_ppv = currentObject ? (void**)((uintptr_t)currentObject + dlBusyWalker.m_ibDle) : nullptr;
	}

	s_pdliFirst = dlBusyWalker.m_pdliNext;

	g_pcm->rMRDAdjust  = g_pcm->rMRD * (1.0 / g_pcm->radFOV);
	baseRenderDistance = g_pcm->rMRDAdjust;
	
	SetupCm(g_pcm);

	AllocateRpl();

	glGlobShader.Use();

	glUniform1f(glslLsmShadow,  psw->lsmDefault.uShadow);
	glUniform1f(glslLsmDiffuse, psw->lsmDefault.uMidtone);

	glUniform1i(glslFogType, g_fogType);
	glUniform1f(glslFogNear, g_pcm->sNearFog);
	glUniform1f(glslFogFar,  g_pcm->sFarFog);
	glUniform1f(glslFogMax,  g_pcm->uFogMax);
	glUniform4fv(glslFogColor, 1, glm::value_ptr(g_pcm->rgbaFog));

	InitCameraUbo();
	InitFrameStream(&ropStream, numRo);

	AllocateLightBlkList();
	AllocateShadows(psw);

	glCelBorderShader.Use();
	InitFrameStream(&rcbStream, numRoCel);
	
	glGeomShader.Use();

	std::cout << "World Loaded Successfully\n";
}

void LoadNameTableFromBrx(CBinaryInputStream* pbis)
{
	pbis->U32Read();
}

void LoadWorldTableFromBrx(CBinaryInputStream* pbis)
{
	// Storing number of world tables from binary file
	int worldTableCount = pbis->U32Read();

	// Loading world table from binary file
	for (int i = 0; i < worldTableCount; i++)
		pbis->ReadStringSw();
}

void AddSwProxySource(SW* psw, LO* ploProxySource, int cploClone)
{
	PSL psl{};

	int cploCloneFree = cploClone - 1;
	psl.cploCloneFree = cploCloneFree;

	if (cploCloneFree > 0)
	{
		psl.aploClone.resize(cploCloneFree);

		for (int i = 0; i < cploCloneFree; i++)
			psl.aploClone[i] = PloCloneLo(ploProxySource, psw, nullptr);
	}

	psw->apsl[psw->cpsl++] = psl;
}

LO* PloGetSwProxySource(SW* psw, int ipsl)
{
	// Loads the psl
	PSL *psl = psw->apsl + ipsl;
	// Returns proxy source LO from that psl
	return psl->aploClone[psl->cploCloneFree -= 1];
}

void GetSwParams(SW* psw, SOP** ppsop)
{

}

void* GetSwIllum(SW *psw)
{
	return &psw->lsmDefault.uMidtone;
}

void* GetSwIllumShadow(SW *psw)
{
	return &psw->lsmDefault.uShadow;
}

void SetSwIllum(SW *psw, float uMidtone)
{
	psw->lsmDefault.uMidtone = uMidtone;
}

void SetSwIllumShadow(SW *psw, float uShadow)
{
	psw->lsmDefault.uShadow = uShadow;
}

void* GetSwSkyRgba(SW *psw)
{
	return &psw->rgbaSky;
}

void* GetSwDarken(SW* psw)
{
	return &psw->rDarken;
}

void SetSwDarken(SW *psw, float rDarken)
{
	psw->rDarken = rDarken;
	psw->rDarkenSmooth = rDarken;
}

void* GetSwDarkenSmooth(SW *psw)
{
	return &psw->rDarkenSmooth;
}

void SetSwDarkenSmooth(SW* psw, float rDarkenSmooth)
{
	psw->rDarkenSmooth = rDarkenSmooth;
}

void MatchSwObject(ALO* ploMatch, GRFFSO grffsoMask, int fIncludeRemoved, int fProxyMatch, LO* ploContext, int cploMax, int* pcploMatch, LO** aplo, int* pcpaloBest)
{
	ALO* current   = nullptr;
	ALO* candidate = nullptr;

	switch (grffsoMask)
	{
		case 1:
		{
			if (ploMatch->ppxr && !fProxyMatch)
				return;

			current = ploMatch->paloParent;
			if (!current)
				return;

			while (current->cpaloFindSwObjects == 0)
			{
				if (current->ppxr)
					return;

				current = current->paloParent;
				if (!current)
					return;
			}

			if (current->cpaloFindSwObjects != 1)
				return;

			break;
		}

		case 2:
		{
			if (ploMatch->ppxr && !fProxyMatch)
				return;

			current = ploMatch->paloParent;

			if (current != reinterpret_cast<ALO*>(ploContext))
				return;

			break;
		}

		case 3:
		{
			if ((ploMatch->pvtlo->grfcid & 1U) == 0)
				return;

			if (ploMatch->cpaloFindSwObjects == 0)
				return;

			break;
		}

		case 4:
		{
			current = ((ploMatch->pvtlo->grfcid & 1U) == 0) ? ploMatch->paloParent : ploMatch;

			while (current)
			{
				int score = current->cpaloFindSwObjects;

				if (score != 0)
				{
					if (score <= *pcpaloBest)
					{
						if (score < *pcpaloBest)
						{
							*pcpaloBest = score;
							*pcploMatch = 0;
						}

						goto ADD_MATCH;
					}
				}

				if (current == candidate)
					return;

				std::shared_ptr<PXR> proxy = current->ppxr;

				if (!proxy)
					current = current->paloParent;
				else if (current == ploMatch && fProxyMatch)
					current = current->paloParent;
				else
				{
					OID contextProxyRoot = OID_Nil;

					ALO* ctx = reinterpret_cast<ALO*>(ploContext);
					while (ctx)
					{
						if (ctx->ppxr)
						{
							contextProxyRoot = ctx->ppxr->oidProxyRoot;
							break;
						}

						ctx = ctx->paloParent;
					}

					if (proxy->oidProxyRoot != contextProxyRoot)
						return;

					candidate = current->paloParent;
					current = current->paloParent;
				}
			}

			if (*pcpaloBest != INT_MAX)
				return;

			break;
		}

		case 5:
		break;

		default:
		goto ADD_MATCH;
	}

ADD_MATCH:
	if (!fIncludeRemoved && FIsLoInWorld(reinterpret_cast<LO*>(ploMatch)) == 0)
		return;

	int matchCount = *pcploMatch;

	if (matchCount < cploMax)
		aplo[matchCount] = reinterpret_cast<LO*>(ploMatch);

	*pcploMatch = matchCount + 1;
}

int CploFindSwObjects(SW* psw, GRFFSO grffso, OID oid, LO* ploContext, int cploMax, LO** aplo)
{
	if (oid == OID_Nil)
		return 0;

	uint32_t grffsoMask = grffso & 0xFF;
	bool shouldLimit = (grffso & 0x200) == 0;
	int fIncludeRemoved = grffso & 0x100;

	ALO* contextAlo = nullptr;
	int cploMatch = 0;
	int cpaloBest = INT_MAX;

	if (grffsoMask == 1 || grffsoMask == 3 || grffsoMask == 4)
	{
		contextAlo = reinterpret_cast<ALO*>(ploContext);

		if (ploContext && ((ploContext->pvtlo->grfcid ^ 1U) & 1))
			contextAlo = ploContext->paloParent;

		int depth = 0;

		for (ALO* alo = contextAlo; alo; alo = alo->paloParent)
		{
			depth++;

			// Original writes bits 41-44.
			// Direct field-name version:
			alo->cpaloFindSwObjects = depth & 0xF;
		}

		if (!ploContext && grffsoMask == 3)
			return 0;

		grffsoMask = 5;
	}

	DL* pdl = PdlFromSwOid(psw, oid);
	if (!pdl)
		return 0;

	for (ALO* match = pdl->paloFirst; match; match = match->dleOid.paloNext)
	{
		if (match->oid != oid)
			continue;

		bool isRemoved = (match->pvtlo->grfcid & 0x100U) != 0;

		bool isProxyRoot = match->ppxr && match->ppxr->oidProxyRoot == oid;

		if (!isRemoved)
		{
			if (!isProxyRoot)
				MatchSwObject(match, grffsoMask, fIncludeRemoved, 0, ploContext, cploMax, &cploMatch, aplo, &cpaloBest);
		}
		else
		{

		}
	}

	for (ALO* alo = contextAlo; alo; alo = alo->paloParent)
		alo->cpaloFindSwObjects = 0;

	if (shouldLimit && cploMatch > cploMax)
		cploMatch = cploMax;

	return cploMatch;
}

int FIsCidDerivedFrom(CID cid, CID cidAncestor)
{
	VT* current = (VT*)g_mpcidpvt[cid];
	while (current) {
		if (current->cid == cidAncestor) {
			return 1;
		}
		current = current->pvtSuper;
	}
	return 0;
}

int  CploFindSwObjectsByClass(SW* psw, GRFFSO grffso, CID cid, LO* ploContext, int cploMax, LO** aplo)
{
	if (cid == CID_Nil)
		return 0;

	const int grffsoMode = grffso & 0xff;
	const bool includeHiddenOrOutOfWorld = (grffso & 0x100) != 0;
	const bool allowOverflowCount = (grffso & 0x200) != 0;

	if (ploContext == nullptr && (grffso & 5) == 0)
		return 0;

	int cploMatch = 0;

	/*
		Mode 3:
		Walk upward through context's parent chain.
	*/
	if (grffsoMode == 3)
	{
		LO* plo = ploContext;

		if (plo != nullptr && (plo->pvtlo->grfcid & 1) == 0)
			plo = (LO*)plo->paloParent;

		while (plo != nullptr)
		{
			if (FIsBasicDerivedFrom((BASIC*)plo, cid))
			{
				if (includeHiddenOrOutOfWorld || FIsLoInWorld(plo))
				{
					if (cploMatch < cploMax)
						aplo[cploMatch] = plo;

					++cploMatch;
				}
			}

			plo = (LO*)plo->paloParent;
		}
	}

	/*
		Mode 2:
		Search direct children of context.
	*/
	else if (grffsoMode == 2 && !includeHiddenOrOutOfWorld)
	{
		if (ploContext == nullptr)
			return 0;

		if ((ploContext->pvtlo->grfcid & 1) == 0)
			return 0;

		for (LO* plo = (LO*)ploContext->dleChild.ploNext; plo != nullptr; plo = plo->dleChild.ploNext)
		{
			if (FIsBasicDerivedFrom((BASIC*)plo, cid))
			{
				if (cploMatch < cploMax)
					aplo[cploMatch] = plo;

				++cploMatch;
			}
		}
	}

	/*
		General search:
		Search SW CID lists.
	*/
	else
	{
		ALO* paloContext = nullptr;
		int cpaloBest = INT_MAX;

		/*
			Modes 1, 3, 4:
			Mark context parent chain with distance values.
			Original stores this in cpaloFindSwObjects.
		*/
		if (grffsoMode == 1 || grffsoMode == 3 || grffsoMode == 4)
		{
			paloContext = (ALO*)ploContext;

			if (ploContext != nullptr && (ploContext->pvtlo->grfcid & 1) == 0)
				paloContext = ploContext->paloParent;

			int depth = 0;

			for (ALO* palo = paloContext; palo != nullptr; palo = palo->paloParent)
			{
				++depth;
				palo->cpaloFindSwObjects = depth & 0xf;
			}
		}

		for (CID testCid = cid; testCid < 0xa2 && FIsCidDerivedFrom(testCid, cid); testCid = (CID)(testCid + CID_LO))
		{
			for (LO* plo = psw->aploCidHead[testCid]; plo != nullptr; plo = plo->ploCidNext)
				MatchSwObject((ALO*)plo, grffsoMode, includeHiddenOrOutOfWorld, false, ploContext, cploMax, &cploMatch, aplo, &cpaloBest);
		}

		/*
			Clear temporary parent-chain distance markers.
		*/
		for (ALO* palo = paloContext; palo != nullptr; palo = palo->paloParent)
			palo->cpaloFindSwObjects = 0;
	}

	if (!allowOverflowCount && cploMatch > cploMax)
		cploMatch = cploMax;

	return cploMatch;
}

LO* PloFindSwObject(SW* psw, GRFFSO grffso, OID oid, LO* ploContext)
{
	LO* plo = nullptr;
	CploFindSwObjects(psw, grffso | 0x200, oid, ploContext, 1, &plo);
	return plo;
}

LO* PloFindSwChild(SW* psw, OID oid, ALO* paloAncestor)
{
	LO* plo = nullptr;
	CploFindSwObjects(psw, 0x201, oid, paloAncestor, 1, &plo);
	return plo;
}

LO* PloFindSwNearest(SW* psw, OID oid, LO* ploContext)
{
	LO* plo = nullptr;
	CploFindSwObjects(psw, 0x204, oid, ploContext, 1, &plo);
	return plo;
}

void DeleteWorld(SW *psw)
{
	numRo = 0;
	numRoCel = 0;

	numFrameObjs = 0;

	g_cframe = 0;

	g_boundVAO = 0;

	g_shdIDBound = -1;

	g_lastAnimateUv = -1;

	g_cFrameGlobs = 0;
	g_cFrameCelGlobs = 0;

	g_dynamicTextureCount = 0;
	g_dynamicTexturePrpl.clear();
	g_dynamicTexturePrpl.shrink_to_fit();

	g_backGroundCount = 0;
	g_backGroundPrpl.clear();
	g_backGroundPrpl.shrink_to_fit();

	g_backGroundBlendCount = 0;
	g_backGroundBlendPrpl.clear();
	g_backGroundBlendPrpl.shrink_to_fit();

	g_blotContextCount = 0;
	g_blotContextPrpl.clear();
	g_blotContextPrpl.shrink_to_fit();

	g_opaqueCount = 0;
	g_opaquePrpl.clear();
	g_opaquePrpl.shrink_to_fit();

	g_cutOutCount = 0;
	g_cutOutPrpl.clear();
	g_cutOutPrpl.shrink_to_fit();

	g_cutOutBlendAddCount = 0;
	g_cutOutBlendAddPrpl.clear();
	g_cutOutBlendAddPrpl.shrink_to_fit();

	g_celBorderCount = 0;
	g_celBorderPrpl.clear();
	g_celBorderPrpl.shrink_to_fit();

	g_projVolumeCount = 0;
	g_projVolumePrpl.clear();
	g_projVolumePrpl.shrink_to_fit();

	g_projVolumeAlphaAddCount = 0;
	g_projVolumeAlphaAddPrpl.clear();
	g_projVolumeAlphaAddPrpl.shrink_to_fit();

	g_projVolumeAddCount = 0;
	g_projVolumeAddPrpl.clear();
	g_projVolumeAddPrpl.shrink_to_fit();

	g_opaqueAfterProjVolumeCount = 0;
	g_opaqueAfterProjVolumePrpl.clear();
	g_opaqueAfterProjVolumePrpl.shrink_to_fit();

	g_cutOutAfterProjVolumeCount = 0;
	g_cutOutAfterProjVolumePrpl.clear();
	g_cutOutAfterProjVolumePrpl.shrink_to_fit();

	g_cutOutAfterProjVolumeAddCount = 0;
	g_cutOutAfterProjVolumeAddPrpl.clear();
	g_cutOutAfterProjVolumeAddPrpl.shrink_to_fit();

	g_celBorderAfterProjVolumeCount = 0;
	g_celBorderAfterProjVolumePrpl.clear();
	g_celBorderAfterProjVolumePrpl.shrink_to_fit();

	g_murkClearCount = 0;
	g_murkClearPrpl.clear();
	g_murkClearPrpl.shrink_to_fit();

	g_murkOpaqueCount = 0;
	g_murkOpaquePrpl.clear();
	g_murkOpaquePrpl.shrink_to_fit();

	g_murkFillCount = 0;
	g_murkFillPrpl.clear();
	g_murkFillPrpl.shrink_to_fit();
	
	g_translucentCount = 0;
	g_translucentPrpl.clear();
	g_translucentPrpl.shrink_to_fit();

	g_translucentAddCount = 0;
	g_translucentAddPrpl.clear();
	g_translucentAddPrpl.shrink_to_fit();

	g_translucentCelBorderCount = 0;
	g_translucentCelBorderPrpl.clear();
	g_translucentCelBorderPrpl.shrink_to_fit();

	g_blipCount = 0;
	g_blipPrpl.clear();
	g_blipPrpl.shrink_to_fit();

	g_foreGroundCount = 0;
	g_foreGroundPrpl.clear();
	g_foreGroundPrpl.shrink_to_fit();

	g_worldMapCount = 0;
	g_worldMapPrpl.clear();
	g_worldMapPrpl.shrink_to_fit();

	g_maxCount = 0;
	g_maxPrpl.clear();
	g_maxPrpl.shrink_to_fit();

	DeallocateLightBlkList();

	for (int i = 0; i < allSWAloObjs.size(); i++)
		DeleteModel(allSWAloObjs[i]);

	DeleteSwCollision();

	for (int i = 0; i < allWorldObjs.size(); i++)
		allWorldObjs[i]->pvtlo->pfnDeleteLo(allWorldObjs[i]);

	allSWAloObjs.clear();
	allSWAloObjs.shrink_to_fit();
	allWorldObjs.clear();
	allWorldObjs.shrink_to_fit();

	DeallocateSoVector();

	UnloadShaders();
	ResetUi(&g_ui);

	g_psw = nullptr;
	g_pcm = nullptr;

	baseRenderDistance = 0.0;

	glDeleteBuffers(1, &cmUBO);

	DeleteFrameStream(&ropStream);
	DeleteFrameStream(&rcbStream);

	glDeleteBuffers(1, &geomUBO);

	DeallocateSwShadows();

	rgbaSky = glm::vec4(0.0);
	std::cout << "World Deleted\n";
}

void DeleteSw(SW* psw)
{
	delete psw;
}

SW* g_psw = nullptr;
glm::vec4 rgbaSky{0.0};
