#include "gl.h"
#include "render.h"

#ifdef _WIN32
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int NvidiaPowerXpressRequestHighPerformance = 1;
}
#endif

void GL::InitGL()
{
	// Create GLFW context and window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	width  = 800;
	height = 800;

	aspectRatio = float(width) / float(height);
	aspectMode = FitToScreen;

	window = glfwCreateWindow(width, height, "Sly 1", nullptr, nullptr);

	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		while (true);
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallBack);
	glfwSetCursorPosCallback(window, MOUSE::CursorPosCallback);
	glfwSetMouseButtonCallback(window, MOUSE::MouseButtonCallback);
	glfwSetScrollCallback(window, MOUSE::MouseWheelCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		while (true);
	}

	// ========== ImGui Setup ==========
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
	ImGui::StyleColorsDark();

	float imguiOffset = ImGui::GetFrameHeight(); // or use a cached value after rendering ImGui menu

	// ========== Framebuffer setup ==========
	CreateFramebuffers(width, height);

	// ========== Screen Quad ==========
	float screen[] = {
		// pos      // tex
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,

		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &sao);
	glGenBuffers(1, &sbo);

	glBindVertexArray(sao);
	glBindBuffer(GL_ARRAY_BUFFER, sbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen), screen, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	glViewport(0, 0, width, height - imguiOffset);

	UpdateGLProjections();

	float quadVertices[] = {
		// x, y,        u, v
		 0.0f, 0.0f,	0.0f, 0.0f, // top-left
		 1.0f, 0.0f,	1.0f, 0.0f, // top-right
		 1.0f, 1.0f,	1.0f, 1.0f, // bottom-right
		 0.0f, 1.0f,	0.0f, 1.0f  // bottom-left
	};

	uint16_t indices[] = {
		0,1,2, 0,2,3
	};

	glGenVertexArrays(1, &gao);
	glGenBuffers(1, &gbo);

	glBindVertexArray(gao);
	glBindBuffer(GL_ARRAY_BUFFER, gbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &geo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	blotProjection = glm::ortho(0.0f, float(width), float(height - imguiOffset), 0.0f, -1.0f, 1.0f);

	glScreenShader.Init("screen.vert", NULL, "screen.frag");
	glScreenShader.Use();
	glUniform1i(glGetUniformLocation(glScreenShader.ID, "screenTexture"), 0);
	//glslScreenSampler = glGetUniformLocation(glScreenShader.ID, "screenTexture");
	//glUniformHandleui64ARB(glslScreenSampler, fbc);

	glDyshadow.Init("dysh.vert", NULL, "dysh.frag");
	glDyshadow.Use();
	
	glslDyshMatWorldClip = glGetUniformLocation(glDyshadow.ID, "matWorldToClip");
	glslDyshModel        = glGetUniformLocation(glDyshadow.ID, "model");

	glGlobShader.Init("glob.vert", NULL, "glob.frag");
	ropStream.bindIndex = 1;
	GLint align = 256;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

	ropStream.stride = (sizeof(ROGL) + align - 1) & ~(align - 1);
	glGlobShader.Use();

	glslLsmShadow  = glGetUniformLocation(glGlobShader.ID, "swp.uShadow");
	glslLsmDiffuse = glGetUniformLocation(glGlobShader.ID, "swp.uMidtone");
	glslFogType    = glGetUniformLocation(glGlobShader.ID, "swp.fogType");
	glslFogNear    = glGetUniformLocation(glGlobShader.ID, "swp.fogNear");
	glslFogFar     = glGetUniformLocation(glGlobShader.ID, "swp.fogFar");
	glslFogMax     = glGetUniformLocation(glGlobShader.ID, "swp.fogMax");
	glslFogColor   = glGetUniformLocation(glGlobShader.ID, "swp.fogColor");

	glslfAlphaTest  = glGetUniformLocation(glGlobShader.ID, "fAlphaTest");
	glslAlphaCutOff = glGetUniformLocation(glGlobShader.ID, "alphaCutOff");

	glslRko              = glGetUniformLocation(glGlobShader.ID, "rko");
	glslfAnimateUv       = glGetUniformLocation(glGlobShader.ID, "fAnimateUv");
	glsluvOffsets		 = glGetUniformLocation(glGlobShader.ID, "uvOffsets");
	glslUnSelfIllum		 = glGetUniformLocation(glGlobShader.ID, "unSelfIllum");
	glslSubGlobPosCenter = glGetUniformLocation(glGlobShader.ID, "subGlobPosCenter");
	glslSubGlobRadius    = glGetUniformLocation(glGlobShader.ID, "subGlobRadius");

	glslAmbientMap  = glGetUniformLocation(glGlobShader.ID, "ambientMap");
	glslDiffuseMap  = glGetUniformLocation(glGlobShader.ID, "diffuseMap");
	glslSaturateMap = glGetUniformLocation(glGlobShader.ID, "saturateMap");

	glCelBorderShader.Init("celborder.vert", NULL, "celborder.frag");
	rcbStream.bindIndex = 1;
	align = 256;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

	rcbStream.stride = (sizeof(ROCEL) + align - 1) & ~(align - 1);
	glGeomShader.Init("geom.vert", NULL, "geom.frag");
	glslGeomModelToClip = glGetUniformLocation(glGeomShader.ID, "modelToClip");

	glBlotShader.Init("blot.vert", NULL, "blot.frag");
	glBlotShader.Use();

	u_projectionLoc = glGetUniformLocation(glBlotShader.ID, "u_projection");
	u_modelLoc      = glGetUniformLocation(glBlotShader.ID, "u_model");
	uvRectLoc       = glGetUniformLocation(glBlotShader.ID, "u_uvRect");
	blotColorLoc    = glGetUniformLocation(glBlotShader.ID, "blotColor");
	u_fontTexLoc    = glGetUniformLocation(glBlotShader.ID, "u_fontTex");

	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

void GL::CreateFramebuffers(int w, int h)
{
	// delete old
	if (fbo)  glDeleteFramebuffers(1, &fbo);
	if (fbc)  glDeleteTextures(1, &fbc);
	if (rbo)  glDeleteRenderbuffers(1, &rbo);

	if (fboMSAA) glDeleteFramebuffers(1, &fboMSAA);
	if (rboColorMSAA) glDeleteRenderbuffers(1, &rboColorMSAA);
	if (rboDepthStencilMSAA) glDeleteRenderbuffers(1, &rboDepthStencilMSAA);

	fbo = fbc = rbo = 0;
	fboMSAA = rboColorMSAA = rboDepthStencilMSAA = 0;

	dyshFbo = 0;
	dyshFbc = 0;
	dyshRbo = 0;

	// resolved FBO (texture)
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &fbc);
	glBindTexture(GL_TEXTURE_2D, fbc);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbc, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Resolved FBO incomplete\n";
		while (true);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// MSAA FBO (renderbuffers)
	if (g_fMsaa)
	{
		GLint maxSamples = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
		g_msaaSamples = std::max(1, std::min(g_msaaSamples, maxSamples));

		glGenFramebuffers(1, &fboMSAA);
		glBindFramebuffer(GL_FRAMEBUFFER, fboMSAA);

		glGenRenderbuffers(1, &rboColorMSAA);
		glBindRenderbuffer(GL_RENDERBUFFER, rboColorMSAA);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_RGBA8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboColorMSAA);

		glGenRenderbuffers(1, &rboDepthStencilMSAA);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencilMSAA);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_DEPTH24_STENCIL8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencilMSAA);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "MSAA FBO incomplete\n";
			while (true);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	dyshWidth = 1024;
	dyshHeight = 1024;

	glGenFramebuffers(1, &dyshFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, dyshFbo);

	glGenRenderbuffers(1, &dyshRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, dyshRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, dyshWidth, dyshHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dyshRbo);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void GL::ResizeFramebuffers(int w, int h)
{
	width  = (float)std::max(1, w);
	height = (float)std::max(1, h);

	// ----------------------------
	// Resolved FBO attachments
	// ----------------------------
	if (fbc)
	{
		glBindTexture(GL_TEXTURE_2D, fbc);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	if (rbo)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	}

	if (fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Resolved FBO incomplete after resize\n";
			while (true) {}
		}
	}

	// ----------------------------
	// MSAA FBO attachments
	// ----------------------------
	if (g_fMsaa && fboMSAA)
	{
		GLint maxSamples = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
		g_msaaSamples = std::max(1, std::min(g_msaaSamples, maxSamples));

		if (rboColorMSAA)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, rboColorMSAA);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_RGBA8, w, h);
		}

		if (rboDepthStencilMSAA)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencilMSAA);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_DEPTH24_STENCIL8, w, h);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fboMSAA);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "MSAA FBO incomplete after resize\n";
			while (true) {}
		}
	}
}

void GL::UpdateGLProjections()
{
	float windowAspect = width / height;

	float scaleX = 1.0f;
	float scaleY = 1.0f;

	if (aspectMode == FitToScreen)
	{
		screenProjection = glm::mat4(1.0f);
	}

	else if (windowAspect > aspectRatio) {
		scaleX = aspectRatio / windowAspect;
		screenProjection = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, 1.0f, 1.0f));
	}
	else {
		scaleY = windowAspect / aspectRatio;
		screenProjection = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, scaleY, 1.0f));
	}
}

void GL::TerminateGL()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &fbc);
	glDeleteRenderbuffers(1, &rbo);
	
	glDeleteFramebuffers(1, &fboMSAA);
	glDeleteTextures(1, &rboColorMSAA);
	glDeleteRenderbuffers(1, &rboDepthStencilMSAA);

	glDeleteFramebuffers(1, &dyshFbo);
	glDeleteTextures(1, &dyshFbc);
	glDeleteRenderbuffers(1, &dyshRbo);

	glDeleteVertexArrays(1, &sao);
	glDeleteBuffers(1, &sbo);
	glDeleteVertexArrays(1, &gao);
	glDeleteBuffers(1, &gbo);
	DeleteFrameStream(&ropStream);
	DeleteFrameStream(&rcbStream);
	glDeleteBuffers(1, &geomUBO);
	glDeleteBuffers(1, &cmUBO);

	glScreenShader.Delete();
	glGlobShader.Delete();
	glCelBorderShader.Delete();
	glGeomShader.Delete();
	glBlotShader.Delete();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void InitCameraUbo()
{
	// Camera UBO
	glGenBuffers(1, &cmUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CMGL), nullptr, GL_DYNAMIC_DRAW);

	GLuint CMGLblockIndex = glGetUniformBlockIndex(glGlobShader.ID, "CMGL");
	glUniformBlockBinding(glGlobShader.ID, CMGLblockIndex, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cmUBO);
}

void InitFrameStream(STREAM* pstream, int streamSize)
{
	if (!pstream->fences)
		pstream->fences = (GLsync*)calloc(g_frames, sizeof(GLsync));

	if (pstream->bufferObject == 0)
		glGenBuffers(1, &pstream->bufferObject);

	glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);

	GLsizeiptr perFrameBytes = (GLsizeiptr)streamSize * pstream->stride;
	GLsizeiptr totalSize = (GLsizeiptr)g_frames * perFrameBytes;

	glBufferData(GL_UNIFORM_BUFFER, totalSize, nullptr, GL_STREAM_DRAW);

	pstream->frameBase = 0;
	pstream->cursor    = 0;
	pstream->frameSize = perFrameBytes;
}

void BeginFrameStream(STREAM* pstream)
{
	int slot = g_cframe % g_frames;

	GLsync s = pstream->fences[slot];
	if (s)
	{
		// poll
		GLenum r = glClientWaitSync(s, 0, 0);
		if (r == GL_TIMEOUT_EXPIRED)
		{
			// only stall if we have to
			glClientWaitSync(s, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
		}
		glDeleteSync(s);
		pstream->fences[slot] = nullptr;
	}

	pstream->frameBase = (GLsizeiptr)slot * pstream->frameSize;
	pstream->cursor = 0;

	glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);
	pstream->mappedPtr = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, pstream->frameBase, pstream->frameSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
}

void AppendStream(STREAM* pstream, void* ptr, int size, int copySize)
{
	if (pstream->cursor + pstream->stride > pstream->frameSize)
	{
		__debugbreak(); // overflow protection
		return;
	}

	GLsizeiptr off = pstream->frameBase + pstream->cursor;
	pstream->cursor += pstream->stride;

	memcpy(pstream->mappedPtr + (off - pstream->frameBase), ptr, copySize);

	glBindBufferRange(GL_UNIFORM_BUFFER, pstream->bindIndex, pstream->bufferObject, off, size);
}

void EndFrameStream(STREAM* pstream)
{
	if (!pstream->mappedPtr)
		return;

	int slot = g_cframe % g_frames;

	glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	pstream->mappedPtr = nullptr;

	pstream->fences[slot] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void DeleteFrameStream(STREAM* pstream)
{
	if (pstream->fences)
	{
		for (int i = 0; i < g_frames; i++)
		{
			if (pstream->fences[i])
				glDeleteSync(pstream->fences[i]);
		}

		free(pstream->fences);
		pstream->fences = nullptr;
	}

	if (pstream->bufferObject)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);

		if (pstream->mappedPtr)
		{
			glUnmapBuffer(GL_UNIFORM_BUFFER);
			pstream->mappedPtr = nullptr;
		}

		glDeleteBuffers(1, &pstream->bufferObject);
		pstream->bufferObject = 0;
	}

	pstream->frameBase = 0;
	pstream->cursor    = 0;
	pstream->frameSize = 0;
}

void ApplyMsaaSettings()
{
	// Use your current render target size (scene size)
	int w = g_gl.width;   // or g_gl.width if that's your scene width
	int h = g_gl.height;   // or g_gl.height if that's your scene height
	w = std::max(w, 1);
	h = std::max(h, 1);

	GLint maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	g_msaaSamples = std::max(1, std::min(g_msaaSamples, maxSamples));

	glGenFramebuffers(1, &g_gl.fboMSAA);
	glBindFramebuffer(GL_FRAMEBUFFER, g_gl.fboMSAA);

	glGenRenderbuffers(1, &g_gl.rboColorMSAA);
	glBindRenderbuffer(GL_RENDERBUFFER, g_gl.rboColorMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_RGBA8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, g_gl.rboColorMSAA);

	glGenRenderbuffers(1, &g_gl.rboDepthStencilMSAA);
	glBindRenderbuffer(GL_RENDERBUFFER, g_gl.rboDepthStencilMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_DEPTH24_STENCIL8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, g_gl.rboDepthStencilMSAA);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "MSAA FBO incomplete\n";
		while (true);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferSizeCallBack(GLFWwindow* window, int width, int height)
{
	float imguiOffset = ImGui::GetFrameHeight();

	int sceneW = width;
	int sceneH = std::max(1, int(float(height) - imguiOffset));

	g_gl.width  = sceneW;
	g_gl.height = sceneH;

	g_gl.ResizeFramebuffers(sceneW, sceneH);

	glViewport(0, 0, sceneW, sceneH);

	g_gl.UpdateGLProjections();

	g_gl.blotProjection = glm::ortho(0.0f, float(sceneW), float(sceneH), 0.0f, -1.0f, 1.0f);

	glBlotShader.Use();
	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

	glGlobShader.Use();

	if (g_pcm != nullptr)
		RecalcCm(g_pcm);

	RepositionAllBlots();
}

GL g_gl;
GLuint cmUBO = 0;
STREAM ropStream;
STREAM rcbStream;
GLuint geomUBO = 0;
GLuint glslLsmShadow = 0;
GLuint glslLsmDiffuse = 0;
GLuint glslFogType = 0;
GLuint glslFogNear = 0;
GLuint glslFogFar = 0;
GLuint glslFogMax = 0;
GLuint glslFogColor = 0;
GLuint glslfAlphaTest = 0;
GLuint glslAlphaCutOff = 0;
GLuint glslRko = 0;
GLuint glslfAnimateUv = 0;
GLuint glsluvOffsets = 0;
GLuint glslUnSelfIllum = 0;
GLuint glslSubGlobPosCenter = 0;
GLuint glslSubGlobRadius = 0;
GLuint glslDyshMatWorldClip = 0;
GLuint glslDyshModel = 0;
GLuint glslAmbientMap = 0;
GLuint glslDiffuseMap = 0;
GLuint glslSaturateMap = 0;
GLuint glslGeomModelToClip = 0;
GLuint u_fontTexLoc = 0;
GLuint glslfCull = 0;
uint64_t screenTextureHandle = 0;
GLuint g_sceneFbo = 0;
int g_msaaSamples = 4;
bool g_fMsaa = false;
int g_frames = 3;