#pragma once
#include "glshaders.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/ImGuiFileDialog.h"
#include "mouse.h"

enum AspectMode
{
	FitToScreen,
	Fixed_4_3,
	Fixed_16_9,
	Fixed_16_10,
};

struct CMGL
{
	glm::mat4 matWorldToClip;
	glm::vec4 cameraPos;
};

struct alignas(16) ROGL
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
};

struct alignas(16) ROCEL
{
	glm::mat4 model;
	glm::vec4 celRgba;
	float     uAlphaCelBorder;
};

struct STREAM
{
	GLuint bufferObject;
	GLuint bindIndex;
	GLsizeiptr frameBase;
	GLsizeiptr cursor;
	GLsizeiptr stride; // use GLsizeiptr, not GLuint
	GLsizeiptr frameSize; // bytes in this frame slice
	uint8_t* mappedPtr;
	GLsync* fences; // length = g_frames
};

class GL
{
	public:
	// Window Object
	GLFWwindow* window;
	// Frame Buffer Object
	GLuint fbo;
	// Frame Buffer Color
	GLuint fbc;
	// Render Buffer Object
	GLuint rbo;

	GLuint dyshFbo;
	GLuint dyshFbc;
	GLuint dyshRbo;

	int dyshWidth;
	int dyshHeight;

	GLuint fboMSAA;
	GLuint rboColorMSAA;
	GLuint rboDepthStencilMSAA;

	// Screen Array Object
	GLuint sao;
	// Screen Buffer Object
	GLuint sbo;
	glm::mat4 screenProjection;

	// Text Array Object
	GLuint gao;
	// Text Buffer Object
	GLuint gbo;
	GLuint geo;
	glm::mat4 blotProjection;

	// Window width
	float width;
	// Window height
	float height;

	float aspectRatio;
	AspectMode aspectMode;

	void InitGL();
	void CreateFramebuffers(int w, int h);
	void ResizeFramebuffers(int w, int h);
	void UpdateGLProjections();
	void TerminateGL();
};

void InitCameraUbo();
void InitFrameStream(STREAM* pstream, int streamSize);
void BeginFrameStream(STREAM* pstream);
void AppendStream(STREAM* pstream, void* ptr, int size, int copySize);
void EndFrameStream(STREAM* pstream);
void DeleteFrameStream(STREAM* pstream);
void ApplyMsaaSettings();
void FrameBufferSizeCallBack(GLFWwindow* window, int width, int height);

extern GL g_gl;
extern GLuint cmUBO;
extern STREAM ropStream;
extern STREAM rcbStream;
extern GLuint geomUBO;
extern GLuint glslLsmShadow;
extern GLuint glslLsmDiffuse;
extern GLuint glslFogType;
extern GLuint glslFogNear;
extern GLuint glslFogFar;
extern GLuint glslFogMax;
extern GLuint glslFogColor;
extern GLuint glslfAlphaTest;
extern GLuint glslAlphaCutOff;
extern GLuint glslRko;
extern GLuint glslfAnimateUv;
extern GLuint glsluvOffsets;
extern GLuint glslUnSelfIllum;
extern GLuint glslSubGlobPosCenter;
extern GLuint glslSubGlobRadius;
extern GLuint glslDyshMatWorldClip;
extern GLuint glslDyshModel;
extern GLuint glslAmbientMap;
extern GLuint glslDiffuseMap;
extern GLuint glslSaturateMap;
extern GLuint glslGeomModelToClip;
extern GLuint u_fontTexLoc;
extern GLuint glslfCull;
extern uint64_t screenTextureHandle;
extern GLuint g_sceneFbo;
extern int g_msaaSamples;
extern bool g_fMsaa;
extern int g_frames;