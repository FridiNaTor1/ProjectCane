#pragma once
#include "bis.h"
#include "gl.h"
#include "util.h"

enum TWPS
{
	TWPS_Shadow = 0,
	TWPS_ShadowMidtone = 1,
	TWPS_ShadowMidtoneSaturate = 2
};

enum RTCK
{
	RTCK_Nil = -1,
	RTCK_None = 0,
	RTCK_All = 1,
	RTCK_WorldZ = 2,
	RTCK_LocalX = 3,
	RTCK_LocalY = 4,
	RTCK_LocalZ = 5,
	RTCK_Max = 6
};

enum WEK
{
	WEK_Nil = -1,
	WEK_XYZ = 0,
	WEK_XY = 1,
	WEK_XZ = 2,
	WEK_YZ = 3,
	WEK_X = 4,
	WEK_Y = 5,
	WEK_Z = 6,
	WEK_Max = 7
};

enum WARPTYPE
{
	WARP_NONE = 0,
	WARP_POS = 1,
	WARP_UV = 2,
	WARP_BOTH = 3
};

enum TRLK
{
	TRLK_Relight = 0,
	TRLK_Baked = 1,
	TRLK_Dynamic = 2
};

struct VERTICE
{
	glm::vec3  pos;
	glm::vec3  normal;
	glm::vec4  color;
	glm::vec2  uv;
	glm::uvec4 boneIndices;
	glm::vec4  boneWeights;
};

struct INDICE
{
	uint16_t v1;
	uint16_t v2;
	uint16_t v3;
};

struct alignas(16) RO
{
	glm::mat4 model;         //   0 -  63
	float     uAlpha;        //  64 -  67
	float     uFog;          //  68 -  71
	float     darken;        //  72 -  75
	int32_t   grfglob;       //  76 -  79
	int32_t   pad0;          //  80 -  83
	int32_t   warpType;      //  84 -  87
	int32_t   warpCmat;      //  88 -  91
	int32_t   warpCvtx;      //  92 -  95
	glm::mat4 amatDpos[4];   //  96 - 351
	glm::mat4 amatDuv[4];    // 352 - 607
	int32_t   fDynamic;      // 608 - 611
	float     sRadius;       // 612 - 615
	int32_t   fDynamicLight; // 616 - 619
	int32_t   trlk;          // 620 - 623
	glm::vec4 posCenter;     // 624 - 639
	float     uAlphaCelBorder;
};

// Render Priority List
struct RPL
{
	RP rp;

	int fTransluscentSort;
	float z;

	union
	{
		ALO  *palo;
		DYSH *pdysh;
	};

	GLOB *pglob;

	RO ro;
};

struct RSGLD
{
	int rko;
	int fAnimateUv;
	glm::vec2 uvOffsets;
	float unSelfIllum;

	uint32_t firstIndex;
	uint32_t indexCount;
	int32_t  baseVertex;

	uint64_t shadowMapHandle;
	uint64_t diffuseMapHandle;
	uint64_t saturateMapHandle;
};

struct alignas(16) MATERIAL
{
	float ambient;
	glm::vec3 _pad0;
	glm::vec4 midtone;
	glm::vec4 light;
};

struct RPLCEL
{
	RP rp;

	GLuint edgeCount;
	GLuint edgeSSBO;

	ROCEL rocel;
};

// Vertex Flag
struct VTXFLG
{
	// Vertex Index
	byte ipos;
	// Normal Index
	byte inormal;
	// UV Index
	byte iuv;
	// Strip Flag
	byte bMisc;
};

struct SUBCEL
{
	std::vector <glm::vec3> positions;
	std::vector <uint16_t>  indices;
	std::vector <float>     weights;
};

struct SUBPOSEF
{
	std::vector <uint16_t> aiposf;
	std::vector <uint16_t> ainormalf;
};

struct TWEF
{
	uint32_t aipos0;
	uint32_t aipos1;
	uint32_t aipos2;
	uint32_t aipos3;
};

struct WEKI
{
	WEK wek;
	float sInner;
	float uInner;
	float sOuter;
	float uOuter;
	glm::mat4 dmat;
};

struct WRBG
{
	struct ALO* palo;
	struct GLOB* pglob;
	OID oid;
	struct WR* pwr;
	int cmat;
	int fDpos;
	int fDuv;
	WARPTYPE warpType;
	WEKI weki;
	std::shared_ptr <WRBG> pwrbgNextGlobset;
	std::shared_ptr <WRBG> pwrbgNextWr;
};

struct GLEAM
{
	glm::vec3 normal;
	CLQC clqc;
};

struct FGFN
{
	float duFogBias;
	float ruFog;
	float sNearFog;
	float duFogPlusClipBias;
};

struct WRBGLOB_GL
{
	WR* pwr;
	int cmat;
	int vertexCount;

	std::vector <glm::vec4> basePos; // size = vertexCount
	std::vector <glm::vec4> state;   // size = cmat * vertexCount

	GLuint ssboState;
};

struct SUBGLOB
{
	uint32_t firstIndex;
	uint32_t indexCount;
	int32_t  baseVertex;

	std::vector <VERTICE> vertices;
	std::vector <INDICE>  indices;

	glm::vec3 posCenter;
	float sRadius;
	// Object brightness
	float unSelfIllum;
	// Shader ID
	int shdID;
	// Object shader property
	struct SHD* pshd;
	int cibnd;
	std::vector <int> aibnd;
	SAI *uvSai;
	bool usesUvAnim;
};

struct SUBGLOBI
{
	float tShadowsValid;
	int cpshadow;
	struct SHADOW* apshadow[4];
};

// Model data
struct GLOB // NOT DONE
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	GLuint ssboCachedMaterial;

	void (*PFNDRAW)(int, int, int);

	// Moodel origin position
	glm::vec3 posCenter;
	float sRadius;
	RP rp;
	int fThreeWay;
	int fTransluscentSort;
	byte grfshd;
	TRLK trlk;
	float sMRD;
	float sCelBorderMRD;
	float gZOrder;
	int fDynamic;
	GRFGLOB grfglob;
	std::shared_ptr <GLEAM> gleam;
	RTCK rtck;
	struct SAA* psaa;
	// Object fog intensity
	float uFog;
	FGFN fgfn;
	float rSubglobRadius;
	WR* pwr;
	std::shared_ptr <WRBG> pwrbg;
	std::shared_ptr<WRBGLOB_GL> pwarpGlob;
	// Number of submodels for model
	int csubglob;
	std::vector <SUBGLOB> asubglob;
	int csubcel;
	std::vector <SUBCEL> asubcel;
	// SSBO: 4 vec4 per edge (E0, E1, OA, OB)
	GLuint edgeSSBO;  // GL_SHADER_STORAGE_BUFFER
	GLsizei edgeCount; // number of edges (== ctwef)
	// Ptr to instance model matrix
	std::shared_ptr <glm::mat4> pdmat;
	struct BLOT* pblot;
	OID oid;
	char* pchzName;
};

struct GLOBI
{
	int grfzon;
	SUBGLOBI asubglobi;
	int cframeStaticLights;
	TWPS twps;
	float uAlpha;
	float tUnfade;
};

struct GLOBSET
{
	int cbnd;
	struct BND* abnd;
	std::vector <OID> mpibndoid;
	uint64_t cglob;
	std::vector <GLOB>  aglob;
	std::vector <GLOBI> aglobi;
	uint32_t grfglobset;
	glm::vec4 rgbaCel;
	int cpose;
	std::vector <float> agPoses;
	std::vector <float> agPosesOrig;
	std::shared_ptr <WRBG> pwrbgFirst;
	int cpsaa;
	std::vector <SAA*> apsaa;
};

// Loads 3D model data from binary file
void LoadGlobsetFromBrx(GLOBSET* pglobset, ALO* palo, CBinaryInputStream* pbis);
// Converts strips to tri lists and stores 3D sub model in VRAM
void BuildSubGlob(GLOB* pglob, SUBGLOB* psubglob, SHD* pshd, std::vector <glm::vec3>& positions, std::vector <glm::vec3>& normals, std::vector <glm::vec4>& colors, std::vector <glm::vec2>& texcoords, std::vector <VTXFLG>& indexes, SUBPOSEF* subposef, std::vector <glm::vec3>& aposfPoses, std::vector <glm::vec3>& anormalfPoses, std::vector <float>& agWeights, int fDynamic);
void BuildSubcel(GLOBSET* pglobset, GLOB* pglob, SUBCEL* psubcel, int cposf, std::vector <glm::vec3>& aposf, int ctwef, std::vector <TWEF>& atwef, std::vector <SUBPOSEF>& asubposef, std::vector <glm::vec3>& aposfPoses, std::vector <float>& agWeights, std::vector <glm::vec4>& totalEdges);
void BuildGlobsetSaaArray(GLOBSET* pglobset);
void PostGlobsetLoad(GLOBSET* pglobset, ALO* palo);
void UpdateGlobset(GLOBSET* pglobset, ALO* palo, float dt);

extern int  g_fogType;
extern bool g_fRenderModels;
extern bool g_fRenderCollision;
extern bool g_fRenderCelBorders;
extern bool g_fBsp;
extern float g_uAlpha;
extern SMP s_smpFade;
extern SMP g_smpAlphaFade;
extern glm::vec4 g_rgbaCel;