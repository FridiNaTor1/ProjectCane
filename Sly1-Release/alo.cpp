#include "alo.h"

ALO* NewAlo()
{
	return new ALO{};
}

void InitAlo(ALO* palo)
{
	InitDl(&palo->dlChild,  offsetof(LO, dleChild));
	InitDl(&palo->dlFreeze, offsetof(ALO, dleFreeze));

	if (palo->paloParent == nullptr)
	{
		palo->zons = 2;
		palo->viss = 2;
	}
	else
	{
		palo->zons = 1;
		palo->viss = 0;
	}

	InitLo(palo);

	palo->sCelBorderMRD = FLT_MAX;
	palo->sMRD = FLT_MAX;
	palo->grfzon = -1;
	palo->mtlk = 0;
	palo->xf.mat = glm::identity<glm::mat3>();
	palo->xf.matWorld = glm::identity<glm::mat3>();
	palo->matOrig = glm::identity<glm::mat3>();

	InitDl(&palo->dlAct, offsetof(ACT, dleAlo));

	if (palo->pvtlo->cid != CID_LIGHT)
		allSWAloObjs.push_back(palo);
}

void RemoveAloHierarchy(ALO* palo)
{
	DLI it{};

	it.m_pdl = &palo->dlChild;
	it.m_ibDle = palo->dlChild.ibDle;
	it.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &it;

	it.m_ppv = (void**)it.m_pdl;

	// Start with the parent
	LO* current = (LO*)palo;

	while (true)
	{
		// Call OnLoAdd for the CURRENT object (parent first, then each child)
		current->pvtlo->pfnOnLoRemove(current);

		// Load next child pointer from the current "next field"
		void* next = *it.m_ppv;
		if (next == nullptr)
			break;

		// Advance iterator to the "next pointer field" inside that next object
		it.m_ppv = (void**)((uintptr_t)next + it.m_ibDle);

		// Move to the next object so we don't keep calling the parent
		current = (LO*)next;
	}

	// palo->pvtlo->pfnSendLoMessage(palo, 1, palo);

	s_pdliFirst = it.m_pdliNext;
}

void OnAloAdd(ALO* palo)
{
	if (!palo) return;

	// Original does this first
	OnLoAdd(palo);

	ALO* parent = palo->paloParent;
	SW* sw = palo->psw;

	if (parent == nullptr)
	{
		// Root object
		palo->paloRoot = palo;

		if (palo->fRealClock == 0)
		{
			// Normal-clock roots go into MRD + Busy processing lists
			AppendDlEntry(&sw->dlMRD, palo);

			palo->fBusy = true;

			AppendDlEntry(&sw->dlBusy, palo);

			// If this LO is an SO-ish thing, also append to dlBusySo
			if ((palo->pvtlo->grfcid & 2u) != 0)
				AppendDlEntry(&sw->dlBusySo, palo);

			// Freeze root init + merge groups
			palo->paloFreezeRoot = palo;
			palo->dlFreeze.paloFirst = palo;
			palo->dlFreeze.paloLast = palo;

			for (int i = palo->cpmrg - 1; i >= 0; --i)
			{
				MRG* pmrg = palo->apmrg[i];
				if (pmrg)
					MergeSwGroup(sw, pmrg);
			}
		}
		else
		{
			// Real-clock roots go into the real-clock MRD list
			AppendDlEntry(&sw->dlMRDRealClock, palo);
		}
	}
	else
	{
		// Child inherits root from parent
		palo->paloRoot = parent->paloRoot;

		// If parent's "freeze-propagate" bit is set, freeze child on add (if supported)
		if ((parent->fFrozen) != 0)
		{
			if (palo->pvtalo && palo->pvtalo->pfnFreezeAlo)
				palo->pvtalo->pfnFreezeAlo(palo, 1);
		}
	}

	// Apply ACT position goal on add
	//if (palo->pactPos != nullptr)
	//{
	//	glm::vec3 w(0.0f);
	//	glm::vec3 v(0.0f);

	//	// Original signature: pfnGetActPositionGoal(0, pactPos, &w, &v)
	//	palo->pactPos->pvtact->pfnGetActPositionGoal(nullptr, palo->pactPos, &w, &v);

	//	palo->pvtalo->pfnTranslateAloToPos(palo, &w);
	//	palo->pvtalo->pfnSetAloVelocityVec(palo, &v);
	//}

	// Apply ACT rotation goal on add
	//if (palo->pactRot != nullptr)
	//{
	//	glm::mat3 mat(1.0f);
	//	glm::vec3 w(0.0f);

	//	// Original signature: pfnGetActRotationGoal(0, pactRot, &mat, &w)
	//	palo->pactRot->pvtact->pfnGetActRotationGoal(nullptr, palo->pactRot, &mat, &w);

	//	palo->pvtalo->pfnRotateAloToMat(palo, &mat);
	//	palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &w);
	//}

	// If flagged, add to camera fade list
	/*if (palo->fForceCameraFade != 0)
		AddCmFadeObject(g_pcm, palo);*/

		// Shadow registration
	if (palo->pshadow != nullptr && palo->psw != nullptr)
		AppendDlEntry(&palo->psw->dlShadow, palo->pshadow.get());

	// Update world transform if the LO has a handler
	if (palo->pvtlo && palo->pvtlo->pfnUpdateLoXfWorld)
		palo->pvtalo->pfnUpdateAloXfWorld(palo);

	//HandleLoSpliceEvent(palo, 4, 0, nullptr);
	ResolveAlo(palo);
}

void OnAloRemove(ALO* palo)
{
	if (!palo) return;
	
	OnLoRemove(palo);

	SW* psw = palo->psw;

	// Root-only list cleanup
	if (palo->paloParent == nullptr)
	{
		if (palo->fRealClock != 0)
		{
			// Root real-clock list
			if (psw) RemoveDlEntry(&psw->dlMRDRealClock, palo);
		}
		else
		{
			// Root normal-clock list
			if (psw) RemoveDlEntry(&psw->dlMRD, palo);

			// If it was also in busy lists, remove and clear the bit
			if (palo->fBusy)
			{
				palo->fBusy = false;

				if (psw) RemoveDlEntry(&psw->dlBusy, palo);

				if (psw && palo->pvtlo && ((palo->pvtlo->grfcid & 2U) != 0))
					RemoveDlEntry(&psw->dlBusySo, palo);
			}

			// Undo freeze-group bookkeeping that was established for the root
			if (psw && palo->paloFreezeRoot)
				SplinterSwFreezeGroup(psw, palo->paloFreezeRoot);

			palo->paloFreezeRoot = nullptr;
			ClearDl(&palo->dlFreeze);
		}
	}

	// Camera fade removal
	/*if (palo->fForceCameraFade)
		RemoveCmFadeObject(g_pcm, palo);*/

		// Shadow list removal
	if (palo->pshadow != nullptr && palo->psw != nullptr)
		RemoveDlEntry(&palo->psw->dlShadow, palo->pshadow.get());

	// If freeze-propagate bit is set, unfreeze on remove (if supported)
	if (palo->fFrozen && palo->pvtalo->pfnFreezeAlo)
		palo->pvtalo->pfnFreezeAlo(palo, false);

	ResolveAlo(palo);
	palo->paloRoot = nullptr;

	//HandleLoSpliceEvent(palo, 5, 0, nullptr);
}

void UpdateAloOrig(ALO* palo)
{
	palo->matOrig = palo->xf.mat;   // glm::mat3 (rotation)
	palo->posOrig = palo->xf.pos;   // glm::vec3

	/*if (palo->pvtalo && palo->pvtalo->pfnUnadjustAloRotation)
		palo->pvtalo->pfnUnadjustAloRotation(palo, &palo->matOrig);*/

		// Decompose to Euler (radians), PS2 logic
	palo->eulOrig = DecomposeRotateMatrixEuler(palo->matOrig);
}

void AdjustAloRtckMat(ALO* palo, CM* pcm, RTCK rtck, glm::vec3* pposCenter, glm::mat4& pmat)
{
	// 1) dpos = -camera X
	glm::vec3 camX = glm::vec3(pcm->mat[1]);
	glm::vec3 dpos = camX;
	glm::vec3 dposN = glm::normalize(dpos);

	// 2) Rotate object Z to dpos (Z-normal billboard)
	glm::mat3 R1;
	glm::vec3 z0 = glm::vec3(pmat[2]);
	BuildRotateVectorsMatrix(&z0, &dposN, &R1);

	glm::mat4 D1;
	LoadMatrixFromPosRot(g_vecZero, R1, D1);

	// Move to center frame, apply rotation
	glm::mat4 M = pmat;
	M[3] = glm::vec4(glm::vec3(M[3]) - *pposCenter, M[3].w);
	glm::mat4 alignedMat = D1 * M;

	// 3) Reflect current X about dpos (swapped axis vs original)
	glm::vec3 vX = glm::vec3(alignedMat[0]);
	float s = 2.0f * glm::dot(vX, dposN);
	glm::vec3 vXr = vX - s * dposN;

	// 4) Write back local_e0 to pmat, keeping X column from local_e0
	pmat = alignedMat;
	pmat[0] = glm::vec4(vX, alignedMat[0].w);

	// 5) Rotate reflected X to camera Z, then compose back around +center
	glm::vec3 camZ = glm::vec3(pcm->mat[2]);
	glm::mat3 R2;
	glm::vec3 vXrN = glm::normalize(vXr);
	glm::vec3 camZN = glm::normalize(camZ);
	BuildRotateVectorsMatrix(&vXrN, &camZN, &R2);

	glm::mat4 D2;
	LoadMatrixFromPosRot(*pposCenter, R2, D2);

	// 6) Final result
	pmat = D2 * pmat;
}

void CloneAloHierarchy(ALO* palo, ALO* paloBase)
{
	DLI it{};

	it.m_pdl = &paloBase->dlChild;
	it.m_ibDle = paloBase->dlChild.ibDle;
	it.m_pdliNext = s_pdliFirst;
	it.m_ppv = (void**)it.m_pdl;

	s_pdliFirst = &it;

	// Clone parent (keep your signature)
	palo->pvtlo->pfnCloneLo(palo, paloBase);

	// Walk base children and clone each
	LO* child = (LO*)*it.m_ppv;
	while (child != nullptr)
	{
		// Advance iterator to next-pointer-field for this child
		it.m_ppv = (void**)((uintptr_t)child + it.m_ibDle);

		PloCloneLo(child, palo->psw, palo);

		child = (LO*)*it.m_ppv;
	}

	s_pdliFirst = it.m_pdliNext;
}

void CloneAlo(ALO* palo, ALO* paloBase)
{
	palo->dlChild = paloBase->dlChild;
	//palo->dleBusy = paloBase->dleBusy;
	//palo->dleMRD = paloBase->dleMRD;
	//palo->paloRoot = paloBase->paloRoot;
	//palo->paloFreezeRoot = paloBase->paloFreezeRoot;
	//palo->dleFreeze = paloBase->dleFreeze;
	//palo->dlFreeze = paloBase->dlFreeze;
	palo->cpmrg = paloBase->cpmrg;
	for (int i = 0; i < 4; ++i)
		palo->apmrg[i] = paloBase->apmrg[i];
	palo->sMRD = paloBase->sMRD;
	palo->sCelBorderMRD = paloBase->sCelBorderMRD;
	palo->grfzon = paloBase->grfzon;
	palo->dsMRDSnap = paloBase->dsMRDSnap;
	palo->frz = paloBase->frz;
	palo->xf = paloBase->xf;
	palo->posOrig = paloBase->posOrig;
	palo->matOrig = paloBase->matOrig;
	palo->eulOrig = paloBase->eulOrig;
	//palo->dlAct = paloBase->dlAct;
	palo->pactPos = paloBase->pactPos;
	palo->pactRot = paloBase->pactRot;
	palo->pactScale = paloBase->pactScale;
	palo->apactPose = paloBase->apactPose;
	palo->pactRestore = paloBase->pactRestore;
	palo->pactla = paloBase->pactla;
	palo->pactbank = paloBase->pactbank;
	palo->pikh = paloBase->pikh;
	palo->pclqPosSpring = paloBase->pclqPosSpring;
	palo->pclqPosDamping = paloBase->pclqPosDamping;
	palo->pclqRotSpring = paloBase->pclqRotSpring;
	palo->pclqRotDamping = paloBase->pclqRotDamping;
	palo->psmpaPos = paloBase->psmpaPos;
	palo->psmpaRot = paloBase->psmpaRot;
	if (paloBase->palox)
		palo->palox = std::make_unique<ALOX>(*paloBase->palox);
	else
		palo->palox.reset();
	palo->cframeStatic = paloBase->cframeStatic;
	palo->globset = paloBase->globset;

	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		if (palo->globset.aglob[i].fThreeWay == 1 && palo->globset.aglob[i].fDynamic == 0)
		{
			size_t totalVerts = 0;

			for (int a = 0; a < palo->globset.aglob[i].asubglob.size(); a++)
				totalVerts += palo->globset.aglob[i].asubglob[a].vertices.size();

			if (palo->globset.aglob[i].fThreeWay == 1 && palo->globset.aglob[i].fDynamic == 0)
			{
				if (palo->globset.aglob[i].pwarpGlob == nullptr)
				{
					palo->globset.aglob[i].trlk = TRLK_Relight;

					glGenBuffers(1, &palo->globset.aglob[i].ssboCachedMaterial);
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, palo->globset.aglob[i].ssboCachedMaterial);
					glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)(totalVerts * sizeof(MATERIAL)), nullptr, GL_STATIC_DRAW);
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
				}
			}
		}

		if (palo->globset.aglob[i].asubglob.size() > 0)
		{
			SetRpCount(&palo->globset.aglob[i], palo->globset.aglob[i].fTransluscentSort);
			numRo++;
		}

		if (palo->globset.aglob[i].edgeCount > 0)
		{
			SetRpCount(&palo->globset.aglob[i], 0);
			numRoCel++;
		}
	}

	palo->pshadow = paloBase->pshadow;
	palo->pthrob = paloBase->pthrob;
	palo->sFastShadowRadius = paloBase->sFastShadowRadius;
	palo->sFastShadowDepth = paloBase->sFastShadowDepth;
	palo->fRealClock = paloBase->fRealClock;
	palo->pfader = paloBase->pfader;
	palo->dtUpdatePause = paloBase->dtUpdatePause;
	palo->pasegd = paloBase->pasegd;
	palo->sRadiusRenderSelf = paloBase->sRadiusRenderSelf;
	palo->sRadiusRenderAll = paloBase->sRadiusRenderAll;
	palo->psfx = paloBase->psfx;
	palo->ficg = paloBase->ficg;
	palo->cposec = paloBase->cposec;
	palo->aposec = paloBase->aposec;
	palo->pactrefCombo = paloBase->pactrefCombo;
	palo->pdlrFirst = paloBase->pdlrFirst;
	palo->fBusy = true;
	//palo->bitfield = paloBase->bitfield;
	palo->ackRot = paloBase->ackRot;

	/*if (palo->fForceCameraFade && FIsLoInWorld(palo))
		AddCmFadeObject(g_pcm, palo);*/

	CloneLo(palo, paloBase);

	ClearDl(&palo->dlChild);
}

bool FIsZeroV(const glm::vec3& v)
{
	return glm::dot(v, v) < 4.0f;
}

bool FIsZeroW(const glm::vec3& w)
{
	return glm::dot(w, w) < 0.0004f;
}

int FIsAloStatic(ALO* palo)
{
	if (!FIsZeroV(palo->xf.v))
		return false;

	if (!FIsZeroW(palo->xf.w))
		return false;

	for (ALO* child = palo->dlChild.paloFirst; child != nullptr; child = child->dleChild.paloNext)
	{
		if ((child->pvtlo->grfcid & 1U) == 0)
			continue;

		if (!FIsAloStatic(child))
			return false;
	}

	return true;
}

void ResolveAlo(ALO* palo)
{
	if (palo->paloRoot != nullptr)
		palo->paloRoot->cframeStatic = 0;
}

void SetAloParent(ALO* palo, ALO* paloParent)
{
	if (palo->paloParent == paloParent)
		return;

	glm::vec3 posWorld = palo->xf.posWorld;
	glm::mat3 matWorld = palo->xf.matWorld;

	palo->pvtalo->pfnRemoveLo(palo);

	ConvertAloPos(nullptr, paloParent, posWorld, palo->xf.pos);
	ConvertAloMat(nullptr, paloParent, matWorld, palo->xf.mat);

	const bool wasRoot = palo->paloParent == nullptr;
	const bool nowRoot = paloParent == nullptr;

	if (wasRoot != nowRoot)
	{
		if (nowRoot)
		{
			// Becoming world/root object.
			palo->zons = 2;

			if (palo->viss != 1)
			{
				palo->zons = 2;
				palo->viss = 2;
			}

			if (palo->mrds != 1)
				palo->mrds = 2;
		}
		else
		{
			// Becoming child object.
			palo->zons = 1;

			if (palo->viss != 1)
			{
				palo->zons = 1;
				palo->viss = 0;
			}

			if (palo->mrds != 1 && !(palo->mrds == 2 && palo->sMRD != 1.0e10f))
				palo->mrds = 0;
		}
	}

	UpdateAloOrig(palo);
	palo->paloParent = paloParent;

	palo->pvtlo->pfnAddLo(palo);
}

void ApplyAloProxy(ALO* palo, PROXY* pproxyApply)
{
	glm::vec3 posWorld{};
	ConvertAloPos((ALO*)pproxyApply, nullptr, palo->xf.pos, posWorld);
	palo->pvtalo->pfnTranslateAloToPos(palo, posWorld);

	glm::mat3 matWorld{};
	ConvertAloMat((ALO*)pproxyApply, nullptr, palo->xf.mat, matWorld);
	palo->pvtalo->pfnRotateAloToMat(palo, matWorld);

	palo->posOrig = palo->xf.pos;
	palo->matOrig = palo->xf.mat;
}

void BindAlo(ALO* palo)
{
	BindAloAlox(palo);
	UpdateAloOrig(palo);
	BindGlobset(&palo->globset, palo);

	LO* plo = palo->dlChild.ploFirst;

	while (plo != nullptr)
	{
		if (plo->pvtalo->pfnBindAlo != nullptr)
			plo->pvtalo->pfnBindAlo((ALO*)plo);

		plo = plo->dleChild.ploNext;
	}
}

void BindGlobset(GLOBSET* pglobset, ALO* palo)
{

}

void UpdateAloXfWorld(ALO* palo)
{
	palo->pvtalo->pfnUpdateAloXfWorldHierarchy(palo);
}

void UpdateAloXfWorldHierarchy(ALO* palo)
{
	ALOX* palox = palo->palox.get();

	// ------------------------------------------------------------
	// 1) Compute posWorld / matWorld
	// ------------------------------------------------------------

	// Choose parent for POSITION
	ALO* parentPos = palo->paloParent;

	if (palox)
	{
		const uint32_t f = palox->grfalox;

		if ((f & 0xCU) != 0)
		{
			if ((f & 4U) != 0)
				parentPos = palox->scj.paloSchRot;
		}
	}

	// posWorld
	if (!parentPos)
		palo->xf.posWorld = palo->xf.pos;
	else
		palo->xf.posWorld = parentPos->xf.matWorld * palo->xf.pos + parentPos->xf.posWorld;

	// Choose parent for ROTATION
	ALO* parentRot = nullptr;

	if (!palox)
		parentRot = palo->paloParent;
	else
	{
		const uint32_t f = palox->grfalox;
		parentRot = ((f & 8U) == 0) ? palo->paloParent : palox->scj.paloSchRot;
	}

	// matWorld
	if (!parentRot)
		palo->xf.matWorld = palo->xf.mat;
	else
		palo->xf.matWorld = parentRot->xf.matWorld * palo->xf.mat;

	// ------------------------------------------------------------
	// 2) VISMAP / grfzon update (replace with your real flag check)
	// ------------------------------------------------------------

	if (palo->zons == 2)
	{
		VISMAP* pvismap = palo->psw->pvismap;
		if (!pvismap)
			palo->grfzon = 0x0FFFFFFF; // 0xfffffff in the original
		else
			ClipVismapSphereOneHop(pvismap, &palo->xf.posWorld, palo->sRadiusRenderAll, &palo->grfzon);
	}

	// ------------------------------------------------------------
	// 3) Shadow update
	// ------------------------------------------------------------
	if (palo->pshadow)
	{
		SetShadowCastPosition(palo->pshadow.get(), palo->xf.posWorld);

		SHD* pshd = palo->pshadow->pshd;
		if (pshd && pshd->shdk == 3)
		{
			glm::vec3 normalCast = -palo->xf.matWorld[2];
			SetShadowCastNormal(palo->pshadow.get(), normalCast);

			glm::vec3 up = palo->xf.matWorld[1];
			SetShadowFrustrumUp(palo->pshadow.get(), &up);
		}
	}

	// ------------------------------------------------------------
	// 4) Recurse children (LO vtable function pointer)
	// ------------------------------------------------------------
	for (ALO* child = palo->dlChild.paloFirst; child; child = child->dleChild.paloNext)
	{
		auto* vt = child->pvtalo;
		if (vt && vt->pfnUpdateAloXfWorldHierarchy)
			vt->pfnUpdateAloXfWorldHierarchy(child);
	}

	// ------------------------------------------------------------
	// 5) IK / scheduler invalidation
	// ------------------------------------------------------------
	/*palox = palo->palox.get();
	if (palox)
	{
		const uint32_t f = palox->grfalox;

		if ((f & 0x8020U) == 0x8020U)
		{
			ALO* shoulder = palox->ikh.paloShoulder;
			ALO* elbow = palox->ikh.paloElbow;

			if (shoulder && shoulder->palox)
				shoulder->palox->ikj.fInvalid = 1;

			if (elbow && elbow->palox)
				elbow->palox->ikj.fInvalid = 1;
		}

		if ((f & 0x8100U) == 0x8100U && palox->ikh.grfik > 0)
		{
			const int count = palox->ikh.grfik;
			ALO** apalo = palox->sch.apalo;

			for (int i = 0; i < count; ++i)
			{
				ALO* a = apalo ? apalo[i] : nullptr;
				if (!a) continue;

				ALOX* ax = a->palox.get();
				if (!ax) continue;

				if (palo == ax->scj.paloSchRot)
					ax->scj.fInvalidRot = 1;

				if (palo == ax->scj.paloSchPos)
					ax->scj.fInvalidPos = 1;
			}
		}
	}*/
}

void UpdateAloHierarchy(ALO* palo, float dt)
{
	if (palo->pvtalo->pfnUpdateAlo != nullptr)
		palo->pvtalo->pfnUpdateAlo(palo, dt);

	int isInSw = FIsLoInWorld(palo);

	if (isInSw == true)
	{
		DLI dlBusyWalker;

		dlBusyWalker.m_pdl = &palo->dlChild;        // Point to the actual DL list
		dlBusyWalker.m_ibDle = palo->dlChild.ibDle; // Offset to the 'next' pointer inside each object
		dlBusyWalker.m_pdliNext = s_pdliFirst;      // Link this walker into a global list of DLI walkers

		// Get the first object (LO) in the busy list
		LO* currentObject = palo->dlChild.ploFirst;

		// Set up the pointer to the "next" object in the list,
		// using offset-based pointer arithmetic from current object
		dlBusyWalker.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle);

		// Save the current DLI walker globally
		s_pdliFirst = &dlBusyWalker;

		// Loop over every object in the busy list
		while (currentObject != nullptr)
		{
			// Call the update function on the current object child
			// This updates the object and all of its attached ALO children
			if ((currentObject->pvtalo->grfcid & 1U) != 0)
				UpdateAloHierarchy(reinterpret_cast<ALO*>(currentObject), dt);

			// Move to the next object in the list using the stored offset
			currentObject = reinterpret_cast<LO*>(*dlBusyWalker.m_ppv);

			// If there is a next object, update the walker’s pointer to its next link
			dlBusyWalker.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle);
		}

		s_pdliFirst = dlBusyWalker.m_pdliNext;
	}
}

void TranslateAloToPos(ALO* palo, glm::vec3& ppos)
{
	palo->xf.pos = ppos;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
}

void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3& pposFrom, glm::vec3& pposTo)
{
	if (paloFrom == paloTo) {
		pposTo = pposFrom;
		return;
	}

	// local -> world (or already world if paloFrom == nullptr)
	glm::vec3 world = pposFrom;
	if (paloFrom)
		world = paloFrom->xf.matWorld * pposFrom + paloFrom->xf.posWorld;

	// world -> paloTo local (or keep world if paloTo == nullptr)
	if (paloTo) {
		glm::vec3 delta = world - paloTo->xf.posWorld;
		glm::mat3 invRot = glm::transpose(paloTo->xf.matWorld);
		pposTo = invRot * delta;
	}
	else
		pposTo = world;
}

void ConvertAloVec(ALO* paloFrom, ALO* paloTo, glm::vec3* pvecFrom, glm::vec3* pvecTo)
{
	glm::vec3 vecWorld = *pvecFrom;

	// Transform from local to world space if paloFrom is valid and different from paloTo
	if (paloFrom && paloFrom != paloTo)
		vecWorld = paloFrom->xf.matWorld * (*pvecFrom);

	// Transform from world to local space of paloTo if it's valid
	if (paloTo)
	{
		glm::mat3 invMat = glm::transpose(paloTo->xf.matWorld);
		*pvecTo = invMat * vecWorld;
	}
	else
		*pvecTo = vecWorld;
}

void RotateAloToMat(ALO* palo, glm::mat3& pmat)
{
	palo->xf.mat = pmat;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
}

void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3& pmatFrom, glm::mat3& pmatTo)
{
	if (paloFrom == paloTo) {
		pmatTo = pmatFrom;
		return;
	}

	glm::mat3 world = pmatFrom;

	if (paloFrom)
		world = paloFrom->xf.matWorld * pmatFrom;

	if (paloTo)
		pmatTo = glm::transpose(paloTo->xf.matWorld) * world;
	else
		pmatTo = world;
}

void SetAloInitialVelocity(ALO* palo, glm::vec3* pv)
{
	const glm::mat4& m = palo->xf.mat;

	glm::vec3 transformedVelocity =
		glm::vec3(m[0]) * pv->x +
		glm::vec3(m[1]) * pv->y +
		glm::vec3(m[2]) * pv->z;

	palo->xf.v = transformedVelocity;
}

void SetAloInitialAngularVelocity(ALO* palo, const glm::vec3* pw)
{
	const glm::mat4& m = palo->xf.mat;

	glm::vec3 transformedAngularVelocity =
		glm::vec3(m[0]) * pw->x +
		glm::vec3(m[1]) * pw->y +
		glm::vec3(m[2]) * pw->z;

	palo->xf.w = transformedAngularVelocity;
}

ASEGD* PasegdEnsureAlo(ALO* palo)
{
	if (palo->pasegd == nullptr)
	{
		palo->pasegd = std::make_shared <ASEGD>();
		palo->pasegd->oidAseg = OID_Nil;
		palo->pasegd->iak = IAK_Time;
		palo->pasegd->tLocal = 0.0f;
		palo->pasegd->svtLocal = 1.0f;
	}

	return palo->pasegd.get();
}

SHADOW* PshadowAloEnsure(ALO* palo)
{
	if (palo->pshadow == nullptr)
	{
		palo->pshadow = std::make_shared <SHADOW>();
		InitShadow(palo->pshadow.get());
		AppendDlEntry(&palo->psw->dlShadow, palo->pshadow.get());
	}

	return palo->pshadow.get();
}

SHADOW* PshadowInferAlo(ALO* palo)
{
	return nullptr;
}

void SetAloAsegdOid(ALO* palo, short oid)
{
	palo->pasegd->oidAseg = (OID)oid;
}

void SetAloAsegdtLocal(ALO* palo, float tLocal)
{
	palo->pasegd->tLocal = tLocal;
}

void SetAloAsegdSvtLocal(ALO* palo, float svtLocal)
{
	palo->pasegd->svtLocal = svtLocal;
}

void SetAloAsegdiak(ALO* palo, int iak)
{
	palo->pasegd->iak = (IAK)iak;
}

void SetAloFrozen(ALO* palo, bool fFrozen)
{
	palo->fFrozen = fFrozen;
}

void SetAloEuler(ALO* palo, glm::vec3* peul)
{

}

void SetAloVelocityLocal(ALO* palo, glm::vec3* pvec)
{
	// Convert local-space velocity to world-space velocity
	glm::vec3 localVelocity = *pvec;
	glm::vec3 worldVelocity = palo->xf.mat * localVelocity;

	//palo->pvtalo->pfnSetAloVelocityVec(palo, &worldVelocity);
}

void SetAloFastShadowRadius(ALO* palo, float sRadius)
{
	palo->sFastShadowRadius = sRadius;
}

void SetAloFastShadowDepth(ALO* palo, float sDepth)
{
	palo->sFastShadowDepth = sDepth;
}

void SetAloCastShadow(ALO* palo, byte fCastShadow)
{
	if (fCastShadow == 0) 
	{
		if (palo->pshadow != nullptr) 
		{
			RemoveDlEntry(&palo->psw->dlShadow, palo->pshadow.get());
			AppendDlEntry(&g_dlShadowPending, palo->pshadow.get());
			palo->pshadow = nullptr;
		}
	}
	else
		PshadowAloEnsure(palo);
}

void SetAloShadowShader(ALO* palo, OID oidShdShadow)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowShader(pshadow, oidShdShadow);
}

void GetAloShadowShader(ALO* palo, OID* poidShdShadow)
{
	if (palo && palo->pshadow && palo->pshadow->pshd) {
		*poidShdShadow = static_cast<OID>(palo->pshadow->pshd->oid);
	}
	else {
		*poidShdShadow = OID_Nil;
	}
}

void GetAloShadowNearRadius(ALO* palo, float* psNearRadius)
{
	SHADOW* pshadow = PshadowInferAlo(palo);
	*psNearRadius = pshadow->sNearRadius;
}

void SetAloShadowNearRadius(ALO* palo, float sNearRadius)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowNearRadius(pshadow, sNearRadius);
}

void SetAloShadowFarRadius(ALO* palo, float sFarRadius)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowFarRadius(pshadow, sFarRadius);
}

void GetAloShadowFarRadius(ALO* palo, float* psFarRadius)
{
	SHADOW* pshadow = PshadowInferAlo(palo);
	*psFarRadius = pshadow->sFarRadius;
}

void SetAloShadowNearCast(ALO* palo, float sNearCast)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowNearCast(pshadow, sNearCast);
}

void GetAloShadowNearCast(ALO* palo, float* psNearCast)
{
	SHADOW* pshadow = PshadowInferAlo(palo);
	*psNearCast = pshadow->sNearCast;
}

void SetAloShadowFarCast(ALO* palo, float sFarCast)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowFarCast(pshadow, sFarCast);
}

void GetAloShadowFarCast(ALO* palo, float* psFarCast)
{
	SHADOW* pshadow = PshadowInferAlo(palo);
	*psFarCast = pshadow->sFarCast;
}

void SetAloShadowConeAngle(ALO* palo, float degConeAngle)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowConeAngle(pshadow, degConeAngle);
}

void GetAloShadowConeAngle(ALO* palo, float* pdegConeAngle)
{
	SHADOW* pshadow = PshadowInferAlo(palo);

	float angleRadians = std::atan2(pshadow->sNearRadius / pshadow->sNearCast, 1.0f);
	*pdegConeAngle = 2.0f * angleRadians * 57.29578f;
}

void SetAloShadowFrustrumUp(ALO* palo, glm::vec3* pvecUp)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowFrustrumUp(pshadow, pvecUp);
}

void GetAloShadowFrustrumUp(ALO* palo, glm::vec3* pvecUp)
{
	SHADOW* pshadow = PshadowInferAlo(palo);

	*pvecUp = pshadow->vecUp;
}

void SetAloDynamicShadowObject(ALO* palo, OID oidDysh)
{
	SHADOW *pshadow = PshadowAloEnsure(palo);
	pshadow->oidDysh = oidDysh;
}

void SetAloNoFreeze(ALO* palo, int fNoFreeze)
{
	palo->fFrozen = fNoFreeze;
}

void SetAloRestorePosition(ALO* palo, int fRestore)
{

}

void SetAloRestorePositionAck(ALO* palo, ACK ack)
{

}

void SetAloPositionSpring(ALO* palo, float r)
{

}

void SetAloPositionSpringDetail(ALO* palo, CLQ* pclq)
{

}

void SetAloPositionDamping(ALO* palo, float r)
{

}

void SetAloPositionDampingDetail(ALO* palo, CLQ* pclq)
{

}

void SetAloRestoreRotation(ALO* palo, int fRestore)
{

}

void SetAloRestoreRotationAck(ALO* palo, ACK ack)
{

}

void SetAloRotationSpring(ALO* palo, float r)
{

}

void SetAloRotationSpringDetail(ALO* palo, CLQ* pclq)
{

}

void SetAloRotationDamping(ALO* palo, float r)
{

}

void SetAloRotationDampingDetail(ALO* palo, CLQ* pclq)
{

}

void SetAloPositionSmooth(ALO* palo, float r)
{

}

void SetAloPositionSmoothDetail(ALO* palo, SMPA* psmpa)
{

}

void SetAloRotationSmooth(ALO* palo, float r)
{

}

void SetAloRotationSmoothDetail(ALO* palo, SMPA* psmpa)
{

}

void SetAloPositionSmoothMaxAccel(ALO* palo, float r)
{

}

void SetAloRotationSmoothMaxAccel(ALO* palo, float r)
{

}

void SetAloDefaultAckPos(ALO* palo, ACK ack)
{

}

void SetAloDefaultAckRot(ALO* palo, ACK ack)
{

}

void SetAloLookAt(ALO* palo, ACK ack)
{

}

void SetAloLookAtIgnore(ALO* palo, float sIgnore)
{

}

void SetAloLookAtPanFunction(ALO* palo, CLQ* pclq)
{

}

void SetAloLookAtPanLimits(ALO* palo, LM* plm)
{

}

void SetAloLookAtTiltFunction(ALO* palo, CLQ* pclq)
{

}

void SetAloLookAtTiltLimits(ALO* palo, LM* plm)
{

}

void SetAloLookAtEnabledPriority(ALO* palo, int nPriority)
{

}

void SetAloLookAtDisabledPriority(ALO* palo, int nPriority)
{

}

void SetAloTargetAttacks(ALO* palo, int grftak)
{

}

void SetAloTargetRadius(ALO* palo, float sRadiusTarget)
{

}

void SetAloThrobKind(ALO* palo, THROBK throbk)
{

}

void SetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor)
{

}

void SetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor)
{

}

void SetAloThrobDtInOut(ALO* palo, float dtInOut)
{

}

void SetAloSfxid(ALO* palo, SFXID sfxid)
{

}

void SetAloSStart(ALO* palo, float sStart)
{

}

void SetAloSFull(ALO* palo, float sFull)
{

}

void SetAloUVolumeSpl(ALO* palo, float uVol)
{

}

void SetAloUVolume(ALO* palo, float uVol)
{

}

void SetAloUPitchSpl(ALO* palo, float uPitch)
{

}

void SetAloUPitch(ALO* palo, float uPitch)
{

}

void SetAloSndRepeat(ALO* palo, LM* plm)
{

}

void SetAloUDoppler(ALO* palo, float uDoppler)
{

}

void SetAloInteractCane(ALO* palo, int grfic)
{

}

void SetAloInteractCaneSweep(ALO* palo, int grfic)
{

}

void SetAloInteractCaneRush(ALO* palo, int grfic)
{

}

void SetAloInteractCaneSmash(ALO* palo, int grfic)
{

}

void SetAloInteractBomb(ALO* palo, int grfic)
{

}

void SetAloInteractShock(ALO* palo, int grfic)
{

}

void SetAloPoseCombo(ALO* palo, OID oidCombo)
{

}

void SetAloForceCameraFade(ALO* palo, int fFade)
{

}

void SetAloCelRgba(ALO* palo, RGBA prgba)
{
	palo->globset.rgbaCel.r = prgba.bRed   / 255.0;
	palo->globset.rgbaCel.g = prgba.bGreen / 255.0;
	palo->globset.rgbaCel.b = prgba.bBlue  / 255.0;
	palo->globset.rgbaCel.a = prgba.bAlpha / 255.0;

	palo->globset.grfglobset = palo->globset.grfglobset | 2;
}

void SetAloOverrideCel(ALO *palo, glm::vec4 *rgba)
{
	palo->globset.grfglobset |= 0x2;
	palo->globset.rgbaCel = *rgba;

	ALO *child = palo->dlChild.paloFirst;

	while (child != nullptr)
	{
		if ((child->pvtlo->grfcid & 0x1) != 0)
			SetAloOverrideCel(child, rgba);

		child = child->dleChild.paloNext;
	}
}

void UpdateAloThrob(ALO* palo, float dt)
{
	THROB* throb = palo->pthrob;

	if (throb->dtInOut <= 0.0f)
		return;

	float t = std::fmod(g_clock.t, throb->dtInOut);
	float wave = std::sin((t * glm::two_pi<float>()) / throb->dtInOut);
	float blend = wave * 0.5f + 0.5f; // 0..1

	glm::vec3 hsv = throb->hsvIn * blend + throb->hsvOut * (1.0f - blend);

	glm::vec3 rgb{};
	ConvertUserHsvToUserRgb(hsv, rgb);

	glm::vec4 overrideCel(rgb, 0.5f); // 0x80 / 255 ~= 0.502

	SetAloOverrideCel(palo, &overrideCel);
}

void* GetAloFrozen(ALO* palo)
{
	return nullptr;
}

void* GetAloXfPos(ALO* palo)
{
	return &palo->xf.pos;
}

void* GetAloXfPosOrig(ALO* palo)
{
	return &palo->posOrig;
}

void* GetAloXfPosWorld(ALO* palo)
{
	return &palo->xf.posWorld;
}

void* GetAloXfMat(ALO* palo)
{
	return &palo->xf.mat;
}

void* GetAloMatOrig(ALO* palo)
{
	return &palo->matOrig;
}

void* GetAloXfMatWorld(ALO* palo)
{
	return &palo->xf.matWorld;
}

void* GetAloEuler(ALO* palo)
{
	return nullptr;
}

void GetAloVelocityLocal(ALO* palo, glm::vec3* pvec)
{
	// Transform world velocity into local space by applying the inverse of the rotation matrix.
	// If the matrix is orthonormal, the inverse is just the transpose.
	glm::mat3 rotation = palo->xf.mat;
	glm::vec3 worldVelocity = palo->xf.v;

	// Convert world velocity to local space
	*pvec = glm::transpose(rotation) * worldVelocity;
}

void* GetAloXfw(ALO* palo)
{
	return &palo->xf.w;
}

void* GetAloXfdv(ALO* palo)
{
	return &palo->xf.dv;
}

void* GetAloXfdw(ALO* palo)
{
	return &palo->xf.dw;
}

void* GetAloRoot(ALO* palo)
{
	return palo->paloRoot;
}

void GetAloFastShadowRadius(ALO* palo, float* psRadius)
{
	*psRadius = palo->sFastShadowRadius;
}

void GetAloFastShadowDepth(ALO* palo, float* psDepth)
{
	*psDepth = palo->sFastShadowDepth;
}

void GetAloCastShadow(ALO* palo, int* pfCastShadow)
{

}

void GetAloLookAtIgnore(ALO* palo, float* psIgnore)
{

}

void GetAloLookAtPanFunction(ALO* palo, CLQ* pclq)
{

}

void GetAloLookAtPanLimits(ALO* palo, LM* plm)
{

}

void GetAloLookAtTiltFunction(ALO* palo, CLQ* pclq)
{

}

void GetAloLookAtTiltLimits(ALO* palo, LM* plm)
{

}

void GetAloLookAtEnabledPriority(ALO* palo, int* pnPriority)
{

}

void GetAloLookAtDisabledPriority(ALO* palo, int* pnPriority)
{

}

int FGetAloChildrenList(ALO* palo, void* pvstate)
{
	return 0;
}

void GetAloThrobKind(ALO* palo, THROBK* pthrobk)
{

}

void GetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor)
{

}

void GetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor)
{

}

void GetAloThrobDtInOut(ALO* palo, float* pdtInOut)
{

}

void GetAloSfxid(ALO* palo, SFXID* psfxid)
{

}

void GetAloSStart(ALO* palo, float* psStart)
{

}

void GetAloSFull(ALO* palo, float* psFull)
{

}

void GetAloUVolume(ALO* palo, float* puVol)
{

}

void GetAloUPitch(ALO* palo, float* puPitch)
{

}

void GetAloSndRepeat(ALO* palo, LM* plmRepeat)
{

}

void GetAloUDoppler(ALO* palo, float* puDoppler)
{

}

void GetAloInteractCane(ALO* palo, int* pgrfic)
{

}

void GetAloInteractCaneSweep(ALO* palo, int* pgrfic)
{

}

void GetAloInteractCaneRush(ALO* palo, int* pgrfic)
{

}

void GetAloInteractCaneSmash(ALO* palo, int* pgrfic)
{

}

void GetAloInteractBomb(ALO* palo, int* pgrfic)
{

}

void GetAloInteractShock(ALO* palo, int* pgrfic)
{

}

void* GetAlofRealClock(ALO* palo)
{
	return &palo->fRealClock;;
}

void CalculateAloMovement(ALO* paloLeaf, ALO* paloBasis, glm::vec3& pos, glm::vec3* pv, glm::vec3* pw, glm::vec3* pdv, glm::vec3* pdw)
{
	glm::vec3 angularVelocitySum(0.0f);
	glm::vec3 angularAccelSum(0.0f);

	if (pv) {
		*pv = glm::vec3(0.0f);
	}

	if (pdv) {
		*pdv = glm::vec3(0.0f);
	}

	std::vector<ALO*> chain;
	chain.reserve(16);

	for (ALO* node = paloLeaf; node != paloBasis && node != nullptr; node = node->paloParent) {
		const std::shared_ptr<ALOX>& palox = node->palox;

		if (!palox || (palox->grfalox & 0xCu) == 0) {
			chain.push_back(node);
		}
	}

	for (int i = static_cast<int>(chain.size()) - 1; i >= 0; --i) {
		ALO* node = chain[i];
		ALO* parent = node->paloParent;

		if (pv || pw || pdv) {
			glm::vec3 w(0.0f);
			ConvertAloVec(parent, paloBasis, &node->xf.w, &w);
			angularVelocitySum += w;
		}

		if (pdw || pdv) {
			glm::vec3 dw(0.0f);
			ConvertAloVec(parent, paloBasis, &node->xf.dw, &dw);
			angularAccelSum += dw;
		}

		if (pv || pdv) {
			glm::vec3 referencePoint = pos;

			if (i > 0) {
				referencePoint = chain[i - 1]->xf.posWorld;
			}

			glm::vec3 offset = referencePoint - node->xf.posWorld;
			ConvertAloVec(nullptr, paloBasis, &offset, &offset);

			if (pv) {
				*pv += glm::cross(angularVelocitySum, offset);

				glm::vec3 v(0.0f);
				ConvertAloVec(parent, paloBasis, &node->xf.v, &v);
				*pv += v;
			}

			if (pdv) {
				*pdv += glm::cross(angularVelocitySum, glm::cross(angularVelocitySum, offset));
				*pdv += glm::cross(angularAccelSum, offset);

				glm::vec3 dv(0.0f);
				ConvertAloVec(parent, paloBasis, &node->xf.dv, &dv);
				*pdv += dv;
			}
		}
	}

	if (pw) {
		*pw = angularVelocitySum;
	}

	if (pdw) {
		*pdw = angularAccelSum;
	}
}

void AddAloHierarchy(ALO* palo)
{
	DLI it{};

	it.m_pdl = &palo->dlChild;
	it.m_ibDle = palo->dlChild.ibDle;
	it.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &it;

	it.m_ppv = (void**)it.m_pdl;

	// Start with the parent
	LO* current = (LO*)palo;

	while (true)
	{
		// Call OnLoAdd for the CURRENT object (parent first, then each child)
		current->pvtlo->pfnOnLoAdd(current);

		// Load next child pointer from the current "next field"
		void* next = *it.m_ppv;
		if (next == nullptr)
			break;

		// Advance iterator to the "next pointer field" inside that next object
		it.m_ppv = (void**)((uintptr_t)next + it.m_ibDle);

		// Move to the next object so we don't keep calling the parent
		current = (LO*)next;
	}

	// palo->pvtlo->pfnSendLoMessage(palo, 1, palo);

	s_pdliFirst = it.m_pdliNext;
}

void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis)
{
	palo->xf.mat = pbis->ReadMatrix();
	palo->xf.pos = pbis->ReadVector();

	palo->zons = pbis->U8Read();
	palo->viss = pbis->U8Read();
	palo->mrds = pbis->U8Read();

	palo->grfzon = pbis->U32Read();
	palo->sMRD = pbis->F32Read();
	palo->sCelBorderMRD = pbis->F32Read();
	palo->sRadiusRenderSelf = pbis->F32Read();
	palo->sRadiusRenderAll = pbis->F32Read();

	if (palo->sMRD == 3.402823e+38f)
		palo->sMRD = 1.0e+10f;

	if (palo->sCelBorderMRD == 3.402823e+38f)
		palo->sCelBorderMRD = (palo->sMRD > 2000.0f) ? 2000.0f : palo->sMRD;

	LoadOptionsFromBrx(palo, pbis);
	LoadGlobsetFromBrx(&palo->globset, palo, pbis);
	LoadAloAloxFromBrx(palo, pbis);

	if (palo->pvtalo && palo->pvtalo->pfnUpdateAloXfWorld)
		palo->pvtalo->pfnUpdateAloXfWorld(palo);

	palo->cposec = pbis->U8Read();
	palo->aposec.resize(palo->cposec);

	for (int i = 0; i < palo->cposec; i++)
	{
		palo->aposec[i].oid = (OID)pbis->S16Read();
		palo->aposec[i].agPoses.resize(palo->globset.cpose);

		for (int a = 0; a < palo->globset.cpose; a++)
			palo->aposec[i].agPoses[a] = pbis->F32Read();
	}

	// Loads ALO children objects
	LoadSwObjectsFromBrx(palo->psw, palo, pbis);
}

void LoadAloAloxFromBrx(ALO* palo, CBinaryInputStream* pbis)
{
	const uint32_t grfalox = pbis->U32Read();

	if (grfalox == 0)
		return;

	ALOX alox;
	palo->palox = std::make_shared <ALOX>(alox);

	palo->palox->grfalox = grfalox;

	int unk_1;

	if (grfalox & 0x01)
		palo->palox->matPreRotation = pbis->ReadMatrix();

	if (grfalox & 0x02)
		palo->palox->matPostRotation = pbis->ReadMatrix();

	if ((grfalox & 0x0C) != 0)
	{
		int16_t schRotId = pbis->S16Read();

		if (schRotId != -1)
			palo->palox->scj.paloSchRot = static_cast<ALO*>(PloFindSwObject(palo->psw, 3, (OID)schRotId, palo));
	}

	if (grfalox & 0x10)
		palo->palox->scj.ipaloRot = static_cast<int16_t>(pbis->S16Read());

	if ((grfalox & 0x20) != 0)
	{
		unk_1 = pbis->S16Read();
		pbis->ReadVector(); // Read Vector
		pbis->ReadVector(); // Read Vector
		pbis->F32Read();
	}

	if ((grfalox & 0x40) != 0)
	{
		unk_1 = pbis->S16Read();
		unk_1 = pbis->S16Read();
	}

	if ((grfalox & 0x80) != 0)
	{
		pbis->U8Read();
	}
}

void BindAloAlox(ALO* palo)
{

}

void SnipAloObjects(ALO* palo, int csnip, SNIP* asnip)
{
	SW* psw = palo->psw;

	for (int i = 0; i < csnip; ++i)
	{
		const SNIP& snip = asnip[i];
		uint32_t grffso = (snip.grfsnip & 0x1) ? 0x105 : 0x101;

		if ((snip.grfsnip & 0x20) == 0)
			grffso &= ~0x100; // Remove 0x100 if bit 0x20 is not set

		LO* plo = PloFindSwObject(psw, grffso, snip.oid, palo);

		if (plo != nullptr)
		{
			if ((snip.grfsnip & 0x08) == 0)
			{
				// Store the pointer to the found object at a specific offset
				*(LO**)((char*)palo + snip.ib) = plo;
			}

			if ((snip.grfsnip & 0x04) == 0)
				SnipLo(plo);

			if ((snip.grfsnip & 0x10) != 0)
				SubscribeLoObject(plo, palo);
		}
	}
}

void PostAloLoad(ALO* palo)
{
	PostLoLoad(palo);
	PostGlobsetLoad(&palo->globset, palo);

	if (palo->pshadow != nullptr)
	{
		SHADOW *pshadow = palo->pshadow.get();

		if (!FShadowRadiusSet(pshadow))
		{
			SetShadowNearRadius(pshadow, palo->sRadiusRenderAll);
			SetShadowFarRadius(pshadow, palo->sRadiusRenderAll * 0.5f);
		}

		if (pshadow->oidDysh != OID_Nil)
		{
			pshadow->pdysh = reinterpret_cast<DYSH*>(PloFindSwNearest(palo->psw, pshadow->oidDysh, palo));

			if (pshadow->pdysh != nullptr)
				SetDyshShadow(pshadow->pdysh, pshadow);
		}

		PostShadowLoad(pshadow);
	}

	// --- Iterate children DL and call each entry's fn at vtbl+0x50 (was inlined dl.h) ---
	DLI dlBusyWalker{};

	dlBusyWalker.m_ibDle = palo->dlChild.ibDle;
	dlBusyWalker.m_pdliNext = s_pdliFirst;
	dlBusyWalker.m_pdl = &palo->dlChild;

	ALO* currentObject = palo->dlChild.paloFirst;

	// Only valid if we have a first element
	dlBusyWalker.m_ppv = currentObject ? (void**)((uintptr_t)currentObject + dlBusyWalker.m_ibDle) : nullptr;

	s_pdliFirst = &dlBusyWalker;

	while (currentObject != nullptr)
	{
		if (currentObject->pvtalo->pfnPostAloLoad)
			currentObject->pvtalo->pfnPostAloLoad(currentObject);

		currentObject = (ALO*)*dlBusyWalker.m_ppv;

		// Guard before computing next pointer-field address
		dlBusyWalker.m_ppv = currentObject ? (void**)((uintptr_t)currentObject + dlBusyWalker.m_ibDle) : nullptr;
	}

	s_pdliFirst = dlBusyWalker.m_pdliNext;
}

void UpdateAlo(ALO* palo, float dt)
{
	UpdateGlobset(&palo->globset, palo, dt);

	if (palo->pshadow != nullptr)
		UpdateShadow(palo->pshadow.get(), dt);

	if (palo->pthrob != nullptr)
		UpdateAloThrob(palo, dt);
}

void RenderFastShadow(ALO* palo, CM* pcm, RO* pro)
{
	RO ro{};
	DupAloRo(palo, pro, &ro);

	glm::vec3 shadowScale(palo->sFastShadowRadius * 0.01f, palo->sFastShadowRadius * 0.01f, palo->sFastShadowDepth * 0.01f);

	glm::vec3 pos = glm::vec3(ro.model[3]);

	ro.model =
		glm::translate(glm::mat4(1.0f), pos) *
		glm::mat4_cast(glm::quat_cast(glm::mat3(1.0f))) *
		glm::scale(glm::mat4(1.0f), shadowScale);

	ALO* pShadowLo = (ALO*)palo->psw->aploStock[0x11];

	if (pShadowLo != nullptr && pShadowLo->pvtalo != nullptr && pShadowLo->pvtalo->pfnRenderAloSelf != nullptr)
		pShadowLo->pvtalo->pfnRenderAloSelf(pShadowLo, pcm, &ro);
}

void RenderAloAll(ALO* palo, CM* pcm, RO* pro)
{
	// hidden / disabled bit
	if (palo->fHidden != 0)
		return;

	// zone visibility
	if (palo->viss == 2)
	{
		if (g_fBsp != 0)
		{
			if ((palo->grfzon & pcm->grfzon) != pcm->grfzon)
				return;
		}
	}

	float uAlpha = 1.0f;
	RO roLocal{};
	RO* proFinal = pro;

	auto ensureLocal = [&]()
	{
			if (proFinal == &roLocal)
				return;

			DupAloRo(palo, proFinal, &roLocal);
			proFinal = &roLocal;
	};

	//if (palo->mrds == 2)
	//{
		glm::vec3 posWorld;

		if (pro != nullptr)
			posWorld = glm::vec3(pro->model[3]);
		else
			posWorld = palo->xf.posWorld;

		// Original does object position - camera position.
		glm::vec3 dpos = posWorld - pcm->pos;

		if (!SphereInFrustum(pcm->frustum, posWorld, palo->sRadiusRenderAll))
			return;

		if (!FInsideCmMrd(pcm, dpos, palo->sRadiusRenderAll, palo->sMRD, uAlpha))
			return;

		if (uAlpha != 1.0f)
		{
			ensureLocal();
			proFinal->uAlpha *= uAlpha;
		}

		if (palo->sCelBorderMRD < palo->sMRD)
		{
			float uAlphaCelBorder = 1.0f;

			if (!FInsideCmMrd(pcm, dpos, palo->sRadiusRenderAll, palo->sCelBorderMRD, uAlphaCelBorder))
				uAlphaCelBorder = 0.0f;

			if (uAlphaCelBorder != 1.0f)
			{
				ensureLocal();
				proFinal->uAlphaCelBorder *= uAlphaCelBorder;
			}
		}
	//}

	// pfader alpha
	if (palo->pfader != nullptr)
	{
		ensureLocal();
		//proFinal->uAlpha *= palo->pfader->uAlpha;
	}

	// ---- SSC scale-compensation block (the big ugly mid-function chunk) ----
	// The original logic:
	//   If proFinal != null and palo has ALOX with (grfalox & 0x400) and joint.fSsc
	//   then try to use parent's pactScale (with parent having ALOX & 0x400 too)
	//   and pre-multiply by inverse parent scale before rendering this ALO.
	//
	// Net effect: compensate for parent scale when SSC is active.
	ACT* scaleAct = palo->pactScale;
	RO* proForSelf = proFinal;

	//if (proFinal != nullptr)
	//{
	//	const bool sscActive =
	//		(palo->palox != nullptr) &&
	//		((palo->palox->grfalox & 0x400u) != 0) &&
	//		(palo->palox->joint.fSsc != 0);

	//	if (sscActive && palo->paloParent != nullptr)
	//	{
	//		ALO* parent = palo->paloParent;

	//		const bool parentSscOk =
	//			(parent->pactScale != nullptr) &&
	//			(parent->palox != nullptr) &&
	//			((parent->palox->grfalox & 0x400u) != 0);

	//		if (parentSscOk)
	//		{
	//			// get parent scale vector (the decomp loads it into roChild.mat + 0x10,
	//			// then inverts X/Y/Z)
	//			glm::vec3 parentScale(1.0f);
	//			parent->pactScale->GetScale(parentScale); // <-- adapt to your ACT API
	//			glm::vec3 invParentScale(
	//				parentScale.x != 0.0f ? 1.0f / parentScale.x : 1.0f,
	//				parentScale.y != 0.0f ? 1.0f / parentScale.y : 1.0f,
	//				parentScale.z != 0.0f ? 1.0f / parentScale.z : 1.0f
	//			);

	//			// Build inverse-scale matrix
	//			glm::mat4 invScale = glm::mat4(1.0f);
	//			invScale[0][0] = invParentScale.x;
	//			invScale[1][1] = invParentScale.y;
	//			invScale[2][2] = invParentScale.z;

	//			// We need a local RO to modify matrix safely
	//			ensureLocal();

	//			// Apply: roLocal.model = roLocal.model * invScale
	//			// (this matches the original: post-multiply by inverse scale)
	//			roLocal.model = roLocal.model * invScale;

	//			proForSelf = &roLocal;
	//		}
	//	}
	//}

	// ---- Render self ----
	palo->pvtalo->pfnRenderAloSelf(palo, pcm, proForSelf);

	// ---- Render children ----
	for (ALO* child = palo->dlChild.paloFirst; child; child = child->dleChild.paloNext)
	{
		if ((child->pvtlo->grfcid & 1u) == 0)
			continue;

		// If we have no RO, original just calls child with null
		if (proForSelf == nullptr)
		{
			child->pvtalo->pfnRenderAloAll(child, pcm, nullptr);
			continue;
		}

		// match original gate: child palox and (grfalox & 0xC) != 0
		const bool useWorldProxy = (child->palox != nullptr) && ((child->palox->grfalox & 0xCu) != 0);

		// 1) Build child's local matrix source
		glm::mat4 childMat(1.0f);
		if (!useWorldProxy)
			childMat = glm::translate(glm::mat4(1.0f), child->xf.pos) * glm::mat4(child->xf.mat);
		else
			childMat = glm::translate(glm::mat4(1.0f), child->xf.posWorld) * glm::mat4(child->xf.matWorld);

		// 2) Choose parent matrix for child
		glm::mat4 parentForChild = proForSelf->model;

		if (useWorldProxy)
		{
			glm::mat4 parentWorldTR = glm::translate(glm::mat4(1.0f), palo->xf.posWorld) * glm::mat4(palo->xf.matWorld);
			glm::mat4 invParentWorldTR = glm::inverse(parentWorldTR);
			parentForChild = proForSelf->model * invParentWorldTR;
		}

		// 3) Final child model
		RO roChild{};
		roChild.model = parentForChild * childMat;

		// 4) Inherit alpha
		roChild.uAlpha = proForSelf->uAlpha;
		roChild.uAlphaCelBorder = proForSelf->uAlphaCelBorder;

		// recurse
		child->pvtalo->pfnRenderAloAll(child, pcm, &roChild);
	}

	// ---- Fast shadow ----
	/*if (palo->sFastShadowRadius > 0.0f)
		RenderFastShadow(palo, pcm, proForSelf);*/
}

void RenderAloSelf(ALO* palo, CM* pcm, RO* pro)
{
	palo->pvtalo->pfnRenderAloGlobset(palo, pcm, pro);
}

void FreezeAlo(ALO* palo, int fFreeze)
{
	if (!fFreeze)
	{
		// Unfreeze.
		palo->fFrozen = false;

		/*palo->pvtalo->pfnSetAloVelocityVec(palo, &palo->frz.v);

		palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &palo->frz.w);

		if (palo->psfx)
		{
			StartSound(palo->psfx->sfxid, &palo->psfx->pamb, palo, nullptr, palo->psfx->sStart, palo->psfx->sFull, palo->psfx->uVol, palo->psfx->uPitch, palo->psfx->uDoppler, &palo->psfx->lmRepeat, nullptr);
		}*/

		return;
	}

	// Freeze: save current velocities.
	palo->frz.v = palo->xf.v;
	palo->frz.w = palo->xf.w;

	palo->dtUpdatePause = 0.0f;

	/*if (palo->psfx)
		StopSound(palo->psfx->pamb, 0);*/

		// 0x28aff0 is almost certainly the global zero VECTOR.
		/*palo->pvtalo->pfnSetAloVelocityVec(palo, (VECTOR*)0x28aff0);
		palo->pvtalo->pfnSetAloAngularVelocityVec(palo, (VECTOR*)0x28aff0);*/

	palo->fFrozen = true;
}

void DupAloRo(ALO* palo, RO* proOrig, RO* proDup)
{
	if (proOrig == nullptr)
	{
		glm::vec3 vecScale = glm::vec3(1.0);
		LoadMatrixFromPosRotScale(palo->xf.posWorld, palo->xf.matWorld, vecScale, proDup->model);
		proDup->uAlpha = 1.0;
		proDup->uAlphaCelBorder = 1.0;
	}
	else
	{
		if (proOrig != proDup)
		{
			proDup->model = proOrig->model;
			proDup->uAlpha = proOrig->uAlpha;
			proDup->uAlphaCelBorder = proOrig->uAlphaCelBorder;
		}
	}
}

void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro)
{
	RPL    rpl{};
	RPLCEL rplCel{};

	DupAloRo(palo, pro, &rpl.ro);

	const glm::mat4 baseModel = rpl.ro.model;
	const float     baseAlpha = rpl.ro.uAlpha;
	const float     baseAlphaCel = rpl.ro.uAlphaCelBorder;

	bool fGlobsetVisible = false;
	const bool doPerGlobMrd = (palo->mrds == 1);

	for (int i = 0; i < palo->globset.cglob; ++i)
	{
		auto& glob = palo->globset.aglob[i];
		auto* pglobi = (palo->globset.aglobi.empty() ? nullptr : &palo->globset.aglobi[i]);

		if (g_fBsp != 0 && pglobi != nullptr && palo->viss == 1)
		{
			if ((pglobi->grfzon & pcm->grfzon) != pcm->grfzon)
				continue;
		}

		glm::vec3 posCenter;

		if (!palo->fFixedPhys)
			posCenter = glm::vec3(baseModel * glm::vec4(glob.posCenter, 1.0f));
		else
			posCenter = glob.posCenter;

		const glm::vec3 dpos3 = posCenter - pcm->pos;
		const glm::vec4 dpos  = glm::vec4(dpos3, 0.0f);

		float uAlphaFromMrd = 1.0f;

		if (doPerGlobMrd)
		{
			if (!SphereInFrustum(pcm->frustum, posCenter, glob.sRadius))
				continue;

			if (!FInsideCmMrd(pcm, dpos, glob.sRadius, glob.sMRD, uAlphaFromMrd))
				continue;
		}
		else
		{
			if (pro == nullptr)
			{
				if (!SphereInFrustum(pcm->frustum, posCenter, glob.sRadius))
					continue;
			}

			uAlphaFromMrd = 1.0f;
		}

		rpl.ro.model = baseModel;
		rpl.ro.uAlpha = baseAlpha * uAlphaFromMrd;
		rpl.ro.uAlphaCelBorder = baseAlphaCel;

		if (glob.pwarpGlob)
		{
			rpl.ro.warpType = glob.pwrbg->warpType;
			rpl.ro.warpCmat = glob.pwarpGlob->pwr->cmat;
			rpl.ro.warpCvtx = glob.pwarpGlob->vertexCount;

			const size_t count = static_cast<size_t>(glob.pwrbg->pwr->cmat);

			switch (rpl.ro.warpType)
			{
				case WARP_POS:
				std::memcpy(rpl.ro.amatDpos, glob.pwrbg->pwr->amatDpos, count * sizeof(*rpl.ro.amatDpos));
				break;

				case WARP_UV:
				std::memcpy(rpl.ro.amatDuv, glob.pwrbg->pwr->amatDuv, count * sizeof(*rpl.ro.amatDuv));
				break;

				case WARP_BOTH:
				std::memcpy(rpl.ro.amatDpos, glob.pwrbg->pwr->amatDpos, count * sizeof(*rpl.ro.amatDpos));
				std::memcpy(rpl.ro.amatDuv,  glob.pwrbg->pwr->amatDuv,  count * sizeof(*rpl.ro.amatDuv));
				break;

				default:
				rpl.ro.warpType = WARP_NONE;
				break;
			}
		}
		else
			rpl.ro.warpType = WARP_NONE;

		if (glob.gleam != nullptr)
		{
			glm::vec3 v = glm::mat3(rpl.ro.model) * glob.gleam->normal;

			const float len2 = glm::dot(v, v);
			glm::vec3 dir = (len2 < 1e-8f) ? glm::vec3(0.0f) : v * glm::inversesqrt(len2);

			const glm::vec3 camBasis = glm::vec3(g_pcm->mat[2]);
			const float intensity = std::abs(glm::dot(dir, camBasis));

			const auto& c = glob.gleam->clqc;
			float gain = c.g0 + intensity * (c.g1 + intensity * (c.g2 + intensity * c.g3));
			gain = GLimitLm(&g_lmZeroOne, gain);

			rpl.ro.uAlpha *= gain;
		}

		if (pglobi != nullptr)
		{
			float target = (g_clock.tReal < pglobi->tUnfade) ? 0.5f : 1.0f;

			if (pglobi->uAlpha != target)
				pglobi->uAlpha = GSmooth(pglobi->uAlpha, target, g_clock.dt, &g_smpAlphaFade, nullptr);

			rpl.ro.uAlpha *= pglobi->uAlpha;
		}

		rpl.ro.uAlpha *= g_uAlpha;

		if (rpl.ro.uAlpha <= 0.0f)
			continue;

		fGlobsetVisible = true;

		rpl.palo = palo;
		rpl.pglob = &glob;
		rpl.rp = glob.rp;

		rpl.ro.uFog = glob.uFog;
		rpl.ro.darken = ((glob.grfglob & 4U) == 0) ? g_psw->rDarken : 1.0f;
		rpl.ro.fDynamic = glob.fDynamic;
		rpl.ro.sRadius = glob.sRadius;
		rpl.ro.posCenter = glm::vec4(posCenter, 1.0f);
		rpl.ro.grfglob = glob.grfglob;

		if (glob.gZOrder != FLT_MAX)
			rpl.z = glob.gZOrder;
		else
			rpl.z = glm::dot(dpos3, dpos3);

		rpl.ro.uAlphaCelBorder *= rpl.ro.uAlpha;

		if (rpl.ro.uAlpha != 1.0f)
		{
			switch (rpl.rp)
			{
				case RP_Opaque:
				case RP_Cutout:
				case RP_OpaqueAfterProjVolume:
				case RP_CutoutAfterProjVolume:
				rpl.rp = RP_Translucent;
				break;

				case RP_CelBorder:
				case RP_CelBorderAfterProjVolume:
				rpl.rp = RP_TranslucentCelBorder;
				break;

				default:
				break;
			}
		}

		int sortT = 0;

		if (rpl.ro.uAlpha < 1.0f)
		{
			if (rpl.rp == RP_Translucent || rpl.rp == RP_TranslucentCelBorder)
				sortT = 1;
		}

		if (!sortT)
		{
			if (rpl.rp == RP_Background ||
				rpl.rp == RP_Cutout ||
				rpl.rp == RP_CutoutAfterProjVolume ||
				rpl.rp == RP_Translucent)
			{
				sortT = glob.fTransluscentSort;
			}
		}

		rpl.fTransluscentSort = sortT;

		if (!allSwDynamicLights.empty() && glob.fThreeWay == 1)
			rpl.ro.fDynamicLight = FindSwDynamicLights(&posCenter, glob.sRadius);
		else
			rpl.ro.fDynamicLight = 0;

		rpl.ro.trlk = glob.trlk;

		if (glob.fDynamic == 1 || glob.pwarpGlob != nullptr)
			rpl.ro.trlk = TRLK_Dynamic;

		const bool bakedThisFrame = (glob.trlk == TRLK_Relight);
		
		if (glob.psaa != nullptr)
		{
			if (glob.psaa && glob.psaa->pvtlooker && glob.psaa->pvtlooker->pfnNotifyLookerRender)
				glob.psaa->pvtlooker->pfnNotifyLookerRender((LOOKER*)glob.psaa, palo, &rpl);
		}

		glm::mat4 submitModel = baseModel;

		if (glob.pdmat != nullptr)
			submitModel = baseModel * (*glob.pdmat);

		if (glob.rtck != RTCK_None)
			AdjustAloRtckMat(palo, pcm, glob.rtck, &posCenter, submitModel);

		rpl.ro.model = submitModel;

		SubmitRpl(&rpl);

		if (glob.trlk == TRLK_Relight && bakedThisFrame)
			glob.trlk = TRLK_Baked;

		if (g_fRenderCelBorders > 0 && glob.csubcel > 0)
		{
			float celAlpha = baseAlphaCel;

			if (glob.sCelBorderMRD < glob.sMRD)
			{
				float dummyCB = 1.0f;

				if (!FInsideCmMrd(pcm, dpos, glob.sRadius, glob.sCelBorderMRD, dummyCB))
					celAlpha = 0.0f;
				else
					celAlpha = baseAlphaCel * uAlphaFromMrd;
			}

			const float cb = celAlpha * rpl.ro.uAlpha;

			if (cb > 0.0f)
			{
				rplCel.rocel.model = submitModel;
				rplCel.rocel.celRgba = ((palo->globset.grfglobset & 2) == 0) ? g_rgbaCel : palo->globset.rgbaCel;

				rplCel.rp = glob.rp;

				if (rpl.ro.uAlpha != 1.0f)
				{
					if (rplCel.rp == RP_CelBorder || rplCel.rp == RP_CelBorderAfterProjVolume)
						rplCel.rp = RP_TranslucentCelBorder;
				}

				rplCel.edgeCount = glob.edgeCount;
				rplCel.edgeSSBO  = glob.edgeSSBO;
				rplCel.rocel.uAlphaCelBorder = cb;

				SubmitRplCel(&rplCel);
			}
		}
	}

	/*
	if (fGlobsetVisible && palo->apactPose != nullptr && palo->globset.cpose > 0)
	{
		if (palo->pvtalo != nullptr && palo->pvtalo->pfnUpdateAloInfluences != nullptr)
			palo->pvtalo->pfnUpdateAloInfluences(palo, pro);

		for (int pose = 0; pose < palo->globset.cpose; ++pose)
		{
			ACT* pact = palo->apactPose[pose];

			if (pact == nullptr)
				palo->globset.agPoses[pose] = palo->globset.agPosesOrig[pose];
			else
				ProjectActPose(pact, pose);
		}
	}
	*/
}

void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha)
{
	glm::vec3 p0 = *ppos0;
	glm::vec3 p1 = *ppos1;

	// Original:
	// dpos   = p1 - p0
	// dposCm = p0 - cameraPos
	const glm::vec3 dir = p1 - p0;
	const glm::vec3 toCam = p0 - pcm->pos;

	//
	// axis1 = normalize(cross(toCam, dir))
	//
	glm::vec3 axis1 = glm::cross(toCam, dir);
	const float axis1Len = glm::length(axis1);

	// Original eventually only renders when this length > 0.01
	if (axis1Len <= 0.01f)
		return;

	axis1 /= axis1Len;

	//
	// axis0 = normalize(cross(axis1, dir))
	// THIS was the mismatch.
	//
	glm::vec3 axis0 = glm::cross(axis1, dir);

	const float axis0Len = glm::length(axis0);

	if (axis0Len < 0.0001f)
		return;

	axis0 /= axis0Len;

	//
	// Original scaling:
	//
	axis1 *= rWidth;
	const glm::vec3 axis2 = dir * 0.01f;

	glm::mat3 rot(1.0f);

	// GLM columns
	rot[0] = axis0;
	rot[1] = axis1;
	rot[2] = axis2;

	glm::mat4 model(1.0f);
	LoadMatrixFromPosRot(p0, rot, model);

	RO ro{};
	ro.model = model;
	ro.uAlpha = uAlpha;
	ro.uAlphaCelBorder = uAlpha;

	palo->pvtalo->pfnRenderAloGlobset(palo, pcm, &ro);
}

void DeleteModel(ALO* palo)
{
	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		GLOB& glob = palo->globset.aglob[i];

		if (glob.VAO != 0)
		{
			glDeleteVertexArrays(1, &glob.VAO);
			glDeleteBuffers(1, &glob.VBO);
			glDeleteBuffers(1, &glob.EBO);
		}

		if (glob.pwarpGlob != nullptr && glob.pwarpGlob->ssboState != 0)
			glDeleteBuffers(1, &glob.pwarpGlob->ssboState);

		if (glob.ssboCachedMaterial != 0)
			glDeleteBuffers(1, &glob.ssboCachedMaterial);

		if (glob.edgeSSBO != 0)
			glDeleteBuffers(1, &glob.edgeSSBO);
	}
}

int GetAloSize()
{
	return sizeof(ALO);
}

void DeleteAlo(ALO* palo)
{
	delete palo;
}

std::vector <ALO*> allSWAloObjs;