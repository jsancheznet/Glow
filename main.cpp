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
global renderer *MainRenderer;
global camera *Camera;

// These variables correspond to the FPS counter, TODO: make them not global
global f32 AverageFPS;
global f32 AverageMillisecondsPerFrame;
global f32 FPSTimerSecondsElapsed = 0.0f;
global f32 FPSCounter = 0.0f;

int main(i32 Argc, char **Argv)
{
    Argc; Argv;

    SDL_Init(SDL_INIT_EVERYTHING); // TODO: Init only the subsystems we need!
    Window = P_CreateOpenGLWindow("No title!", WindowWidth, WindowHeight);
    MainRenderer = R_CreateRenderer(Window);

    // TODO: Move this to our new standard *Mouse = P_CreateMouse, P_CreateClock, etc...
    I_Init(&Keyboard, &Mouse);
    P_InitClock(&Clock);

    Camera = G_CreateCamera(WindowWidth, WindowHeight);

    u32 Image = R_CreateTexture("textures/wanderer.png");

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
                        Camera->Yaw = -90.0f; // Set the Yaw to -90 so the mouse faces to 0, 0, 0 in the first frame X
                        Camera->Pitch = 0.0f;
                        Mouse.FirstMouse = false;
                    }
                    Camera->Yaw += Mouse.RelX * Mouse.Sensitivity;
                    Camera->Pitch += -Mouse.RelY *Mouse.Sensitivity; // reversed since y-coordinates range from bottom to top
                    if(Camera->Pitch > 89.0f)
                    {
                        Camera->Pitch =  89.0f;
                    }
                    else if(Camera->Pitch < -89.0f)
                    {
                        Camera->Pitch = -89.0f;
                    }
                    glm::vec3 Front;
                    Front.x = cos(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
                    Front.y = sin(glm::radians(Camera->Pitch));
                    Front.z = sin(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
                    Camera->Front = glm::normalize(Front);
                }

                // Handle Window input stuff
                if(I_IsPressed(SDL_SCANCODE_ESCAPE))
                {
                    IsRunning = false;
                }
                if(I_IsReleased(SDL_SCANCODE_RETURN) && I_IsPressed(SDL_SCANCODE_LALT))
                {
                    P_ToggleFullscreen(Window);
                    i32 Width, Height;
                    SDL_GL_GetDrawableSize(Window->Handle, &Width, &Height);
                    R_ResizeRenderer(MainRenderer, Width, Height);
                }

                // Handle Camera Input
                if(I_IsPressed(SDL_SCANCODE_W) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    Camera->Position += Camera->Front * Camera->Speed * (f32)Clock.DeltaTime;
                }
                if(I_IsPressed(SDL_SCANCODE_S) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    Camera->Position -= Camera->Speed * Camera->Front * (f32)Clock.DeltaTime;
                }
                if(I_IsPressed(SDL_SCANCODE_A) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    Camera->Position -= glm::normalize(glm::cross(Camera->Front, Camera->Up)) * Camera->Speed * (f32)Clock.DeltaTime;
                }
                if(I_IsPressed(SDL_SCANCODE_D) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    Camera->Position += glm::normalize(glm::cross(Camera->Front, Camera->Up)) * Camera->Speed * (f32)Clock.DeltaTime;
                }
                if(I_IsPressed(SDL_SCANCODE_SPACE) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    G_ResetCamera(Camera, WindowWidth, WindowHeight);
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
                    MainRenderer->Exposure += 0.01f;
                }
                if(I_IsPressed(SDL_SCANCODE_DOWN))
                {
                    MainRenderer->Exposure -= 0.01f;
                }
            }

            // Update Camera Matrices
            Camera->View = glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
            Camera->Projection = glm::perspective(glm::radians(Camera->FoV), (f32)WindowWidth / (f32)WindowHeight, Camera->Near, Camera->Far);

        } // SECTION END: Update

        { // SECTION: Render

            R_BeginFrame(MainRenderer);

            { // Render Cube, TODO(Jorge): Create a function for this!
                glUseProgram(MainRenderer->CubeShader);
                glm::mat4 Model = glm::mat4(1.0);
                Model = glm::translate(Model, PlayerPosition);
                R_SetUniform(MainRenderer->CubeShader, "Model", Model);
                R_SetUniform(MainRenderer->CubeShader, "View", Camera->View);
                R_SetUniform(MainRenderer->CubeShader, "Projection", Camera->Projection);
                R_SetUniform(MainRenderer->CubeShader, "Color", glm::vec3(800.0f, 2.0f, 100.0f));
                glBindVertexArray(MainRenderer->CubeVAO);
                // Render Glowing Cube
                glDrawArrays(GL_TRIANGLES, 0, 36);
                // Render not glowing cube
                Model = glm::translate(Model, glm::vec3(-3.0f, 0.0f, 0.0f));
                Model = glm::mat4(1.0f);
                R_SetUniform(MainRenderer->CubeShader, "Model", Model);
                R_SetUniform(MainRenderer->CubeShader, "Color", glm::vec3(0.0f, 0.0f, 1.0f));
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            R_EndFrame(MainRenderer);
        } // SECTION END: Render
    }

    SDL_GL_DeleteContext(Window->Handle);

    return 0;
}
