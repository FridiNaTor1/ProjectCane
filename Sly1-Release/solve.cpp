#include "solve.h"

void SolveSw(SW* psw, float dt, GRFSG grfsg)
{
    /*std::vector <SO*> apso(psw->cpsoRoot);
    std::vector <SO*> apsoLocked(psw->cpsoRoot);

    int cpsoGroup = 0;
    int cpsoLocked = 0;

    if (grfsg & 1U)
    {
        DLI dliBusy;

        dliBusy.m_pdl = &psw->dlBusy;
        dliBusy.m_ibDle = psw->dlBusy.ibDle;

        SO* so = (SO*)psw->dlBusy.ploFirst;

        dliBusy.m_pdliNext = s_pdliFirst;
        s_pdliFirst = &dliBusy;

        while (so != nullptr)
        {
            dliBusy.m_ppv = (void**)((uintptr_t)&so + dliBusy.m_ibDle);

            bool shouldProject = false;

            if ((so->pvtalo->grfcid & 2U) == 0)
                shouldProject = so->cframeStatic < 2;
            else if ((*(uint64_t*)&so->bspcCamera.absp & 0x0040000000000000ULL) != 0)
                shouldProject = so->cframeStatic < 2;

            if (shouldProject)
                so->pvtalo->pfnProjectAloTransform(g_clock.dt, so, 0);

            so = (SO*)*dliBusy.m_ppv;
        }

        s_pdliFirst = dliBusy.m_pdliNext;
    }

    for (SO* so = psw->dlBusySo.psoFirst; so != nullptr; so = so->dleBusySo.psoNext)
    {
        uint64_t flags = *(uint64_t*)&so->bspcCamera.absp;

        if (flags & 0x0100000000000000ULL)
            continue;

        if (flags & 0x0040000000000000ULL)
            continue;

        cpsoGroup = 0;
        cpsoLocked = 0;

        BuildContactGroup(so, &cpsoGroup, apso.data(), &cpsoLocked, apsoLocked.data());

        if (grfsg & 1U)
        {
            bool needsProjection = false;

            for (int i = 0; i < cpsoGroup; i++)
            {
                if (apso[i]->cframeStatic < 2)
                {
                    needsProjection = true;
                    break;
                }
            }

            if (!needsProjection)
            {
                for (int i = 0; i < cpsoLocked; i++)
                {
                    if (apsoLocked[i]->cframeStatic < 2)
                    {
                        needsProjection = true;
                        break;
                    }
                }
            }

            if (!needsProjection)
                continue;

            for (int i = 0; i < cpsoGroup; i++)
            {
                SO* member = apso[i];

                member->pvtalo->pfnProjectAloTransform(g_clock.dt, member, 0);
            }
        }

        SplitSwGroup(psw, cpsoGroup, apso.data(), dt, grfsg);

        for (int i = 0; i < cpsoGroup; i++)
        {
            SO* member = apso[i];

            if (!FIsAloStatic((ALO*)member))
                ResolveAlo(member);
        }
    }

    for (SO* so = psw->dlRoot.psoFirst; so != nullptr; so = so->dleRoot.psoNext)
    {
        *(uint64_t*)&so->bspcCamera.absp &= ~0x0100000000000000ULL;
    }*/
}
