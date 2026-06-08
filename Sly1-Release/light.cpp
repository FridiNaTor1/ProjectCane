#include "light.h"

LIGHT* NewLight()
{
	return new LIGHT{};
}

void InitSwLightDl(SW* psw)
{
	InitDl(&psw->dlLight, offsetof(LIGHT, dleLight));
}

void InitLight(LIGHT* plight)
{
	plight->lightk = LIGHTK_Direction;
	plight->fNoFreeze = 1;
	plight->normalLocal.x = 0.0;
	plight->normalLocal.y = 0.0;
	plight->normalLocal.z = 1.0;
	plight->vecUpLocal.x = 0.0;
	plight->vecUpLocal.y = 0.0;
	plight->vecUpLocal.z = 1.0;
	plight->lmFallOffS.gMin = 200.0;
	plight->lmFallOffS.gMax = 2000.0;
	plight->degMidtone = 240.0;
	plight->degShadow = 180.0;
	plight->degCone = 60.0;
	plight->degHighlight = 180.0;

	RebuildLightFrustrum(plight);
	InitAlo(plight);

	numSwLights++;
}

int GetLightSize()
{
	return sizeof(LIGHT);
}

void OnLightAdd(LIGHT* plight)
{
	OnAloAdd(plight);
	AddLightToSw(plight);
}

void OnLightRemove(LIGHT* plight)
{
	OnAloRemove(plight);
	RemoveLightFromSw(plight);
}

void RebuildLightFrustrum(LIGHT* plight)
{
	// 1) Clamp cone and hotspot angles
	float xfov = glm::clamp(plight->degCone, 0.0f, 175.0f); // outer cone
	float yfov = glm::clamp(plight->degHotSpot, 0.0f, xfov);   // inner hotspot

	// 2) Convert to tan(half-angle)
	const float DEG_TO_HALF_RAD = 0.008726647f;

	float outerTan = tanf(xfov * DEG_TO_HALF_RAD);
	float innerTan = tanf(yfov * DEG_TO_HALF_RAD);

	// Symmetric frustum: same extent in X/Y
	plight->rx = outerTan;
	plight->ry = outerTan;

	// 3) Inner/outer ratio (protect from divide-by-zero)
	float ratio = 0.0f;
	if (outerTan > 0.0f)
		ratio = innerTan / outerTan;

	// 4) Fill falloff structs exactly like original
	plight->lmFallOffAbsX.gMax = 1.0f;
	plight->lmFallOffAbsY.gMax = 1.0f;

	plight->lmFallOffAbsX.gMin = ratio;
	plight->lmFallOffAbsY.gMin = ratio;

	// Penumbra falloff: sqrt(ratio)
	plight->lmFallOffPenumbra.gMax = 1.0f;
	plight->lmFallOffPenumbra.gMin = sqrtf(ratio);
}
void UpdateLightXfWorldHierarchy(LIGHT* plight)
{
	UpdateAloXfWorldHierarchy(plight);
	RebuildLight(plight);
	UpdateLightBeamGrfzon(plight);
}

void UpdateLightBeamGrfzon(LIGHT* plight)
{
	if (!plight || plight->fDynamic == 0)
		return;

	switch (plight->lightk)
	{
		case LIGHTK_Direction:
		plight->grfzon = 0x0FFFFFFF;
		break;

		case LIGHTK_Position:
		ClipVismapSphereOneHop(plight->psw->pvismap, &(plight->xf).posWorld, (plight->lmFallOffS).gMax, &plight->grfzon);
		break;

		case LIGHTK_Frustrum:
		case LIGHTK_Spot:
		{
			float gMax = (plight->lmFallOffS).gMax;

			float halfAngleRad = glm::radians(plight->degCone * 0.5f);
			float baseRadius = std::tanf(halfAngleRad) * gMax;

			float halfLen = gMax * 0.5f;

			// Forward vector from look-at matrix (matches original direction multiply)
			glm::vec3 forward = -glm::vec3(plight->matLookAt[2]);

			float lenSq = glm::dot(forward, forward);
			if (lenSq > 0.0f)
				forward *= glm::inversesqrt(lenSq);

			glm::vec3 posBeamWorld = glm::vec3((plight->xf).posWorld) + forward * halfLen;

			// Bounding sphere radius
			float sphereRadius = std::sqrt(halfLen * halfLen + baseRadius * baseRadius);

			ClipVismapSphereOneHop(plight->psw->pvismap, &posBeamWorld, sphereRadius, &plight->grfzon);
		}
		break;

		default:
		break;
	}
}

void CloneLight(LIGHT* plight, LIGHT* plightBase)
{
	CloneAlo(plight, plightBase);

	// Clone the LIGHTK structure (direct copy)
	plight->lightk = plightBase->lightk;

	// Clone the TWPS structure (direct copy)
	plight->twps = plightBase->twps;

	// Clone the color vectors (HSV and RGBA)
	plight->vecHighlight = plightBase->vecHighlight;
	plight->rgbaColor = plightBase->rgbaColor;

	// Clone the LTFN structure (direct copy)
	plight->ltfn = plightBase->ltfn;

	// Clone the falloff vectors
	plight->agFallOff = plightBase->agFallOff;

	// Clone the matrix values (frustum and matLookAt)
	plight->frustum = plightBase->frustum;
	plight->matLookAt = plightBase->matLookAt;

	// Clone the float values
	plight->gMidtone = plightBase->gMidtone;
	plight->gShadow = plightBase->gShadow;
	plight->degHighlight = plightBase->degHighlight;
	plight->degMidtone = plightBase->degMidtone;
	plight->degShadow = plightBase->degShadow;
	plight->degCone = plightBase->degCone;
	plight->degHotSpot = plightBase->degHotSpot;
	plight->rx = plightBase->rx;
	plight->ry = plightBase->ry;

	// Clone the vectors for local direction and normals
	plight->vecDirectionOrig = plightBase->vecDirectionOrig;
	plight->normalLocal = plightBase->normalLocal;
	plight->vecUpLocal = plightBase->vecUpLocal;

	// Clone the LM structures (light falloff and penumbra)
	plight->lmFallOffS = plightBase->lmFallOffS;
	plight->lmFallOffPenumbra = plightBase->lmFallOffPenumbra;
	plight->lmFallOffAbsX = plightBase->lmFallOffAbsX;
	plight->lmFallOffAbsY = plightBase->lmFallOffAbsY;

	// Clone the frustrum vector array (size 6)
	for (int i = 0; i < 6; ++i)
		plight->avecFrustrum[i] = plightBase->avecFrustrum[i];

	// Clone the grfzonBeam (int value)
	plight->grfzonBeam = plightBase->grfzonBeam;

	// Clone the dynamic flags (int values)
	plight->fDynamic = plightBase->fDynamic;
	plight->fExcludeDynamicObjects = plightBase->fExcludeDynamicObjects;

	if (plight->fDynamic == 1)
		allSwDynamicLights.push_back(plight);

	numSwLights++;
}

void AddLightToSw(LIGHT* plight)
{
	AppendDlEntry(&plight->psw->dlLight, plight);

	plight->pvtalo->pfnUpdateAloXfWorld(plight);
}

void FitLinearFunction(float x0, float y0, float x1, float y1, float& pdu, float& pru)
{
	bool abs = FFloatsNear(x0, x1, 0.0001);

	if (abs == 0)
	{
		float fVar2 = (y1 - y0) / (x1 - x0);
		pru = fVar2;
		pdu = y0 - fVar2 * x0;
	}

	else
	{
		pru = 0.0;
		pdu = y0;
	}
}

void FitRecipFunction(float x0, float y0, float x1, float y1, float* pdu, float* pru)
{
	float fVar2 = 0.0001;
	bool ABS = FFloatsNear(x0, x1, 0.0001);

	if (ABS == 0) {
		if (fVar2 <= abs(x0))
		{
			if (fVar2 <= abs(x1))
			{
				fVar2 = (x0 * x1 * (y1 - y0)) / (x0 - x1);
				*pru = fVar2;
				*pdu = y0 - fVar2 / x0;
				return;
			}

			*pru = 0.0;
		}

		else
			*pru = 0.0;
	}

	else
		*pru = 0.0;

	*pdu = y0;
}

void ConvertFallOff(LM* plm, float* pdu, float* pru)
{
	FitLinearFunction(plm->gMin, 1.0, plm->gMax, 0.0, *pdu, *pru);
}

void ConvertAngleStrength(float deg0, float g0, float deg1, float g1, float& pdu, float& pru)
{
	float cos0 = cosf(deg0 * 0.008726647);
	float cos1 = cosf(deg1 * 0.008726647);

	FitLinearFunction(cos0 + cos0 * cos0 * cos0, g0 * 0.003921569, cos1 + cos1 * cos1 * cos1, g1 * 0.003921569, pdu, pru);
}

void RebuildLight(LIGHT* plight)
{
	if (plight->degHighlight > 0.0001)
	{
		if (plight->vecHighlight.z <= 0.0001)
		{
			if (0.0001 < plight->degMidtone)
			{
				if (0.0001 < plight->gMidtone)
					plight->twps = TWPS_ShadowMidtone;

				else
					plight->twps = TWPS_Shadow;
			}

			else
				plight->twps = TWPS_Shadow;
		}

		plight->twps = TWPS_ShadowMidtoneSaturate;
	}

	else
	{
		if (plight->degMidtone > 0.0001)
		{
			if (0.0001 < plight->gMidtone)
				plight->twps = TWPS_ShadowMidtone;

			else
				plight->twps = TWPS_Shadow;
		}

		else
			plight->twps = TWPS_Shadow;
	}

	glm::vec3 rgba{};

	if (plight->vecHighlight.z > 0.0001)
	{
		ConvertUserHsvToUserRgb(plight->vecHighlight, rgba);

		plight->agFallOff = rgba;
		plight->agFallOff.x = 1.0 / plight->vecHighlight.z;
		plight->agFallOff.y = 0.0;

		rgba = rgba * plight->agFallOff.x;
	}
	else
		rgba = glm::vec3(0.0f);

	plight->rgbaColor = rgba;

	ConvertAngleStrength(plight->degHighlight, 0.0, 0.0, plight->vecHighlight.z, plight->ltfn.duHighlight, plight->ltfn.ruHighlight);
	ConvertAngleStrength(plight->degMidtone, 0.0, 0.0, plight->gMidtone, plight->ltfn.duMidtone, plight->ltfn.ruMidtone);
	ConvertAngleStrength(360.0 - plight->degShadow, 0.0, 360.0, plight->gShadow, plight->ltfn.duShadow, plight->ltfn.ruShadow);

	if (plight->lightk == LIGHTK_Position)
		FitRecipFunction(plight->lmFallOffS.gMin, 1.0, plight->lmFallOffS.gMax, 0.0, &plight->agFallOff.x, &plight->agFallOff.y);

	if (plight->lightk == LIGHTK_Frustrum || plight->lightk == LIGHTK_Spot)
	{
		glm::mat4* pmat = &plight->matLookAt;
		glm::vec4* anormalFrustrum = plight->avecFrustrum;

		glm::vec3 dirWorld;
		glm::vec3 vecUp;

		// local to world
		ConvertAloVec(plight, nullptr, &plight->normalLocal, &dirWorld);
		ConvertAloVec(plight, nullptr, &plight->vecUpLocal, &vecUp);

		//flip direction
		dirWorld = -dirWorld;

		// Build basis: Z = forward
		glm::vec3 f = glm::normalize(dirWorld);
		glm::vec3 r = glm::normalize(glm::cross(vecUp, f));
		if (!glm::all(glm::isfinite(r)) || glm::length2(r) < 1e-10f)
		{
			glm::vec3 alt = (std::abs(f.z) < 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
			r = glm::normalize(glm::cross(alt, f));
		}
		glm::vec3 u = glm::normalize(glm::cross(f, r));

		*pmat = glm::mat4(1.0f);
		(*pmat)[0] = glm::vec4(r, 0.0f);
		(*pmat)[1] = glm::vec4(u, 0.0f);
		(*pmat)[2] = glm::vec4(f, 0.0f);

		const glm::vec3 X = r;
		const glm::vec3 Y = u;
		const glm::vec3 Z = f;

		// Approximate BuildFrustrum
		anormalFrustrum[0] = glm::vec4(glm::normalize(Z + plight->rx * X), 0.0f); // Left
		anormalFrustrum[1] = glm::vec4(glm::normalize(Z - plight->rx * X), 0.0f); // Right
		anormalFrustrum[2] = glm::vec4(glm::normalize(Z + plight->ry * Y), 0.0f); // Top
		anormalFrustrum[3] = glm::vec4(glm::normalize(Z - plight->ry * Y), 0.0f); // Bottom

		const glm::vec3 eye = plight->xf.posWorld;

		// world-space plane w terms
		for (int i = 0; i < 4; ++i)
		{
			glm::vec3 N = glm::vec3(anormalFrustrum[i]);
			anormalFrustrum[i].w = -glm::dot(eye, N);
		}

		// Distance planes: both along Z (forward), different offsets
		const float gMax = plight->lmFallOffS.gMax;
		{
			// plane 4: far-ish, using gMax
			glm::vec3 N = Z;
			float dotPos = glm::dot(N, eye);
			plight->avecFrustrum[4] = glm::vec4(N, -(gMax + dotPos));
		}
		{
			// plane 5: fixed 50.0 slice along same normal
			glm::vec3 N = Z;
			float dotPos = glm::dot(N, eye);
			plight->avecFrustrum[5] = glm::vec4(N, -(50.0f + dotPos));
		}

		// Projection (still using your convention)
		const float fovy = 2.0f * std::atan(plight->ry);
		const float aspect = plight->rx / plight->ry;
		const float zNear = 50.0f;
		const float zFar = gMax;

		glm::mat4 matProj = glm::perspectiveRH_ZO(fovy, aspect, zNear, zFar);
		glm::mat4 matView = glm::lookAtRH(eye, eye + Z, u);
		glm::mat4 matFrustrum = matProj * matView;

		plight->frustum = matFrustrum;

		// Falloff packing: same mapping you already had
		if (plight->lightk == LIGHTK_Frustrum)
		{
			float bx, sx; ConvertFallOff(&plight->lmFallOffAbsX, &bx, &sx);
			float by, sy; ConvertFallOff(&plight->lmFallOffAbsY, &by, &sy);
			float bs, ss; ConvertFallOff(&plight->lmFallOffS, &bs, &ss);

			plight->falloffBias  = glm::vec4(bx, by, 1.0f, bs);
			plight->falloffScale = glm::vec4(sx, sy, 0.0f, ss);
		}
		else // LIGHTK_Spot
		{
			float bp, sp; ConvertFallOff(&plight->lmFallOffPenumbra, &bp, &sp);
			float bs, ss; ConvertFallOff(&plight->lmFallOffS, &bs, &ss);

			plight->falloffBias  = glm::vec4(1.0f, 1.0f, bp, bs);
			plight->falloffScale = glm::vec4(0.0f, 0.0f, sp, ss);
		}
	}
}

void* GetLightKind(LIGHT* plight)
{
	return &plight->lightk;
}

void SetLightKind(LIGHT* plight, LIGHTK lightk)
{
	plight->lightk = lightk;
	plight->fNoFreeze = (lightk == LIGHTK_Direction);
	RebuildLight(plight);
	UpdateLightBeamGrfzon(plight);
}

void* GetLightHighlightColor(LIGHT* plight)
{
	return &plight->vecHighlight;
}

void SetLightHighlightColor(LIGHT* plight, glm::vec3& pvecHighlight)
{
	plight->vecHighlight = pvecHighlight;
	RebuildLight(plight);
}

void* GetLightMidtoneStrength(LIGHT* plight)
{
	return &plight->gMidtone;
}

void SetLightMidtoneStrength(LIGHT* plight, float gMidtone)
{
	plight->gMidtone = gMidtone;
	RebuildLight(plight);
}

void* GetLightShadowStrength(LIGHT* plight)
{
	return &plight->gShadow;
}

void SetLightShadowStrength(LIGHT* plight, float gShadow)
{
	plight->gShadow = gShadow;
	RebuildLight(plight);
}

void* GetLightHighlightAngle(LIGHT* plight)
{
	return &plight->degHighlight;
}

void SetLightHighlightAngle(LIGHT* plight, float degHighlight)
{
	plight->degHighlight = degHighlight;
	RebuildLight(plight);
}

void* GetLightMidtoneAngle(LIGHT* plight)
{
	return &plight->degMidtone;
}

void SetLightMidtoneAngle(LIGHT* plight, float degMidtone)
{
	plight->degMidtone = degMidtone;
	RebuildLight(plight);
}

void* GetLightShadowAngle(LIGHT* plight)
{
	return &plight->degShadow;
}

void SetLightShadowAngle(LIGHT* plight, float degShadow)
{
	plight->degShadow = degShadow;
	RebuildLight(plight);
}

void* GetLightDirection(LIGHT* plight)
{
	return &plight->vecDirectionOrig;
}

void SetLightDirection(LIGHT* plight, glm::vec3& pvecDirection)
{
	plight->vecDirectionOrig = pvecDirection;
	RebuildLight(plight);
	UpdateLightBeamGrfzon(plight);
}

void* GetLightDynamic(LIGHT* plight)
{
	return &plight->fDynamic;
}

void SetLightDynamic(LIGHT* plight, int fDynamic)
{
	if (fDynamic != plight->fDynamic)
	{
		plight->fDynamic = fDynamic;
		RebuildLight(plight);
		allSwDynamicLights.push_back(plight);
	}
}

void* GetLightFallOff(LIGHT* plight)
{
	return &plight->lmFallOffS;
}

void SetLightFallOff(LIGHT* plight, LM* plm)
{
	plight->lmFallOffS = *plm;
	RebuildLight(plight);
	UpdateLightBeamGrfzon(plight);
}

void* GetLightConeAngle(LIGHT* plight)
{
	return &plight->degCone;
}

void SetLightConeAngle(LIGHT* plight, float degCone)
{
	plight->degCone = degCone;
	RebuildLightFrustrum(plight);
	RebuildLight(plight);
	UpdateLightBeamGrfzon(plight);
}

void* GetLightHotSpotAngle(LIGHT* plight)
{
	return &plight->degHotSpot;
}

void SetLightHotSpotAngle(LIGHT* plight, float degHotSpot)
{
	plight->degHotSpot = degHotSpot;
	RebuildLightFrustrum(plight);
	RebuildLight(plight);
}

void* GetLightFrustrumUp(LIGHT* plight)
{
	return &plight->vecUpLocal;
}

void SetLightFrustrumUp(LIGHT* plight, glm::vec3& pvecUpLocal)
{
	plight->vecUpLocal = pvecUpLocal;
	RebuildLight(plight);
	UpdateLightBeamGrfzon(plight);
}

void RemoveLightFromSw(LIGHT* plight)
{
	RemoveDlEntry(&plight->psw->dlLight, plight);
}

void CreateSwDefaultLights(SW* psw)
{
	LIGHT* plight;

	if ((g_grfdfl & 1U) != 0)
	{
		plight = (LIGHT*)PloNew(CID_LIGHT, psw, nullptr, (OID)0x201, -1);
		SetLightHighlightColor(plight, g_vecHighlight);
		SetLightHighlightAngle(plight, g_degHighlight);
		SetLightMidtoneStrength(plight, g_gMidtone);
		SetLightMidtoneAngle(plight, g_degMidtone);
		SetLightShadowStrength(plight, g_gShadow);
		SetLightShadowAngle(plight, g_degShadow);
		SetLightDirection(plight, s_vecDirectionDefault);
	}

	if ((g_grfdfl & 2U) != 0)
	{
		plight = (LIGHT*)PloNew(CID_LIGHT, psw, nullptr, (OID)0x201, -1);
		SetLightKind(plight, LIGHTK_Position);
		SetLightHighlightColor(plight, g_vecHighlight);
		SetLightHighlightAngle(plight, g_degHighlight);
		SetLightMidtoneStrength(plight, g_gMidtone);
		SetLightMidtoneAngle(plight, g_degMidtone);
		SetLightShadowStrength(plight, g_gShadow);
		SetLightShadowAngle(plight, g_degShadow);
		SetLightFallOff(plight, &s_lmFallOffDefault);
		plight->pvtlight->pfnTranslateAloToPos(plight, s_posDefault);
	}

	if ((g_grfdfl & 4U) != 0)
	{
		plight = (LIGHT*)PloNew(CID_LIGHT, psw, nullptr, (OID)0x201, -1);
		SetLightKind(plight, LIGHTK_Frustrum);
		SetLightHighlightColor(plight, g_vecHighlight);
		SetLightHighlightAngle(plight, g_degHighlight);
		SetLightMidtoneStrength(plight, g_gMidtone);
		SetLightMidtoneAngle(plight, g_degMidtone);
		SetLightShadowStrength(plight, g_gShadow);
		SetLightShadowAngle(plight, g_degShadow);
		SetLightDirection(plight, s_vecDirectionDefault);
		SetLightFallOff(plight, &s_lmFallOffDefault);
		plight->pvtlight->pfnTranslateAloToPos(plight, s_posDefault);
	}

	if ((g_grfdfl & 8U) != 0)
	{
		plight = (LIGHT*)PloNew(CID_LIGHT, psw, nullptr, (OID)0x201, -1);
		SetLightKind(plight, LIGHTK_Spot);
		SetLightHighlightColor(plight, g_vecHighlight);
		SetLightHighlightAngle(plight, g_degHighlight);
		SetLightMidtoneStrength(plight, g_gMidtone);
		SetLightMidtoneAngle(plight, g_degMidtone);
		SetLightShadowStrength(plight, g_gShadow);
		SetLightShadowAngle(plight, g_degShadow);
		SetLightDirection(plight, s_vecDirectionDefault);
		SetLightFallOff(plight, &s_lmFallOffDefault);
		plight->pvtlight->pfnTranslateAloToPos(plight, s_posDefault);
	}
}

static GLsizeiptr AlignUp(GLsizeiptr v, GLsizeiptr a) { return (v + a - 1) & ~(a - 1); }

void AllocateLightBlkList()
{
	if (g_psw->dlLight.plightFirst == nullptr)
		return;

	lightBlk.resize(numSwLights);

	LIGHT* plight = g_psw->dlLight.plightFirst;
	int idx = 0;

	while (plight != nullptr && idx < (int)lightBlk.size())
	{
		switch (plight->lightk)
		{
			case LIGHTK_Direction:
		
			lightBlk[idx].lightk   = plight->lightk;
			lightBlk[idx].fDynamic = plight->fDynamic;
			lightBlk[idx].dir      = glm::vec4(plight->xf.matWorld[2], 0.0f);
			lightBlk[idx].color    = glm::vec4(plight->rgbaColor, 0.0f);
			lightBlk[idx].ru       = glm::vec4(plight->ltfn.ruShadow, plight->ltfn.ruMidtone, plight->ltfn.ruHighlight, 0.0f);
			lightBlk[idx].du       = glm::vec4(plight->ltfn.duShadow, plight->ltfn.duMidtone, plight->ltfn.duHighlight, 0.0f);
			break;

			case LIGHTK_Position:
		
			lightBlk[idx].lightk   = plight->lightk;
			lightBlk[idx].fDynamic = plight->fDynamic;
			lightBlk[idx].pos      = glm::vec4(plight->xf.posWorld, 1.0f);
			lightBlk[idx].color    = glm::vec4(plight->rgbaColor, 1.0f);
			lightBlk[idx].constant = plight->agFallOff.x;
			lightBlk[idx].invDst   = plight->agFallOff.y;
			lightBlk[idx].dst      = plight->lmFallOffS.gMax;
			lightBlk[idx].ru       = glm::vec4(plight->ltfn.ruShadow, plight->ltfn.ruMidtone, plight->ltfn.ruHighlight, 0.0f);
			lightBlk[idx].du       = glm::vec4(plight->ltfn.duShadow, plight->ltfn.duMidtone, plight->ltfn.duHighlight, 0.0f);
			break;

			case LIGHTK_Frustrum:
			case LIGHTK_Spot:
			lightBlk[idx].lightk       = plight->lightk;
			lightBlk[idx].fDynamic     = plight->fDynamic;
			lightBlk[idx].pos          = glm::vec4(plight->xf.posWorld, 1.0f);
			lightBlk[idx].color        = glm::vec4(plight->rgbaColor, 1.0f);
			lightBlk[idx].matFrustrum  = plight->frustum;
			lightBlk[idx].falloffScale = plight->falloffScale;
			lightBlk[idx].falloffBias  = plight->falloffBias;
			lightBlk[idx].ru           = glm::vec4(plight->ltfn.ruShadow, plight->ltfn.ruMidtone, plight->ltfn.ruHighlight, 0.0f);
			lightBlk[idx].du           = glm::vec4(plight->ltfn.duShadow, plight->ltfn.duMidtone, plight->ltfn.duHighlight, 0.0f);
			break;
		}

		plight = plight->dleLight.plightNext;
		++idx;
	}

	// ------------------------------------------------------------
	// Main light SSBO
	// ------------------------------------------------------------
	glGenBuffers(1, &g_lightSsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_lightSsbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_lightSsbo);

	const GLsizeiptr headerSize = sizeof(LightSSBOHeader);
	const GLsizeiptr lightsSize = sizeof(LIGHTBLK) * lightBlk.size();
	const GLsizeiptr totalSize = headerSize + lightsSize;

	glBufferData(GL_SHADER_STORAGE_BUFFER, totalSize, nullptr, GL_DYNAMIC_DRAW);

	LightSSBOHeader header = {};
	header.numLights = (int)lightBlk.size();

	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, headerSize, &header);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, headerSize, lightsSize, lightBlk.data());

	// ------------------------------------------------------------
	// Active-lights SSBO
	// ------------------------------------------------------------
	glGenBuffers(1, &g_activeLightsSsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_activeLightsSsbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, g_activeLightsSsbo);

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ACTIVELIGHTS), &activeLights, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

bool FindSwDynamicLights(glm::vec3* pposCenter, float sRadius)
{
	glm::vec3 center = *pposCenter;

	for (const LIGHT*plight : allSwDynamicLights)
	{
		if (g_fBsp == 1 && ((plight->grfzon & g_pcm->grfzon) != g_pcm->grfzon))
			continue;

		switch (plight->lightk)
		{
			case LIGHTK_Direction:
			{
				// Direction lights affect everything
				return true;
			}

			case LIGHTK_Position:
			{
				glm::vec3 d = glm::vec3(plight->xf.posWorld) - center;
				float r = plight->lmFallOffS.gMax + sRadius;

				if (glm::dot(d, d) <= r * r)
					return true;

				break;
			}

			case LIGHTK_Frustrum:
			case LIGHTK_Spot:
			{
				bool inside = true;

				glm::mat4 M = plight->frustum;

				// Extract clip planes (same pattern as your GLSL)
				glm::vec4 planes[6];
				planes[0] = M[3] + M[0]; // left
				planes[1] = M[3] - M[0]; // right
				planes[2] = M[3] + M[1]; // bottom
				planes[3] = M[3] - M[1]; // top
				planes[4] = M[3] + M[2]; // near
				planes[5] = M[3] - M[2]; // far

				for (int i = 0; i < 6; ++i)
				{
					glm::vec3 n = glm::vec3(planes[i]);
					float len = glm::length(n);
					if (len < 1e-6f)
						continue; // degenerate plane, ignore

					n /= len;
					float d = planes[i].w / len;

					float dist = glm::dot(n, center) + d;

					if (dist < -sRadius)
					{
						inside = false;
						break;
					}
				}

				if (inside)
					return true;

				break;
			}

			default:
			break;
		}
	}

	return false;
}

void PrepareSwLights(SW* psw, CM* pcm)
{
	bool useZones = (g_fBsp != 0);
	GLsizeiptr headerSize = sizeof(LightSSBOHeader);

	activeLights.numStaticLights  = 0;
	activeLights.numDynamicLights = 0;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_lightSsbo);

	int idx = 0;
	for (LIGHT* plight = psw->dlLight.plightFirst; plight; plight = plight->dleLight.plightNext, ++idx)
	{
		if (idx >= MAX_LIGHTS)
			break;

		if (useZones && ((plight->grfzon & pcm->grfzon) != pcm->grfzon))
			continue;

		if (plight->fDynamic)
		{
			if (activeLights.numDynamicLights < MAX_LIGHTS)
			{
				activeLights.dynamicLightIndices[activeLights.numDynamicLights++] = idx;
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, headerSize + (GLintptr)(sizeof(LIGHTBLK) * idx), sizeof(LIGHTBLK), &lightBlk[idx]);
			}
		}
		else
		{
			if (activeLights.numStaticLights < MAX_LIGHTS)
				activeLights.staticLightIndices[activeLights.numStaticLights++] = idx;
		}
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_activeLightsSsbo);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ACTIVELIGHTS), &activeLights);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void DeallocateLightBlkList()
{
	lightBlk.clear();
	lightBlk.shrink_to_fit();

	allSwDynamicLights.clear();
	allSwDynamicLights.shrink_to_fit();

	glDeleteBuffers(1, &g_lightSsbo);
	glDeleteBuffers(1, &g_activeLightsSsbo);
}

void DeleteLight(LIGHT* plight)
{
	delete plight;
}

void DeallocateLightVector()
{
	allSwLights.clear();
	allSwLights.shrink_to_fit();
}

std::vector <LIGHT*> allSwLights;
std::vector<LIGHT*> allSwDynamicLights;
ACTIVELIGHTS activeLights;
GLuint g_lightSsbo;
GLuint g_activeLightsSsbo;
int numSwLights;
std::vector <LIGHTBLK> lightBlk;
glm::vec3 g_vecHighlight = glm::vec3(0.0, 255.0, 255.0);
float g_degHighlight = 0.0;
float g_gMidtone = 255.0;
float g_degMidtone = 240.0;
float g_gShadow = 0.0;
float g_degShadow = 180.0;
glm::vec3 s_vecDirectionDefault = glm::vec3(-0.2, 0.3, -1.0);
LM s_lmFallOffDefault{ 500, 2500 };
glm::vec3 s_posDefault = glm::vec3(100, -150, 500);
GRFDFL g_grfdfl = 1;