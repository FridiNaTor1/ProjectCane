#include "shape.h"

SHAPE* NewShape()
{
    return new SHAPE{};
}

void InitSwShapeDl(SW* psw)
{
    InitDl(&psw->dlShape, offsetof(SHAPE, dleShape));
}

void InitShape(SHAPE* pshape)
{
    InitLo(pshape);
    AppendDlEntry(&pshape->psw->dlShape, pshape);
}

int GetShapeSize()
{
    return sizeof(SHAPE);
}

void CloneShape(SHAPE* pshape, SHAPE* pshapeBase)
{
    CloneLo(pshape, pshapeBase);

    pshape->dleShape = pshapeBase->dleShape;
    pshape->pcrv = pshapeBase->pcrv;
}

void SetShapeParent(SHAPE* pshape, ALO* paloParent)
{
    glm::mat4 matSrc(1.0f);
    glm::mat4 matDst(1.0f);

    ALO* oldParent = pshape->paloParent;

    if (oldParent != nullptr)
        LoadMatrixFromPosRot(oldParent->xf.posWorld, oldParent->xf.matWorld, matSrc);

    if (paloParent != nullptr)
        LoadMatrixFromPosRot(paloParent->xf.posWorld, paloParent->xf.matWorld, matDst);

    CRV *pcrv = pshape->pcrv.get();

    /*if (pcrv != nullptr && pcrv->pvtcrv != nullptr && pcrv->pvtcrvl->pfnConvertCrvl != nullptr)
        pcrv->pvtcrvl->pfnConvertCrvl((CRVL*)pcrv, &matSrc, &matDst);*/

    SetLoParent(pshape, paloParent);
}

void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis)
{
    byte crvk = pbis->U8Read();

    pshape->pcrv = PcrvNew((CRVK)crvk);
    pshape->pcrv->pvtcrvl->pfnLoadCrvlFromBrx((CRVL*)pshape->pcrv.get(), pbis);

    LoadOptionsFromBrx(pshape, pbis);
}

void DeleteShape(SHAPE* pshape)
{
    delete pshape;
}