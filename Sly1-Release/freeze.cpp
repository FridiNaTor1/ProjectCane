#include "freeze.h"
#include "alo.h"

void RemergeSwObject(SW* psw, ALO* palo)
{
    if (palo->fRemerge)
        return;

    palo->fRemerge = true;

    psw->apaloRemerge[psw->cpaloRemerge] = palo;
    psw->cpaloRemerge++;
}

void MergeSwGroup(SW *psw, MRG *pmrg)
{
    int firstInWorld = -1;

    for (int i = 0; i < pmrg->cpalo; ++i)
    {
        if (FIsLoInWorld(pmrg->apalo[i]))
        {
            firstInWorld = i;
            break;
        }
    }

    if (firstInWorld < 0)
        return;

    ALO* baseRoot = pmrg->apalo[firstInWorld]->paloFreezeRoot;

    for (int i = firstInWorld; i < pmrg->cpalo; ++i)
    {
        ALO* palo = pmrg->apalo[i];

        if (!FIsLoInWorld(palo))
            continue;

        MergeSwFreezeGroups(psw, baseRoot, palo->paloFreezeRoot);
    }
}

void RemoveFromArray(int* count, void** items, void* item)
{
    for (int i = *count - 1; i >= 0; --i)
    {
        if (items[i] != item)
            continue;

        const int moveCount = *count - i - 1;

        if (moveCount > 0)
            std::memmove(&items[i], &items[i + 1], moveCount * sizeof(void*));

        --(*count);
        return;
    }
}

void RemoveSwMergeGroup(SW* psw, MRG* pmrg)
{
    if (pmrg->cpalo <= 1)
        return;

    for (int i = 0; i < pmrg->cpalo; ++i)
    {
        ALO* palo = pmrg->apalo[i];

        RemoveFromArray(&palo->cpmrg, (void**)palo->apmrg, pmrg);
    }

    for (int i = 0; i < pmrg->cpalo; ++i)
    {
        ALO* freezeRoot = pmrg->apalo[i]->paloFreezeRoot;

        if (freezeRoot)
        {
            SplinterSwFreezeGroup(psw, freezeRoot);
            return;
        }
    }
}

void RemergeSwObjects(SW* psw)
{
    for (int ipalo = psw->cpaloRemerge - 1; ipalo >= 0; ipalo--)
    {
        ALO* palo = psw->apaloRemerge[ipalo];
        ALO* paloFreezeRoot = palo->paloFreezeRoot;

        palo->fRemerge = false;

        if (paloFreezeRoot == palo)
        {
            for (int ipmrg = paloFreezeRoot->cpmrg - 1; ipmrg >= 0; ipmrg--)
            {
                MRG* pmrg = paloFreezeRoot->apmrg[ipmrg];
                MergeSwGroup(psw, pmrg);
            }
        }
    }

    psw->cpaloRemerge = 0;
}

void RefreezeSwObjects(SW* psw)
{
    for (ALO* freezeRoot = psw->dlMRD.paloFirst; freezeRoot != nullptr; freezeRoot = freezeRoot->dleMRD.paloNext)
    {
        // Only process true freeze roots.
        if (freezeRoot->paloFreezeRoot != freezeRoot)
            continue;

        ALO* first = freezeRoot->dlFreeze.paloFirst;

        bool shouldBeBusy = false;

        for (ALO* palo = first; palo != nullptr; palo = palo->dleFreeze.paloNext)
        {
            
            if (palo->fNoFreeze)
            {
                shouldBeBusy = false;
                break;
            }

            if ((palo->grfzon & g_pcm->grfzon) != g_pcm->grfzon)
                continue;

            if (g_cmlk == CMLK_Grfzon)
            {
                shouldBeBusy = false;
                break;
            }

            const float radius = palo->sMRD + palo->sRadiusRenderAll;

            glm::vec3 delta;

            if (g_cmlk == CMLK_Mrd)
                delta = palo->xf.posWorld - glm::vec3(0.0);
            else
                delta = palo->xf.posWorld - g_pcm->pos;

            const float distSq =
                delta.x * delta.x +
                delta.y * delta.y +
                delta.z * delta.z;

            if (distSq <= radius * radius)
            {
                shouldBeBusy = true;
                break;
            }
        }

        for (ALO* palo = first; palo != nullptr; palo = palo->dleFreeze.paloNext)
        {
            if (palo->fBusy == shouldBeBusy)
                continue;

            if (shouldBeBusy)
            {
                FreezeAloHierarchy(palo, false);
                AppendDlEntry(&psw->dlBusy, palo);

                if (palo->pvtlo->grfcid & 2U)
                    AppendDlEntry(&psw->dlBusySo, palo);
            }
            else
            {
                FreezeAloHierarchy(palo, true);
                RemoveDlEntry(&psw->dlBusy, palo);

                if (palo->pvtlo->grfcid & 2U)
                    RemoveDlEntry(&psw->dlBusySo, palo);
            }

            palo->fBusy = shouldBeBusy;
        }
    }
}

void MergeSwFreezeGroups(SW* psw, ALO* freezeGroup1, ALO* freezeGroup2)
{
    if (freezeGroup1 == freezeGroup2)
        return;

    // Move every object in group 2 over to group 1.
    for (ALO* palo = freezeGroup2->dlFreeze.paloFirst; palo != nullptr; palo = palo->dleFreeze.paloNext)
        palo->paloFreezeRoot = freezeGroup1;

    // Append group 2's freeze list into group 1's freeze list.
    MergeDl(&freezeGroup1->dlFreeze, &freezeGroup2->dlFreeze);
}

void SplinterSwFreezeGroup(SW* psw, ALO* paloFreezeRoot)
{
    DLI dli;

    dli.m_pdl = &paloFreezeRoot->dlFreeze;
    dli.m_ibDle = paloFreezeRoot->dlFreeze.ibDle;

    ALO* palo = paloFreezeRoot->dlFreeze.paloFirst;

    dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<char*>(palo) + dli.m_ibDle);

    dli.m_pdliNext = s_pdliFirst;
    s_pdliFirst = &dli;

    while (palo != nullptr)
    {
        if (palo != paloFreezeRoot)
        {
            RemoveDlEntry(&paloFreezeRoot->dlFreeze, palo);

            palo->paloFreezeRoot = palo;

            palo->dlFreeze.paloFirst = palo;
            palo->dlFreeze.paloLast = palo;
        }

        RemergeSwObject(psw, palo);

        palo = static_cast<ALO*>(*dli.m_ppv);

        if (palo != nullptr)
            dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<char*>(palo) + dli.m_ibDle);
    }

    s_pdliFirst = dli.m_pdliNext;
}

void FreezeAloHierarchy(ALO* palo, int freeze)
{
    if (palo == nullptr)
        return;

    if (palo->pvtalo->pfnFreezeAlo != nullptr)
        palo->pvtalo->pfnFreezeAlo(palo, freeze);

    for (ALO* child = palo->dlChild.paloFirst; child != nullptr; child = child->dleChild.paloNext)
    {
        if ((child->pvtlo->grfcid & 1) != 0)
            FreezeAloHierarchy(child, freeze);
    }
}

void UpdateSwBusyList(SW* psw)
{
    RemergeSwObjects(psw);
    RefreezeSwObjects(psw);
}