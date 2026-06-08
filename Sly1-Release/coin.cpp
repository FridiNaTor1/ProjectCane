#include "coin.h"

DPRIZE* NewDprize()
{
	return new DPRIZE{};
}

void InitSwDprizeDl(SW* psw)
{
	InitDl(&psw->dlDprize, offsetof(DPRIZE, dleDprize));
}

void InitDprize(DPRIZE* pdprize)
{
	pdprize->dprizesInit = DPRIZES_Normal;
	pdprize->dprizes = DPRIZES_Nil;
	pdprize->svcAttract = 30.0;
	pdprize->oidInitialState = OID_Nil;
	InitAlo(pdprize);
	AppendDlEntry(&pdprize->psw->dlDprize, pdprize);

}

int GetDprizeSize()
{
	return sizeof(DPRIZE);
}

void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pdprize, pbis);
}

void CloneDprize(DPRIZE* pdprize, DPRIZE* pdprizeBase)
{
	CloneAlo(pdprize, pdprizeBase);

	pdprize->dprizes = pdprizeBase->dprizes;
	pdprize->tDprizes = pdprizeBase->tDprizes;
	pdprize->dprizesInit = pdprizeBase->dprizesInit;
	pdprize->oidInitialState = pdprizeBase->oidInitialState;
	pdprize->dtInitialSkip = pdprizeBase->dtInitialSkip;

	pdprize->psm = pdprizeBase->psm;
	pdprize->psma = pdprizeBase->psma;
	pdprize->ptarget = pdprizeBase->ptarget;

	pdprize->posCenter = pdprizeBase->posCenter;
	pdprize->vCenter = pdprizeBase->vCenter;
	pdprize->dvCenter = pdprizeBase->dvCenter;

	pdprize->uGlintChance = pdprizeBase->uGlintChance;
	pdprize->ppntFrontGlint = pdprizeBase->ppntFrontGlint;
	pdprize->ppntBackGlint = pdprizeBase->ppntBackGlint;

	pdprize->fLeft = pdprizeBase->fLeft;
	pdprize->tGlint = pdprizeBase->tGlint;

	pdprize->fNeverReuse = pdprizeBase->fNeverReuse;
	pdprize->fReuseCandidate = pdprizeBase->fReuseCandidate;
	pdprize->fLastBounce = pdprizeBase->fLastBounce;

	pdprize->svLastBounceMax = pdprizeBase->svLastBounceMax;
	pdprize->svLastBounce = pdprizeBase->svLastBounce;
	pdprize->sRadiusBounce = pdprizeBase->sRadiusBounce;
	pdprize->sRadiusCollect = pdprizeBase->sRadiusCollect;
	pdprize->rzBounce = pdprizeBase->rzBounce;
	pdprize->rxyBounce = pdprizeBase->rxyBounce;
	pdprize->radSmooth = pdprizeBase->radSmooth;
	pdprize->normalSmooth = pdprizeBase->normalSmooth;

	pdprize->fSwirlDone = pdprizeBase->fSwirlDone;
	pdprize->dleDprize = pdprizeBase->dleDprize;
	pdprize->ichkCollected = pdprizeBase->ichkCollected;

	pdprize->pexplCollect = pdprizeBase->pexplCollect;
	pdprize->pexplAttract = pdprizeBase->pexplAttract;
	pdprize->svcAttract = pdprizeBase->svcAttract;
	pdprize->cAttract = pdprizeBase->cAttract;
}

void PostDprizeLoad(DPRIZE* pdprize)
{
	PostAloLoad(pdprize);
}

void UpdateDprize(DPRIZE* pdprize, float dt)
{
	UpdateAlo(pdprize, dt);
}

void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro)
{
	if (pdprize->fHidden != 0)
		return;

	RO roLocal;
	RO* proRender = pro;

	const DPRIZES state = pdprize->dprizes;

	// Original: (DVar1 < DPRIZES_Lose) && (DPRIZES_Fall < DVar1)
	if (state > DPRIZES_Fall && state < DPRIZES_Lose)
	{
		const bool isSwirl = (state == DPRIZES_Swirl);

		const float duration = isSwirl ? DT_DprizeSwirl : DT_DprizeStick;
		const CLQ* pClq = isSwirl ? &s_clqUToRSwirl : &s_clqUToRStick;

		// t = saturate((g_clock.t - tDprizes) / duration)
		float t = (g_clock.t - pdprize->tDprizes) / duration;
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;

		// Decomp: (int)(g0 + t*(g1 + t*g2)) then broadcast to x/y/z
		float s = pClq->g0 + t * (pClq->g1 + t * pClq->g2);
		s = static_cast<float>(static_cast<int>(s)); // keep original quantization behavior

		glm::mat4 scaleMat(1.0f);
		scaleMat = glm::scale(scaleMat, glm::vec3(s, s, s));

		DupAloRo(pdprize, pro, &roLocal);

		// Decomp did: ro.mat * mat
		roLocal.model = roLocal.model * scaleMat;

		proRender = &roLocal;
	}

	RenderAloAll(pdprize, pcm, proRender);
}

void DeleteDprize(DPRIZE* pdprize)
{
	delete pdprize;
}

CHARM* NewCharm()
{
	return new CHARM{};
}

void InitCharm(CHARM* pcharm)
{
	InitDprize(pcharm);
}

int GetCharmSize()
{
	return sizeof(CHARM);
}

void CloneCharm(CHARM* pcharm, CHARM* pcharmBase)
{
	CloneDprize(pcharm, pcharmBase);
}

void DeleteCharm(CHARM* pcharm)
{
	delete pcharm;
}

void StartupCoinCtr(COINCTR* pcoinctr)
{
	pcoinctr->pvtcoinctr = &g_vtcoinctr;
}

COIN* NewCoin()
{
	return new COIN{};
}

void InitCoin(COIN* pcoin)
{
	InitDprize(pcoin);
}

int GetCoinSize()
{
	return sizeof(COIN);
}

void CloneCoin(COIN* pcoin, COIN* pcoinBase)
{
	CloneDprize(pcoin, pcoinBase);
}

void UpdateCoin(COIN* pcoin, float dt)
{
	UpdateDprize(pcoin, dt);
}

void DeleteCoin(COIN* pcoin)
{
	delete pcoin;
}

void StartupLifeCtr(LIFECTR* plifectr)
{
	plifectr->pvtlifectr = &g_vtlifectr;
}

LIFETKN* NewLifetkn()
{
	return new LIFETKN{};
}

int GetLifetknSize()
{
	return sizeof(LIFETKN);
}

void CloneLifetkn(LIFETKN* plifetkn, LIFETKN* plifetknBase)
{
	CloneDprize(plifetkn, plifetknBase);
}

void DeleteLifetkn(LIFETKN* plifetkn)
{
	delete plifetkn;
}

void StartupKeyCtr(KEYCTR* pkeyctr)
{
	pkeyctr->pvtkeyctr = &g_vtkeyctr;
}

KEY* NewKey()
{
	return new KEY{};
}

void InitKey(KEY* pkey)
{
	InitDprize(pkey);

	pkey->sRadiusCollect = 35.0;
	pkey->svLastBounceMax = 500.0;
	pkey->svLastBounce = 250.0;
	pkey->rzBounce = 0.6;
	pkey->uGlintChance = 0.75;
	pkey->sRadiusBounce = 35.0;
	pkey->rxyBounce = 0.6;
}

int GetKeySize()
{
	return sizeof(KEY);
}

void CloneKey(KEY* pkey, KEY* pkeyBase)
{
	CloneDprize(pkey, pkeyBase);
}

void DeleteKey(KEY* pkey)
{
	delete pkey;
}

void StartupGoldCtr(GOLDCTR* pgoldctr)
{
	pgoldctr->pvtgoldctr = &g_vtgoldctr;
}

GOLD* NewGold()
{
	return new GOLD{};
}

void InitGold(GOLD* pgold)
{
	InitDprize(pgold);
	pgold->psw->cgoldAll++;
}

int GetGoldSize()
{
	return sizeof(GOLD);
}

void CloneGold(GOLD* pgold, GOLD* pgoldBase)
{
	CloneDprize(pgold, pgoldBase);

	pgoldBase = pgold;
}

void DeleteGold(GOLD* pgold)
{
	delete pgold;
}

LIFECTR g_lifectr;
COINCTR g_coinctr;
GOLDCTR g_goldctr;
KEYCTR  g_keyctr;
float DT_DprizeSwirl = 1.0;
float DT_DprizeStick = 0.25;
CLQ s_clqUToRStick = { 0.69999999, -0.4, 0.0, 0.0 };
CLQ s_clqUToRSwirl = { 1.0, -0.3, 0.0, 0.0 };