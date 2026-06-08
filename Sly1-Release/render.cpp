#include "render.h"
#include "main.h"

void SetRpCount(GLOB* pglob, int fTransluscent)
{
	switch (pglob->rp)
	{
		case RP_DynamicTexture:
		pglob->PFNDRAW = DrawSubGlob;
		g_dynamicTextureCount++;
		break;

		case RP_Background:
		pglob->PFNDRAW = DrawSubGlob;
		if (fTransluscent == 1)
			g_backGroundBlendCount++;
		else
			g_backGroundCount++;
		break;

		case RP_BlotContext:
		pglob->PFNDRAW = DrawSubGlob;
		g_blotContextCount++;
		break;

		case RP_Opaque:
		pglob->PFNDRAW = DrawSubGlob;
		g_opaqueCount++;
		g_translucentCount++;
		break;

		case RP_Cutout:
		pglob->PFNDRAW = DrawTranslucent;
		if (fTransluscent == 1)
		{
			g_cutOutCount++;
			g_translucentCount++;
		}
		else
		{
			g_cutOutBlendAddCount++;
			g_translucentCount++;
		}
		break;

		case RP_CelBorder:
		g_celBorderCount++;
		g_translucentCelBorderCount++;
		break;

		case RP_ProjVolume:
		switch (pglob->grfshd)
		{
			case 0:
			pglob->PFNDRAW = DrawProjVolume;
			g_projVolumeCount++;
			break;

			case 2:
			pglob->PFNDRAW = DrawProjVolumeAlphaAdd;
			g_projVolumeAlphaAddCount++;
			break;

			case 1:
			case 3:
			pglob->PFNDRAW = DrawProjVolumeAdd;
			g_projVolumeAddCount++;
			break;
		}
		break;

		case RP_OpaqueAfterProjVolume:
		pglob->PFNDRAW = DrawSubGlob;
		g_opaqueAfterProjVolumeCount++;
		g_translucentCount++;
		break;

		case RP_CutoutAfterProjVolume:
		pglob->PFNDRAW = DrawTranslucent;
		if (fTransluscent == 1)
		{
			g_cutOutAfterProjVolumeCount++;
			g_translucentCount++;
		}
		else
		{
			g_cutOutAfterProjVolumeAddCount++;
			g_translucentCount++;
		}
		break;

		case RP_CelBorderAfterProjVolume:
		g_celBorderAfterProjVolumeCount++;
		g_translucentCelBorderCount++;
		break;

		case RP_MurkClear:
		pglob->PFNDRAW = DrawMurkClear;
		g_murkClearCount++;
		break;

		case RP_MurkOpaque:
		pglob->PFNDRAW = DrawSubGlob;
		g_murkOpaqueCount++;
		break;

		case RP_MurkFill:
		pglob->PFNDRAW = DrawMurkFill;
		g_murkFillCount++;
		break;

		case RP_Translucent:
		pglob->PFNDRAW = DrawTranslucent;
		if (fTransluscent == 1)
			g_translucentCount++;
		else
			g_translucentAddCount++;
		break;

		case RP_TranslucentCelBorder:
		g_translucentCelBorderCount++;
		break;

		case RP_Blip:
		pglob->PFNDRAW = DrawSubGlob;
		g_blipCount++;
		break;

		case RP_Foreground:
		pglob->PFNDRAW = DrawSubGlob;
		g_foreGroundCount++;
		break;

		case RP_WorldMap:
		pglob->PFNDRAW = DrawSubGlob;
		g_worldMapCount++;
		break;

		case RP_Max:
		pglob->PFNDRAW = DrawSubGlob;
		g_maxCount++;
		break;
	}
}

void AllocateRpl()
{
	g_dynamicTexturePrpl.resize(g_dynamicTextureCount);
	g_backGroundPrpl.resize(g_backGroundCount);
	g_backGroundBlendPrpl.resize(g_backGroundBlendCount);
	g_blotContextPrpl.resize(g_blotContextCount);
	g_opaquePrpl.resize(g_opaqueCount);
	g_cutOutBlendAddPrpl.resize(g_cutOutBlendAddCount);
	g_cutOutPrpl.resize(g_cutOutCount);
	g_celBorderPrpl.resize(g_celBorderCount);
	g_projVolumePrpl.resize(g_projVolumeCount);
	g_projVolumeAlphaAddPrpl.resize(g_projVolumeAlphaAddCount);
	g_projVolumeAddPrpl.resize(g_projVolumeAddCount);
	g_opaqueAfterProjVolumePrpl.resize(g_opaqueAfterProjVolumeCount);
	g_cutOutAfterProjVolumeAddPrpl.resize(g_cutOutAfterProjVolumeAddCount);
	g_cutOutAfterProjVolumePrpl.resize(g_cutOutAfterProjVolumeCount);
	g_celBorderAfterProjVolumePrpl.resize(g_celBorderAfterProjVolumeCount);
	g_murkClearPrpl.resize(g_murkClearCount);
	g_murkOpaquePrpl.resize(g_murkOpaqueCount);
	g_murkFillPrpl.resize(g_murkFillCount);
	g_translucentAddPrpl.resize(g_translucentAddCount);
	g_translucentPrpl.resize(g_translucentCount);
	g_translucentCelBorderPrpl.resize(g_translucentCelBorderCount);
	g_blipPrpl.resize(g_blipCount);
	g_foreGroundPrpl.resize(g_foreGroundCount);
	g_worldMapPrpl.resize(g_worldMapCount);
	g_maxPrpl.resize(g_maxCount);

	g_dynamicTextureCount = 0;
	g_backGroundCount = 0;
	g_backGroundBlendCount = 0;
	g_blotContextCount = 0;
	g_opaqueCount = 0;
	g_cutOutBlendAddCount = 0;
	g_cutOutCount = 0;
	g_celBorderCount = 0;
	g_projVolumeCount = 0;
	g_projVolumeAlphaAddCount = 0;
	g_projVolumeAddCount = 0;
	g_opaqueAfterProjVolumeCount = 0;
	g_cutOutAfterProjVolumeAddCount = 0;
	g_cutOutAfterProjVolumeCount = 0;
	g_celBorderAfterProjVolumeCount = 0;
	g_murkClearCount = 0;
	g_murkOpaqueCount = 0;
	g_murkFillCount = 0;
	g_translucentAddCount = 0;
	g_translucentCount = 0;
	g_translucentCelBorderCount = 0;
	g_blipCount = 0;
	g_foreGroundCount = 0;
	g_worldMapCount = 0;
}

void RenderSw(SW* psw, CM* pcm)
{
	DLI dlBusyWalker;
	dlBusyWalker.m_pdl = &psw->dlBusy;
	dlBusyWalker.m_ibDle = psw->dlBusy.ibDle;
	dlBusyWalker.m_pdliNext = s_pdliFirst;

	LO* currentObject = psw->dlBusy.ploFirst;

	dlBusyWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle) : nullptr;

	// push
	s_pdliFirst = &dlBusyWalker;

	while (currentObject != nullptr)
	{
		currentObject->pvtalo->pfnRenderAloAll((ALO*)currentObject, pcm, nullptr);

		currentObject = reinterpret_cast<LO*>(*dlBusyWalker.m_ppv);
		dlBusyWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle) : nullptr;
	}

	// pop (critical)
	s_pdliFirst = dlBusyWalker.m_pdliNext;
}

void RenderSwAloAll(SW* psw, CM* pcm)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
	{
		CID cid = allSWAloObjs[i]->pvtalo->cid;

		allSWAloObjs[i]->pvtalo->pfnRenderAloAll(allSWAloObjs[i], pcm, nullptr);
	}
}

void RenderSwGlobset(SW* psw, CM* pcm)
{
	for (int i = 0; i < 40; i++)
		allSWAloObjs[i]->pvtalo->pfnRenderAloGlobset(allSWAloObjs[i], pcm, nullptr);
}

void SubmitRpl(RPL* prpl)
{
	switch (prpl->rp)
	{
		case RP_DynamicTexture:
		g_dynamicTexturePrpl[g_dynamicTextureCount] = *prpl;
		g_dynamicTextureCount++;
		break;

		case RP_Background:
		if (prpl->fTransluscentSort == 1)
		{
			g_backGroundBlendPrpl[g_backGroundBlendCount] = *prpl;
			g_backGroundBlendCount++;
		}
		else
		{
			g_backGroundPrpl[g_backGroundCount] = *prpl;
			g_backGroundCount++;
		}
		break;

		case RP_BlotContext:
		g_blotContextPrpl[g_blotContextCount] = *prpl;;
		g_blotContextCount++;
		break;

		case RP_Opaque:
		g_opaquePrpl[g_opaqueCount] = *prpl;;
		g_opaqueCount++;
		break;

		case RP_Cutout:
		if (prpl->fTransluscentSort == 1)
		{
			g_cutOutPrpl[g_cutOutCount] = *prpl;
			g_cutOutCount++;
		}
		else
		{
			g_cutOutBlendAddPrpl[g_cutOutBlendAddCount] = *prpl;
			g_cutOutBlendAddCount++;
		}
		break;

		case RP_ProjVolume:
		switch (prpl->pglob->grfshd)
		{
			case 0:
			g_projVolumePrpl[g_projVolumeCount] = *prpl;
			g_projVolumeCount++;
			break;

			case 2:
			g_projVolumeAlphaAddPrpl[g_projVolumeAlphaAddCount] = *prpl;
			g_projVolumeAlphaAddCount++;
			break;

			case 1:
			case 3:
			g_projVolumeAddPrpl[g_projVolumeAddCount] = *prpl;
			g_projVolumeAddCount++;
			break;
		}
		break;

		case RP_OpaqueAfterProjVolume:
		g_opaqueAfterProjVolumePrpl[g_opaqueAfterProjVolumeCount] = *prpl;
		g_opaqueAfterProjVolumeCount++;
		break;

		case RP_CutoutAfterProjVolume:
		if (prpl->fTransluscentSort == 1)
		{
			g_cutOutAfterProjVolumePrpl[g_cutOutAfterProjVolumeCount] = *prpl;
			g_cutOutAfterProjVolumeCount++;
		}
		else
		{
			g_cutOutAfterProjVolumeAddPrpl[g_cutOutAfterProjVolumeAddCount] = *prpl;
			g_cutOutAfterProjVolumeAddCount++;
		}
		break;

		case RP_MurkClear:
		g_murkClearPrpl[g_murkClearCount] = *prpl;
		g_murkClearCount++;
		break;

		case RP_MurkOpaque:
		g_murkOpaquePrpl[g_murkOpaqueCount] = *prpl;
		g_murkOpaqueCount++;
		break;

		case RP_MurkFill:
		g_murkFillPrpl[g_murkFillCount] = *prpl;
		g_murkFillCount++;
		break;

		case RP_Translucent:
		if (prpl->fTransluscentSort == 1)
		{
			g_translucentPrpl[g_translucentCount] = *prpl;
			g_translucentCount++;
		}
		else
		{
			g_translucentAddPrpl[g_translucentAddCount] = *prpl;
			g_translucentAddCount++;
		}
		break;

		case RP_Blip:
		g_blipPrpl[g_blipCount] = *prpl;
		g_blipCount++;
		break;

		case RP_Foreground:
		g_foreGroundPrpl[g_foreGroundCount] = *prpl;
		g_foreGroundCount++;
		break;

		case RP_WorldMap:
		g_worldMapPrpl[g_worldMapCount] = *prpl;
		g_worldMapCount++;
		break;

		case RP_Max:
		g_maxPrpl[g_maxCount] = *prpl;
		g_maxCount++;
		break;
	}

	g_cFrameGlobs++;
	//numFrameObjs += prpl->pglob->asubglob.size();
}

void SubmitRplCel(RPLCEL* prplcel)
{
	switch (prplcel->rp)
	{
		case RP_CelBorder:
		g_celBorderPrpl[g_celBorderCount] = *prplcel;
		g_celBorderCount++;
		break;

		case RP_CelBorderAfterProjVolume:
		g_celBorderAfterProjVolumePrpl[g_celBorderAfterProjVolumeCount] = *prplcel;
		g_celBorderAfterProjVolumeCount++;
		break;

		case RP_TranslucentCelBorder:
		g_translucentCelBorderPrpl[g_translucentCelBorderCount] = *prplcel;
		g_translucentCelBorderCount++;
		break;
	}

	g_cFrameCelGlobs++;
	//numFrameObjs++;
}

void SortRenderRpl()
{
	if (g_backGroundBlendCount > 1)
		std::stable_sort(g_backGroundBlendPrpl.begin(), g_backGroundBlendPrpl.begin() + g_backGroundBlendCount, compareZ);

	if (g_cutOutCount > 1)
		std::stable_sort(g_cutOutPrpl.begin(), g_cutOutPrpl.begin() + g_cutOutCount, compareZ);

	if (g_cutOutAfterProjVolumeCount > 1)
		std::stable_sort(g_cutOutAfterProjVolumePrpl.begin(), g_cutOutAfterProjVolumePrpl.begin() + g_cutOutAfterProjVolumeCount, compareZ);

	if (g_translucentCount > 1)
		std::stable_sort(g_translucentPrpl.begin(), g_translucentPrpl.begin() + g_translucentCount, compareZ);
}

inline bool compareZ(const RPL& prpl0, const RPL& prpl1)
{
	return prpl0.z > prpl1.z;
}

void DrawSw(SW* psw, CM* pcm)
{
	/*std::cout << numFrameObjs << "\n";*/
	//numFrameObjs = 0;

	glGlobShader.Use();

	//std::cout << g_cFrameGlobs << "\n";
	BeginFrameStream(&ropStream);

	bool anyCel = g_cFrameCelGlobs > 0;

	if (anyCel == true)
		BeginFrameStream(&rcbStream);
	
	SortRenderRpl();

	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &pcm->matWorldToClip);

	PrepareSwLights(psw, pcm);
	PrepareSwShadows(psw, pcm);

	glLineWidth(3.5);
	glEnable(GL_CULL_FACE);

	if (g_dynamicTextureCount > 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, g_gl.dyshFbo);
		glViewport(0, 0, g_gl.dyshWidth, g_gl.dyshHeight);

		/*glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		glDyshadow.Use();

		for (int i = 0; i < g_dynamicTextureCount; i++)
			DrawDysh(&g_dynamicTexturePrpl[i]);

		glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);
		glViewport(0, 0, g_gl.width, g_gl.height);

		//glDisable(GL_BLEND);

		glDepthFunc(GL_LESS);
		glDepthMask(true);

		glGlobShader.Use();

		g_dynamicTextureCount = 0;
	}

	if (g_backGroundCount > 0)
	{
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundCount; i++)
			DrawGlob(&g_backGroundPrpl[i]);

		glDepthFunc(GL_LESS);
		glDepthMask(true);

		g_backGroundCount = 0;
	}

	if (g_backGroundBlendCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundBlendCount; i++)
			DrawGlob(&g_backGroundBlendPrpl[i]);

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glDepthMask(true);

		g_backGroundBlendCount = 0;
	}

	if (g_blotContextCount > 0)
	{
		for (int i = 0; i < g_blotContextCount; i++)
			DrawGlob(&g_blotContextPrpl[i]);

		g_blotContextCount = 0;
	}

	if (g_opaqueCount > 0)
	{
		for (int i = 0; i < g_opaqueCount; i++)
			DrawGlob(&g_opaquePrpl[i]);

		g_opaqueCount = 0;
	}

	if (g_cutOutBlendAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutBlendAddCount; i++)
			DrawGlob(&g_cutOutBlendAddPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutBlendAddCount = 0;
	}

	if (g_cutOutCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutCount; i++)
			DrawGlob(&g_cutOutPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutCount = 0;
	}

	if (g_celBorderCount > 0)
	{
		glCelBorderShader.Use();

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_celBorderCount; i++)
			DrawCelBorder(&g_celBorderPrpl[i]);

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		glGlobShader.Use();

		g_celBorderCount = 0;
	}

	if (g_projVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LESS);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeCount; i++)
			DrawGlob(&g_projVolumePrpl[i]);

		glStencilMask(0xFF);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeCount = 0;
	}

	if (g_projVolumeAlphaAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LESS);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeAlphaAddCount; i++)
			DrawGlob(&g_projVolumeAlphaAddPrpl[i]);

		glStencilMask(0xFF);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeAlphaAddCount = 0;
	}

	if (g_projVolumeAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LESS);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeAddCount; i++)
		{
			g_grfshd = g_projVolumeAddPrpl[i].pglob->grfshd;
			DrawGlob(&g_projVolumeAddPrpl[i]);
		}

		glStencilMask(0xFF);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeAddCount = 0;
	}

	if (g_opaqueAfterProjVolumeCount > 0)
	{
		for (int i = 0; i < g_opaqueAfterProjVolumeCount; i++)
			DrawGlob(&g_opaqueAfterProjVolumePrpl[i]);

		g_opaqueAfterProjVolumeCount = 0;
	}

	if (g_cutOutAfterProjVolumeAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutAfterProjVolumeAddCount; i++)
			DrawGlob(&g_cutOutAfterProjVolumeAddPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutAfterProjVolumeAddCount = 0;
	}

	if (g_cutOutAfterProjVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutAfterProjVolumeCount; i++)
			DrawGlob(&g_cutOutAfterProjVolumePrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutAfterProjVolumeCount = 0;
	}

	if (g_celBorderAfterProjVolumeCount > 0)
	{
		glCelBorderShader.Use();

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_celBorderAfterProjVolumeCount; i++)
			DrawCelBorder(&g_celBorderAfterProjVolumePrpl[i]);

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		glGlobShader.Use();

		g_celBorderAfterProjVolumeCount = 0;
	}

	if (g_murkClearCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_murkClearCount; i++)
			DrawGlob(&g_murkClearPrpl[i]);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		g_murkClearCount = 0;
	}

	if (g_murkOpaqueCount > 0)
	{
		glUniform1i(glslfAlphaTest, 1);
		glUniform1f(glslAlphaCutOff, 0.1);

		for (int i = 0; i < g_murkOpaqueCount; i++)
			DrawGlob(&g_murkOpaquePrpl[i]);

		glUniform1i(glslfAlphaTest, 0);

		g_murkOpaqueCount = 0;
	}

	if (g_murkFillCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(false);
		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_murkFillCount; i++)
			DrawGlob(&g_murkFillPrpl[i]);

		glDisable(GL_BLEND);
		glDepthMask(true);
		glDepthFunc(GL_LESS);

		g_murkFillCount = 0;
	}

	if (g_translucentAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_translucentAddCount; i++)
			DrawGlob(&g_translucentAddPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_translucentAddCount = 0;
	}

	if (g_translucentCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_translucentCount; i++)
			DrawGlob(&g_translucentPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_translucentCount = 0;
	}

	if (g_translucentCelBorderCount > 0)
	{
		glCelBorderShader.Use();

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_translucentCelBorderCount; i++)
			DrawCelBorder(&g_translucentCelBorderPrpl[i]);

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		glGlobShader.Use();

		g_translucentCelBorderCount = 0;
	}

	if (g_blipCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glDepthMask(false);

		for (int i = 0; i < g_blipCount; i++)
			DrawGlob(&g_blipPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_blipCount = 0;
	}

	if (g_foreGroundCount > 0)
	{
		for (int i = 0; i < g_foreGroundCount; i++)
			DrawGlob(&g_foreGroundPrpl[i]);

		g_foreGroundCount = 0;
	}

	if (g_worldMapCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glDepthMask(false);
		//glDepthFunc(GL_ALWAYS);

		for (int i = 0; i < g_worldMapCount; i++)
			DrawGlob(&g_worldMapPrpl[i]);

		glDepthMask(true);
		//glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		g_worldMapCount = 0;
	}

	if (g_maxCount > 0)
	{
		for (int i = 0; i < g_maxCount; i++)
			DrawGlob(&g_maxPrpl[i]);

		g_maxCount = 0;
	}

	EndFrameStream(&ropStream);

	if (anyCel == true)
		EndFrameStream(&rcbStream);

	g_cFrameGlobs = 0;
	g_cFrameCelGlobs = 0;

	glDisable(GL_CULL_FACE);
	glClearStencil(0x00);
}

void DrawDysh(RPL* prpl)
{
	DYSH* pdysh = prpl->pdysh;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pdysh->shadowTex, 0);

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 matWorldToClip = g_uvToClip * pdysh->pshadowGen->matWorldToUv;

	glUniformMatrix4fv(glslDyshMatWorldClip, 1, GL_FALSE, glm::value_ptr(matWorldToClip));
	glUniformMatrix4fv(glslDyshModel, 1, GL_FALSE, glm::value_ptr(prpl->ro.model));

	for (int i = 0; i < pdysh->globset.aglob.size(); i++)
	{
		glBindVertexArray(pdysh->globset.aglob[i].VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pdysh->globset.aglob[i].EBO);

		for (int a = 0; a < pdysh->globset.aglob[i].asubglob.size(); a++)
		{
			SUBGLOB& sg = pdysh->globset.aglob[i].asubglob[a];
			glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)sg.indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(sg.firstIndex * sizeof(uint32_t)), (GLint)sg.baseVertex);
		}
	}
}

void DrawGlob(RPL* prpl)
{
	AppendStream(&ropStream, &prpl->ro, sizeof(ROGL), sizeof(ROGL));

	if (prpl->pglob->fThreeWay == 1)
	{
		if ((prpl->ro.trlk == TRLK_Relight) || (prpl->ro.trlk == TRLK_Baked && prpl->ro.fDynamic == 0))
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, prpl->pglob->ssboCachedMaterial);
	}

	glBindVertexArray(prpl->pglob->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prpl->pglob->EBO);

	if (prpl->ro.warpType != WARP_NONE)
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, prpl->pglob->pwarpGlob->ssboState);

	for (int i = 0; i < prpl->pglob->asubglob.size(); i++)
	{
		SUBGLOB& sub = prpl->pglob->asubglob[i];
		SAI* sai = sub.uvSai;

		int animate = (sai != nullptr && (sai->grfsai & 0x2)) ? 1 : 0;

		glUniform1i(glslfAnimateUv, animate);

		if (animate)
			glUniform2f(glsluvOffsets, sai->tcx.du, sai->tcx.dv);

		int iframe = 0;

		if (sai != nullptr && sai->pshd != nullptr && sai->pshd->cframe > 0)
		{
			iframe = sai->iframe;

			if (iframe < 0)
				iframe = 0;
			else if (iframe >= sai->pshd->cframe)
				iframe = sai->pshd->cframe - 1;
		}

		SHD* pshd = sub.pshd;

		if (pshd != nullptr && !pshd->atex.empty() && !pshd->atex[0].abmp.empty())
		{
			if (iframe >= (int)pshd->atex[0].abmp.size())
				iframe = (int)pshd->atex[0].abmp.size() - 1;

			BMP* pbmp = pshd->atex[0].abmp[iframe];

			if (pbmp != nullptr)
			{
				if (pshd->shdk != SHDK_ThreeWay)
				{
					glUniform1i(glslRko, 0);
					glUniformHandleui64ARB(glslDiffuseMap, pbmp->hDiffuseMap);
				}
				else
				{
					glUniform1i(glslRko, 1);
					glUniformHandleui64ARB(glslAmbientMap,  pbmp->hShadowMap);
					glUniformHandleui64ARB(glslDiffuseMap,  pbmp->hDiffuseMap);
					glUniformHandleui64ARB(glslSaturateMap, pbmp->hSaturateMap);
				}
			}
		}

		if (activeShadows.numShadows > 0)
		{
			glm::vec4 subGlobPoscenter = prpl->ro.model * glm::vec4(sub.posCenter, 1.0);
			float subGlobRadius = prpl->pglob->rSubglobRadius * sub.sRadius;

			glUniform3f(glslSubGlobPosCenter, subGlobPoscenter.x, subGlobPoscenter.y, subGlobPoscenter.z);
			glUniform1f(glslSubGlobRadius, subGlobRadius);
		}

		glUniform1f(glslUnSelfIllum, sub.unSelfIllum);

		prpl->pglob->PFNDRAW(sub.baseVertex, sub.firstIndex, sub.indexCount);
	}
}

void DrawCelBorder(RPLCEL* prplcel)
{
	AppendStream(&rcbStream, &prplcel->rocel, sizeof(ROCEL), sizeof(ROCEL));

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, prplcel->edgeSSBO);

	glDrawArrays(GL_LINES, 0, prplcel->edgeCount * 2);
}

void DrawSubGlob(int baseVertex, int firstIndex, int indexCount)
{
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawProjVolume(int baseVertex, int firstIndex, int indexCount)
{
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_ZERO, GL_REPLACE, GL_ZERO);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_ZERO, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 128);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawProjVolumeAlphaAdd(int baseVertex, int firstIndex, int indexCount)
{
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_NONE, GL_REPLACE, GL_NONE);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_NONE, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 128);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawProjVolumeAdd(int baseVertex, int firstIndex, int indexCount)
{
	switch (g_grfshd)
	{
		case 1:
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
		break;

		case 3:
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		break;
	}

	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_ZERO, GL_REPLACE, GL_ZERO);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_ZERO, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 128);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawMurkClear(int baseVertex, int firstIndex, int indexCount)
{
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
	glFrontFace(GL_CW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glFrontFace(GL_CCW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawMurkFill(int baseVertex, int firstIndex, int indexCount)
{
	glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA, GL_ONE, GL_NONE);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawTranslucent(int baseVertex, int firstIndex, int indexCount)
{
	glUniform1i(glslfAlphaTest, 1);
	glDepthMask(true);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
	glUniform1i(glslfAlphaTest, 0);

	glDepthMask(false);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawSwCollisionAll(CM* pcm)
{
	glGeomShader.Use();

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cmUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &pcm->matWorldToClip);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, geomUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, geomUBO);

	glLineWidth(2.0);

	glm::mat4 model{};

	for (int i = 0; i < allSWSoObjs.size(); i++)
	{
		SO* pso = allSWSoObjs[i];

		if (!SphereInFrustum(pcm->frustum, pso->xf.posWorld, pso->geomLocal.sRadius))
			continue;

		LoadMatrixFromPosRot(pso->xf.posWorld, pso->xf.matWorld, model);

		glm::mat4 modelToClip = pcm->matWorldToClip * model;
		glUniformMatrix4fv(glslGeomModelToClip, 1, GL_FALSE, glm::value_ptr(modelToClip));

		if (pso->geomLocal.VAO != 0) 
		{
			glBindVertexArray(pso->geomLocal.VAO);
			glDrawElements(GL_LINES, (GLsizei)pso->geomLocal.indices.size(), GL_UNSIGNED_SHORT, 0);
		}

		if (pso->geomCameraLocal.VAO != 0) 
		{
			glBindVertexArray(pso->geomCameraLocal.VAO);
			glDrawElements(GL_LINES, (GLsizei)pso->geomCameraLocal.indices.size(), GL_UNSIGNED_SHORT, 0);
		}
	}

	glBindVertexArray(0);
}

int numRo = 0;
int numRoCel = 0;

int g_cFrameGlobs = 0;
int g_cFrameCelGlobs = 0;

int numFrameObjs = 0;

int g_cframe = 0;

int g_boundVAO = 0;

int g_shdIDBound = 0;
int g_lastAnimateUv = -1;

int g_grfshd = 0;

int g_dynamicTextureCount = 0;
std::vector <RPL> g_dynamicTexturePrpl;

int g_backGroundCount = 0;
std::vector <RPL> g_backGroundPrpl;

int g_backGroundBlendCount = 0;
std::vector <RPL> g_backGroundBlendPrpl;

int g_blotContextCount = 0;
std::vector <RPL> g_blotContextPrpl;

int g_opaqueCount = 0;
std::vector <RPL> g_opaquePrpl;

int g_cutOutCount = 0;
std::vector <RPL> g_cutOutPrpl;

int g_cutOutBlendAddCount = 0;
std::vector <RPL> g_cutOutBlendAddPrpl;

int g_celBorderCount = 0;
std::vector <RPLCEL> g_celBorderPrpl;

int g_projVolumeCount = 0;
std::vector <RPL> g_projVolumePrpl;

int g_projVolumeAlphaAddCount = 0;
std::vector <RPL> g_projVolumeAlphaAddPrpl;

int g_projVolumeAddCount = 0;
std::vector <RPL> g_projVolumeAddPrpl;

int g_opaqueAfterProjVolumeCount = 0;
std::vector <RPL> g_opaqueAfterProjVolumePrpl;

int g_cutOutAfterProjVolumeCount = 0;
std::vector <RPL> g_cutOutAfterProjVolumePrpl;

int g_cutOutAfterProjVolumeAddCount = 0;
std::vector <RPL> g_cutOutAfterProjVolumeAddPrpl;

int g_celBorderAfterProjVolumeCount = 0;
std::vector <RPLCEL> g_celBorderAfterProjVolumePrpl;

int g_murkClearCount = 0;
std::vector <RPL> g_murkClearPrpl;

int g_murkOpaqueCount = 0;
std::vector <RPL> g_murkOpaquePrpl;

int g_murkFillCount = 0;
std::vector <RPL> g_murkFillPrpl;

int g_translucentCount = 0;
std::vector <RPL> g_translucentPrpl;

int g_translucentAddCount = 0;
std::vector <RPL> g_translucentAddPrpl;

int g_translucentCelBorderCount = 0;
std::vector <RPLCEL> g_translucentCelBorderPrpl;

int g_blipCount = 0;
std::vector <RPL> g_blipPrpl;

int g_foreGroundCount = 0;
std::vector <RPL> g_foreGroundPrpl;

int g_worldMapCount = 0;
std::vector <RPL> g_worldMapPrpl;

int g_maxCount = 0;
std::vector <RPL> g_maxPrpl;

bool g_fVsync = true;