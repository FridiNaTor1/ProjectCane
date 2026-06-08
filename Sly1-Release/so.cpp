#include "so.h"
#include "proxy.h"

SO* NewSo()
{
	return new SO{};
}

void InitSwBusySoDl(SW* psw)
{
	InitDl(&psw->dlBusySo, offsetof(SO, dleBusySo));
}

void InitSwRootDl(SW *psw)
{
	InitDl(&psw->dlRoot, offsetof(SO, dleRoot));
}

void InitSo(SO* pso)
{
	InitDl(&pso->dlPhys, offsetof(SO, dlePhys));
	InitAlo(pso);

	InitGeom(&pso->geomLocal);

	allSWSoObjs.push_back(pso);
}

int GetSoSize()
{
	return sizeof(SO);
}

void OnSoAdd(SO *pso)
{
	pso->psw->cpsoAll++;

	if (pso->paloParent == nullptr)
	{
		pso->psw->cpsoRoot++;
		AppendDlEntry(&pso->psw->dlRoot, pso);
	}

	OnAloAdd(pso);
}

void OnSoRemove(SO* pso)
{
	OnAloRemove(pso);
	pso->psw->cpsoAll--;

	if (pso->paloParent == nullptr)
	{
		RemoveDlEntry(&pso->psw->dlRoot, pso);
		pso->psw->cpsoRoot--;
	}

	pso->pstso = nullptr;
}

void CloneSo(SO* pso, SO* psoBase)
{
	pso->dleRoot = psoBase->dleRoot;
	pso->dlPhys = psoBase->dlPhys;
	pso->dlePhys = psoBase->dlePhys;
	pso->momintLocal = psoBase->momintLocal;
	pso->momintInvLocal = psoBase->momintInvLocal;
	pso->dvGravity = psoBase->dvGravity;
	pso->gBuoyancy = psoBase->gBuoyancy;
	pso->gViscosity = psoBase->gViscosity;
	pso->m = psoBase->m;
	pso->posWorldPrev = psoBase->posWorldPrev;
	pso->geomLocal = psoBase->geomLocal;
	pso->geomWorld = psoBase->geomWorld;
	pso->plvo = psoBase->plvo;
	pso->sRadiusSelf = psoBase->sRadiusSelf;
	pso->sRadiusAll = psoBase->sRadiusAll;
	pso->sRadiusPrune = psoBase->sRadiusPrune;
	pso->posPrune = psoBase->posPrune;
	pso->bspc = psoBase->bspc;
	pso->cnpg = psoBase->cnpg;
	pso->anpg = psoBase->anpg;
	pso->mpibspinpg = psoBase->mpibspinpg;
	pso->chsg = psoBase->chsg;
	pso->ahsg = psoBase->ahsg;
	pso->mpisurfihsgMic = psoBase->mpisurfihsgMic;
	pso->dleBusySo = psoBase->dleBusySo;
	pso->posMin = psoBase->posMin;
	pso->posMax = psoBase->posMax;
	pso->constrForce = psoBase->constrForce;
	pso->constrTorque = psoBase->constrTorque;
	pso->poxa = psoBase->poxa;
	pso->dpos = psoBase->dpos;
	pso->drot = psoBase->drot;
	pso->pxa = psoBase->pxa;
	pso->pxpInternal = psoBase->pxpInternal;
	pso->grfpvaXpValid = psoBase->grfpvaXpValid;
	pso->ipsoRoot = psoBase->ipsoRoot;
	pso->ipso = psoBase->ipso;
	pso->posComLocal = psoBase->posComLocal;
	pso->psoPhysHook = psoBase->psoPhysHook;
	pso->bspcCamera = psoBase->bspcCamera;
	pso->cmk = psoBase->cmk;
	pso->egk = psoBase->egk;
	pso->fSphere = psoBase->fSphere;
	pso->fClone = psoBase->fClone;
	pso->fNoXpsAll = psoBase->fNoXpsAll;
	pso->fNoXpsSelf = psoBase->fNoXpsSelf;
	pso->fNoXpsCenter = psoBase->fNoXpsCenter;
	pso->fActive = psoBase->fActive;
	pso->fVelcro = psoBase->fVelcro;
	pso->fIgnoreLocked = psoBase->fIgnoreLocked;
	pso->fIceable = psoBase->fIceable;
	pso->fRoot = psoBase->fRoot;
	pso->fPhys = psoBase->fPhys;
	pso->fNoGravity = psoBase->fNoGravity;
	pso->fCenterXp = psoBase->fCenterXp;
	pso->fLockedSelf = psoBase->fLockedSelf;
	pso->fLockedAll = psoBase->fLockedAll;
	pso->fLockedAbove = psoBase->fLockedAbove;
	pso->fCpsoBuildContactGroup = psoBase->fCpsoBuildContactGroup;
	pso->fCpxpBuildArray = psoBase->fCpxpBuildArray;
	pso->fUpdateXaList1 = psoBase->fUpdateXaList1;
	pso->fUpdateXaList2 = psoBase->fUpdateXaList2;
	pso->fRecalcSwXpAll = psoBase->fRecalcSwXpAll;
	pso->fHandleDiveEffect = psoBase->fHandleDiveEffect;
	pso->fGenSpliceTouchEvents = psoBase->fGenSpliceTouchEvents;
	pso->pstso = psoBase->pstso;

	if ((pso->geomWorld).cpos != 0) {
		CloneGeom(&psoBase->geomWorld, nullptr, &pso->geomWorld);
	}

	if ((pso->geomCameraWorld).cpos != 0) {
		CloneGeom(&psoBase->geomCameraWorld, nullptr, &pso->geomCameraWorld);
	}

	CloneAlo(pso, psoBase);
}

void SetSoParent(SO* pso, ALO* paloParent)
{
	if (pso->paloParent != paloParent)
	{
		SetAloParent(pso, paloParent);
	}
}

void ApplySoProxy(SO* pso, PROXY* pproxyApply)
{
	ApplyAloProxy(pso, pproxyApply);

	/*glm::mat3 &mat = pproxyApply->xf.mat;

	glm::vec3 normalForce  = mat * pso->constrForce.normal;
	glm::vec3 normalTorque = mat * pso->constrTorque.normal;

	SetSoConstraints(pso, pso->constrForce.ct, &normalForce, pso->constrTorque.ct, &normalTorque);*/
}

void UpdateSoXfWorldHierarchy(SO* pso)
{
	UpdateAloXfWorldHierarchy(pso);

	glm::vec3 pos = pso->xf.posWorld;
	glm::mat3 mat = pso->xf.matWorld;

	UpdateGeomWorld(&pso->geomLocal, &pso->geomWorld, pos, mat);
	UpdateGeomWorld(&pso->geomCameraLocal, &pso->geomCameraWorld, pos, mat);
}

void UpdateSoXfWorld(SO* pso)
{
	UpdateAloXfWorld(pso);
}

void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis)
{
	pso->fFixedPhys = pbis->U8Read() != 0;
	ReadGeom(&pso->geomLocal, pbis);

	pso->sRadiusSelf = pso->geomLocal.sRadius;
	pso->sRadiusAll  = pso->geomLocal.sRadius;

	if (!pso->fFixedPhys)
		CloneGeom(&pso->geomLocal, nullptr, &pso->geomWorld);
	else
		pso->geomWorld = pso->geomLocal;

	ReadBspc(&pso->geomWorld, &pso->bspc, pbis);

	pso->m = pbis->F32Read();

	pso->momintLocal =  pbis->ReadMatrix();
	pso->posComLocal = pbis->ReadVector();

	pso->cnpg = pbis->U16Read();
	pso->anpg.resize(pso->cnpg);

	for (int i = 0; i < pso->cnpg; i++)
	{
		pso->anpg[i].cmk = pbis->S16Read();
		pso->anpg[i].ipglob = pbis->U16Read();
	}

	pso->mpibspinpg.resize(pbis->U32Read());

	for (int i = 0; i < pso->mpibspinpg.size(); i++)
		pso->mpibspinpg[i] = pbis->S16Read();

	pso->chsg = pbis->U32Read();
	pso->ahsg.resize(pso->chsg);

	for (int i = 0; i < pso->chsg; i++)
	{
		pso->ahsg[i].ipglob = pbis->S16Read();
		pso->ahsg[i].ipsubglob = pbis->S16Read();
	}

	ReadGeom(&pso->geomCameraLocal, pbis);

	if (!pso->fFixedPhys)
		CloneGeom(&pso->geomCameraLocal, nullptr, &pso->geomCameraWorld);
	else
		pso->geomCameraWorld = pso->geomCameraLocal;

	ReadBspc(&pso->geomCameraWorld, &pso->bspcCamera, pbis);

	LoadAloFromBrx(pso, pbis);
}

void TranslateSoToPos(SO* pso, glm::vec3& ppos)
{
	pso->xf.pos = ppos;

	if (pso->paloRoot != nullptr)
		pso->pvtalo->pfnUpdateAloXfWorld(pso);
}

void RotateSoToMat(SO* pso, glm::mat3& pmat)
{
	pso->xf.mat = pmat;

	if (pso->paloRoot != nullptr)
		pso->pvtalo->pfnUpdateAloXfWorld(pso);
}

void UpdateSo(SO* pso, float dt)
{
	UpdateAlo(pso, dt);
}

void FreezeSo(SO* pso, int fFreeze)
{
	if (fFreeze == 0)
	{
		FreezeAlo(pso, 0);

		//SetSoConstraints(pso, pso->frz.ctForce, nullptr, pso->frz.ctTorque, nullptr);
	}
	else
	{
		// Save current constraint modes.
		pso->frz.ctForce  = pso->constrForce.ct;
		pso->frz.ctTorque = pso->constrTorque.ct;

		//// Lock movement/rotation while frozen.
		//SetSoConstraints(pso, CT_Locked, nullptr, CT_Locked, nullptr);

		//// Clear accumulated velocity deltas.
		pso->xf.dw = glm::vec3{};
		pso->xf.dv = glm::vec3{};

		//// Remove active STSO state while frozen.
		//FreeSwStsoList(pso->psw, pso->pstso);
		pso->pstso = nullptr;

		FreezeAlo(pso, 1);
	}
}

void RenderSoSelf(SO* pso, CM* pcm, RO* pro)
{
	RenderAloSelf(pso, pcm, pro);
}

void DeleteSo(SO *pso)
{
	delete pso;
}

void DeleteSwCollision()
{
	for (int i = 0; i < allSWSoObjs.size(); i++)
	{
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomLocal.VAO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomLocal.VBO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomLocal.EBO);

		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomCameraLocal.VAO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomCameraLocal.VBO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomCameraLocal.EBO);
	}
}

void DeleteSoGeom(SO* pso)
{
	glDeleteVertexArrays(1, &pso->geomLocal.VAO);
	glDeleteVertexArrays(1, &pso->geomLocal.VBO);
	glDeleteVertexArrays(1, &pso->geomLocal.EBO);

	glDeleteVertexArrays(1, &pso->geomCameraLocal.VAO);
	glDeleteVertexArrays(1, &pso->geomCameraLocal.VBO);
	glDeleteVertexArrays(1, &pso->geomCameraLocal.EBO);
}

void DeallocateSoVector()
{
	allSWSoObjs.clear();
	allSWSoObjs.shrink_to_fit();
}

std::vector <SO*> allSWSoObjs;