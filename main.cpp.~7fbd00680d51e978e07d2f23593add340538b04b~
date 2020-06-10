#ifdef _WIN32
extern "C"
{
    // http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
    __declspec( dllexport ) unsigned long int NvOptimusEnablement = 0x00000001;
    // https://gpuopen.com/amdpowerxpressrequesthighperformance/
    __declspec( dllexport ) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

//#include <windows.h>
#include <stdio.h>
#include <vector>

#include "external/glad.c"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>

#include "shared.h"
#include "platform.cpp"
#include "input.cpp"
#include "renderer.cpp"
#include "sound.cpp"
#include "collision.cpp"
#include "entity.cpp"

// TODO(Jorge): Textures transparent background is not bleding correctly
// TODO(Jorge): Make Bullets little! everything will look better.
// TODO(Jorge): Sound system should be able to play two sound effects atop of each other!
// TODO(Jorge): Colors are different while rendering with nVidia card, and Intel card
// TODO(Jorge): Add License to all files
// TODO(Jorge): Remove unused functions from final version
// TODO(Jorge): Delele all unused data files

entity *Point = NULL;

enum state
{
    State_Initial,
    State_Game,
    State_Pause,
    State_Gameover,
};

// Game Variables
global f32 WorldBottom = -11.0f;
global f32 WorldTop = 11.0f;
global f32 WorldLeft = -20.0f;
global f32 WorldRight = 20.0f;
global f32 HalfWorldWidth = WorldRight;
global f32 HalfWorldHeight = WorldTop;
// global glm::vec2 MouseWorldPosition;

// Entity Variables
global f32 PlayerSpeed = 4.0f;
global f32 PlayerDrag = 0.8f;
global f32 EnemySpeed = 8.0f;
global f32 EnemyDrag = 0.8f;
global i32 MaxBullets = 100;
global f32 BulletSpeed = 10.0f;
global f32 BulletDrag =  1.0f;
global glm::vec3 BulletSize = glm::vec3(1.0);
global std::vector<entity*> Bullets;

// Variables
global b32 IsRunning = 1;
global keyboard *Keyboard;
global mouse *Mouse;
global clock *Clock;
global window *Window;
global renderer *Renderer;
global sound_system *SoundSystem;
global camera *Camera;
global state CurrentState = State_Initial;

i32 main(i32 Argc, char **Argv)
{
    /*
      GAME IDEA:
      Breakout, but the player paddle can be rotated using the mouse, and can shoot balls/bullets
     */

    Argc; Argv; // This makes the compiler not throw a warning for unused variables.

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

    Window       = P_CreateOpenGLWindow("Glow", 1366, 768);
    Renderer     = R_CreateRenderer(Window);
    Keyboard     = I_CreateKeyboard();
    Mouse        = I_CreateMouse();
    Clock        = P_CreateClock();
    SoundSystem  = S_CreateSoundSystem();
    Camera       = R_CreateCamera(Window->Width, Window->Height, glm::vec3(0.0f, 0.0f, 11.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    SDL_Event Event;
    while(IsRunning)
    {
        P_UpdateClock(Clock);
        R_CalculateFPS(Renderer, Clock);

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

            I_UpdateKeyboard(Keyboard);
            I_UpdateMouse(Mouse);
            // Get Mouse World Position
            Mouse->WorldPosition.x = MapRange((f32)(Mouse->X), 0.0f, (f32)Window->Width, Camera->Position.x - HalfWorldWidth, Camera->Position.x + HalfWorldWidth);
            Mouse->WorldPosition.y = MapRange((f32)(Mouse->Y), 0.0f, (f32)Window->Height, Camera->Position.y + HalfWorldHeight, Camera->Position.y - HalfWorldHeight);

            switch(CurrentState)
            {
                case State_Initial:
                {
                    if(I_IsPressed(SDL_SCANCODE_ESCAPE))
                    {
                        IsRunning = 0;
                    }

                    if(I_IsPressed(SDL_SCANCODE_SPACE))
                    {
                        CurrentState = State_Game;
                    }

                    if(I_IsReleased(SDL_SCANCODE_RETURN) && I_IsPressed(SDL_SCANCODE_LALT))
                    {
                        P_ToggleFullscreen(Window);
                        R_ResizeRenderer(Renderer, Window->Width, Window->Height);
                    }
                    break;
                }
                case State_Game:
                {
                    // Game state input handling
                    if(I_IsPressed(SDL_SCANCODE_ESCAPE))
                    {
                        CurrentState = State_Pause;
                    }

                    if(I_IsPressed(SDL_SCANCODE_LSHIFT))
                    {
                        // Camera Stuff
                        if(Mouse->FirstMouse)
                        {
                            Camera->Yaw = -90.0f; // Set the Yaw to -90 so the mouse faces to 0, 0, 0 in the first frame X
                            Camera->Pitch = 0.0f;
                            Mouse->FirstMouse = false;
                        }
                        Camera->Yaw += Mouse->RelX * Mouse->Sensitivity;
                        Camera->Pitch += -Mouse->RelY *Mouse->Sensitivity; // reversed since y-coordinates range from bottom to top
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
                    if(I_IsReleased(SDL_SCANCODE_RETURN) && I_IsPressed(SDL_SCANCODE_LALT))
                    {
                        P_ToggleFullscreen(Window);
                        R_ResizeRenderer(Renderer, Window->Width, Window->Height);
                    }

                    // Handle Camera Input
                    if(I_IsPressed(SDL_SCANCODE_W) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Camera->Position += Camera->Front * Camera->Speed * (f32)Clock->DeltaTime;
                    }
                    if(I_IsPressed(SDL_SCANCODE_S) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Camera->Position -= Camera->Speed * Camera->Front * (f32)Clock->DeltaTime;
                    }
                    if(I_IsPressed(SDL_SCANCODE_A) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Camera->Position -= glm::normalize(glm::cross(Camera->Front, Camera->Up)) * Camera->Speed * (f32)Clock->DeltaTime;
                    }
                    if(I_IsPressed(SDL_SCANCODE_D) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Camera->Position += glm::normalize(glm::cross(Camera->Front, Camera->Up)) * Camera->Speed * (f32)Clock->DeltaTime;
                    }
                    if(I_IsPressed(SDL_SCANCODE_SPACE) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                    {
                        R_ResetCamera(Camera, Window->Width, Window->Height,
                                      glm::vec3(0.0f, 0.0f, 23.0f),
                                      glm::vec3(0.0f, 0.0f, -1.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
                        I_ResetMouse(Mouse);
                    }

                    if(I_IsPressed(SDL_SCANCODE_W) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {

                    }
                    if(I_IsPressed(SDL_SCANCODE_S) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {

                    }
                    if(I_IsPressed(SDL_SCANCODE_A) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {

                    }
                    if(I_IsPressed(SDL_SCANCODE_D) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {

                    }
                    if(I_IsPressed(SDL_SCANCODE_SPACE) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {

                    }

                    if(I_IsPressed(SDL_SCANCODE_UP))
                    {
                        Renderer->Exposure += 0.01f;
                    }
                    if(I_IsPressed(SDL_SCANCODE_DOWN))
                    {
                        Renderer->Exposure -= 0.01f;
                    }

                    if(I_IsPressed(SDL_SCANCODE_P))
                    {
                        S_SetMusicVolume(SoundSystem, ++SoundSystem->MusicVolume);
                        S_SetEffectsVolume(SoundSystem, ++SoundSystem->EffectsVolume);
                    }
                    if(I_IsPressed(SDL_SCANCODE_N))
                    {
                        S_SetMusicVolume(SoundSystem, --SoundSystem->MusicVolume);
                        S_SetEffectsVolume(SoundSystem, --SoundSystem->EffectsVolume);
                    }

                    if(I_IsMouseButtonPressed(SDL_BUTTON_LEFT))
                    {

                    }
                    if(I_IsMouseButtonPressed(SDL_BUTTON_RIGHT))
                    {

                    }

                    break;
                }
                case State_Pause:
                {
                    if(I_IsPressed(SDL_SCANCODE_ESCAPE) && I_WasNotPressed(SDL_SCANCODE_ESCAPE))
                    {
                        IsRunning = 0;
                    }

                    if(I_IsPressed(SDL_SCANCODE_SPACE) && I_WasNotPressed(SDL_SCANCODE_SPACE))
                    {
                        CurrentState = State_Game;
                    }

                    if(I_IsReleased(SDL_SCANCODE_RETURN) && I_IsPressed(SDL_SCANCODE_LALT))
                    {
                        P_ToggleFullscreen(Window);
                        R_ResizeRenderer(Renderer, Window->Width, Window->Height);
                    }
                    break;
                }
                default:
                {
                    // Invalid code path
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", "InputHandling invalid code path: default", Window->Handle);
                    exit(0);
                    break;
                }
            }

            {  // SECTION: Update
                switch(CurrentState)
                {
                    case State_Initial:
                    {

                    } break;
                    case State_Game:
                    {

                    } break;
                    case State_Pause:
                    {
                        break;
                    }
                    default:
                    {
                        // Invalid code path
                        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", "UpdateState invalid code path: default", Window->Handle);
                        exit(0);;
                        break;
                    }
                }
            }

            R_UpdateCamera(Renderer, Camera);

            // FPS
            char WindowTitle[60];
            sprintf_s(WindowTitle, sizeof(WindowTitle),"Untitled - FPS: %2.2f", Renderer->FPS);
            SDL_SetWindowTitle(Window->Handle, WindowTitle);

         } // END: Update

        { // SECTION: Render

            R_BeginFrame(Renderer);
            switch(CurrentState)
            {
                case State_Initial:
                {
                    Renderer->BackgroundColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
                    break;
                }
                case State_Game:
                {
                    Renderer->BackgroundColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                    break;
                }
                case State_Pause:
                {
                    Renderer->BackgroundColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
                    break;
                }
                default:
                {
                    break;
                }
            }
            R_EndFrame(Renderer);

        } // SECTION END: Render
    }

    SDL_GL_DeleteContext(Window->Handle);

    return 0;
}
