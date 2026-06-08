#include "suv.h"

SUV* NewSuv()
{
	return new SUV{};
}

void InitSuv(SUV* psuv)
{
	InitPo(psuv);

    /*psuv->sRadiusFrontWheel = 50.0f;
    psuv->sRadiusRearWheel = 50.0f;

    psuv->svMax = 3000.0f;
    psuv->dyMax = 1300.0f;

    psuv->muSxp = 2.0f;

    psuv->cLapMax = 3;

    psuv->clqTune = s_clqTune_994;

    std::memcpy(psuv->asvrb, s_asvrb_993, sizeof(psuv->asvrb));

    ResetSuv(psuv);*/
}

int GetSuvSize()
{
	return sizeof(SUV);
}

void UpdateSuvXfWorld(SUV* psuv)
{
	UpdateSoXfWorld(psuv);
    UpdateSuvShapes(psuv);
}

void PostSuvLoad(SUV* psuv)
{
    PostAloLoad(psuv);

    //SnipAloObjects(psuv, 0x14, s_asnip_998);

    //// Store current facing angle as target angle.
    //psuv->radTarget = atan2f(psuv->xf.mat[0][1], psuv->xf.mat[0][0]);

    //// Setup puncher state machine.
    //if (psuv->psmPuncher != nullptr)
    //{
    //    SMA* psma = PsmaApplySm(psuv->psmPuncher, psuv, OID_Nil, 0);

    //    OID oidGoal = OID_passive;

    //    if (psuv->suvgk == SUVGK_Chase)
    //        oidGoal = OID_state_ready;

    //    psuv->psmaPuncher = psma;

    //    SetSmaGoal(psma, oidGoal);
    //}

    //// Start SUV engine sounds.
    //StartSound(SFXID_EnvCarEngine1_Muggshot, &psuv->pambRunning, psuv, nullptr, 6000.0f, 1000.0f, 1.0f,2.0f, 0.0f, nullptr, nullptr);

    //StartSound(SFXID_EnvRaceMurrayIdle_Muggshot, &psuv->pambIdle, psuv, nullptr, 6000.0f, 1000.0f, 1.0f, 2.0f, 0.0f, nullptr, nullptr);

    //// Random top wobble / frequency offset.
    //psuv->dfrqTop = GRandInRange(-0.2f, 0.2f);

    //// Find Murray for auto-collect.
    //MURRAY* pmurray = reinterpret_cast<MURRAY*>(PloFindSwObject(g_psw, 0x104, OID_murray, psuv));

    //if (pmurray != nullptr)
    //{
    //    psuv->pzi.paloCollect = reinterpret_cast<ALO*>(pmurray);
    //    psuv->pzi.sAutoCollect = 200.0f;
    //    psuv->pmurray = pmurray;
    //}

    //// Chase mode setup.
    //if (psuv->suvgk == SUVGK_Chase)
    //{
    //    LO* aplo[16]{};

    //    int count = CploFindSwObjects(psuv->psw, 0x105, OID_suv_prize, nullptr, 16, aplo);

    //    for (int i = 0; i < count; ++i)
    //        aplo[i]->pvtlo->pfnSubscribeLoObject(aplo[i], psuv);

    //    memset(g_scores.an, 0, sizeof(g_scores.an));
    //}

    //// Initialize suspension/wheel current positions.
    //for (int i = 0; i < 4; ++i)
    //{
    //    SXP& sxp = psuv->asxp[i];

    //    if (sxp.paloWheel != nullptr)
    //        sxp.posCur = sxp.paloWheel->xf.pos;
    //}

    //ResetSuv(psuv);
}

void UpdateSuv(SUV* psuv, float dt)
{
    int cpsuvInFront = 0;

    UpdatePo(psuv, dt);

    //switch (psuv->suvs)
    //{
    //    case SUVS_Auto:
    //    {
    //        UpdateSuvBalance(psuv);
    //        UpdateSuvLine(psuv, &cpsuvInFront);
    //        UpdateSuvHeading(psuv);
    //        break;
    //    }

    //    case SUVS_Stop:
    //    {
    //        psuv->svTarget = 0.0f;
    //        break;
    //    }

    //    case SUVS_Manual:
    //    default:
    //    {
    //        break;
    //    }
    //}

    //UpdateSuvSounds(psuv, dt);
    //UpdateSuvWheels(psuv);
    //UpdateSuvExpls(psuv);
    //UpdateSuvVolumes(psuv, cpsuvInFront);
    //UpdateSuvPuncher(psuv);

    //// mat[2][2] / forward-up or up-z check depending on matrix layout.
    //if (psuv->xf.mat[2][2] > 0.5f)
    //    psuv->tUpright = g_clock.t;

    //if (psuv->suvs != SUVS_Stop)
    //    ResolveAlo(psuv);
}

void UpdateSuvShapes(SUV* psuv)
{
    /*if (psuv->pshapeTrack == nullptr)
        return;

    CRV* pcrv = psuv->pshapeTrack->pcrv.get();

    glm::vec3 posClosest{};
    glm::vec3 normalClosest{};

    auto pfnFindClosest = pcrv->pvtcrv->pfnFindCrvClosestPointFromU;

    if (pfnFindClosest != nullptr)
        pfnFindClosest(psuv->uTrack, pcrv, &psuv->xf.pos, 0, &posClosest, &normalClosest, &psuv->uTrack, 0);

    glm::vec3 normal = glm::normalize(glm::cross(g_normalZ, normalClosest));

    psuv->sTrack = pcrv->pvtcrv->pfnSFromCrvU(psuv->uTrack);

    psuv->dyTrack = glm::dot(psuv->xf.pos - posClosest, normal);*/
}

void RenderSuvSelf(SUV* psuv, CM* pcm, RO* pro)
{
	RenderSoSelf(psuv, pcm, pro);
}

void CloneSuv(SUV* psuv, SUV* psuvBase)
{
    ClonePo(psuv, psuvBase);

    psuv->sRadiusFrontWheel = psuvBase->sRadiusFrontWheel;
    psuv->sRadiusRearWheel = psuvBase->sRadiusRearWheel;
    psuv->svMax = psuvBase->svMax;
    psuv->dyMax = psuvBase->dyMax;
    psuv->clqTune = psuvBase->clqTune;

    for (int i = 0; i < 2; ++i)
        psuv->asvrb[i] = psuvBase->asvrb[i];

    psuv->svrb = psuvBase->svrb;
    psuv->paloShadow = psuvBase->paloShadow;
    psuv->radTarget = psuvBase->radTarget;
    psuv->svTarget = psuvBase->svTarget;
    psuv->radFront = psuvBase->radFront;
    psuv->xsxp = psuvBase->xsxp;

    for (int i = 0; i < 4; ++i)
        psuv->asxp[i] = psuvBase->asxp[i];

    psuv->pshapeTrack = psuvBase->pshapeTrack;
    psuv->sTrackMax = psuvBase->sTrackMax;
    psuv->uTrack = psuvBase->uTrack;
    psuv->sTrack = psuvBase->sTrack;
    psuv->dyTrack = psuvBase->dyTrack;
    psuv->dyTarget = psuvBase->dyTarget;
    psuv->dsTrackFinish = psuvBase->dsTrackFinish;
    psuv->pshapeLine = psuvBase->pshapeLine;
    psuv->uLine = psuvBase->uLine;
    psuv->tUpright = psuvBase->tUpright;
    psuv->tBoost = psuvBase->tBoost;
    psuv->cBoost = psuvBase->cBoost;
    psuv->rsvBalance = psuvBase->rsvBalance;
    psuv->rsvGoal = psuvBase->rsvGoal;
    psuv->tBalance = psuvBase->tBalance;
    psuv->muSxp = psuvBase->muSxp;
    psuv->tPunched = psuvBase->tPunched;
    psuv->csve = psuvBase->csve;

    for (int i = 0; i < 16; ++i)
        psuv->asve[i] = psuvBase->asve[i];

    psuv->psveCheckFirst = psuvBase->psveCheckFirst;
    psuv->psveCheckCur = psuvBase->psveCheckCur;
    psuv->psveFeatureCur = psuvBase->psveFeatureCur;
    psuv->cLap = psuvBase->cLap;
    psuv->cLapMax = psuvBase->cLapMax;
    psuv->nPlace = psuvBase->nPlace;
    psuv->nPlaceMax = psuvBase->nPlaceMax;
    psuv->tPlace = psuvBase->tPlace;
    psuv->pemitterBoost = psuvBase->pemitterBoost;
    psuv->pexplDirt = psuvBase->pexplDirt;
    psuv->pexplDust = psuvBase->pexplDust;
    psuv->cParticleDirt = psuvBase->cParticleDirt;
    psuv->cParticleDust = psuvBase->cParticleDust;
    psuv->paloFrontAxle = psuvBase->paloFrontAxle;
    psuv->paloRearAxle = psuvBase->paloRearAxle;
    psuv->suvgk = psuvBase->suvgk;
    psuv->suvs = psuvBase->suvs;
    psuv->tSuvs = psuvBase->tSuvs;
    psuv->ppathzone = psuvBase->ppathzone;
    psuv->psoPrizeCur = psuvBase->psoPrizeCur;
    psuv->psmPuncher = psuvBase->psmPuncher;
    psuv->psmaPuncher = psuvBase->psmaPuncher;
    psuv->psoPuncher = psuvBase->psoPuncher;
    psuv->cpsoIgnore = psuvBase->cpsoIgnore;

    for (int i = 0; i < 8; ++i)
        psuv->apsoIgnore[i] = psuvBase->apsoIgnore[i];

    psuv->pxpPuncher = psuvBase->pxpPuncher;
    psuv->pmurray = psuvBase->pmurray;
    psuv->fFreeWheeling = psuvBase->fFreeWheeling;
    psuv->pambSkid = psuvBase->pambSkid;
    psuv->pambRunning = psuvBase->pambRunning;
    psuv->pambIdle = psuvBase->pambIdle;
    psuv->pambBoost = psuvBase->pambBoost;
    psuv->volBoostTarget = psuvBase->volBoostTarget;
    psuv->volBoostCur = psuvBase->volBoostCur;
    psuv->volTarget = psuvBase->volTarget;
    psuv->volCur = psuvBase->volCur;
    psuv->dvolInc = psuvBase->dvolInc;
    psuv->frqTarget = psuvBase->frqTarget;
    psuv->frqCur = psuvBase->frqCur;
    psuv->dfrqInc = psuvBase->dfrqInc;
    psuv->dfrqTop = psuvBase->dfrqTop;
}

void DeleteSuv(SUV* psuv)
{
	delete psuv;
}

void StartupLapCtr(LAPCTR* plapctr)
{
    plapctr->pvtlapctr = &g_vtlapctr;
}

void StartupBoostCtr(BOOSTCTR* pboostctr)
{
    pboostctr->pvtboostctr = &g_vtboostctr;
}

void StartupPlaceCtr(PLACECTR* placectr)
{
    placectr->pvtplacectr = &g_vtplacectr;
}

LAPCTR g_lapctr;
BOOSTCTR g_boostctr;
PLACECTR g_placectr;