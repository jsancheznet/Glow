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
global camera *Camera;
global b32 DebugText = 0;

// These variables correspond to the FPS counter, TODO: make them not global
global f32 AverageFPS;
global f32 AverageMillisecondsPerFrame;
global f32 FPSTimerSecondsElapsed = 0.0f;
global f32 FPSCounter = 0.0f;

int main(i32 Argc, char **Argv)
{
    Argc; Argv;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS); // TODO: Init only the subsystems we need!

    Window = P_CreateOpenGLWindow("Untitled", WindowWidth, WindowHeight);
    MainRenderer = R_CreateRenderer(Window);
    Keyboard = I_CreateKeyboard();
    Mouse = I_CreateMouse();
    Clock = P_CreateClock();
    Camera = G_CreateCamera(WindowWidth, WindowHeight);

    texture *BlackHole = R_CreateTexture("textures/Black Hole.png");
    texture *Bullet = R_CreateTexture("textures/Bullet.png");
    texture *Glow = R_CreateTexture("textures/Glow.png");
    texture *Laser = R_CreateTexture("textures/Laser.png");
    texture *Player = R_CreateTexture("textures/Player.png");
    texture *Pointer = R_CreateTexture("textures/Pointer.png");
    texture *Seeker = R_CreateTexture("textures/Seeker.png");
    texture *Wanderer = R_CreateTexture("textures/Wanderer.png");

    font *NovaSquare = R_CreateFont(MainRenderer, "fonts/NovaSquare-Regular.ttf", 60, 0);

    Mix_Music *Music;
    Mix_Chunk *Effect;
    { // Black Mesa audio test chamber brackets
        if(Mix_Init(MIX_INIT_MP3) != MIX_INIT_MP3)
        {
            printf("Error Initializing Mixer: %s\n", Mix_GetError());
        }

        //Initialize SDL_mixer
        if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        {
            printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        }

        // load the MP3 file "music.mp3" to play as music
        Music = Mix_LoadMUS("audio/Music.mp3");
        if(!Music)
        {
            printf("Mix_LoadMUS(\"music.mp3\"): %s\n", Mix_GetError());
            // this might be a critical error...
        }
        Mix_VolumeMusic(0); // 0-128
        // Mix_Music *music; // I assume this has been loaded already
        if(Mix_PlayMusic(Music, -1) == -1)
        {
            printf("Mix_PlayMusic: %s\n", Mix_GetError());
            // well, there's no music, but most games don't break without music...
        }

        Effect = Mix_LoadWAV("audio/shoot-02.wav");
        // Volume is 0 to MIX_MAX_VOLUME (128), this only applies to sound effects
        Mix_Volume(-1, 1);
    }

    glm::vec3 PlayerPosition = glm::vec3(0);
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

                if(I_IsReleased(SDL_SCANCODE_F1))
                {
                    DebugText = !DebugText;
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
                f32 PlayerSpeed = 0.05f;
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
                    Mix_PlayChannel( -1, Effect, 0 );
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
            }

            // Update Camera Matrices
            Camera->View = glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
            Camera->Projection = glm::perspective(glm::radians(Camera->FoV), (f32)WindowWidth / (f32)WindowHeight, Camera->Near, Camera->Far);
            Camera->Ortho = glm::ortho(0.0f, (f32)WindowWidth, 0.0f, (f32)WindowHeight);
        } // END: Update

        { // SECTION: Render

            // TODO: Camera Values should go into UBO
            R_BeginFrame(MainRenderer);

            R_DrawTexture(MainRenderer, Camera, Wanderer, PlayerPosition, glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), Angle);
            R_DrawTexture(MainRenderer, Camera, Player, glm::vec3(10, 10, 0), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), Angle);
            R_DrawTexture(MainRenderer, Camera, Pointer, glm::vec3(20, 10, 0), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), Angle);
            R_DrawTexture(MainRenderer, Camera, BlackHole, glm::vec3(20, 20, 0), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), Angle);
            R_DrawTexture(MainRenderer, Camera, Bullet, glm::vec3(-20, -20, 0), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), Angle);
            R_DrawTexture(MainRenderer, Camera, Laser, glm::vec3(20, -20, 0), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), Angle);
            R_DrawTexture(MainRenderer, Camera, Seeker, glm::vec3(-20, 20, 0), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), Angle);

            if(DebugText)
            {
                char FPSText[20] = {};
                sprintf_s(FPSText, sizeof(FPSText),"FPS: %2.2f", AverageFPS);
                R_DrawText2D(MainRenderer, Camera, FPSText, NovaSquare, glm::vec2(0, WindowHeight - 30), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

                char ExposureText[20] = {};
                sprintf_s(ExposureText, sizeof(ExposureText),"Exposure: %2.2f", MainRenderer->Exposure);
                R_DrawText2D(MainRenderer, Camera, ExposureText, NovaSquare, glm::vec2(0, WindowHeight - 60), glm::vec2(0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
            }

            R_EndFrame(MainRenderer);
        } // SECTION END: Render
    }

    SDL_GL_DeleteContext(Window->Handle);

    return 0;
}
