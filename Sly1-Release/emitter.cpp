#include "emitter.h"
#include "blip.h"
#include "shape.h"

EXPLO* NewExplo()
{
	return new EXPLO{};
}

void InitExplo(EXPLO* pexplo)
{
	InitXfm(pexplo);
	pexplo->oidShape = OID_Nil;
	pexplo->oidReference = OID_Nil;
}

int GetExploSize()
{
	return sizeof(EXPLO);
}

void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis)
{
	EMITB emitb{};

	pexplo->pemitb = std::make_shared<EMITB>();

	LoadXfmFromBrx(pexplo, pbis);
	int8_t crvk = pbis->S8Read();

	if (crvk != -1)
	{
		std::shared_ptr <CRV> pcrv = PcrvNew((CRVK)crvk);
		pcrv->pvtcrvl->pfnLoadCrvlFromBrx((CRVL*)pcrv.get(), pbis);
	}

	if (pexplo->pemitb->emito.emitok == EMITOK_Mesh)
		LoadEmitMeshFromBrx(&pexplo->pemitb->emito.emitmeshOrigin, pbis);

	uint16_t crgba = pbis->U16Read();

	if (crgba != 0)
	{
		LoadEmitblipColorsFromBrx(nullptr, crgba, pbis);
	}
}

void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase)
{
	CloneExpl(pexplo, pexploBase);

	pexplo->pemitb = pexploBase->pemitb;
	pexplo->oidReference = pexploBase->oidReference;
	pexplo->oidShape = pexploBase->oidShape;
}

EMITOK* PemitbEnsureExploEmitok(EXPLO* pexplo, ENSK ensk)
{
	return &pexplo->pemitb.get()->emito.emitok;;
}

void BindExplo(EXPLO* pexplo)
{

}

void DeleteExplo(EXPLO* pexplo)
{
	delete pexplo;
}

EMITTER* NewEmitter()
{
	return new EMITTER{};
}

void InitEmitter(EMITTER* pemitter)
{
	InitAlo(pemitter);
	pemitter->lmSvcParticle.gMin = 10.0;
	pemitter->tUnpause = -1.0;
	pemitter->oidGroup = OID_Nil;
	pemitter->cParticle = -1;
	pemitter->lmSvcParticle.gMax = 10.0;
	pemitter->oidReference = OID_Nil;
	pemitter->oidRender = OID_Nil;
	pemitter->oidNextRender = OID_Nil;
	pemitter->oidTouch = OID_Nil;
	pemitter->oidShape = OID_Nil;
	pemitter->emitrk = EMITRK_Nil;
	InitDl(&pemitter->dlGroup, offsetof(EMITTER, dleGroup));
}

int GetEmitterSize()
{
	return sizeof(EMITTER);
}

void LoadEmitMeshFromBrx(EMITMESH* pemitmesh, CBinaryInputStream* pbis)
{
	uint16_t cpos = pbis->U16Read();

	for (int i = 0; i < cpos; i++)
		pbis->ReadVector();

	uint16_t cemittri = pbis->U16Read();

	for (int i = 0; i < cemittri; i++)
	{
		for (int i = 0; i <= 2; i++)
			pbis->U16Read();

		pbis->F32Read();
	}

	pbis->ReadVector();
}

void LoadEmitblipColorsFromBrx(EMITBLIP *pemitblip, int crgba, CBinaryInputStream* pbis)
{
	if (pemitblip != nullptr)
	{
		const int storeCount = std::min<int>(crgba, 0x20);

		pemitblip->crgba = storeCount;
		pemitblip->argba.resize(storeCount);

		pemitblip->fColorRanges = static_cast<int>(static_cast<int8_t>(pbis->U8Read()));

		constexpr float inv255 = 1.0f / 255.0f;

		for (int i = 0; i < crgba; ++i)
		{
			const u32 packed = pbis->U32Read();
			
			if (i < storeCount)
			{
				const float r = ((packed >> 0)  & 0xFF) * inv255;
				const float g = ((packed >> 8)  & 0xFF) * inv255;
				const float b = ((packed >> 16) & 0xFF) * inv255;
				const float a = ((packed >> 24) & 0xFF) * inv255;

				pemitblip->argba[i] = glm::vec4(r, g, b, a);
			}
		}
	}
	else
	{
		byte colorRanges = pbis->U8Read();

		for (int i = 0; i < crgba; i++)
			pbis->U32Read();
	}
}

void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis)
{
	EMITB emitb{};
	pemitter->pemitb = std::make_shared <EMITB>(emitb);

	InitEmitb(pemitter->pemitb.get());
	LoadAloFromBrx(pemitter, pbis);

	int8_t crvk = pbis->S8Read();

	if (crvk != -1)
	{
		std::shared_ptr <CRV> pcrv = PcrvNew((CRVK)crvk);
		pemitter->pemitb->emito.emitcrvOrigin.pcrv = pcrv.get();
		pcrv->pvtcrvl->pfnLoadCrvlFromBrx((CRVL*)pcrv.get(), pbis);
	}

	if (pemitter->pemitb->emito.emitok == EMITOK_Mesh)
		LoadEmitMeshFromBrx(&pemitter->pemitb->emito.emitmeshOrigin, pbis);

	uint16_t crgba = pbis->U16Read();

	if (crgba != 0)
	{
		EMITB *pemitb = PemitbEnsureEmitter(pemitter, ENSK_Set);
		LoadEmitblipColorsFromBrx(&pemitb->emitp.emitblip, crgba, pbis);
	}
}

void CloneEmitter(EMITTER* pemitter, EMITTER* pemitterBase)
{
	CloneAlo(pemitter, pemitterBase);

	pemitter->pemitb = pemitterBase->pemitb;
	pemitter->emitrk = pemitterBase->emitrk;
	pemitter->cParticle = pemitterBase->cParticle;
	pemitter->lmSvcParticle = pemitterBase->lmSvcParticle;
	pemitter->fCountIsDensity = pemitterBase->fCountIsDensity;
	pemitter->uPauseProb = pemitterBase->uPauseProb;
	pemitter->lmDtPause = pemitterBase->lmDtPause;
	pemitter->cParticleConstant = pemitterBase->cParticleConstant;
	pemitter->oidReference = pemitterBase->oidReference;
	pemitter->oidRender = pemitterBase->oidRender;
	pemitter->oidTouch = pemitterBase->oidTouch;
	pemitter->oidNextRender = pemitterBase->oidNextRender;
	pemitter->fAutoPause = pemitterBase->fAutoPause;
	pemitter->oidShape = pemitterBase->oidShape;
	pemitter->oidGroup = pemitterBase->oidGroup;
	pemitter->dlGroup = pemitterBase->dlGroup;
	pemitter->dleGroup = pemitterBase->dleGroup;
	pemitter->svcParticle = pemitterBase->svcParticle;
	pemitter->dtRecalcSvc = pemitterBase->dtRecalcSvc;
	pemitter->tRecalcSvc = pemitterBase->tRecalcSvc;
	pemitter->rDensity = pemitterBase->rDensity;
	pemitter->sBoxRadius = pemitterBase->sBoxRadius;
	pemitter->uParticle = pemitterBase->uParticle;
	pemitter->tUnpause = pemitterBase->tUnpause;
	pemitter->pripg = pemitterBase->pripg;
	pemitter->pblipg = pemitterBase->pblipg;
	pemitter->fValuesChanged = pemitterBase->fValuesChanged;
}

void UnpauseEmitter(EMITTER* pemitter)
{
	pemitter->tUnpause = -1.0;
}

int FPausedEmitter(EMITTER* emitter)
{
	if (emitter->tUnpause > g_clock.t)
		return 1;

	const int isActiveNonBurst = (EMITRK_Nil < emitter->emitrk) && (emitter->emitrk < EMITRK_Burst);

	return (isActiveNonBurst && emitter->cParticle == 0) ? 1 : 0;
}

EMITTER* PemitterEnsureEmitter(EMITTER* pemitter, ENSK ensk)
{
	if (ensk == ENSK_Set) {
		pemitter->fValuesChanged = 1;
	}

	return pemitter;
}

EMITB* PemitbEnsureEmitter(EMITTER* pemitter, ENSK ensk)
{
	/*if (ensk == ENSK_Set) 
	{
		pemitter->fValuesChanged = 1;
		pemitter->pemitb = PemitbCopyOnWrite(pemitter->pemitb);
		pemitter->pemitb->pchzName = PchzFromLo(pemitter);
	}*/

	return pemitter->pemitb.get();
}

EMITOK* PemitbEnsureEmitterEmitok(EMITTER* pemitter, ENSK ensk)
{

	return &pemitter->pemitb.get()->emito.emitok;
}

glm::vec3* PemitbEnsureEmitterEmitokVec(EMITTER* pemitter, ENSK ensk)
{
	return &pemitter->pemitb.get()->emito.posOrigin;
}

EMITRK *PemitbEnsureEmitterEmitrk(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->emitrk;
}

LM* PemitbEnsureEmitterlmSvcParticle(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->lmSvcParticle;
}

float* PemitbEnsureEmittercParticleConstant(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->cParticleConstant;
}

float* PemitEnsureEmitteruPauseProb(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->uPauseProb;
}

LM* PemitbEnsureEmitterlmDtPause(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->lmDtPause;
}

void GetEmitterEnabled(EMITTER* pemitter, int* pfEnabled)
{
	bool isPaused = FPausedEmitter(pemitter);
	*pfEnabled = !isPaused;
}

void SetEmitterEnabled(EMITTER* pemitter, int fEnabled)
{
	if (fEnabled == 0) {
		PauseEmitterIndefinite(pemitter);
	}
	else {
		UnpauseEmitter(pemitter);
	}
}

int* GetEmitterfCountIsDensity(EMITTER* pemitter)
{
	return &pemitter->fCountIsDensity;
}

void SetEmitterfCountIsDensity(EMITTER* pemitter, bool fCountDensity)
{
	pemitter->fCountIsDensity = fCountDensity;
}

void SetEmitterOidReference(EMITTER* pemitter, OID oidReference)
{
	pemitter->oidReference = oidReference;
}

OID* GetEmitterOidReference(EMITTER* pemitter)
{
	return &pemitter->oidReference;
}

void* GetEmitterOidRender(EMITTER* pemitter)
{
	return &pemitter->oidRender;
}

void SetEmitterOidRender(EMITTER* pemitter, OID oidRender)
{
	pemitter->oidRender = oidRender;
}

void* GetEmitterOidTouch(EMITTER* pemitter)
{
	return &pemitter->oidTouch;
}

void SetEmitterOidTouch(EMITTER* pemitter, OID oidTouch)
{
	pemitter->oidTouch = oidTouch;
}

void* GetEmitterOidNextRender(EMITTER* pemitter)
{
	return &pemitter->oidNextRender;
}

void SetEmitterOidNextRender(EMITTER* pemitter, OID oidNextRender)
{
	pemitter->oidNextRender = oidNextRender;
}

void* GetEmitterOidGroup(EMITTER* pemitter)
{
	return &pemitter->oidGroup;
}

void SetEmitterOidGroup(EMITTER* pemitter, OID oidGroup)
{
	pemitter->oidGroup = oidGroup;
}

void PauseEmitter(EMITTER* pemitter, float dtPause)
{
	pemitter->tUnpause = g_clock.t + dtPause;
}

void GetEmitterPaused(EMITTER* pemitter, int* pfPaused)
{
	*pfPaused = FPausedEmitter(pemitter);
}

void* GetEmitterOidShape(EMITTER* pemitter)
{
	return &pemitter->oidShape;
}

void SetEmitterOidShape(EMITTER* pemitter, OID oidShape)
{
	pemitter->oidShape = oidShape;
}

EMITNK* PemitbEnsureEmitterEmitnk(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->pemitb->emito.emitnk;
}

glm::vec3* PemitbEnsureEmitterEmitoVec(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->pemitb->emito.vec;
}

LM* PemitbEnsureEmitterlmSOffset(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->pemitb->emito.lmSOffset;
}

void SetEmitterParticleCount(EMITTER* pemitter, int cParticle)
{
	if (pemitter->emitrk == EMITRK_Nil) {
		pemitter->cParticle = cParticle;
	}
	else if (pemitter->emitrk == EMITRK_Continuous) {
		if (cParticle == 0) {
			PauseEmitterIndefinite(pemitter);
		}
		else if (cParticle == -1) {
			UnpauseEmitter(pemitter);
		}
		else {
			pemitter->cParticle = cParticle;
		}
	}
	pemitter->fValuesChanged = 1;
}

void SetEmitterAutoPause(EMITTER* pemitter, int fAutoPause)
{
	pemitter->fAutoPause = fAutoPause;
	pemitter->fValuesChanged = 1;
}

void PauseEmitterIndefinite(EMITTER* pemitter)
{
	pemitter->tUnpause = 3.402823e+38;
}

void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro)
{

}

void BindEmitter(EMITTER* pemitter)
{
	BindAlo(pemitter);

	const bool needsSpecialBinding =
		pemitter->oidReference != OID_Nil ||
		pemitter->oidRender != OID_Nil ||
		pemitter->oidNextRender != OID_Nil ||
		pemitter->oidShape != OID_Nil ||
		pemitter->oidTouch != OID_Nil ||
		pemitter->pemitb->emito.emitok == EMITOK_Skeleton ||
		(pemitter->pemitb->emitp.emitpk == EMITPK_Blip &&
			pemitter->pemitb->emitp.emitblip.oidSplineTarget != OID_Nil);

	EMITB* pemitb = pemitter->pemitb.get();

	if (needsSpecialBinding)
	{
		pemitb = PemitbEnsureEmitter(pemitter, ENSK_Set);

		if (pemitter->oidReference != OID_Nil)
		{
			auto* paloReference = reinterpret_cast<ALO*>(PloFindSwObject(pemitter->psw, 0x104, pemitter->oidReference, reinterpret_cast<LO*>(pemitter)));

			pemitb->emito.paloReference = paloReference;

			if (paloReference != nullptr)
			{
				//PostSwCallback(pemitter->psw, BindEmitterCallback, pemitter, MSGID_callback, nullptr);
			}
		}

		if (pemitter->oidRender != OID_Nil)
			pemitb->emitp.emitrip.paloRender = reinterpret_cast<ALO*>(PloFindSwObject(pemitter->psw, 0x104, pemitter->oidRender, reinterpret_cast<LO*>(pemitter)));

		if (pemitter->oidNextRender != OID_Nil)
			pemitb->emitp.emitrip.paloNextRender = reinterpret_cast<ALO*>(PloFindSwObject(pemitter->psw, 0x104, pemitter->oidNextRender, reinterpret_cast<LO*>(pemitter)));

		if (pemitter->oidTouch != OID_Nil)
		{
			auto* psoTouch = reinterpret_cast<SO*>(PloFindSwObject(pemitter->psw, 0x104, pemitter->oidTouch, reinterpret_cast<LO*>(pemitter)));

			if (FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(psoTouch), CID_SO) != 0)
				pemitb->emitp.emitrip.psoTouch = psoTouch;
		}

		if (pemitter->oidShape != OID_Nil)
		{
			SHAPE* pshape = (SHAPE*)PloFindSwObject(pemitter->psw, 0x104, pemitter->oidShape, pemitter);

			if (pshape != nullptr)
			{
				// Original code copies the VECTOR at pshape[1].field0_0x0
				pemitb->emito.emitcrvOrigin.pcrv = pshape->pcrv.get();
				pemitb->emito.paloReference = pshape->paloParent;
			}
		}
	}

	BindEmitb(pemitb, pemitter);
}

void InitEmitb(EMITB* pemitb)
{
	pemitb->emito.emitok = EMITOK_Box;
	pemitb->emito.lmSOffset.gMin = 1.0;
	pemitb->emitv.rvDamping = 750;
	pemitb->emitv.swCurl = 750;
	pemitb->emitv.dv.x = 750;
	pemitb->emitv.dv.y = 750;
	pemitb->emitv.dv.z = 1.0;
	pemitb->emitv.normalCurl.z = -1500;

	pemitb->emitp.emitrip.ript = RIPT_Nil;
	pemitb->emitp.emitrip.riptTrail = RIPT_Nil;
}

void BindEmitb(EMITB* pemitb, LO* ploContext)
{
	if (pemitb->emitp.emitpk == EMITPK_Blip)
	{
		OID oidSplineTarget = pemitb->emitp.emitblip.oidSplineTarget;

		if (oidSplineTarget != OID_Nil)
		{
			pemitb->emitp.emitblip.pexploSplineTarget = reinterpret_cast<EXPLO*>(PloFindSwObject(ploContext->psw, 0x104, oidSplineTarget, ploContext));
		}
	}

	if (pemitb->emito.emitok != EMITOK_Skeleton)
		return;

	auto& skel = pemitb->emito.skelOrigin;
	float totalWeight = 0.0f;

	for (int i = 0; i < skel.cskelp; ++i)
	{
		SKELP& skelp = skel.askelp[i];

		for (int j = 0; j < 2; ++j)
		{
			skelp.apalo[j] = reinterpret_cast<ALO*>(PloFindSwObject(ploContext->psw, 0x104, skelp.aoid[j], ploContext));
		}

		if (skelp.apalo[0] == nullptr || skelp.apalo[1] == nullptr)
		{
			skelp.s = 0.0f;
			skelp.gWeight = 0.0f;
			continue;
		}

		const glm::vec3 p0(
			skelp.apalo[0]->xf.posWorld.x,
			skelp.apalo[0]->xf.posWorld.y,
			skelp.apalo[0]->xf.posWorld.z
		);

		const glm::vec3 p1(
			skelp.apalo[1]->xf.posWorld.x,
			skelp.apalo[1]->xf.posWorld.y,
			skelp.apalo[1]->xf.posWorld.z
		);

		const float segmentLength = glm::length(p0 - p1);
		skelp.s = segmentLength;

		const float avgDensity = 0.5f * (skelp.agDensity[0] + skelp.agDensity[1]);
		skelp.gWeight = avgDensity * segmentLength;

		totalWeight += skelp.gWeight;
	}

	pemitb->emito.posOrigin.z = totalWeight;
}

void SetBlipgEmitb(BLIPG* pblipg, EMITB* pemitb)
{
	if (pblipg == nullptr || pemitb == nullptr)
		return;

	EMITBLIP& emitblip = pemitb->emitp.emitblip;
	EMITV& emitv = pemitb->emitv;

	pblipg->pchzName = pemitb->pchzName;
	pblipg->blipok = emitblip.blipok;
	pblipg->rSFlying = emitblip.rSFlying;
	pblipg->crgba = emitblip.crgba;

	for (int i = 0; i < pblipg->crgba; ++i)
	{
		for (int i = 0; i < pblipg->crgba; ++i)
		{
			pblipg->argba[i] = (emitblip.argba[i] + glm::vec4(1.0f)) * 0.5f;
		}
	}

	pblipg->fColorRanges = emitblip.fColorRanges;

	SetBlipgShader(pblipg, emitblip.oidShader);

	std::memset(&pblipg->clqTexture, 0, sizeof(pblipg->clqTexture));

	if (pblipg->cqwTexture > 1)
	{
		if (emitblip.dtShaderLoop == 0.0f)
		{
			if (emitblip.fShaderSpan == 0)
			{
				pblipg->clqTexture = emitblip.clqTexture;
			}
			else
			{
				pblipg->clqTexture.g1 = static_cast<float>(pblipg->cqwTexture) / emitv.dtLifetime;
			}
		}
		else
		{
			pblipg->clqTexture.g1 = static_cast<float>(pblipg->cqwTexture) / emitblip.dtShaderLoop;
		}
	}

	PrescaleClq(&emitv.clqAlpha, 1.0f / emitv.dtLifetime, 0.0f, &pblipg->clqAlpha);
	PrescaleClq(&emitblip.clqScale, 1.0f / emitv.dtLifetime, 0.0f, &pblipg->clqScale);
	PrescaleClq(&emitblip.clqColor, 1.0f / emitv.dtLifetime, 0.0f, &pblipg->clqColor);

	pblipg->blipmk = emitblip.blipmk;

	if (pblipg->blipmk == BLIPMK_Accel)
	{
		const glm::vec3& dv = emitv.dv;
		const bool zeroDv =
			std::abs(dv.x) < 0.0001f &&
			std::abs(dv.y) < 0.0001f &&
			std::abs(dv.z) < 0.0001f;

		if (zeroDv && emitv.rvDamping == 0.0f && emitv.swCurl == 0.0f)
		{
			pblipg->blipmk = BLIPMK_Constant;
		}
	}

	std::memset(&pblipg->blipgm, 0, sizeof(pblipg->blipgm));

	if (pblipg->blipmk == BLIPMK_Accel)
	{
		SetEmitdvEmitb(reinterpret_cast<EMITDV*>(&pblipg->blipgm), pemitb);
	}
}

void SetRipgEmitb(RIPG* pripg, EMITB* pemitb)
{

}

void SetEmitdvEmitb(EMITDV* pemitdv, EMITB* pemitb)
{
	if (pemitdv == nullptr || pemitb == nullptr)
		return;

	auto& emitv = pemitb->emitv;
	auto& emito = pemitb->emito;

	pemitdv->rvDamping = emitv.rvDamping;
	pemitdv->dv = emitv.dv;
	pemitdv->swCurl = emitv.swCurl;

	const float len = glm::length(emitv.normalCurl);
	pemitdv->normalCurl = (len < 0.0001f)
		? g_normalZ
		: (emitv.normalCurl / len);

	pemitdv->emitcnk = emitv.emitcnk;
	pemitdv->paloCurlRef = emito.paloReference;
}

void PostEmitterLoad(EMITTER* pemitter)
{
	PostAloLoad(pemitter);

	if (pemitter->pemitb->emitp.emitrip.ript != RIPT_Nil)
		pemitter->pemitb->emitp.emitpk = EMITPK_Rip;

	if (pemitter->emitrk == EMITRK_Nil)
	{
		if (pemitter->uPauseProb == 0.0f && pemitter->fAutoPause == 0)
			pemitter->emitrk = (pemitter->cParticleConstant == 0.0f) ? EMITRK_Continuous : EMITRK_ConstantCount;
		else
			pemitter->emitrk = EMITRK_BurstOld;
	}

	switch (pemitter->emitrk)
	{
	case EMITRK_Burst:
	{
		pemitter->tRecalcSvc = FLT_MAX;
		pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);

		if (pemitter->fAutoPause != 0)
		{
			PauseEmitterIndefinite(pemitter);
		}
		else if (GRandInRange(0.0f, 1.0f) <= pemitter->uPauseProb)
		{
			const float dtPause = GRandInRange(pemitter->lmDtPause.gMin, pemitter->lmDtPause.gMax);
			PauseEmitter(pemitter, dtPause);
		}
		break;
	}

	case EMITRK_Continuous:
	case EMITRK_ConstantCount:
	{
		pemitter->dtRecalcSvc = 1.0f;
		pemitter->tRecalcSvc  = 1.0f;
		pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);
		break;
	}

	case EMITRK_BurstOld:
	{
		pemitter->tRecalcSvc = FLT_MAX;
		pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax) * 60.0f;

		if (pemitter->fAutoPause != 0)
		{
			PauseEmitterIndefinite(pemitter);
		}
		else if (GRandInRange(0.0f, 1.0f) <= pemitter->uPauseProb)
		{
			const float dtPause = GRandInRange(pemitter->lmDtPause.gMin, pemitter->lmDtPause.gMax);
			PauseEmitter(pemitter, dtPause);
		}
		break;
	}

	default:
		break;
	}

	pemitter->rDensity = 1.0f;

	EMITB* pemitb = pemitter->pemitb.get();

	if (pemitb->emito.emitok == EMITOK_Box)
	{
		const BOX& box = pemitb->emito.boxOrigin;

		const glm::vec3 bmin(box.posMin.x, box.posMin.y, box.posMin.z);
		const glm::vec3 bmax(box.posMax.x, box.posMax.y, box.posMax.z);
		const glm::vec3 size = glm::abs(bmax - bmin);

		if (pemitter->oidReference != OID_Nil)
			pemitter->sBoxRadius = 0.5f * glm::compMax(size);

		if (pemitter->fCountIsDensity != 0)
		{
			if (pemitter->cParticleConstant == 0.0f)
				pemitter->rDensity = size.y * size.z * 0.0001f;
			else
				pemitter->rDensity = size.x * size.y * size.z * 1.0e-6f;
		}
	}

	if (pemitter->oidGroup == OID_Nil)
		return;

	EMITTER* pGroupEmitter = reinterpret_cast<EMITTER*>(PloFindSwChild(pemitter->psw, pemitter->oidGroup, nullptr));

	if (pGroupEmitter == nullptr)
	{
		pGroupEmitter = reinterpret_cast<EMITTER*>(PloNew(CID_EMITTER, pemitter->psw, nullptr, pemitter->oidGroup, -1));

		pGroupEmitter->pemitb = std::make_shared<EMITB>();
		*pGroupEmitter->pemitb = *pemitter->pemitb;
		pGroupEmitter->pemitb->emito.paloReference = nullptr;

		pGroupEmitter->emitrk = pemitter->emitrk;
		pGroupEmitter->cParticle = pemitter->cParticle;
		pGroupEmitter->lmSvcParticle = pemitter->lmSvcParticle;
		pGroupEmitter->fCountIsDensity = pemitter->fCountIsDensity;
		pGroupEmitter->uPauseProb = pemitter->uPauseProb;
		pGroupEmitter->lmDtPause = pemitter->lmDtPause;
		pGroupEmitter->cParticleConstant = pemitter->cParticleConstant;
		pGroupEmitter->fAutoPause = pemitter->fAutoPause;

		pGroupEmitter->fFixedPhys = 1;
		pGroupEmitter->fNoFreeze = 1;

		pGroupEmitter->xf.matWorld = pemitter->xf.mat;
		pGroupEmitter->xf.mat = pGroupEmitter->xf.matWorld;

		if (VTEMITTER* pvtemitter = pGroupEmitter->pvtemitter)
		{
			if (pvtemitter->pfnBindEmitter != nullptr)
			{
				pvtemitter->pfnBindEmitter(pGroupEmitter);
			}
		}
	}

	pemitter->fValuesChanged = 0;
	AppendDlEntry(&pGroupEmitter->dlGroup, pemitter);
}

void OnEmitterValuesChanged(EMITTER* pemitter)
{
	if (pemitter == nullptr)
		return;

	pemitter->fValuesChanged = 0;

	if (pemitter->pblipg != nullptr)
		SetBlipgEmitb(pemitter->pblipg, pemitter->pemitb.get());

	if (pemitter->pripg != nullptr)
		SetRipgEmitb(pemitter->pripg, pemitter->pemitb.get());

	float svc = GRandInRange(
		pemitter->lmSvcParticle.gMin,
		pemitter->lmSvcParticle.gMax);

	if (pemitter->emitrk == EMITRK_BurstOld)
		svc *= 60.0f;

	pemitter->svcParticle = svc;
}

void EmitParticles(int cParticle, EMITB* pemitb, EMITG* pemitg)
{
	if (cParticle == 0)
		return;

	EMITGEN emitgen{};
	EMITGEN emitgenTarget{};

	emitgen.fConvertPosVec = 1;
	OriginateParticles(cParticle, pemitb, &emitgen);
	
	if (pemitb->emitp.emitpk == EMITPK_Rip)
	{
		//EmitRips(pemitb, pemitg, cParticle, emitgen.apos, emitgen.av, emitgen.atCreated, emitgen.atDestroy);
	}
	else 
	{
		if (pemitb->emitp.emitblip.blipmk == BLIPMK_Spline)
		{
			//OriginateSplineSinkParticles(cParticle, pemitb, &emitgen, &emitgenTarget);
		}

		EmitBlips(pemitb, pemitg, cParticle, emitgen.apos.data(), emitgen.av.data(), emitgen.atCreated.data(), emitgen.atDestroy.data(), emitgenTarget.apos.data(), emitgenTarget.av.data());
	}

}

void ModifyEmitterParticles(EMITTER* pemitter)
{
	if (pemitter == nullptr)
		return;

	/*const float radius = pemitter->sBoxRadius;
	if (radius <= 0.0001f)
		return;

	BOX boxWorld{};
	const glm::vec3 halfExtent(radius);
	const glm::vec3 fullExtent(radius * 2.0f);

	ALO* reference = pemitter->pemitb->emito.paloReference;
	if (reference == nullptr)
	{
		boxWorld.posMin = -halfExtent;
		boxWorld.posMax = halfExtent;
	}
	else
	{
		const glm::vec3 center = reference->xf.posWorld;
		boxWorld.posMin = center - halfExtent;
		boxWorld.posMax = center + halfExtent;
	}

	if (pemitter->pemitb->emitp.emitpk != EMITPK_Blip || pemitter->pblipg == nullptr)
		return;

	for (BLIP* pblip = pemitter->pblipg->dlBlip.pblipFirst; pblip != nullptr; pblip = pblip->pblipNext)
	{
		BLIPF* pframe = pblip->ablipf + pblip->iblipfLatest;

		for (int i = 0; i < pblip->cblipe; ++i)
		{
			glm::vec3& pos = pframe->ablipp[0];

			if (pos.x < boxWorld.posMin.x || pos.x > boxWorld.posMax.x)
			{
				pos.x = boxWorld.posMin.x +
					GModPositive(pos.x - boxWorld.posMin.x, fullExtent.x);
			}

			if (pos.y < boxWorld.posMin.y || pos.y > boxWorld.posMax.y)
			{
				pos.y = boxWorld.posMin.y +
					GModPositive(pos.y - boxWorld.posMin.y, fullExtent.y);
			}

			if (pos.z < boxWorld.posMin.z || pos.z > boxWorld.posMax.z)
			{
				pos.z = boxWorld.posMin.z +
					GModPositive(pos.z - boxWorld.posMin.z, fullExtent.z);
			}

			++pframe;
		}
	}*/
}

void OriginateParticles(int cParticle, EMITB* pemitb, EMITGEN* pemitgen)
{
	pemitgen->apos.resize(cParticle);
	pemitgen->av.resize(cParticle);
	pemitgen->atCreated.resize(cParticle);
	pemitgen->atDestroy.resize(cParticle);

	EMITVX emitvx{};
	CalculateEmitvx(pemitb->emitv.cParticlePerRing, &pemitb->emitv.lmTilt, cParticle, &emitvx);

	if (cParticle <= 0) {
		return;
	}

	EMITO* emito = &pemitb->emito;

	for (int i = 0; i < cParticle; ++i)
	{
		glm::vec3 normal{};

		// 1. Choose spawn position + surface normal
		ChooseEmitoPos(emito, i, cParticle, &pemitgen->apos[i], &normal);

		// 2. Choose velocity + lifetime
		ChooseEmitvVelocityAge(&pemitb->emitv, &emitvx, emito, i, &pemitgen->apos[i], &normal, &pemitgen->av[i], &pemitgen->atCreated[i], &pemitgen->atDestroy[i]);

		// 3. Apply local emitter transform (if present)
		EMITOLXF* localXf = emito->pemitolxf;

		if (localXf != nullptr)
		{
			glm::vec3 localPos = pemitgen->apos[i];
			glm::vec3 localVel = pemitgen->av[i];

			const glm::mat3& R = localXf->matLocal;
			const glm::vec3& t = localXf->posLocal;
			const glm::vec3& v = localXf->vLocal;
			const glm::vec3& w = localXf->wLocal;

			glm::vec3 worldPos = R * localPos + t;
			glm::vec3 offset = worldPos - t;

			glm::vec3 worldVel = (R * localVel) + v + glm::cross(w, offset);

			pemitgen->apos[i] = worldPos;
			pemitgen->av[i] = worldVel;
		}

		// 4. Optional conversion step
		if (pemitgen->fConvertPosVec != 0)
			ConvertEmitoPosVec(emito, &pemitgen->apos[i], &pemitgen->av[i]);
	}
}

void CalculateEmitvx(int cParticlePerRing, LM* plmTilt, int cParticle, EMITVX* pemitvx)
{
	if (cParticlePerRing < 1) {
		pemitvx->cParticlePerRing = cParticle;
	}
	else {
		if (cParticlePerRing > cParticle) {
			cParticlePerRing = cParticle;
		}

		pemitvx->cParticlePerRing = cParticlePerRing;
	}

	const int particlesPerRing = pemitvx->cParticlePerRing;
	if (particlesPerRing == 0) {
		return;
	}

	const float tiltMin = plmTilt->gMin;
	const float tiltMax = plmTilt->gMax;
	const float tiltSpan = RadNormalize(tiltMax - tiltMin);

	const int ringCount = (cParticle - 1 + particlesPerRing) / particlesPerRing;
	const float tiltStep = tiltSpan / static_cast<float>(ringCount);

	pemitvx->dradPanSlice = glm::two_pi<float>() / static_cast<float>(particlesPerRing);
	pemitvx->dradTiltRing = tiltStep;
	pemitvx->radTiltMin = tiltMin + tiltStep * 0.5f;
	pemitvx->radPanMin = GRandInRange(0.0f, glm::two_pi<float>());
}

void ChooseEmitoPos(EMITO* pemito, int iParticle, int cParticle, glm::vec3* pposRet, glm::vec3* pnormalRet)
{
	auto NormalizeAndOffset = [&](glm::vec3& normal)
		{
			const float len = glm::length(normal);
			*pnormalRet = (len < 0.0001f) ? g_normalX : (normal / len);

			const float offset = GRandInRange(pemito->lmSOffset.gMin, pemito->lmSOffset.gMax);
			*pposRet += (*pnormalRet) * offset;
		};

	switch (pemito->emitok)
	{
	case EMITOK_Point:
	{
		*pposRet = pemito->posOrigin;
		break;
	}

	case EMITOK_Box:
	{
		pposRet->x = GRandInRange(pemito->boxOrigin.posMin.x, pemito->boxOrigin.posMax.x);
		pposRet->y = GRandInRange(pemito->boxOrigin.posMin.y, pemito->boxOrigin.posMax.y);
		pposRet->z = GRandInRange(pemito->boxOrigin.posMin.z, pemito->boxOrigin.posMax.z);
		break;
	}

	case EMITOK_Curve:
	{
		//auto* pcurve = pemito->curveOrigin.pcurve;
		//if (pcurve == nullptr)
		//	break;

		//const float totalLength = pcurve->GetLength(); // replace with your actual accessor
		//const float slice = totalLength / static_cast<float>(cParticle);
		//const float t0 = static_cast<float>(iParticle) * slice;
		//const float t = GRandInRange(t0, t0 + slice);

		//switch (pemito->emitnk)
		//{
		//case EMITNK_CurveTangent:
		//{
		//	pcurve->Evaluate(t, *pposRet, pnormalRet);
		//	NormalizeAndOffset(*pnormalRet);
		//	return;
		//}

		//case EMITNK_CurveNormal:
		//{
		//	glm::vec3 tangent{};
		//	pcurve->Evaluate(t, *pposRet, &tangent);

		//	glm::vec3 normal = glm::cross(tangent, pemito->vec);
		//	NormalizeAndOffset(normal);
		//	return;
		//}

		//default:
		//{
		//	pcurve->Evaluate(t, *pposRet, nullptr);
		//	break;
		//}
		//}

		break;
	}

	case EMITOK_Skeleton:
	{
		const float totalWeight = pemito->posOrigin.z;
		const float slice = totalWeight / static_cast<float>(cParticle);
		float sample = GRandInRange(
			static_cast<float>(iParticle) * slice,
			static_cast<float>(iParticle + 1) * slice);

		SKELP* chosen = nullptr;
		const int cskelp = pemito->skelOrigin.cskelp;
		SKELP* askelp = pemito->skelOrigin.askelp;

		if (cskelp > 0 && askelp != nullptr)
		{
			chosen = askelp;
			sample -= chosen->gWeight;

			int i = 0;
			while (sample >= 0.0f && i + 1 < cskelp)
			{
				++i;
				chosen = &askelp[i];
				sample -= chosen->gWeight;
			}
		}

		if (chosen == nullptr)
			chosen = askelp;

		glm::vec3 axis = chosen->apalo[1]->xf.posWorld - chosen->apalo[0]->xf.posWorld;
		const float axisLen = glm::length(axis);

		if (axisLen < 0.0001f)
			axis = g_normalX;
		else
			axis /= axisLen;

		glm::vec3 basisX{};
		glm::vec3 basisY{};
		GetNormalVectors(axis, basisX, basisY, g_normalX, g_normalZ);

		const float density0 = chosen->agDensity[0];
		const float accumulated = sample + chosen->gWeight;

		float roots[2]{};
		if (chosen->agDensity[1] == density0)
		{
			roots[0] = accumulated / density0;
		}
		else
		{
			CSolveQuadratic(
				((chosen->agDensity[1] - density0) * 0.5f) / chosen->s,
				density0,
				-accumulated,
				roots);
		}

		const float s = roots[0];
		const float t = (std::abs(chosen->s) < 0.0001f) ? 0.0f : (s / chosen->s);
		const float pan = GRandInRange(-glm::pi<float>(), glm::pi<float>());

		glm::vec3 cyl{};
		SetVectorCylind(
			cyl,
			pan,
			(1.0f - t) * chosen->asRadius[0] + t * chosen->asRadius[1],
			s);

		glm::vec3 worldPos =
			chosen->apalo[0]->xf.posWorld +
			basisX * cyl.x +
			basisY * cyl.y +
			axis * cyl.z;

		ConvertAloPos(nullptr, pemito->paloReference, worldPos, *pposRet);

		if (pemito->emitnk == EMITNK_CurveTangent)
		{
			glm::vec3 normal = axis;
			ConvertAloVec(nullptr, pemito->paloReference, &normal, pnormalRet);
			NormalizeAndOffset(*pnormalRet);
			return;
		}

		if (pemito->emitnk == EMITNK_CurveNormal)
		{
			glm::vec3 normal = glm::cross(axis, pemito->vec);
			const float len = glm::length(normal);
			if (len < 0.0001f)
				normal = g_normalX;
			else
				normal /= len;

			ConvertAloVec(nullptr, pemito->paloReference, &normal, pnormalRet);
			NormalizeAndOffset(*pnormalRet);
			return;
		}

		break;
	}

	case EMITOK_Mesh:
	{
		float u = GRandInRange(0.0f, 1.0f);

		EMITTRI* chosenTri = nullptr;
		const int cemittri = pemito->emitmeshOrigin.cemittri;
		EMITTRI* aemittri = pemito->emitmeshOrigin.aemittri;

		if (cemittri > 0 && aemittri != nullptr)
		{
			float areaSample = u * pemito->emitmeshOrigin.sTotalArea;

			for (int i = 0; i < cemittri; ++i)
			{
				chosenTri = &aemittri[i];
				areaSample -= chosenTri->sArea;
				if (areaSample <= 0.0f)
					break;
			}
		}

		if (chosenTri == nullptr)
			break;

		const glm::vec3& v0 = pemito->emitmeshOrigin.apos[chosenTri->aipos[0]];
		const glm::vec3& v1 = pemito->emitmeshOrigin.apos[chosenTri->aipos[1]];
		const glm::vec3& v2 = pemito->emitmeshOrigin.apos[chosenTri->aipos[2]];

		const float b0 = GRandInRange(0.0f, 1.0f);
		const float remain = 1.0f - b0;
		const float b1 = GRandInRange(0.0f, remain);
		const float b2 = remain - b1;

		*pposRet = v0 * b0 + v1 * b1 + v2 * b2;

		if (pemito->emitnk == EMITNK_MeshNormal)
		{
			glm::vec3 normal = glm::cross(v2 - v0, v1 - v0);
			const float len = glm::length(normal);
			*pnormalRet = (len < 0.0001f) ? g_normalX : (normal / len);

			const float offset = GRandInRange(pemito->lmSOffset.gMin, pemito->lmSOffset.gMax);
			*pposRet += *pnormalRet * offset;
			return;
		}

		break;
	}

	default:
		break;
	}

	// Shared tail from original function.
	switch (pemito->emitnk)
	{
	case EMITNK_Radial:
	{
		*pnormalRet = *pposRet - pemito->vec;
		break;
	}

	case EMITNK_Normal:
	{
		*pnormalRet = pemito->vec;
		break;
	}

	case EMITNK_Screen:
	{
		if (pemito->paloReference == nullptr)
		{
			*pnormalRet = g_pcm->mat * pemito->vec; // replace with your camera-space vec transform
		}
		else
		{
			glm::vec3 viewVec = g_pcm->mat * pemito->vec;
			*pnormalRet = pemito->paloReference->xf.matWorld * viewVec;
		}
		break;
	}

	default:
	{
		*pnormalRet = g_normalX;
		break;
	}
	}

	NormalizeAndOffset(*pnormalRet);
}

void ChooseEmitvVelocityAge(EMITV* pemitv, EMITVX* pemitvx, EMITO* pemito, int iParticle, glm::vec3* ppos, glm::vec3* pnormal, glm::vec3* pv, float* ptCreated, float* ptDestroy)
{
	// Choose velocity
	ChooseEmitVelocity(pemitvx, pemitv->uRandomRad, pemitv->rSvz, &pemitv->lmSv, pnormal, iParticle, pv);

	// Birth time offset
	float dtBirth = GRandInRange(pemitv->lmDtBirth.gMin, pemitv->lmDtBirth.gMax);

	float tCreated = g_clock.t - dtBirth;

	*ptCreated = tCreated;
	*ptDestroy = tCreated + pemitv->dtLifetime;

	// Skip time (advance particle immediately)
	float dtSkip = GRandInRange(pemitv->lmDtSkip.gMin, pemitv->lmDtSkip.gMax);

	if (dtSkip != 0.0f) {
		*ppos += (*pv) * dtSkip;
	}
}

void ChooseEmitVelocity(EMITVX* pemitvx, float uRandom, float rSvz, LM* plmSv, glm::vec3* pvecNormal, int iParticle, glm::vec3* pv)
{
	const int particlesPerRing = pemitvx->cParticlePerRing;

	if (particlesPerRing == 0) {
		return;
	}

	const int ringIndex = iParticle / particlesPerRing;
	const int sliceIndex = iParticle % particlesPerRing;

	float tilt = pemitvx->radTiltMin +
		static_cast<float>(ringIndex) * pemitvx->dradTiltRing;

	float pan = pemitvx->radPanMin +
		static_cast<float>(sliceIndex) * pemitvx->dradPanSlice;

	if (uRandom != 0.0f) {
		tilt += uRandom * GRandInRange(-0.5f, 0.5f) * pemitvx->dradTiltRing;
		pan += uRandom * GRandInRange(-0.5f, 0.5f) * pemitvx->dradPanSlice;
	}

	const float sphereTilt = RadNormalize(tilt + glm::half_pi<float>());
	const float spherePan = RadNormalize(pan);

	const float speed = GRandInRange(plmSv->gMin, plmSv->gMax);

	glm::vec3 localVelocity{};
	SetVectorSphere(&localVelocity, spherePan, sphereTilt, speed);

	// Original code scales the local Z contribution before basis transform.
	localVelocity.z *= rSvz;

	// Build an orthonormal basis around the supplied normal.
	glm::vec3 tangent{};
	glm::vec3 bitangent{};
	GetNormalVectors(*pvecNormal, tangent, bitangent, g_normalX, g_normalZ);

	// Transform from local emitter space to world space.
	// local.x -> tangent
	// local.y -> bitangent
	// local.z -> normal
	*pv =
		tangent * localVelocity.x +
		bitangent * localVelocity.y +
		(*pvecNormal) * localVelocity.z;
}

void ConvertEmitoPosVec(EMITO* pemito, glm::vec3* ppos, glm::vec3* pv)
{
	if (pemito->paloReference == nullptr) {
		return;
	}

	// Always convert the spawn position into world space if we have a reference ALO.
	ConvertAloPos(pemito->paloReference, nullptr, *ppos, *ppos);

	// If velocity is already world-space, nothing else to do.
	if (pemito->emitvk == EMITVK_World) {
		return;
	}

	// Convert the velocity vector by the reference transform.
	ConvertAloVec(pemito->paloReference, nullptr, pv, pv);

	// Some modes stop after the vector-space conversion.
	if (pemito->emitvk == EMITVK_NoRelative) {
		return;
	}

	glm::vec3 movement{};

	// Original code switches which position is used when calculating inherited motion.
	glm::vec3* pMovementPos = ppos;
	if (pemito->emitvk != EMITVK_RelativeSpin) {
		pMovementPos = &pemito->paloReference->xf.posWorld;
	}

	CalculateAloMovement(pemito->paloReference, nullptr, *pMovementPos, &movement, nullptr, nullptr, nullptr);

	// Inherit motion from the reference object.
	*pv += movement;
}

void EmitBlips(EMITB* pemitb, EMITG* pemitg, int cblipeRequested, glm::vec3* apos, glm::vec3* av, float* atCreated, float* atDestroy, glm::vec3* aposFinal, glm::vec3* avFinal)
{
	if (pemitb == nullptr || cblipeRequested <= 0)
		return;

	BLIPG* pblipg = nullptr;

	if (pemitg != nullptr && pemitg->ppblipg != nullptr)
		pblipg = *pemitg->ppblipg;

	if (pblipg == nullptr)
	{
		if (g_psw == nullptr)
			return;

		/*if (g_psw->slotheapBlip.pslotFree == nullptr)
			return;*/

		/*if (pemitb->emitp.emitblip.blipmk == BLIPMK_Spline && g_psw->slotheapBlipsp.pslotFree == nullptr)
		{
			return;
		}*/

		pblipg = PblipgNew(g_psw);
		if (pblipg == nullptr)
			return;

		if (pemitg != nullptr && pemitg->ppblipg != nullptr)
			*pemitg->ppblipg = pblipg;

		SetBlipgEmitb(pblipg, pemitb);

		/*if (pemitg != nullptr && pemitg->ploSubscribe != nullptr && pblipg->pvtlo != nullptr && pblipg->pvtlo->pfnSubscribeLoObject != nullptr)
			pblipg->pvtlo->pfnSubscribeLoObject(pblipg);*/
	}

	BLIP* pblip = pblipg->dlBlip.pblipFirst;
	int iblipeRequested = 0;

	while (iblipeRequested < cblipeRequested)
	{
		if (pblip == nullptr)
		{
			pblip = PblipNew(pblipg);
			if (pblip == nullptr)
				return;
		}

		uint32_t iBlip = pblip->cblipe;

		while (iBlip < 0x2c && iblipeRequested < cblipeRequested)
		{
			// start data
			pblip->ablipf[0].ablipp[iBlip].x = apos[iblipeRequested].x;
			pblip->ablipf[0].ablipp[iBlip].y = apos[iblipeRequested].y;
			pblip->ablipf[0].ablipp[iBlip].z = apos[iblipeRequested].z;
			pblip->ablipf[0].ablipp[iBlip].tCreated = atCreated[iblipeRequested];

			// velocity / destroy
			pblip->ablipf[1].ablipp[iBlip].x = av[iblipeRequested].x;
			pblip->ablipf[1].ablipp[iBlip].y = av[iblipeRequested].y;
			pblip->ablipf[1].ablipp[iBlip].z = av[iblipeRequested].z;
			pblip->ablipf[1].ablipp[iBlip].tCreated = atDestroy[iblipeRequested];

			// extra runtime data
			std::memset(&pblip->ablipf[0].ablipx[iBlip], 0, sizeof(BLIPX));

			uint32_t irgba = 0;
			if (pblipg->crgba > 0)
				irgba = NRandInRange(0, pblipg->crgba - 1);

			if (pblipg->fColorRanges != 0)
				irgba &= 0xfffffffe;

			pblip->ablipf[0].ablipx[iBlip].irgba = irgba;

			if (pemitb->emitp.emitblip.fRandomFrame != 0 && pblipg->cqwTexture > 0)
			{
				pblip->ablipf[0].ablipx[iBlip].itex0 = NRandInRange(0, pblipg->cqwTexture - 1);
			}

			if (pblipg->blipok == BLIPOK_Rolling)
			{
				float swRoll = GRandInRange(pemitb->emitp.emitblip.lmSw.gMin, pemitb->emitp.emitblip.lmSw.gMax);

				if (pemitb->emitp.emitblip.bliprk == BLIPRK_Mirror && g_pcm != nullptr)
				{
					glm::vec3 camAxis(g_pcm->mat[1]);

					float d = glm::dot(camAxis, av[iblipeRequested]);
					if (d > 0.0f)
						swRoll = -swRoll;
				}

				pblip->ablipf[0].ablipx[iBlip].swRoll = swRoll;

				if (pemitb->emitp.emitblip.fRandomRoll != 0)
				{
					pblip->ablipf[0].ablipx[iBlip].radRoll =
						GRandInRange(-3.14159265358979323846f, 3.14159265358979323846f);
				}
			}

			if (pblipg->blipmk == BLIPMK_Spline && pblip->pblipsp != nullptr)
			{
				pblip->pblipsp->ablipsx[iBlip].posFinal.x = aposFinal[iblipeRequested].x;
				pblip->pblipsp->ablipsx[iBlip].posFinal.y = aposFinal[iblipeRequested].y;
				pblip->pblipsp->ablipsx[iBlip].posFinal.z = aposFinal[iblipeRequested].z;

				pblip->pblipsp->ablipsx[iBlip].vFinal.x = avFinal[iblipeRequested].x;
				pblip->pblipsp->ablipsx[iBlip].vFinal.y = avFinal[iblipeRequested].y;
				pblip->pblipsp->ablipsx[iBlip].vFinal.z = avFinal[iblipeRequested].z;
			}

			++pblip->cblipe;
			++pblipg->cblipe;
			++iBlip;
			++iblipeRequested;
		}

		pblip = pblip->dle.pblipNext;
	}
}

void UpdateEmitter(EMITTER* pemitter, float dt)
{
	EMITG emitg{};
	int particleCountToEmit = 0;

	if (pemitter->fValuesChanged != 0)
		OnEmitterValuesChanged(pemitter);

	UpdateAlo(pemitter, dt);

	if (pemitter->oidGroup != OID_Nil) {
		return;
	}

	if (!FIsDlEmpty(&pemitter->dlGroup))
	{
		EMITTER* groupEmitter = reinterpret_cast<EMITTER*>(pemitter->dlGroup.pblipFirst);

		if (groupEmitter == nullptr) {
			return;
		}

		while (groupEmitter != nullptr) {
			if ((groupEmitter->cpaloFindSwObjects & 2) == 0) {
				if (FIsLoInWorld(reinterpret_cast<LO*>(groupEmitter)) != 0) {
					if (groupEmitter->paloParent != nullptr) {
						float maxRelevantDist = groupEmitter->sMRD * g_pcm->rMRDAdjust;

						/*glm::vec3 cameraPos;
						if (g_cmlk == CMLK_Mrd) {
							cameraPos = g_posCmLock;
						}
						else {
							cameraPos = g_pcm->pos;
						}*/

						glm::vec3 delta = groupEmitter->xf.posWorld - g_pcm->pos;
						float distSq = glm::dot(delta, delta);
						float maxDistSq = maxRelevantDist * maxRelevantDist;

						if (distSq < maxDistSq) {
							goto emitter_is_relevant;
						}
					}
				}
			}

			groupEmitter = reinterpret_cast<EMITTER*>(groupEmitter->dleGroup.pblipNext);
		}

		return;
	}

	if (pemitter->paloParent != nullptr) 
	{
		float maxRelevantDist = pemitter->sMRD * g_pcm->rMRDAdjust;

		/*glm::vec3 cameraPos;
		if (g_cmlk == CMLK_Mrd) {
			cameraPos = g_posCmLock;
		}
		else {
			cameraPos = g_pcm->pos;
		}*/

		glm::vec3 delta = pemitter->xf.posWorld - g_pcm->pos;
		float distSq = glm::dot(delta, delta);
		float maxDistSq = maxRelevantDist * maxRelevantDist;

		if (distSq > maxDistSq) {
			return;
		}
	}

	emitter_is_relevant:

	if (FPausedEmitter(pemitter) != 0) {
		return;
	}

	EMITRK emitMode = pemitter->emitrk;
	particleCountToEmit = 0;

	if (emitMode == EMITRK_ConstantCount) 
	{
		float density = pemitter->rDensity;
		float constantCount = pemitter->cParticleConstant;
		int liveParticleCount = 0;

		if (pemitter->pemitb->emitp.emitpk == EMITPK_Blip) {
			if (pemitter->pblipg != nullptr) {
				liveParticleCount = pemitter->pblipg->cblipe;
			}
		}
		else {
			if (pemitter->pripg != nullptr) {
				liveParticleCount = CPvDl(&pemitter->pripg->dlRip);
			}
		}

		int wantedCount = static_cast<int>(constantCount * density) - liveParticleCount;
		if (wantedCount > 0) {
			particleCountToEmit = wantedCount;
		}
	}

	else if (emitMode == EMITRK_Continuous) 
	{
		float accumulated = pemitter->uParticle + pemitter->svcParticle * pemitter->rDensity * dt;
		particleCountToEmit = static_cast<int>(accumulated);
		pemitter->uParticle = accumulated - static_cast<float>(particleCountToEmit);

		if (particleCountToEmit != 0) {
			int remaining = pemitter->cParticle;
			if (remaining >= 0) {
				if (remaining <= particleCountToEmit) {
					particleCountToEmit = remaining;
				}
				pemitter->cParticle = remaining - particleCountToEmit;
			}
		}
	}
	else if (emitMode == EMITRK_Burst) {
		int remaining = pemitter->cParticle;
		particleCountToEmit = static_cast<int>(pemitter->svcParticle);

		if (remaining >= 0) {
			if (remaining <= particleCountToEmit) {
				particleCountToEmit = remaining;
			}
			pemitter->cParticle = remaining - particleCountToEmit;
		}

		if (pemitter->fAutoPause == 0) {
			float roll = GRandInRange(0.0f, 1.0f);

			if (roll <= pemitter->uPauseProb) {
				float pauseDt = GRandInRange(pemitter->lmDtPause.gMin, pemitter->lmDtPause.gMax);
				PauseEmitter(pemitter, pauseDt);
			}

			pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);
		}
		else {
			PauseEmitterIndefinite(pemitter);
			pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);
		}
	}
	else if (emitMode == EMITRK_BurstOld) {
		float accumulated = pemitter->uParticle + pemitter->svcParticle * pemitter->rDensity * dt;
		particleCountToEmit = static_cast<int>(accumulated);
		pemitter->uParticle = accumulated - static_cast<float>(particleCountToEmit);

		if (particleCountToEmit > 1) {
			int remaining = pemitter->cParticle;
			if (remaining >= 0) {
				if (remaining <= particleCountToEmit) {
					particleCountToEmit = remaining;
				}
				pemitter->cParticle = remaining - particleCountToEmit;
			}

			if (pemitter->fAutoPause == 0) {
				float roll = GRandInRange(0.0f, 1.0f);

				if (roll <= pemitter->uPauseProb) {
					float pauseDt = GRandInRange(pemitter->lmDtPause.gMin, pemitter->lmDtPause.gMax);
					PauseEmitter(pemitter, pauseDt);
				}

				pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax) * 60.0f;
			}
			else {
				PauseEmitterIndefinite(pemitter);
				pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax) * 60.0f;
			}
		}
	}

	if (particleCountToEmit != 0) 
	{
		emitg.ppripg = &pemitter->pripg;
		emitg.ppblipg = &pemitter->pblipg;

		ALO* oldReference = pemitter->pemitb->emito.paloReference;

		if (pemitter->oidReference == OID_Nil) 
			pemitter->pemitb->emito.paloReference = pemitter;

		emitg.ploSubscribe = pemitter;
		EmitParticles(particleCountToEmit, pemitter->pemitb.get(), &emitg);
		pemitter->pemitb->emito.paloReference = oldReference;
	}

	if (pemitter->tRecalcSvc <= g_clock.t) 
	{
		pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);
		pemitter->tRecalcSvc = g_clock.t + pemitter->dtRecalcSvc;
	}

	ModifyEmitterParticles(pemitter);
}

void DeleteEmitter(EMITTER *pemitter)
{
	delete pemitter;
}

EXPL* NewExpl()
{
	return new EXPL{};
}

int GetExplSize()
{
	return sizeof(EXPL);
}

void CloneExpl(EXPL* pexpl, EXPL* pexplBase)
{
	CloneXfm(pexpl, pexplBase);

	pexpl->pexplgParent = pexplBase->pexplgParent;
}

void PostExplLoad(EXPL* pexpl)
{
	PostLoLoad(pexpl);
	pexpl->pvtlo->pfnRemoveLo(pexpl);
}

void DeleteExpl(EXPL* pexpl)
{
	delete pexpl;
}

EXPLS* NewExpls()
{
	return new EXPLS{};
}

void InitExpls(EXPLS* pexpls)
{
	InitExplo(pexpls);
	pexpls->oidTouch = OID_Nil;
	pexpls->oidRender = OID_Nil;
	pexpls->oidNextRender = OID_Nil;
}

int GetExplsSize()
{
	return sizeof(EXPLS);
}

void CloneExpls(EXPLS* pexpls, EXPLS* pexplsBase)
{
	CloneExplo(pexpls, pexplsBase);

	pexpls->psfx = pexplsBase->psfx;
	pexpls->lmcParticle = pexplsBase->lmcParticle;
	pexpls->oidRender = pexplsBase->oidRender;
	pexpls->oidNextRender = pexplsBase->oidNextRender;
	pexpls->oidTouch = pexplsBase->oidTouch;
	pexpls->dtDelay = pexplsBase->dtDelay;
	pexpls->fGrouped = pexplsBase->fGrouped;
	pexpls->pripg = pexplsBase->pripg;
	pexpls->pblipg = pexplsBase->pblipg;
	pexpls->tExplodeNext = pexplsBase->tExplodeNext;
	pexpls->fExplodeSiblings = pexplsBase->fExplodeSiblings;

}

void BindExpls(EXPLS* pexpls)
{
	BindExplo(pexpls);
}

void DeleteExpls(EXPLS* pexpls)
{
	delete pexpls;
}

EXPLG* NewExplg()
{
	return new EXPLG{};
}

int GetExplgSize()
{
	return sizeof(EXPLG);
}

void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis)
{
	LoadXfmFromBrx(pexplg, pbis);

	uint16_t numExplgObjs = pbis->S16Read();

	for (int i = 0; i < numExplgObjs; i++)
	{
		CID cid = (CID)pbis->S16Read();
		OID oid = (OID)pbis->S16Read();
		uint16_t isplice = pbis->S16Read();

		LO* plo = PloNew(cid, pexplg->psw, pexplg->paloParent, oid, isplice);
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
	}
}

void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase)
{
	CloneExpl(pexplg, pexplgBase);

	pexplg->cpexpl = pexplgBase->cpexpl;
	pexplg->apexpl = pexplgBase->apexpl;
}

void BindExplg(EXPLG* pexplg)
{

}

void DeleteExplg(EXPLG* pexplg)
{
	delete pexplg;
}