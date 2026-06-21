#include "main.h"
#include "iso_extractor.h"

int main(int cphzArgs, char* aphzArgs[])
{
    if (cphzArgs == 3 && std::string(aphzArgs[1]) == "--extract-iso")
    {
        IsoExtractionResult result = EnsureSly1RetailIsoExtracted(aphzArgs[2]);
        std::cout << result.message << "\n";

        if (result.ok)
        {
            std::cout << "Cache: " << result.cacheDirectory << "\n";
            std::cout << "Maps: " << result.levels.size() << "\n";
            return 0;
        }

        return 1;
    }

    Startup();

    while (!glfwWindowShouldClose(g_gl.window) && fQuitGame != true)
    {
        if (g_transition.m_fPending != 0)
            g_transition.Execute(file);

        // 1) Render scene to MSAA (or resolved if MSAA off)
        g_sceneFbo = g_fMsaa ? g_gl.fboMSAA : g_gl.fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);

        glViewport(0, 0, g_gl.width, g_gl.height);
        glClearColor(rgbaSky.r, rgbaSky.g, rgbaSky.b, rgbaSky.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        g_joy.Update(g_gl.window);

        RenderMenuGui(g_psw);

        if (g_psw != nullptr)
        {
            UpdateUi(&g_ui);

            SetupCm(g_pcm);
            MarkClockTick(&g_clock);
            UpdateSw(g_psw, g_clock.dt);
            UpdateCpman(g_gl.window, &g_pcm->cpman, nullptr, g_clock.dt);

            if (g_fRenderModels == true)
            {
                RenderSw(g_psw, g_pcm);
                //RenderSwGlobset(g_psw, g_pcm);
                DrawSw(g_psw, g_pcm);
            }

            if (g_fRenderCollision == true)
                DrawSwCollisionAll(g_pcm);

            DrawUi(&g_ui);
        }

        // 2) Resolve MSAA -> resolved texture FBO (ONLY if MSAA)
        if (g_fMsaa)
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, g_gl.fboMSAA);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_gl.fbo);
            glBlitFramebuffer(0, 0, g_gl.width, g_gl.height, 0, 0, g_gl.width, g_gl.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        // 3) Present: draw fullscreen quad sampling resolved texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, g_gl.width, g_gl.height);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        glScreenShader.Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_gl.fbc);

        glBindVertexArray(g_gl.sao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(g_gl.window);
        glfwPollEvents();

        g_cframe++;
    }

    if (g_psw != nullptr)
        DeleteWorld(g_psw);

    g_gl.TerminateGL();
    return 0;
}

void Startup()
{
    g_gl.InitGL();

    std::cout << "Sly Cooper 2002 Sony Computer Entertainment America & Sucker Punch Productions\n";
    SetPhase(PHASE_Startup);

    StartupClock();
    StartupBrx();
    StartupScreen();
    StartupUi();
    StartupFrame();
}

bool fQuitGame;
