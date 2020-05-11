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
#include "entity.cpp"

// TODO(Jorge): Rewrite rendering using spritebatch or instanced rendering.
// TODO(Jorge): Sound system should be able to play two sound effects atop of each other!
// TODO(Jorge): Colors are different while rendering with nVidia card, and Intel card
// TODO(Jorge): Add License to all files
// TODO(Jorge): Remove unused functions from final version
// TODO(Jorge): Delele all unused data files

enum state
{
    State_Initial,
    State_Game,
    State_Pause,
    State_Gameover,
};

//
// Globals
//
global i32 WindowWidth = 1366;
global i32 WindowHeight = 768;
global b32 ShowDebugText = 0;
global b32 IsRunning = 1;
global f32 WorldBottom = -11.0f;
global f32 WorldTop = 11.0f;
global f32 WorldLeft = -20.0f;
global f32 WorldRight = 20.0f;
global f32 HalfWorldWidth = WorldRight;
global f32 HalfWorldHeight = WorldTop;
global glm::vec2 MouseWorldPosition;
global f32 BulletSizeX = 1.0f;
global f32 BulletSizeY = 1.0f;
global f32 BulletSizeZ = 1.0f;
global i32 MaxBullets = 100;
global keyboard *Keyboard;
global mouse *Mouse;
global clock *Clock;
global window *Window;
global renderer *MainRenderer; // TODO: Rename to "Renderer"
global sound_system *SoundSystem;
global camera *Camera;
global state CurrentState = State_Initial;

entity *Bullet;

global std::vector<entity*> Bullets; // TODO: Delete me!

i32 main(i32 Argc, char **Argv)
{
    Argc; Argv; // This makes the compiler not throw a warning for unused variables.

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS); // TODO: Init only the subsystems we need!

    Window       = P_CreateOpenGLWindow("Glow", WindowWidth, WindowHeight);
    MainRenderer = R_CreateRenderer(Window);
    Keyboard     = I_CreateKeyboard();
    Mouse        = I_CreateMouse();
    Clock        = P_CreateClock();
    SoundSystem  = S_CreateSoundSystem();

    Camera = R_CreateCamera(WindowWidth, WindowHeight,
                            glm::vec3(0.0f, 0.0f, 11.5f),
                            glm::vec3(0.0f, 0.0f, -1.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));

    texture *PlayerTexture = R_CreateTexture("textures/Seeker.png");
    texture *EnemyTexture =  R_CreateTexture("textures/Black Hole.png");
    // texture *BulletTexture = R_CreateTexture("textures/MyOwnBullet.png");
    texture *BulletTexture = R_CreateTexture("textures/Bullet1.png");
    texture *BackgroundTexture = R_CreateTexture("textures/background.jpg");

    font *NovaSquare = R_CreateFont(MainRenderer, "fonts/NovaSquare-Regular.ttf", 60, 0);

    sound_effect *Test = S_CreateSoundEffect("audio/shoot-01.wav");
    sound_music *TestMusic = S_CreateMusic("audio/Music.mp3");
    S_SetMusicVolume(SoundSystem, 0);
    S_PlayMusic(TestMusic);

    // TODO: Refactor
    f32 PlayerSpeed = 8.0f;
    f32 PlayerDrag = 0.8f;
    entity *Player = E_CreateEntity(PlayerTexture,
                                    glm::vec3(0.0f), glm::vec3(1.0f), // Position, Size
                                    glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, // Direction, RotationAngle
                                    PlayerSpeed, PlayerDrag); // Speed, Drag
    f32 EnemySpeed = 8.0f;
    f32 EnemyDrag = 0.8f;
    entity *Enemy = E_CreateEntity(EnemyTexture,
                                   glm::vec3(0.0f), glm::vec3(1.0f), // Position, Size
                                   glm::vec3(0.0f), 0.0f, // Direction, RotationAngle
                                   EnemySpeed, EnemyDrag); // Speed, Drag

    // TODO: Create a VBO that holds the Camera, View Uniforms. Update
    // it everyframe. Remove every camera *Camera Input in every
    // function.

    SDL_Event Event;
    while(IsRunning)
    {
        P_UpdateClock(Clock);
        R_CalculateFPS(MainRenderer, Clock);

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
            MouseWorldPosition.x = MapRange((f32)(Mouse->X), 0.0f, (f32)Window->Width, Camera->Position.x - HalfWorldWidth, Camera->Position.x + HalfWorldWidth);
            MouseWorldPosition.y = MapRange((f32)(Mouse->Y), 0.0f, (f32)Window->Height, Camera->Position.y + HalfWorldHeight, Camera->Position.y - HalfWorldHeight);

            switch(CurrentState)
            {
                case State_Initial:
                {
                    // Do initial input handling here
                    if(I_IsPressed(SDL_SCANCODE_ESCAPE))
                    {
                        IsRunning = 0;
                    }

                    if(I_IsPressed(SDL_SCANCODE_SPACE))
                    {
                        CurrentState = State_Game;
                    }

                    // Handle Window input stuff
                    if(I_IsReleased(SDL_SCANCODE_RETURN) && I_IsPressed(SDL_SCANCODE_LALT))
                    {
                        P_ToggleFullscreen(Window);
                        R_ResizeRenderer(MainRenderer, Window->Width, Window->Height);
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

                    // Debug Text
                    if(I_IsReleased(SDL_SCANCODE_F1))
                    {
                        ShowDebugText = !ShowDebugText;
                    }

                    // Handle Window input stuff
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
                        R_ResetCamera(Camera, WindowWidth, WindowHeight,
                                      glm::vec3(0.0f, 0.0f, 23.0f),
                                      glm::vec3(0.0f, 0.0f, -1.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
                        I_ResetMouse(Mouse);
                    }

                    if(I_IsPressed(SDL_SCANCODE_W) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Player->Acceleration.y += Player->Speed;
                    }
                    if(I_IsPressed(SDL_SCANCODE_S) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Player->Acceleration.y -= Player->Speed;
                    }
                    if(I_IsPressed(SDL_SCANCODE_A) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Player->Acceleration.x -= Player->Speed;
                    }
                    if(I_IsPressed(SDL_SCANCODE_D) && I_IsNotPressed(SDL_SCANCODE_LSHIFT))
                    {
                        Player->Acceleration.x += Player->Speed;
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
                        if(Bullets.size() < MaxBullets)
                        {
                            // TODO: Refactor
                            f32 DeltaX = MouseWorldPosition.x - Player->Position.x;
                            f32 DeltaY = MouseWorldPosition.y - Player->Position.y;
                            f32 RotationAngle = (((f32)atan2(DeltaY, DeltaX) * (f32)180.0f) / 3.14159265359f);
                            f32 BulletSpeed = 10.0f;
                            f32 BulletDragCoefficient =  1.0f;
                            Bullet = E_CreateEntity(BulletTexture,
                                                    Player->Position,
                                                    glm::vec3(BulletSizeX, BulletSizeY, BulletSizeZ),
                                                    glm::normalize(glm::vec3(MouseWorldPosition.x, MouseWorldPosition.y, 0.0f) - Player->Position),
                                                    RotationAngle,
                                                    BulletSpeed,
                                                    BulletDragCoefficient);
                            Bullets.push_back(Bullet);
                        }
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

                    // Handle Window input stuff
                    if(I_IsReleased(SDL_SCANCODE_RETURN) && I_IsPressed(SDL_SCANCODE_LALT))
                    {
                        P_ToggleFullscreen(Window);
                        R_ResizeRenderer(MainRenderer, Window->Width, Window->Height);
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
                        E_CalculateMotion(Player, (f32)Clock->DeltaTime);

                        { // SECTION: Update Bullets
                            for(u32 i = 0;
                                i < Bullets.size();
                                i++)
                            {
                                entity *BulletA = Bullets[i];
                                BulletA->Direction = glm::normalize(BulletA->Direction);
                                BulletA->Acceleration = BulletA->Direction * BulletA->Speed;
                                // TODO: Update Entity.Rect;
                                E_CalculateMotion(BulletA, (f32)Clock->DeltaTime);
                            }
                        }

                        if(Player->Position.x < WorldLeft)
                        {
                            Player->Position.x = WorldLeft;
                            Player->Velocity.x = 0.0f;
                        }
                        else if(Player->Position.x > WorldRight)
                        {
                            Player->Position.x = WorldRight;
                            Player->Velocity.x = 0.0f;
                        }

                        if(Player->Position.y < WorldBottom)
                        {
                            Player->Position.y = WorldBottom;
                            Player->Velocity.y = 0.0f;
                        }
                        else if(Player->Position.y > WorldTop)
                        {
                            Player->Position.y = WorldTop;
                            Player->Velocity.y = 0.0f;
                        }

                        // Update Player Rotation Angle
                        f32 DeltaX = MouseWorldPosition.x - Player->Position.x;
                        f32 DeltaY = MouseWorldPosition.y - Player->Position.y;
                        Player->RotationAngle = (((f32)atan2(DeltaY, DeltaX) * (f32)180.0f) / 3.14159265359f);

                        // Remove Bullets that are far away
                        for(i32 i = 0;
                            i < Bullets.size();
                            i++)
                        {
                            if( Abs(Bullets[i]->Position.x) > WorldRight ||
                                Abs(Bullets[i]->Position.y) > WorldTop)
                            {
                                Bullets.erase( Bullets.begin() + i ); // Deleting the fourth element
                            }
                        }

                        // Check for colliisions with all bullets
                        for(i32 i = 0; i < Bullets.size(); i++)
                        {
                            if(E_EntitiesCollide(Bullets[i], Enemy))
                            {
                                printf("Bullet Collision!\n");
                            }

                        }
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

            // Update Camera Matrices
            // TODO: Camera Values should go into UBO
            // TODO: Update Camera UBO Right here!
            Camera->View = glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
            Camera->Projection = glm::perspective(glm::radians(Camera->FoV), (f32)WindowWidth / (f32)WindowHeight, Camera->Near, Camera->Far);
            Camera->Ortho = glm::ortho(0.0f, (f32)WindowWidth, 0.0f, (f32)WindowHeight);

            // FPS
            char WindowTitle[60];
            sprintf_s(WindowTitle, sizeof(WindowTitle),"Glow - FPS: %2.2f", MainRenderer->FPS);
            SDL_SetWindowTitle(Window->Handle, WindowTitle);

            // TODO: Remove
            printf("Number of items in Bullets vector: %zd\n", Bullets.size());

         } // END: Update

        { // SECTION: Render

            R_BeginFrame(MainRenderer);

            // Draw Background
            R_DrawTexture(MainRenderer, Camera,
                          BackgroundTexture, glm::vec3(0.0f, 0.0f, -10.0f),
                          glm::vec3(100.0f, 50.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f);

            switch(CurrentState)
            {
                case State_Initial:
                {
                    R_DrawText2D(MainRenderer, Camera, "Glow", NovaSquare,
                                 glm::vec2( (1366 / 2) - 120, 768 - 140), // Position
                                 glm::vec2(2.0f), glm::vec3(2.0f, 2.0f, 2.0f)); // Scale, Color
                    R_DrawText2D(MainRenderer, Camera, "Press Space to Begin", NovaSquare,
                                 glm::vec2( (1366 / 2) - 300, 768 - 430), // Position
                                 glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)); // Scale, Color
                    break;
                }
                case State_Game:
                {

                    R_DrawEntity(MainRenderer, Camera, Player);
                    R_DrawEntity(MainRenderer, Camera, Enemy);

                    { // SECTION: Render Bullets
                        for(u32  i = 0;
                            i < Bullets.size();
                            i++)
                        {
                            R_DrawEntity(MainRenderer, Camera, Bullets[i]);
                        }
                    }

                    if(ShowDebugText)
                    {
                        char TextBuffer[60]; // Buffer used for rendering text using sprintf_s

                        // FPS
                        sprintf_s(TextBuffer, sizeof(TextBuffer),"FPS: %2.2f", MainRenderer->FPS);
                        R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 15), glm::vec2(0.25f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // Renderer Exposure
                        sprintf_s(TextBuffer, sizeof(TextBuffer),"Renderer->Exposure: %2.2f", MainRenderer->Exposure);
                        R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 30), glm::vec2(0.25f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // OpenGL variables
                        R_DrawText2D(MainRenderer, Camera, (char*)MainRenderer->HardwareVendor, NovaSquare, glm::vec2(0, WindowHeight - 45), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));
                        R_DrawText2D(MainRenderer, Camera, (char*)MainRenderer->HardwareModel, NovaSquare, glm::vec2(0, WindowHeight - 60), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));
                        R_DrawText2D(MainRenderer, Camera, (char*)MainRenderer->OpenGLVersion, NovaSquare, glm::vec2(0, WindowHeight - 75), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));
                        R_DrawText2D(MainRenderer, Camera, (char*)MainRenderer->GLSLVersion, NovaSquare, glm::vec2(0, WindowHeight - 100), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // Player Position
                        sprintf_s(TextBuffer, sizeof(TextBuffer),"Player->Position: %2.2f,%2.2f", Player->Position.x, Player->Position.y);
                        R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 115), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // Camera Position
                        sprintf_s(TextBuffer, sizeof(TextBuffer),"Camera->Position: %2.2f,%2.2f,%2.2f", Camera->Position.x, Camera->Position.y, Camera->Position.z);
                        R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 130), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // Sound System
                        sprintf_s(TextBuffer, sizeof(TextBuffer),"MusicVolume: %d", SoundSystem->MusicVolume);
                        R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 145), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));
                        sprintf_s(TextBuffer, sizeof(TextBuffer),"EffectsVolume: %d", SoundSystem->EffectsVolume);
                        R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 160), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));

                        sprintf_s(TextBuffer, sizeof(TextBuffer),"Mouse Position: x:%d y:%d", Mouse->X, Mouse->Y);
                        R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 180), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));


                        sprintf_s(TextBuffer, sizeof(TextBuffer),"World Mouse Position: x:%2.2f y:%2.2f", MouseWorldPosition.x, MouseWorldPosition.y);
                        R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 200), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));

                        sprintf_s(TextBuffer, sizeof(TextBuffer),"Rotation Angle: x:%2.2f", Player->RotationAngle);
                        R_DrawText2D(MainRenderer, Camera, TextBuffer, NovaSquare, glm::vec2(0, WindowHeight - 220), glm::vec2(0.2f), glm::vec3(1.0f, 1.0f, 1.0f));

                    }

                    break;
                }
                case State_Pause:
                {
                    R_DrawText2D(MainRenderer, Camera, "Glow", NovaSquare,
                                 glm::vec2( (1366 / 2) - 120, 768 - 140), // Position
                                 glm::vec2(2.0f), glm::vec3(1.0f, 1.0f, 1.0f)); // Scale, Color
                    R_DrawText2D(MainRenderer, Camera, "Press Space to Continue", NovaSquare,
                                 glm::vec2( (1366 / 2) - 360, 768 - 430), // Position
                                 glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)); // Scale, Color
                    R_DrawText2D(MainRenderer, Camera, "Press Escape to Quit", NovaSquare,
                                 glm::vec2( (1366 / 2) - 300, 768 - 600), // Position
                                 glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)); // Scale, Color
                    break;
                }
                default:
                {
                    break;
                }
            }

            R_EndFrame(MainRenderer);

        } // SECTION END: Render
    }

    SDL_GL_DeleteContext(Window->Handle);

    return 0;
}
