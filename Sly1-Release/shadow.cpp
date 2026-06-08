#include "shadow.h"

void InitSwShadowDl(SW* psw)
{
	InitDl(&psw->dlShadow, offsetof(SHADOW, dle));
}

void InitShadow(SHADOW* pshadow)
{
	pshadow->sNearRadius = -1.0f;
	pshadow->sFarRadius = -1.0f;
	pshadow->sNearCast = 100.0f;
	pshadow->sFarCast = 400.0f;
	pshadow->oidDysh = OID_Nil;

	// Initialize up vector to g_normalY
	pshadow->vecUp = glm::vec3(0.0, 1.0, 0.0);

	// Compute normalCast = g_normalZ * -1.0f
	pshadow->normalCast = glm::vec3(-0.0, -0.0, -1.0);
	//pshadow->normalCast.gUnused = -g_normalZ.gUnused;
}

void SetShadowShader(SHADOW* pshadow, OID oidShdShadow)
{
	OID oid = (OID)0x2C;

	if (oidShdShadow != OID_Nil)
		oid = oidShdShadow;

	pshadow->pshd = PshdFindShader(oid);
}

void SetShadowNearRadius(SHADOW* pshadow, float sNearRadius)
{
	pshadow->sNearRadius = sNearRadius;
	if (pshadow->sFarRadius < 0.0) {
		pshadow->sFarRadius = sNearRadius;
	}
}

void SetShadowFarRadius(SHADOW* pshadow, float sFarRadius)
{
	pshadow->sFarRadius = sFarRadius;
	if (pshadow->sNearRadius < 0.0) {
		pshadow->sNearRadius = sFarRadius;
	}
}

void SetShadowNearCast(SHADOW* pshadow, float sNearCast)
{
	pshadow->sNearCast = sNearCast;
	RebuildShadowRegion(pshadow);
}

void SetShadowFarCast(SHADOW* pshadow, float sFarCast)
{
	pshadow->sFarCast = sFarCast;
	RebuildShadowRegion(pshadow);
}

void SetShadowConeAngle(SHADOW* pshadow, float degConeAngle)
{
	// Convert degrees to radians: 0.008726647 / 360
	float coneTangent = std::tanf(degConeAngle * 0.008726647f);

	pshadow->sNearRadius = coneTangent * pshadow->sNearCast;
	pshadow->sFarRadius = coneTangent * pshadow->sFarCast;
}

void SetShadowFrustrumUp(SHADOW* pshadow, glm::vec3* pvecUp)
{
	pshadow->vecUp = *pvecUp;
}

void SetShadowCastPosition(SHADOW* pshadow, const glm::vec3& posCast)
{
	if (!glm::all(glm::epsilonEqual(pshadow->posCast, posCast, 0.0001f))) {
		pshadow->posCast = posCast;
		RebuildShadowRegion(pshadow);
	}
}

void SetShadowCastNormal(SHADOW* pshadow, const glm::vec3& normalCast)
{
	// Avoid unnecessary updates if the normal hasn't changed significantly
	if (!glm::all(glm::epsilonEqual(pshadow->normalCast, normalCast, 0.0001f))) {
		pshadow->normalCast = normalCast;
		RebuildShadowRegion(pshadow);
	}
}

int FShadowValid(SHADOW* pshadow, GRFGLOB grfglob)
{
	if ((grfglob & 1) != 0 && pshadow->pshd->shdk == 2)
		return true;

	if ((grfglob & 2) != 0 && pshadow->pshd->shdk == 3)
		return true;

	return false;
}

int FShadowRadiusSet(SHADOW* shadow)
{
	return (shadow->sNearRadius >= 0.0f && shadow->sFarRadius >= 0.0f);
}

int FShadowIntersectsSphere(SHADOW* pshadow, const glm::vec3& pos, float sRadius)
{
	uint32_t zon = pshadow->pshd->grfzon;
	bool zoneValid = (zon & 0x10000000) != 0 || (g_pcm->grfzon & zon) == g_pcm->grfzon;

	if (!zoneValid) return 0;

	// Convert posEffect (assumed to be glm::vec4 or VECTOR) to vec3 for distance
	glm::vec3 shadowPos = glm::vec3(pshadow->posEffect.x, pshadow->posEffect.y, pshadow->posEffect.z);

	float totalRadius = sRadius + pshadow->sRadiusEffect;
	float distSq = glm::distance2(pos, shadowPos);

	return distSq <= totalRadius * totalRadius;
}

void FindSwShadows(SW* psw, glm::vec3* ppos, float sRadius, int cpshadowMax, int* pcpshadow, SHADOW** apshadow)
{
	int cpshadow = 0;
	SHADOW* pshadow = psw->dlShadow.pshadowFirst;

	while (pshadow != nullptr)
	{
		if (FShadowIntersectsSphere(pshadow, *ppos, sRadius))
		{
			if (cpshadow >= cpshadowMax)
			{
				*pcpshadow = cpshadow;
				return;
			}

			*apshadow = pshadow;
			++apshadow;
			++cpshadow;
		}

		pshadow = pshadow->dle.pshadowNext;
	}

	*pcpshadow = cpshadow;
}

void PostShadowLoad(SHADOW* pshadow)
{
	if (pshadow->pshd == nullptr)
		SetShadowShader(pshadow, (OID)0x2C);

	if (!FShadowRadiusSet(pshadow))
	{
		SetShadowNearRadius(pshadow, 100.0f);
		SetShadowFarRadius(pshadow, 400.0f);
	}
}

void RebuildShadowRegion(SHADOW* pshadow)
{
	float nearRadius = pshadow->sNearRadius;
	float farRadius = pshadow->sFarRadius;
	float nearCast = pshadow->sNearCast;
	float farCast = pshadow->sFarCast;

	float effectDistance = 0.0f;
	float effectRadius = 0.0f;

	if (!FFloatsNear(nearRadius, farRadius, 0.0001f))
	{
		float dCast = farCast - nearCast;

		// Solve for the base offset of the shadow parabola
		float base = (nearCast * farRadius - farCast * nearRadius) / (nearRadius - farRadius);

		// Compute the midpoint position along the parabola
		float height = std::sqrt(dCast * dCast + farRadius * farRadius);
		float z1 = ((nearCast + base) * height) / dCast;
		float focus = (z1 * (z1 + 0.5f * height)) / (nearCast + base);

		// Compute final effect position and radius
		float z2 = (farCast + base) - focus;
		effectRadius = std::sqrt(z2 * z2 + farRadius * farRadius);
		effectDistance = focus - base;
	}
	else
	{
		// Fallback: cone case or degenerate radius
		float halfLength = std::abs(farCast - nearCast) * 0.5f;
		effectDistance = nearCast + halfLength;
		effectRadius = halfLength;
	}

	pshadow->sRadiusEffect = effectRadius;

	// Calculate posEffect = posCast + normalCast * effectDistance
	pshadow->posEffect.x = pshadow->posCast.x + pshadow->normalCast.x * effectDistance;
	pshadow->posEffect.y = pshadow->posCast.y + pshadow->normalCast.y * effectDistance;
	pshadow->posEffect.z = pshadow->posCast.z + pshadow->normalCast.z * effectDistance;
	//pshadow->posEffect.gUnused = pshadow->posCast.gUnused + pshadow->normalCast.gUnused * effectDistance;
}

void CombineShadowEyeLookAtProj(const glm::vec3& posEye, const glm::mat3& matLookAt, const glm::mat4& matProj, glm::mat4& out)
{
	glm::mat4 mat(1.0f);

	mat[0] = glm::vec4(-matLookAt[1], 0.0f);
	mat[1] = glm::vec4(-matLookAt[2], 0.0f);
	mat[2] = glm::vec4(matLookAt[0], 0.0f);
	mat[3] = glm::vec4(posEye, 1.0f);

	out = matProj * glm::inverse(mat);
}

void UpdateShadow(SHADOW* pshadow, float dt)
{

}

void RebuildShadow(SHADOW* pshadow)
{
	constexpr float kFadeDistance = 50.0f;

	glm::vec3 posCast = pshadow->posCast;
	glm::vec3 normalCast = pshadow->normalCast;
	glm::vec3 vecUpRef = pshadow->vecUp;

	glm::mat3 matLookAt(1.0f);
	BuildOrthonormalMatrixZ(normalCast, vecUpRef, matLookAt);

	const float sNearCast = pshadow->sNearCast;
	const float sFarCast = pshadow->sFarCast;
	const float sNearRadius = pshadow->sNearRadius;
	const float sFarRadius = pshadow->sFarRadius;

	glm::vec3 posEye = posCast;
	glm::mat4 matProj(1.0f);

	const bool isPerspective = !FFloatsNear(sNearRadius, sFarRadius, 0.0001f);

	if (isPerspective)
	{
		float eyeOffset = (sFarRadius * sNearCast - sNearRadius * sFarCast) / (sFarRadius - sNearRadius);

		float sNear = sNearCast - eyeOffset;
		float sFar = sFarCast - eyeOffset;

		posEye = posCast + normalCast * eyeOffset;

		float scale = 0.4f / (sNearRadius / sNear);

		BuildSimpleProjectionMatrix(scale, scale, 0.5f, 0.5f, sNear, sFar, matProj);

		pshadow->rsh.wMin = std::min(sNear, sFar);
		pshadow->rsh.wMax = std::max(sNear, sFar);
		pshadow->rsh.wFadeMin = pshadow->rsh.wMin - kFadeDistance;
	}
	else
	{
		posEye = posCast;

		matProj = glm::mat4(1.0f);

		matProj[0][0] = 0.5f / sNearRadius;
		matProj[1][1] = 0.5f / sNearRadius;

		matProj[3][0] = 0.5f;
		matProj[3][1] = 0.5f;

		pshadow->rsh.wMin = -FLT_MAX;
		pshadow->rsh.wMax = FLT_MAX;
		pshadow->rsh.wFadeMin = sNearCast - kFadeDistance;
	}

	//CombineEyeLookAtProj(posEye, matLookAt, matProj, pshadow->matWorldToUv);
	CombineShadowEyeLookAtProj(posEye, matLookAt, matProj, pshadow->matWorldToUv);

	pshadow->rsh.matWorldToUv = pshadow->matWorldToUv;
	pshadow->rsh.matClipToUv = glm::mat4(1.0f);

	pshadow->rsh.rgba = pshadow->pshd->rgba;

	pshadow->rsh.shdk = pshadow->pshd->shdk;

	pshadow->rsh.posEffect = glm::vec4(pshadow->posCast, 1.0f);
	pshadow->rsh.sRadiusEffect = pshadow->sRadiusEffect;

	pshadow->rsh.normalCast = glm::vec4(pshadow->normalCast, 0.0f);

	if (pshadow->rsh.fDynamic == 0)
	{
		GLuint64 handle = pshadow->pshd->atex[0].abmp[0]->hDiffuseMap;

		pshadow->rsh.textureHandle[0] = uint32_t(handle & 0xFFFFFFFFull);
		pshadow->rsh.textureHandle[1] = uint32_t(handle >> 32);
	}
}

void AllocateShadows(SW* psw)
{
	SHADOW* pshadow = psw->dlShadow.pshadowFirst;
	int idx = 0;

	while (pshadow != nullptr)
	{
		RebuildShadow(pshadow);

		pshadow->ssboIndex = idx;
		shadowBlk.push_back(pshadow->rsh);

		pshadow = pshadow->dle.pshadowNext;
		++idx;
	}

	// ------------------------------------------------------------
	// Main shadows SSBO
	// ------------------------------------------------------------
	glGenBuffers(1, &shadowSsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shadowSsbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, shadowSsbo);

	const GLsizeiptr headerSize = sizeof(SHADOWSSBOHEADER);
	const GLsizeiptr shadowSize = sizeof(SHADOWBLK) * shadowBlk.size();
	const GLsizeiptr totalSize = headerSize + shadowSize;

	glBufferData(GL_SHADER_STORAGE_BUFFER, totalSize, nullptr, GL_DYNAMIC_DRAW);

	SHADOWSSBOHEADER header = {};
	header.numShadows = (int)shadowBlk.size();

	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, headerSize, &header);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, headerSize, shadowSize, shadowBlk.data());

	// ------------------------------------------------------------
	// Active-shadows SSBO
	// ------------------------------------------------------------
	glGenBuffers(1, &activeShadowsSsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, activeShadowsSsbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, activeShadowsSsbo);

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ACTIVESHADOWS), &activeShadows, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void PrepareSwShadows(SW* psw, CM* pcm)
{
	//GLsizeiptr headerSize = sizeof(SHADOWSSBOHEADER);

	activeShadows.numShadows = 0;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shadowSsbo);

	int idx = 0;
	for (SHADOW* pshadow = psw->dlShadow.pshadowFirst; pshadow; pshadow = pshadow->dle.pshadowNext, ++idx)
	{
		if (idx >= MAX_SHADOWS)
			break;

		if (g_fBsp > 0)
		{
			uint32_t grfzon = pshadow->pshd->grfzon;
			bool visibleInZone = ((grfzon & 0x10000000u) != 0) || ((pcm->grfzon & grfzon) == pcm->grfzon);

			if (!visibleInZone)
				continue;
		}

		activeShadows.shadowsIndices[activeShadows.numShadows++] = idx;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, activeShadowsSsbo);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ACTIVESHADOWS), &activeShadows);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void DeallocateSwShadows()
{
	glDeleteBuffers(1, &activeShadowsSsbo);

	shadowBlk.clear();
	shadowBlk.shrink_to_fit();
}

GLuint shadowSsbo = 0;
GLuint activeShadowsSsbo = 0;
std::vector <SHADOWBLK> shadowBlk;
ACTIVESHADOWS activeShadows;
DL g_dlShadowPending;