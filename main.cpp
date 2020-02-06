#ifdef _WIN32
extern "C"
{
    // http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
    __declspec( dllexport ) unsigned long int NvOptimusEnablement = 0x00000001;
    // https://gpuopen.com/amdpowerxpressrequesthighperformance/
    __declspec( dllexport ) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include <windows.h>
#include <stdio.h>

#include "external/glad.c"
#include <SDL.h>
#include <SDL_opengl.h>

// NOTE: http://casual-effects.com/data/index.html <--- Cool site for test models
#include "shared.h"
#include "platform.cpp"
#include "input.cpp"
#include "renderer.cpp"
#include "game.cpp"

// TODO(Jorge): Create game.cpp and put al things that belong to the game in here, such as camera, etc..
// TODO(Jorge): Fix togglefullscreen bug!
// TODO(Jorge): Replace every printf with MessageBox!
// TODO: Make sure everything is gamma corrected, lighting calculations also need to be done with Gamma correction!
// TODO: Game hotloading?
// TODO: AABBvsAABB;
// TODO: DrawDebugLine();
// TODO: DrawLine();
// TODO: Test Tweening Functions!
// TODO(Jorge): Shader Hot reloading: http://antongerdelan.net/opengl/shader_hot_reload.html
// TODO(Jorge): Visualize the different opengl buffers, depth buffer, stencil, color buffer, etc...
// TODO(Jorge): Create a lot of bullets and render them using instanced rendering
// TODO(Jorge): Colors are different while rendering with nVidia card, and Intel card
// TODO(Jorge): Load Sphere model
// TODO(Jorge): Load Tetrahedron model
// TODO(Jorge): Draw Tetrahedron correctly
// TODO(Jorge): Add another shape! maybe Torus or Sphere
// TODO(Jorge): Add a uniform Color to renderable_objects
// TODO(Jorge): Add Normals to objects
// TODO(Jorge): Add License to all files
// TODO(Jorge): Changing focus to the console window clicking on it and switching to the main game window crashes the program. Is this a bug?
// TODO(Jorge): Remove unused functions from final version
// TODO(Jorge): Put license in all files

// IDEAS: Load variables from a file, such as SetSwapInterval
// IDEAS: Stop using relative mouse mode, use regular so nSight can work correctly, and we can have mouse navigatable debug in game menus

//
// Globals
//
global i32 WindowWidth = 1366;
global i32 WindowHeight = 768;
global b32 IsRunning = 1;
global clock Clock;
global mouse Mouse;
global keyboard Keyboard;
global window *Window;
global renderer *Renderer;
global camera *MainCamera;

// These variables correspond to the FPS counter, TODO: make them not global
global f32 AverageFPS;
global f32 AverageMillisecondsPerFrame;
global f32 FPSTimerSecondsElapsed = 0.0f;
global f32 FPSCounter = 0.0f;

#if 0
void R_ResizeRenderer(renderer *Renderer, i32 Width, i32 Height)
{
    Assert(Renderer);
    Assert(Width > 0);
    Assert(Height > 0);

    // Delete old buffer textures and renderbuffers, also pingpong buffers
    // u32 Framebuffer;
    // u32 ColorBuffer;
    // u32 BrightnessBuffer;
    // u32 DepthStencilRenderbuffer;
    // u32 Attachments[2];
    // u32 PingPongFBO[2];
    // u32 PingPongBuffer[2];

}
#endif

int main(i32 Argc, char **Argv)
{
    Argc; Argv;

    SDL_Init(SDL_INIT_EVERYTHING); // TODO: Init only the subsystems we need!

    Window = P_CreateOpenGLWindow("No title!", WindowWidth, WindowHeight);
    Renderer = R_CreateRenderer(Window);

    I_Init(&Keyboard, &Mouse);

    P_InitClock(&Clock);

    MainCamera = G_CreateCamera(WindowWidth, WindowHeight);
    glm::vec3 PlayerPosition = glm::vec3(0);

    f32 Angle = 0.0f;
    SDL_Event Event;
    while(IsRunning)
    {
        Angle += 0.01f;
        P_UpdateClock(&Clock);

        { // Compute Average FPS - Average Milliseconds Per Frame
            f32 FramesPerSecondToShow = 2; // How many times per second to calculate fps
            if(FPSTimerSecondsElapsed > (1.0f / FramesPerSecondToShow))
            {
                AverageFPS = FPSCounter / FPSTimerSecondsElapsed;
                AverageMillisecondsPerFrame = (FPSTimerSecondsElapsed / FPSCounter) * 1000.0f;
                FPSCounter = 0;
                FPSTimerSecondsElapsed = 0.0f;

                char Title[60] = {};
                sprintf_s(Title, sizeof(Title),"Average FPS: %2.2f - Average Ms per frame: %2.2f", AverageFPS, AverageMillisecondsPerFrame);
                SDL_SetWindowTitle(Window->Handle, Title);
            }
            else
            {
                FPSCounter += 1.0f;
                FPSTimerSecondsElapsed += (f32)Clock.DeltaTime;
            }
        }

        { // SECTION: Input Handling

            while(SDL_PollEvent(&Event))
            {
                switch(Event.type)
                {
                    case SDL_QUIT:
                    {
                        IsRunning = 0;
                        break;
                    }
                }
            }

            { // SECTION: Update
                I_UpdateKeyboard(&Keyboard);
                I_UpdateMouse(&Mouse);

                if(I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    // Camera Stuff
                    if(Mouse.FirstMouse)
                    {
                        MainCamera->Yaw = -90.0f; // Set the Yaw to -90 so the mouse faces to 0, 0, 0 in the first frame X
                        MainCamera->Pitch = 0.0f;
                        Mouse.FirstMouse = false;
                    }
                    MainCamera->Yaw += Mouse.RelX * Mouse.Sensitivity;
                    MainCamera->Pitch += -Mouse.RelY *Mouse.Sensitivity; // reversed since y-coordinates range from bottom to top
                    if(MainCamera->Pitch > 89.0f)
                    {
                        MainCamera->Pitch =  89.0f;
                    }
                    else if(MainCamera->Pitch < -89.0f)
                    {
                        MainCamera->Pitch = -89.0f;
                    }
                    glm::vec3 Front;
                    Front.x = cos(glm::radians(MainCamera->Yaw)) * cos(glm::radians(MainCamera->Pitch));
                    Front.y = sin(glm::radians(MainCamera->Pitch));
                    Front.z = sin(glm::radians(MainCamera->Yaw)) * cos(glm::radians(MainCamera->Pitch));
                    MainCamera->Front = glm::normalize(Front);
                }

                // Handle Window input stuff
                if(I_IsPressed(SDL_SCANCODE_ESCAPE))
                {
                    IsRunning = false;
                }
                if(I_IsReleased(SDL_SCANCODE_RETURN) && I_IsPressed(SDL_SCANCODE_LALT))
                {
                    P_ToggleFullscreen(Window);
                }

                // Handle Camera Input
                if(I_IsPressed(SDL_SCANCODE_W) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    MainCamera->Position += MainCamera->Front * MainCamera->Speed * (f32)Clock.DeltaTime;
                }
                if(I_IsPressed(SDL_SCANCODE_S) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    MainCamera->Position -= MainCamera->Speed * MainCamera->Front * (f32)Clock.DeltaTime;
                }
                if(I_IsPressed(SDL_SCANCODE_A) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    MainCamera->Position -= glm::normalize(glm::cross(MainCamera->Front, MainCamera->Up)) * MainCamera->Speed * (f32)Clock.DeltaTime;
                }
                if(I_IsPressed(SDL_SCANCODE_D) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    MainCamera->Position += glm::normalize(glm::cross(MainCamera->Front, MainCamera->Up)) * MainCamera->Speed * (f32)Clock.DeltaTime;
                }
                if(I_IsPressed(SDL_SCANCODE_SPACE) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    G_ResetCamera(MainCamera, WindowWidth, WindowHeight);
                    I_ResetMouse(&Mouse);
                }

                // Handle Player Input
                f32 PlayerSpeed = 0.01f;
                if(I_IsPressed(SDL_SCANCODE_W) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    PlayerPosition.y += PlayerSpeed;
                }
                if(I_IsPressed(SDL_SCANCODE_S) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    PlayerPosition.y -= PlayerSpeed;
                }
                if(I_IsPressed(SDL_SCANCODE_A) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    PlayerPosition.x -= PlayerSpeed;
                }
                if(I_IsPressed(SDL_SCANCODE_D) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    PlayerPosition.x += PlayerSpeed;
                }
                if(I_IsPressed(SDL_SCANCODE_SPACE) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    // Jump?!?!?
                }

                if(I_IsPressed(SDL_SCANCODE_UP))
                {
                    Renderer->Exposure += 0.01f;
                }
                if(I_IsPressed(SDL_SCANCODE_DOWN))
                {
                    Renderer->Exposure -= 0.01f;
                }
            }

            // Update MainCamera Matrices
            MainCamera->View = glm::lookAt(MainCamera->Position, MainCamera->Position + MainCamera->Front, MainCamera->Up);
            MainCamera->Projection = glm::perspective(glm::radians(MainCamera->FoV), (f32)WindowWidth / (f32)WindowHeight, MainCamera->Near, MainCamera->Far);

        } // SECTION END: Update

        { // SECTION: Render

            R_BeginFrame(Renderer);

            // R_DrawCube(renderable_cube *Cube);

            // render Cube
            glUseProgram(Renderer->CubeShader);
            glm::mat4 Model = glm::mat4(1.0);
            Model = glm::translate(Model, PlayerPosition);
            R_SetUniform(Renderer->CubeShader, "Model", Model);
            R_SetUniform(Renderer->CubeShader, "View", MainCamera->View);
            R_SetUniform(Renderer->CubeShader, "Projection", MainCamera->Projection);
            R_SetUniform(Renderer->CubeShader, "Color", glm::vec3(800.0f, 2.0f, 100.0f));
            glBindVertexArray(Renderer->CubeVAO);
            // Render Glowing Cube
            glDrawArrays(GL_TRIANGLES, 0, 36);
            // Render not glowing cube
            Model = glm::translate(Model, glm::vec3(-3.0f, 0.0f, 0.0f));
            Model = glm::mat4(1.0f);
            R_SetUniform(Renderer->CubeShader, "Model", Model);
            R_SetUniform(Renderer->CubeShader, "Color", glm::vec3(0.0f, 0.0f, 1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            R_EndFrame(Renderer);
        } // SECTION END: Render
    }

    SDL_GL_DeleteContext(Window->Handle);

    return 0;
}
