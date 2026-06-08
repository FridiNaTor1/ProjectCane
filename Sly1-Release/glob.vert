#version 430 core

#define RKO_OneWay   0
#define RKO_ThreeWay 1

#define WARP_NONE 0
#define WARP_POS  1
#define WARP_UV   2
#define WARP_BOTH 3

#define LIGHTK_Direction 0
#define LIGHTK_Position  1
#define LIGHTK_Frustrum  2
#define LIGHTK_Spot      3

#define MAX_LIGHTS 256
#define MAX_SHADOWS 255
#define MAX_OBJECT_SHADOWS 16

#define TRLK_Relight 0
#define TRLK_Baked   1
#define TRLK_Dynamic 2

#define FOG_NONE 0
#define FOG_PS2  1
#define FOG_PS3  2

layout (location = 0) in vec3  vertex;
layout (location = 1) in vec3  normal;
layout (location = 2) in vec4  color;
layout (location = 3) in vec2  uv;
layout (location = 4) in uvec4 boneIndices;
layout (location = 5) in vec4  boneWeights;

struct SWP // Scene world properties
{
    float uShadow;
    float uMidtone;
    int   fogType;
    float fogNear;
    float fogFar;
    float fogMax;
    vec4  fogColor;
}; uniform SWP swp;

struct LIGHT
{
    int   lightk;
    int   pad1;
    int   pad2;
    int   fDynamic;
    vec4  pos;
    vec4  dir;
    vec4  color;
    float constant;
    float invDst;
    float pad4;
    float dst;
    vec4  ru;
    vec4  du;
    mat4  matFrustrum;
    vec4  falloffScale;
    vec4  falloffBias;
};

struct SHADOW
{
    mat4  matWorldToUv;
    mat4  matClipToUv;
    vec4  rgba;
    float wMin;
    float wMax;
    float gReserved;
    float wFadeMin;
    uvec2 textureHandle;
    uvec2 _pad0;
    vec4  posEffect;
    float sRadiusEffect;
    int   fDynamic;
    int   shdk;
    int   _pad1;
    vec4  normalCast;
};

struct MATERIAL
{
    float ambient;
    vec4  midtone;
    vec4  light;
};

layout(std140, binding = 0) uniform CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

layout(std140, binding = 1) uniform RO
{
    mat4  model; 
    float uAlpha;
    float uFog;
    float darken;
    int   grfglob;
    int   pad0;
    int   warpType;
    int   warpCmat;
    int   warpCvtx;
    mat4  amatDpos[4];
    mat4  amatDuv[4];
    int   fDynamic;
    float sRadius;
    int   fDynamicLight;
    int   trlk;
    vec4  posCenter;
} op;

layout(std430, binding = 2) readonly buffer LIGHTBLK
{   
    int numLights;
    int pad[3];
    LIGHT lights[];
};

layout(std430, binding = 3) readonly buffer ACTIVELIGHTS
{
    int numStaticLights;
    int staticLightIndices[MAX_LIGHTS];
    int numDynamicLights;
    int dynamicLightIndices[MAX_LIGHTS];
};

layout(std430, binding = 4) readonly buffer SHADOWBLK
{
    int numLevelShadows;
    int padShadowBlk[3];
    SHADOW shadows[];
};

layout(std430, binding = 5) readonly buffer ACTIVESHADOWS
{
    int numActiveShadows;
    int shadowIndices[MAX_SHADOWS];
};

layout(std430, binding = 6) readonly buffer WARPSTATE
{
    vec4 warpState[];
};

layout(std430, binding = 7) buffer CACHEDLIGHTING
{
    MATERIAL cachedMaterial[];
};

uniform int   rko;
uniform int   fAnimateUv;
uniform vec2  uvOffsets;
uniform float unSelfIllum;

uniform vec3  subGlobPosCenter;
uniform float subGlobRadius;

vec3 normalWorld;
vec2 uvLocal;

float objectShadow;
float objectMidtone;
vec3  light;

MATERIAL baseMaterial;

out vec4 worldPos;
out vec3 worldNormal;
out vec4 vertexColor;
out vec2 texcoord;

out MATERIAL material;
flat out int vShadowCount;
flat out int vShadowIndices[MAX_OBJECT_SHADOWS];
out float fogIntensity;

void ApplyWarp(inout vec4 vLocal, inout vec2 uvLocal);
void StartThreeWay();
void InitGlobLighting();
void ApplyStaticLightsRelight();
void ApplyStaticLights();
void ApplyDynamicLights();
void AddDynamicMaterial();
vec4 AddDirectionLight(LIGHT dirlight);
vec4 AddDynamicLight(vec4 dir, vec4 color, vec4 ru, vec4 du);
bool SphereIntersectsPositionLight(LIGHT L, vec3 center, float radius);
vec4 AddPositionLight(LIGHT pointlight);
vec4 AddPositionLightDynamic(LIGHT pointlight);
bool SphereIntersectsFrustum(LIGHT L, vec3 center, float radius);
vec4 AddFrustrumLight(LIGHT frustumlight);
vec4 AddFrustrumLightDynamic(LIGHT frustumlight);
bool FShadowValid(SHADOW shadow, int grfglob);
bool ShadowIntersectsSphere(SHADOW shadow, vec3 objectCenter, float objectRadius);
void CacheWriteMaterial();
void CacheReadMaterial();
void ProcessGlobLighting();
void CalculateFog();
void CalculateFogPS2();
void CalculateFogPS3();

void main()
{
    worldPos    = op.model * vec4(vertex, 1.0);
    worldNormal = normalize(mat3(op.model) * normal);
    uvLocal     = uv.xy;

    if (fAnimateUv > 0)
        uvLocal += uvOffsets;

    if (op.warpType != WARP_NONE)
        ApplyWarp(worldPos, uvLocal);

    vertexColor = color;
    texcoord    = uvLocal;

    if (rko == RKO_ThreeWay)
        StartThreeWay();

    vShadowCount = 0;

    if (numActiveShadows > 0)
    {
        int shadowCount = min(numActiveShadows, MAX_SHADOWS);

        for (int i = 0; i < shadowCount; ++i)
        {
            int shIdx = shadowIndices[i];
            SHADOW shadow = shadows[shIdx];

            if (!FShadowValid(shadow, op.grfglob))
                continue;

            if (!ShadowIntersectsSphere(shadow, subGlobPosCenter, subGlobRadius))
                continue;

            if (vShadowCount < MAX_OBJECT_SHADOWS)
            {
                vShadowIndices[vShadowCount] = shIdx;
                vShadowCount++;
            }
        }
    }

    if (swp.fogType != FOG_NONE)
        CalculateFog();

    gl_Position = cm.matWorldToClip * worldPos;
}

void ApplyWarp(inout vec4 vLocal, inout vec2 uvLocal)
{
    switch(op.warpType)
    {
        case WARP_POS:
        vec4 sumPos0 = vec4(0.0);

        for (int imat = 0; imat < op.warpCmat; ++imat)
        {
            vec4 st  = warpState[imat * op.warpCvtx + gl_VertexID];
            sumPos0 += op.amatDpos[imat] * st;
        }

        vLocal.xyz += sumPos0.xyz;
        break;

        case WARP_UV:
        vec2 sumUv0 = vec2(0.0);

        for (int imat = 0; imat < op.warpCmat; ++imat)
        {
             vec4 st = warpState[imat * op.warpCvtx + gl_VertexID];
             sumUv0 += (op.amatDuv[imat] * st).xy;
        }

        uvLocal += sumUv0;
        break;

        case WARP_BOTH:
        vec4 sumPos1 = vec4(0.0);
        vec2 sumUv1  = vec2(0.0);

        for (int imat = 0; imat < op.warpCmat; ++imat)
        {
             vec4 st  = warpState[imat * op.warpCvtx + gl_VertexID];
             sumPos1 += op.amatDpos[imat] * st;
             sumUv1  += (op.amatDuv[imat] * st).xy;
        }

        vLocal.xyz += sumPos1.xyz;
        uvLocal    += sumUv1;
        break;
    }
}

void StartThreeWay()
{
    switch(op.trlk)
    {
        // Static bake
        case TRLK_Relight:
        InitGlobLighting();
        ApplyStaticLightsRelight();
        ProcessGlobLighting();
        // Bake static lights
        CacheWriteMaterial();
        // Show dynamic this frame
        if (op.fDynamicLight == 1)
            AddDynamicMaterial();
        break;

        case TRLK_Baked:
        // Read cached static
        CacheReadMaterial();
        // Dynamic on top (PS2 DownloadRelight idea)
        if (op.fDynamicLight == 1)
            AddDynamicMaterial();
        break;

        case TRLK_Dynamic:
        default:
        // Full relight (no cache)
        InitGlobLighting();
        ApplyStaticLights();
        ApplyDynamicLights();
        ProcessGlobLighting();
        break;
    };
}

void InitGlobLighting()
{
    objectShadow  = swp.uShadow;
    objectMidtone = swp.uMidtone + unSelfIllum * 0.000031;
    light = vec3(0.0);

    normalWorld = normalize(mat3(op.model) * normal);
}

void ApplyStaticLightsRelight()
{
    for (int i = 0; i < numLights; ++i)
    {
        if (lights[i].fDynamic > 0)
            continue;

        switch (lights[i].lightk)
        {
            case LIGHTK_Direction:
            light.rgb += AddDirectionLight(lights[i]).rgb;
            break;

            case LIGHTK_Position:
            if (!SphereIntersectsPositionLight(lights[i], op.posCenter.xyz, op.sRadius))
                continue;

            light.rgb += AddPositionLight(lights[i]).rgb;
            break;

            case LIGHTK_Frustrum:
            case LIGHTK_Spot:
            if (!SphereIntersectsFrustum(lights[i], op.posCenter.xyz, op.sRadius))
                continue;

            light.rgb += AddFrustrumLight(lights[i]).rgb;
            break;
        }
    }
}

void ApplyStaticLights()
{
    if (numStaticLights == 0)
        return;

    if (op.fDynamic != 1)
    {
        for (int i = 0; i < numStaticLights; ++i)
        {
            int idx = staticLightIndices[i];

            switch (lights[idx].lightk)
            {
                case LIGHTK_Direction:
                light.rgb += AddDirectionLight(lights[idx]).rgb;
                break;

                case LIGHTK_Position:
                if (!SphereIntersectsPositionLight(lights[idx], op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddPositionLight(lights[idx]).rgb;
                break;

                case LIGHTK_Frustrum:
                case LIGHTK_Spot:
                if (!SphereIntersectsFrustum(lights[idx], op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddFrustrumLight(lights[idx]).rgb;
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < numStaticLights; ++i)
        {
            int idx = staticLightIndices[i];

            switch (lights[idx].lightk)
            {
                case LIGHTK_Direction:
                light.rgb += AddDynamicLight(lights[idx].dir, lights[idx].color, lights[idx].ru, lights[idx].du).rgb;
                break;

                case LIGHTK_Position:
                if (!SphereIntersectsPositionLight(lights[idx], op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddPositionLightDynamic(lights[idx]).rgb;
                break;

                case LIGHTK_Frustrum:
                case LIGHTK_Spot:
                if (!SphereIntersectsFrustum(lights[idx], op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddFrustrumLightDynamic(lights[idx]).rgb;
                break;
            }
        }
    }
}

void ApplyDynamicLights()
{
    if (numDynamicLights == 0)
        return;

    if (op.fDynamic != 1)
    {
        for (int i = 0; i < numDynamicLights; ++i)
        {
            int idx = dynamicLightIndices[i];
            switch (lights[idx].lightk)
            {
                case LIGHTK_Direction:
                light.rgb += AddDirectionLight(lights[idx]).rgb;
                break;

                case LIGHTK_Position:
                if (!SphereIntersectsPositionLight(lights[idx], op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddPositionLight(lights[idx]).rgb;
                break;

                case LIGHTK_Frustrum:
                case LIGHTK_Spot:
                if (!SphereIntersectsFrustum(lights[idx], op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddFrustrumLight(lights[idx]).rgb;
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < numDynamicLights; ++i)
        {
            int idx = dynamicLightIndices[i];
            switch (lights[idx].lightk)
            {
                case LIGHTK_Direction:
                light.rgb += AddDynamicLight(lights[idx].dir, lights[idx].color, lights[idx].ru, lights[idx].du).rgb;
                break;

                case LIGHTK_Position:
                if (!SphereIntersectsPositionLight(lights[idx], op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddPositionLightDynamic(lights[idx]).rgb;
                break;

                case LIGHTK_Frustrum:
                case LIGHTK_Spot:
                if (!SphereIntersectsFrustum(lights[idx], op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddFrustrumLightDynamic(lights[idx]).rgb;
                break;
            }
        }
    }
}

void AddDynamicMaterial()
{
    // Save cached base
    baseMaterial = material;

    // Compute dynamic-only into material
    objectShadow  = 0.0;
    objectMidtone = 0.0;
    light         = vec3(0.0);

    normalWorld   = normalize(mat3(op.model) * normal);

    ApplyDynamicLights();
    ProcessGlobLighting(); // material = dynamic-only buckets

    // Add on top
    material.ambient = baseMaterial.ambient + material.ambient;
    material.midtone = baseMaterial.midtone + material.midtone;
    material.light   = baseMaterial.light   + material.light;
}

vec4 AddDirectionLight(LIGHT dirlight)
{
    vec3 L = normalize(dirlight.dir.xyz);

    float f = dot(normalWorld, L);

    // PS2 curve: dot + dot^3
    f = f + f * f * f;

    float shadow    = max(f * dirlight.ru.x + dirlight.du.x, 0.0);
    float midtone   = max(f * dirlight.ru.y + dirlight.du.y, 0.0);
    float highlight = max(f * dirlight.ru.z + dirlight.du.z, 0.0);

    objectShadow  += shadow;
    objectMidtone += midtone;

    return vec4(dirlight.color * highlight);
}

vec4 AddDynamicLight(vec4 dir, vec4 color, vec4 ru, vec4 du)
{
    vec3 lightDir = normalize(mat3(transpose(op.model)) * vec3(dir));
    float diffuse = dot(lightDir, normal);
    diffuse += diffuse * diffuse * diffuse;

    float shadow    = diffuse * ru.x + du.x;
    float midtone   = diffuse * ru.y + du.y;
    float highlight = diffuse * ru.z + du.z;

    objectShadow  += max(shadow,  0.0);
    objectMidtone += max(midtone, 0.0);
    highlight      = max(highlight, 0.0);

    return color * highlight;
}

bool SphereIntersectsPositionLight(LIGHT L, vec3 center, float radius)
{
    // Vector from light to object center
    vec3 toLight = L.pos.xyz - center;

    // Squared distance between centers
    float distSq = dot(toLight, toLight);

    // Position-light influence radius:
    // L.dst = light falloff (gMax), same as PS2 lmFallOffS.gMax
    float maxDist = L.dst + radius;   // linear units

    // Compare using squared distance
    return distSq <= maxDist * maxDist;
}

vec4 AddPositionLight(LIGHT pointlight)
{
    vec3 toLight = pointlight.pos.xyz - worldPos.xyz;

    float invLen = inversesqrt(dot(toLight, toLight));
    vec3 L = toLight * invLen;

    vec3 N = normalWorld;
    float n2 = dot(N, N);

    if (n2 >= 0.00000001)
        N *= inversesqrt(n2);
    else
        N = vec3(0.0);

    float ndotl = dot(N, L);

    float diffuse = ndotl + ndotl * ndotl * ndotl;

    float attenuation = clamp(pointlight.constant + pointlight.invDst * invLen, 0.0, 1.0);

    float shadow    = max(diffuse * pointlight.ru.x + pointlight.du.x, 0.0) * attenuation;
    float midtone   = max(diffuse * pointlight.ru.y + pointlight.du.y, 0.0) * attenuation;
    float highlight = max(diffuse * pointlight.ru.z + pointlight.du.z, 0.0) * attenuation;

    objectShadow  += shadow;
    objectMidtone += midtone;

    return vec4(pointlight.color * highlight);
}

vec4 AddPositionLightDynamic(LIGHT pointlight)
{
    vec3  direction = pointlight.pos.xyz - op.posCenter.xyz;
    float distance = length(direction);

    float attenuation = 1.0 / distance * pointlight.invDst + pointlight.constant;
    attenuation = clamp(attenuation, 0.0, 1.0);

    vec4 color = AddDynamicLight(vec4(direction, 0.0), pointlight.color * attenuation, pointlight.ru * attenuation, pointlight.du * attenuation) * attenuation;

    return color;
}

bool SphereIntersectsFrustum(LIGHT L, vec3 center, float radius)
{
    mat4 M = L.matFrustrum;

    vec4 planes[6];
    planes[0] = M[3] + M[0];
    planes[1] = M[3] - M[0];
    planes[2] = M[3] + M[1];
    planes[3] = M[3] - M[1];
    planes[4] = M[3] + M[2];
    planes[5] = M[3] - M[2];

    for (int i = 0; i < 6; ++i)
    {
        vec3 n = planes[i].xyz;
        float len = length(n);

        if (len < 1e-6) continue;

        n /= len;
        float d = planes[i].w / len;

        float dist = dot(n, center) + d;
        if (dist < -radius)
            return false;
    }

    return true;
}

vec4 AddFrustrumLight(LIGHT frustumlight)
{
    vec4 clipL = frustumlight.matFrustrum * vec4(worldPos.xyz, 1.0);

    if (clipL.w <= 0.0) return vec4(0.0);
    if (abs(clipL.x) > clipL.w) return vec4(0.0);
    if (abs(clipL.y) > clipL.w) return vec4(0.0);
    if (clipL.z < 0.0 || clipL.z > clipL.w) return vec4(0.0);

    float invW = 1.0 / clipL.w;

    float u  = abs(clipL.x * invW);
    float v  = abs(clipL.y * invW);
    float r2 = u * u + v * v;
    float wv = clipL.w;

    float fx = clamp(frustumlight.falloffScale.x * u  + frustumlight.falloffBias.x, 0.0, 1.0);
    float fy = clamp(frustumlight.falloffScale.y * v  + frustumlight.falloffBias.y, 0.0, 1.0);
    float fr = clamp(frustumlight.falloffScale.z * r2 + frustumlight.falloffBias.z, 0.0, 1.0);
    float fw = clamp(frustumlight.falloffScale.w * wv + frustumlight.falloffBias.w, 0.0, 1.0);

    float mask = fx * fy * fr * fw;
    if (mask <= 0.0) return vec4(0.0);

    vec3 Ldir = frustumlight.dir.xyz;
    float len2 = dot(Ldir, Ldir);
    Ldir = (len2 > 1e-8) ? Ldir * inversesqrt(len2) : vec3(0.0);

    float NL = dot(normalWorld, Ldir);
    NL = NL + NL*NL*NL;

    float shadow    = max(0.0, NL * frustumlight.ru.x + frustumlight.du.x);
    float midtone   = max(0.0, NL * frustumlight.ru.y + frustumlight.du.y);
    float highlight = max(0.0, NL * frustumlight.ru.z + frustumlight.du.z);

    objectShadow  += shadow  * mask;
    objectMidtone += midtone * mask;

    return frustumlight.color * (highlight * mask);
}

vec4 AddFrustrumLightDynamic(LIGHT frustumlight)
{
    vec4 clipL = frustumlight.matFrustrum * vec4(op.posCenter.xyz, 1.0);

    if (clipL.w <= 0.0) return vec4(0.0);
    if (abs(clipL.x) > clipL.w) return vec4(0.0);
    if (abs(clipL.y) > clipL.w) return vec4(0.0);
    if (clipL.z < 0.0 || clipL.z > clipL.w) return vec4(0.0);

    float invW = 1.0 / clipL.w;

    float u  = abs(clipL.x * invW);
    float v  = abs(clipL.y * invW);
    float r2 = u * u + v * v;
    float wv = clipL.w;

    float fx = clamp(frustumlight.falloffScale.x * u  + frustumlight.falloffBias.x, 0.0, 1.0);
    float fy = clamp(frustumlight.falloffScale.y * v  + frustumlight.falloffBias.y, 0.0, 1.0);
    float fr = clamp(frustumlight.falloffScale.z * r2 + frustumlight.falloffBias.z, 0.0, 1.0);
    float fw = clamp(frustumlight.falloffScale.w * wv + frustumlight.falloffBias.w, 0.0, 1.0);

    float mask = fx * fy * fr * fw;
    if (mask <= 0.0) return vec4(0.0);

    vec4 ruScaled = frustumlight.ru * mask;
    vec4 duScaled = frustumlight.du * mask;

    return AddDynamicLight(frustumlight.dir, frustumlight.color, ruScaled, duScaled);
}

bool FShadowValid(SHADOW shadow, int grfglob)
{
    if ((grfglob & 1) != 0 && shadow.shdk == 2)
        return true;

    if ((grfglob & 2) != 0 && shadow.shdk == 3)
        return true;

    return false;
}

bool ShadowIntersectsSphere(SHADOW shadow, vec3 objectCenter, float objectRadius)
{
    if (shadow.fDynamic == 1)
    {
        vec3 N = normalize(worldNormal);
        vec3 castDir = normalize(shadow.normalCast.xyz);

        if (dot(N, -castDir) <= -0.5)
            return false;
    }

    vec3  d = objectCenter - vec3(shadow.posEffect);
    float r = objectRadius + shadow.sRadiusEffect;

    return dot(d, d) <= r * r;
}

void CacheWriteMaterial()
{
    cachedMaterial[gl_VertexID].ambient = material.ambient;
    cachedMaterial[gl_VertexID].midtone = material.midtone;
    cachedMaterial[gl_VertexID].light   = material.light;
}

void CacheReadMaterial()
{
    material.ambient = cachedMaterial[gl_VertexID].ambient;
    material.midtone = cachedMaterial[gl_VertexID].midtone;
    material.light   = cachedMaterial[gl_VertexID].light;
}

void ProcessGlobLighting()
{
    // Find the brightest channel in the light color (used to gauge overall intensity)
    float dominantLight = max(max(light.r, light.g), light.b);
    // Invert brightness to get potential shadow strength (brighter light = less shadow)
    float shadowModifier = 1.0 - dominantLight;
    // Clamp the object's midtone value so it doesn't exceed remaining shadow range
    float clampedMidtone = clamp(objectMidtone, 0.0, shadowModifier);
    // Determine how much of the remaining light can go toward shadows (bounded by objectShadow)
    float shadowContribution = max(min(shadowModifier - clampedMidtone, objectShadow), 0.0);
    // Approximate base light intensity from average of vertex color (grayscale luminance)
    float baseIntensity = dot(vertexColor.rgb, vec3(0.3333333));

    // Assign lighting output
    material.ambient   = shadowContribution * baseIntensity;
    material.midtone.rgb = clampedMidtone * vertexColor.rgb;
    material.light.rgb = min(light.rgb, vec3(1.0)) * baseIntensity;
}

void CalculateFog()
{
    switch(swp.fogType)
    {
        case FOG_PS2:
        CalculateFogPS2();
        break;

        case FOG_PS3:
        CalculateFogPS3();
        break;
    }
}

void CalculateFogPS2()
{
    float z = length(cm.cameraPos.xyz - worldPos.xyz);
    float recipZ = 1.0 / max(z, 1e-4);

    float recipNear = 1.0 / swp.fogNear;
    float recipFar  = 1.0 / swp.fogFar;

    float denom = max(recipNear - recipFar, 1e-6);
    float fog   = clamp((recipNear - recipZ) * (1.0 / denom), 0.0, 1.0);

    float fogMult = mix(swp.fogMax, swp.fogMax * op.uFog, step(0.001, op.uFog));
    fogIntensity  = clamp((fog * fogMult) * swp.fogColor.a, 0.0, 1.0);
}

void CalculateFogPS3()
{
    vec3 offset = cm.cameraPos.xyz - worldPos.xyz;

    float distance2 = dot(offset, offset);
    float distanceToCamera = sqrt(distance2);

    float invFogRange = 1.0 / max(swp.fogFar - swp.fogNear, 1e-6);
    float fog = clamp((distanceToCamera - swp.fogNear) * invFogRange, 0.0, 1.0);

    float fogMult = mix(swp.fogMax, swp.fogMax * op.uFog, step(0.001, op.uFog));
    fogIntensity  = clamp((fog * fogMult) * swp.fogColor.a, 0.0, 1.0);
}