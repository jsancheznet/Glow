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
#include <SDL_mixer.h>

// NOTE: http://casual-effects.com/data/index.html <--- Cool site for test models
#include "shared.h"
#include "platform.cpp"
#include "input.cpp"
#include "renderer.cpp"
#include "sound.cpp"
#include "game.cpp"

// TODO(Jorge): Shader Hot reloading: http://antongerdelan.net/opengl/shader_hot_reload.html
// TODO(Jorge): Replace every printf with MessageBox!
// TODO: DrawDebugLine();
// TODO: DrawLine();
// TODO: Test Tweening Functions!
// TODO(Jorge): Create a lot of bullets and render them using instanced rendering
// TODO(Jorge): Colors are different while rendering with nVidia card, and Intel card
// TODO(Jorge): Add License to all files
// TODO(Jorge): Remove unused functions from final version

//
// Globals
//
global i32 WindowWidth = 1366;
global i32 WindowHeight = 768;
global b32 IsRunning = 1;
global keyboard *Keyboard;
global mouse *Mouse;
global clock *Clock;
global window *Window;
global renderer *MainRenderer;
global sound_system *SoundSystem;
global camera *Camera;

global b32 ShowDebugText = 0;

// These variables correspond to the FPS counter, TODO: make them not global
global f32 AverageFPS;
global f32 AverageMillisecondsPerFrame;
global f32 FPSTimerSecondsElapsed = 0.0f;
global f32 FPSCounter = 0.0f;

global entity Entity = {};
global entity Enemy = {};

struct rectangle
{
    glm::vec2 Origin;
    glm::vec2 Size;
};

b32 Overlapping(f32 MinA, f32 MaxA, f32 MinB, f32 MaxB)
{
    return MinB <= MaxA && MinA <= MaxB;
}

b32 RectanglesCollide(rectangle A, rectangle B)
{
    /*
      |----------------|
      |                |
      |                |
      |----------------|
     */

    // Compute MinA, MaxA, MinB, MaxB for horizontal plane
    f32 ALeft = A.Origin.x;
    f32 ARight = ALeft + A.Size.x;
    f32 BLeft = B.Origin.x;
    f32 BRight = BLeft + B.Size.x;

    // Compute MinA, MaxA, MinB, MaxB for vertical plane
    f32 ABottom = A.Origin.y;
    f32 ATop = ABottom + A.Size.y;
    f32 BBottom = B.Origin.y;
    f32 BTop = BBottom + B.Size.y;

    return Overlapping(ALeft, ARight, BLeft, BRight) && Overlapping(ABottom, ATop, BBottom, BTop);
}

int main(i32 Argc, char **Argv)
{
    Argc; Argv;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS); // TODO: Init only the subsystems we need!

    Window = P_CreateOpenGLWindow("Untitled", WindowWidth, WindowHeight);
    MainRenderer = R_CreateRenderer(Window);
    Keyboard = I_CreateKeyboard();
    Mouse = I_CreateMouse();
    Clock = P_CreateClock();
    SoundSystem = S_CreateSoundSystem();
    Camera = G_CreateCamera(WindowWidth, WindowHeight);

    texture *Player = R_CreateTexture("textures/Player.png");
    texture *EnemyTexture = R_CreateTexture("textures/Wanderer.png");

    font *NovaSquare = R_CreateFont(MainRenderer, "fonts/NovaSquare-Regular.ttf", 60, 0);

    sound_effect *Test = S_CreateSoundEffect("audio/shoot-01.wav");
    sound_music *TestMusic = S_CreateMusic("audio/Music.mp3");
    S_PlayMusic(TestMusic);

    Entity.Physics.Position = glm::vec3(-4.0f, 4.0f, 0.0f);
    Enemy.Physics.Position = glm::vec3(0.0f);

    f32 Angle = 0.0f;
    SDL_Event Event;
    while(IsRunning)
    {
        Angle += 0.01f;
        P_UpdateClock(Clock);

        { // Compute Average FPS - Average Milliseconds Per Frame
            f32 FramesPerSecondToShow = 2; // How many times per second to calculate fps
            if(FPSTimerSecondsElapsed > (1.0f / FramesPerSecondToShow))
            {
                AverageFPS = FPSCounter / FPSTimerSecondsElapsed;
                AverageMillisecondsPerFrame = (FPSTimerSecondsElapsed / FPSCounter) * 1000.0f;
                FPSCounter = 0;
                FPSTimerSecondsElapsed = 0.0f;
            }
            else
            {
                FPSCounter += 1.0f;
                FPSTimerSecondsElapsed += (f32)Clock->DeltaTime;
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

            {  // SECTION: Update
                I_UpdateKeyboard(Keyboard);
                I_UpdateMouse(Mouse);

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

                // Debug Text
                if(I_IsReleased(SDL_SCANCODE_F1))
                {
                    ShowDebugText = !ShowDebugText;
                }

                // Handle Window input stuff
                if(I_IsPressed(SDL_SCANCODE_ESCAPE))
                {
                    IsRunning = false;
                }
                if(I_IsReleased(SDL_SCANCODE_RETURN) && I_IsPressed(SDL_SCANCODE_LALT))
                {
                    P_ToggleFullscreen(Window);
                    R_ResizeRenderer(MainRenderer, Window->Width, Window->Height);
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
                    G_ResetCamera(Camera, WindowWidth, WindowHeight);
                    I_ResetMouse(Mouse);
                }

                // Handle Player Input
                f32 PlayerSpeed = 600.0f;
                // f32 PlayerSpeed = 3500.0f;
                if(I_IsPressed(SDL_SCANCODE_W) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    Entity.Physics.Acceleration.y += PlayerSpeed;
                }
                if(I_IsPressed(SDL_SCANCODE_S) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    Entity.Physics.Acceleration.y -= PlayerSpeed;
                }
                if(I_IsPressed(SDL_SCANCODE_A) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    Entity.Physics.Acceleration.x -= PlayerSpeed;
                }
                if(I_IsPressed(SDL_SCANCODE_D) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    Entity.Physics.Acceleration.x += PlayerSpeed;
                }
                if(I_IsPressed(SDL_SCANCODE_SPACE) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                {
                    // Jump?!?!?
                    S_PlaySoundEffect(Test);
                }

                if(I_IsPressed(SDL_SCANCODE_UP))
                {
                    MainRenderer->Exposure += 0.01f;
                }
                if(I_IsPressed(SDL_SCANCODE_DOWN))
                {
                    MainRenderer->Exposure -= 0.01f;
                }

                if(I_IsPressed(SDL_SCANCODE_SPACE) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
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
            }

            // Update Camera Matrices
            Camera->View = glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
            Camera->Projection = glm::perspective(glm::radians(Camera->FoV), (f32)WindowWidth / (f32)WindowHeight, Camera->Near, Camera->Far);
            Camera->Ortho = glm::ortho(0.0f, (f32)WindowWidth, 0.0f, (f32)WindowHeight);

            ComputeNewtonMotion(&Entity.Physics, (f32)Clock->DeltaTime);

            if(Entity.Physics.Position.x < -40.0f)
            {
                Entity.Physics.Position.x = -40.0f;
            }
            else if(Entity.Physics.Position.x > 40.0f)
            {
                Entity.Physics.Position.x = 40.0f;
            }

            if(Entity.Physics.Position.y < -20.0f)
            {
                Entity.Physics.Position.y = -20.0f;
            }
            else if(Entity.Physics.Position.y > 20.0f)
            {
                Entity.Physics.Position.y = 20.0f;
            }


            rectangle PlayerRect;
            PlayerRect.Origin.x = Entity.Physics.Position.x;
            PlayerRect.Origin.y = Entity.Physics.Position.y;
            PlayerRect.Size = glm::vec2(1.0f);

            rectangle EnemyRect;
            EnemyRect.Origin.x = Enemy.Physics.Position.x;
            EnemyRect.Origin.y = Enemy.Physics.Position.y;
            EnemyRect.Size = glm::vec2(1.0f);

            b32 Result = RectanglesCollide(PlayerRect, EnemyRect);
            if(Result) printf("Collision!\n"); else printf("\n");

        } // END: Update

        { // SECTION: Render
            // TODO: Camera Values should go into UBO
            R_BeginFrame(MainRenderer);

            R_DrawTexture(MainRenderer, Camera, Player, Entity.Physics.Position, glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0);
            R_DrawTexture(MainRenderer, Camera, EnemyTexture, Enemy.Physics.Position, glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0);

            if(ShowDebugText)
            {
                char TextBuffer[60];
                // FPS
                sprintf_s(TextBuffer, sizeof(TextBuffer),"FPS: %2.2f", AverageFPS);
                R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 30), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

                // Exposure
                sprintf_s(TextBuffer, sizeof(TextBuffer),"Renderer->Exposure: %2.2f", MainRenderer->Exposure);
                R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 60), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

                // OpenGL Thingys
                R_DrawText2D(MainRenderer, Camera, (char*)MainRenderer->HardwareVendor, NovaSquare, glm::vec2(0, WindowHeight - 90), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
                R_DrawText2D(MainRenderer, Camera, (char*)MainRenderer->HardwareModel, NovaSquare, glm::vec2(0, WindowHeight - 120), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
                R_DrawText2D(MainRenderer, Camera, (char*)MainRenderer->OpenGLVersion, NovaSquare, glm::vec2(0, WindowHeight - 150), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
                R_DrawText2D(MainRenderer, Camera, (char*)MainRenderer->GLSLVersion, NovaSquare, glm::vec2(0, WindowHeight - 180), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

                // Player Position
                sprintf_s(TextBuffer, sizeof(TextBuffer),"Player->Position: %2.2f,%2.2f", Entity.Physics.Position.x, Entity.Physics.Position.y);
                R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 210), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

                // Camera Position
                sprintf_s(TextBuffer, sizeof(TextBuffer),"Camera->Position: %2.2f,%2.2f,%2.2f", Camera->Position.x, Camera->Position.y, Camera->Position.z);
                R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 240), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

                // Sound System
                sprintf_s(TextBuffer, sizeof(TextBuffer),"MusicVolume: %d", SoundSystem->MusicVolume);
                R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 270), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
                sprintf_s(TextBuffer, sizeof(TextBuffer),"EffectsVolume: %d", SoundSystem->EffectsVolume);
                R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 300), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
            }

            R_EndFrame(MainRenderer);
        } // SECTION END: Render
    }

    SDL_GL_DeleteContext(Window->Handle);

    return 0;
}
