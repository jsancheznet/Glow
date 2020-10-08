#ifdef _WIN32
extern "C"
{
    // http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
    __declspec( dllexport ) unsigned long int NvOptimusEnablement = 0x00000001;
    // https://gpuopen.com/amdpowerxpressrequesthighperformance/
    __declspec( dllexport ) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include <stdio.h>

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

// TODO(Jorge): Once the project is done. Pull out the SAT implementation into a single header library so it may be reused in other pojects.
// TODO(Jorge): Once sat algorithm is in a single file header lib, make a blog post detailing how SAT works.


// TODO(Jorge): Remove GLM and implement my own math library
// TODO(Jorge): Run through a static code analyzer to find new bugs. maybe clang-tidy? cppcheck?, scan-build?
// TODO(Jorge): Textures transparent background is not blending correctly
// TODO(Jorge): Make Bullets little! everything will look better.
// TODO(Jorge): Sound system should be able to play two sound effects on top of each other!
// TODO(Jorge): Colors are different while rendering with nVidia card, and Intel card
// TODO(Jorge): Add License to all files, use unlicense or CC0. It seems lawyers and github's code like a standard license rather than simplu stating public domain.
// TODO(Jorge): Remove unused functions from final version
// TODO(Jorge): Delete all unused data files
// TODO(Jorge): Make sure all movement uses DeltaTime so movement is independent from framerate
// TODO(Jorge): Implement R_DrawText2DCentered to remove all hardcoded stuff in text rendering

enum state
{
    State_Initial,
    State_Game,
    State_Pause,
    State_Gameover,
};

// Platform
global u32 WindowWidth = 1366;
global u32 WindowHeight = 768;

// Game Variables
global f32 WorldBottom     = -11.0f;
global f32 WorldTop        = 11.0f;
global f32 WorldLeft       = -20.0f;
global f32 WorldRight      = 20.0f;
global f32 HalfWorldWidth  = WorldRight;
global f32 HalfWorldHeight = WorldTop;

// Variables
global b32 IsRunning = 1;
global keyboard     *Keyboard;
global mouse        *Mouse;
global clock        *Clock;
global window       *Window;
global renderer     *Renderer;
global sound_system *SoundSystem;
global camera *Camera;
global state CurrentState = State_Initial;

i32 main(i32 Argc, char **Argv)
{
    /*
      GAME IDEA:
      Breakout, but the player paddle can be rotated using the mouse, and can shoot balls/bullets

      - Limited number of bullets to clear the whole screen!
     */


    Argc; Argv; // This makes the compiler not throw a warning for unused variables.

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

    Window       = P_CreateOpenGLWindow("Glow", WindowWidth, WindowHeight);
    Renderer     = R_CreateRenderer(Window);
    Keyboard     = I_CreateKeyboard();
    Mouse        = I_CreateMouse();
    Clock        = P_CreateClock();
    SoundSystem  = S_CreateSoundSystem();
    Camera       = R_CreateCamera(Window->Width, Window->Height, glm::vec3(0.0f, 0.0f, 11.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    font *DebugFont = R_CreateFont(Renderer, "fonts/arial.ttf", 14, 14);
    font *MenuFont = R_CreateFont(Renderer, "fonts/RobotY.ttf", 100, 100);

    texture *PlayerTexture = R_CreateTexture("textures/Yellow.png");
    texture *FireOpal = R_CreateTexture("textures/FireOpal.png");
    texture *CircleTest = R_CreateTexture("textures/CircleTest.png");

    f32 PlayerSpeed = 4.0f;
    f32 PlayerDrag = 0.8f;
    entity *Player = E_CreateEntity(PlayerTexture,
                                    glm::vec3(0.0f, -10.0f, 0.0f),
                                    glm::vec3(5.0f, 1.0f, 0.0f),
                                    glm::vec3(0.0f),
                                    0.0f,
                                    PlayerSpeed,
                                    PlayerDrag);

    f32 EnemySpeed = 4.0f;
    f32 EnemyDrag = 0.8f;
    entity *Enemy = E_CreateEntity(FireOpal,
                                   glm::vec3(0.0f),
                                   glm::vec3(10.0, 1.0f, 0.0f),
                                   glm::vec3(0.0f),
                                   0.0f,
                                   EnemySpeed,
                                   EnemyDrag);

    f32 BallSpeed = 4.0f;
    f32 BallDrag = 0.8f;
    entity *Ball = E_CreateEntity(CircleTest,
                                  glm::vec3(0.0f, 10.0f, 0.0f),
                                  glm::vec3(5.25f, 5.25f, 0.0f),
                                  glm::vec3(0.0f),
                                  0.0f,
                                  BallSpeed,
                                  BallDrag);

    f32 SquareSpeed = 4.0f;
    f32 SquareDrag = 0.8f;
    entity *Square = E_CreateEntity(FireOpal,
                                    glm::vec3(0.0f, 0.0f, 0.0f),
                                    glm::vec3(4.0f, 4.0f, 0.0f),
                                    glm::vec3(0.0f),
                                    0.0f,
                                    SquareSpeed,
                                    SquareDrag);

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
                                      glm::vec3(0.0f, 0.0f, 11.5f),
                                      glm::vec3(0.0f, 0.0f, -1.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
                        I_ResetMouse(Mouse);
                    }

                    // Player
                    if(I_IsPressed(SDL_SCANCODE_A) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Player->Acceleration.x -= Player->Speed;
                    }
                    if(I_IsPressed(SDL_SCANCODE_D) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Player->Acceleration.x += Player->Speed;
                    }
                    if(I_IsPressed(SDL_SCANCODE_W) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Player->Acceleration.y += Player->Speed;
                    }
                    if(I_IsPressed(SDL_SCANCODE_S) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Player->Acceleration.y -= Player->Speed;
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
            } // SECTION END: Input Handling

            {  // SECTION: Update
                switch(CurrentState)
                {
                    case State_Initial:
                    {
                        break;
                    }
                    case State_Game:
                    {
                        // Player->RotationAngle = GetRotationAngle(Mouse->WorldPosition.x - Player->Position.x, Mouse->WorldPosition.y - Player->Position.y) + 90.0f;
                        Enemy->RotationAngle += 0.1f;
                        Player->RotationAngle -= 0.1f * 2.0f;

                        // Update Entities
                        E_Update(Player, (f32)Clock->DeltaTime);
                        E_Update(Enemy, (f32)Clock->DeltaTime);
                        // E_Update(Ball, (f32)Clock->DeltaTime);

                        // TODO: Animate Ball Size using an animation Curve!
                        f32 Test = EaseOutBounce((f32)Clock->SecondsElapsed / 5.0f);
                        Ball->Position.x = Test;

                        collision_result CollisionResult;
                        if(E_EntitiesCollide(Player, Enemy, &CollisionResult))
                        {
                            glm::vec2 I = CollisionResult.Direction * CollisionResult.Overlap;
                            Player->Position.x += I.x / 2.0f;
                            Player->Position.y += I.y / 2.0f;

                            Enemy->Position.x -= I.x / 2.0f;
                            Enemy->Position.y -= I.y / 2.0f;
                        }

                        break;
                    }
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

         } // SECTION END: Update

        { // SECTION: Render

            R_BeginFrame(Renderer);
            switch(CurrentState)
            {
                case State_Initial:
                {
                    Renderer->BackgroundColor = MenuBackgroundColor;

                    // TODO(Jorge): Implement R_DrawText2DCentered to remove all hardcoded stuff in text rendering
                    f32 HardcodedFontWidth = 38.0f * 1.5f;
                    f32 XPos = (Window->Width / 2.0f) - HardcodedFontWidth * 4;
                    R_DrawText2D(Renderer, "Untitled", MenuFont,
                                 glm::vec2(XPos, Window->Height / 2.0f + 50.0f),
                                 glm::vec2(1.5f, 1.5f),
                                 glm::vec3(1.0f, 1.0f, 1.0f));

                    R_DrawText2D(Renderer, "press space to begin", MenuFont,
                                 glm::vec2(Window->Width / 2.0f - 38.0f * 10.0f * 0.7f, Window->Height / 2.0f - 100.0f),
                                 glm::vec2(0.7, 0.7),
                                 glm::vec3(1.0f, 1.0f, 1.0f));

                    R_DrawText2D(Renderer, "press escape to exit", MenuFont,
                                 glm::vec2(Window->Width / 2.0f - 38.0f * 10.0f * 0.7f, Window->Height / 2.0f - 200.0f),
                                 glm::vec2(0.7, 0.7),
                                 glm::vec3(1.0f, 1.0f, 1.0f));
                    break;
                }
                case State_Game:
                {
                    Renderer->BackgroundColor = BackgroundColor;
                    R_DrawEntity(Renderer, Player);
                    R_DrawEntity(Renderer, Enemy);
                    R_DrawEntity(Renderer, Ball);
                    R_DrawEntity(Renderer, Square);

                    b32 RenderDebugText = true;
                    if(RenderDebugText)
                    {
                        char String[80];

                        // Player
                        sprintf_s(String, "Player Collision Data:");
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(0.0f, Window->Height-DebugFont->Height),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "Center: X:%4.4f Y:%4.4f", Player->Rect.Center.x, Player->Rect.Center.y);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*2),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "HalfWidth: %4.4f", Player->Rect.HalfWidth);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*3),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "HalfHeight: %4.4f", Player->Rect.HalfHeight);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*4),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "Angle: %4.4f", Player->Rect.Angle);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*5),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));


                        // Enemy
                        sprintf_s(String, "Enemy Collision Data:");
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(0.0f, Window->Height-DebugFont->Height*6),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "Center: X:%4.4f Y:%4.4f", Enemy->Rect.Center.x, Enemy->Rect.Center.y);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*7),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "HalfWidth: %4.4f", Enemy->Rect.HalfWidth);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*8),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "HalfHeight: %4.4f", Enemy->Rect.HalfHeight);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*9),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "Angle: %4.4f", Enemy->Rect.Angle);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*10),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));

                        // Mouse
                        sprintf_s(String, "Mouse:");
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(0.0f, Window->Height-DebugFont->Height*11),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "Screen Space Position: X:%d Y:%d", Mouse->X, Mouse->Y);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*12),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(String, "World Position: X:%4.4f Y:%4.4f", Mouse->WorldPosition.x, Mouse->WorldPosition.y);
                        R_DrawText2D(Renderer, String, DebugFont,
                                     glm::vec2(DebugFont->Width*2, Window->Height-DebugFont->Height*13),
                                     glm::vec2(1.0f, 1.0f),
                                     glm::vec3(1.0f, 1.0f, 1.0f));

                    }

                    break;
                }
                case State_Pause:
                {
                    Renderer->BackgroundColor = MenuBackgroundColor;

                    f32 HardcodedFontWidth = 38.0f * 1.5f;
                    f32 XPos = (Window->Width / 2.0f) - HardcodedFontWidth * 2.5f;
                    R_DrawText2D(Renderer, "Pause", MenuFont,
                                 glm::vec2(XPos, Window->Height / 2.0f + 50.0f),
                                 glm::vec2(1.5f, 1.5f),
                                 glm::vec3(1.0f, 1.0f, 1.0f));

                    R_DrawText2D(Renderer, "press space to continue", MenuFont,
                                 glm::vec2(Window->Width / 2.0f - 38.0f * 11.5f * 0.7f, Window->Height / 2.0f - 100.0f),
                                 glm::vec2(0.7, 0.7),
                                 glm::vec3(1.0f, 1.0f, 1.0f));

                    R_DrawText2D(Renderer, "press escape to exit", MenuFont,
                                 glm::vec2(Window->Width / 2.0f - 38.0f * 10.0f * 0.7f, Window->Height / 2.0f - 200.0f),
                                 glm::vec2(0.7, 0.7),
                                 glm::vec3(1.0f, 1.0f, 1.0f));

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
