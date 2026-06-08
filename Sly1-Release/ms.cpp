#include "ms.h"

MS* NewMs()
{
	return new MS{};
}

int GetMsSize()
{
	return sizeof(MS);
}

void RenderMsGlobset(MS* pms, CM* pcm, RO* pro)
{
    RPL rpl{};
    glm::mat4 baseModelMatrix{};
    LoadMatrixFromPosRot(pms->xf.posWorld, pms->xf.matWorld, baseModelMatrix);

    for (int i = 0; i < pms->globset.cglob; ++i)
    {
        auto& glob  = pms->globset.aglob[i];
        auto& globi = pms->globset.aglobi[i];

        // Zone mask gate (matches: (*puVar4 & grfzonCamera) == grfzonCamera)
        if (g_fBsp != 0)
        {
            if ((globi.grfzon & pcm->grfzon) != pcm->grfzon)
                continue;
        }

        // Fast frustum sphere test
        if (!SphereInFrustum(pcm->frustum, glob.posCenter, glob.sRadius))
            continue;

        // MRD test: original uses (posCenter - camPos)
        float dummy = 1.0f;
        const glm::vec4 dpos = glm::vec4(glob.posCenter, 1.0f) - glm::vec4(pcm->pos, 1.0f);
        if (!FInsideCmMrd(pcm, dpos, glob.sRadius, glob.sMRD, dummy))
            continue;

        // Base defaults per-glob like the original rpl reuse would imply
        rpl = {};
        rpl.pglob = &glob;

        // Unfade smoothing (target 0.5 until tUnfade, else 1.0)
        float target = (g_clock.tReal < globi.tUnfade) ? 0.5f : 1.0f;
        if (globi.uAlpha != target)
            globi.uAlpha = GSmooth(globi.uAlpha, target, g_clock.dt, &s_smpFade, nullptr);

        // Alpha combine (matches: uAlpha * fade * global)
        const float alpha = 1.0f * globi.uAlpha * g_uAlpha;
        if (alpha <= 0.0f)
            continue;

        // Fill RPL fields used by DrawGlob / sorting
        rpl.palo = pms;
        rpl.ro.posCenter = glm::vec4(glob.posCenter, 1.0f);
        rpl.ro.uAlpha = alpha;
        rpl.ro.uFog = glob.uFog;
        rpl.rp = glob.rp;

        // Darken selection (yours was fine; keep)
        rpl.ro.darken = ((glob.grfglob & 4U) == 0) ? g_psw->rDarken : 1.0f;

        // Warp data (keep your logic)
        if (glob.pwarpGlob != nullptr)
        {
            rpl.ro.warpType = glob.pwrbg->warpType;
            rpl.ro.warpCmat = glob.pwarpGlob->pwr->cmat;
            rpl.ro.warpCvtx = glob.pwarpGlob->vertexCount;
            
            const size_t count = (size_t)glob.pwrbg->cmat;

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

        // Z order (matches: if FLT_MAX compute squared distance)
        if (glob.gZOrder != FLT_MAX)
            rpl.z = glob.gZOrder;
        else
        {
            glm::vec3 d = glob.posCenter - pcm->pos;
            rpl.z = glm::dot(d, d);
        }

        // RP remap when alpha < 1 (original also handles CelBorder -> TranslucentCelBorder)
        if (alpha < 1.0f)
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

        // Original multiplies cel-border alpha by final alpha
        rpl.ro.uAlphaCelBorder *= rpl.ro.uAlpha;

        // Translucent sort flag (keep your scheme)
        int sortT = 0;
        if (alpha < 1.0f)
        {
            // already mapped above; alpha<1 implies "always sort" behavior for these types
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

        // Per-glob RO flags
        rpl.ro.fDynamic = glob.fDynamic;
        rpl.ro.sRadius  = glob.sRadius;

        rpl.ro.grfglob = glob.grfglob;

        // Model matrix (base * pdmat if present)
        rpl.ro.model = baseModelMatrix;

        if (glob.pdmat != nullptr)
            rpl.ro.model = baseModelMatrix * *glob.pdmat;

        // SAA notify hook (missing in your version)
        // Original: if (psaa && psaa->...->pfnNotifySaaRender) call it (may mutate rpl)
        if (glob.psaa != nullptr)
        {
            if (glob.psaa && glob.psaa->pvtlooker && glob.psaa->pvtlooker->pfnNotifyLookerRender)
			    glob.psaa->pvtlooker->pfnNotifyLookerRender((LOOKER*)glob.psaa, pms, &rpl);
        }

        // RTCK adjustment after pdmat (original does this after optional pdmat multiply)
        if (glob.rtck != RTCK_None)
            AdjustAloRtckMat(pms, pcm, glob.rtck, &glob.posCenter, rpl.ro.model);

        // Dynamic light pick (keep yours)
        if (!allSwDynamicLights.empty())
        {
            if (glob.fThreeWay == 1)
                rpl.ro.fDynamicLight = FindSwDynamicLights(&glob.posCenter, glob.sRadius);
            else
                rpl.ro.fDynamicLight = 0;
        }
        else
            rpl.ro.fDynamicLight = 0;

        // TRLK selection (keep yours)
        rpl.ro.trlk = glob.trlk;

        if (glob.fDynamic == 1 || glob.pwarpGlob != nullptr)
            rpl.ro.trlk = TRLK_Dynamic;

        bool bakedThisFrame = (glob.trlk == TRLK_Relight);;

        // Submit
        SubmitRpl(&rpl);

        if (glob.trlk == TRLK_Relight && bakedThisFrame)
            glob.trlk = TRLK_Baked;

        // Cel border alpha default (original sets this only when csubcel != 0)
        rpl.ro.uAlphaCelBorder = 1.0f;

        // Cel-border MRD logic (missing in your version)
        if (g_fRenderCelBorders > 0 && glob.csubcel > 0)
        {
            if (glob.sCelBorderMRD < glob.sMRD)
            {
                float dummy2 = 1.0f;
                if (!FInsideCmMrd(pcm, dpos, glob.sRadius, glob.sCelBorderMRD, dummy2))
                    rpl.ro.uAlphaCelBorder = 0.0f;
            }
            else
                rpl.ro.uAlphaCelBorder = 1.0f;
        }

        // NOTE: PS2 version resets the matrix back to identity if it modified it, because it reuses one RPL.
        // You rebuild rpl.ro.model every iteration, so no reset needed here.
    }
}

void DeleteMs(MS* pms)
{
	delete pms;
}