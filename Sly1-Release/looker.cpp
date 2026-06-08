#include "looker.h"

LOOKER* NewLooker()
{
    return new LOOKER{};
}

void LoadLookerFromBrx(LOOKER* plooker, CBinaryInputStream* pbis)
{
    float uCenter = pbis->F32Read();
    float vCenter = pbis->F32Read();
    float uMin = pbis->F32Read();
    float uMax = pbis->F32Read();
    float vMin = pbis->F32Read();
    float vMax = pbis->F32Read();

    plooker->uCenter = uCenter;
    plooker->vCenter = vCenter;

    plooker->duMin = uMin - uCenter;
    plooker->duMax = uMax - uCenter;

    plooker->dvMin = vMin - vCenter;
    plooker->dvMax = vMax - vCenter;
}

void InitLooker(LOOKER* plooker, SAAF* psaaf)
{
    InitSaa((SAA*)plooker, psaaf);

    plooker->sai.grfsai = (plooker->sai.grfsai & 0xfffffffe) | 2;
}

void NotifyLookerRender(LOOKER* plooker, ALO* palo, RPL* prpl)
{

}

void DeleteLooker(LOOKER* plooker)
{
    delete plooker;
}