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

/*
  TODO:
  STB_Image HDR Loading
  Create and Draw Sprites
  Draw the sprites with HDR floating point values in order to make them glow
  DrawText2D
  PlaySound
  PlayMusic
  AABB Collision Detection
 */

// TODO(Jorge): Create game.cpp and put al things that belong to the game in here, such as camera, etc..
// TODO(Jorge): Replace every printf with MessageBox!
// TODO: Make sure everything is gamma corrected, lighting calculations also need to be done with Gamma correction!
// TODO: DrawDebugLine();
// TODO: DrawLine();
// TODO: Test Tweening Functions!
// TODO(Jorge): Shader Hot reloading: http://antongerdelan.net/opengl/shader_hot_reload.html
// TODO(Jorge): Create a lot of bullets and render them using instanced rendering
// TODO(Jorge): Colors are different while rendering with nVidia card, and Intel card
// TODO(Jorge): Add License to all files
// TODO(Jorge): Changing focus to the console window clicking on it and switching to the main game window crashes the program. Is this a bug?
// TODO(Jorge): Remove unused functions from final version

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

void
DrawText2D(renderer *Renderer, char *Text, font *Font, glm::vec2 Position, glm::vec2 Scale, glm::vec3 Color)
{
    Assert(Renderer);
    Assert(Text);
    Assert(Font);

    glUseProgram(Renderer->Shaders.Text);
    glm::mat4 Identity = glm::mat4(1.0f);
    R_SetUniform(Renderer->Shaders.Text, "Model", Identity);
    R_SetUniform(Renderer->Shaders.Text, "View", Identity);
    // TODO(Jorge): We are using the camera global, fix this shit
    R_SetUniform(Renderer->Shaders.Text, "Projection", Camera->Ortho);
    R_SetUniform(Renderer->Shaders.Text, "TextColor", Color);

    glActiveTexture(GL_TEXTURE0); // TODO: Read why do we need to activate textures! NOTE: read this https://community.khronos.org/t/when-to-use-glactivetexture/64913
    glBindVertexArray(Renderer->TextVAO);

    // Iterate through all the characters in string
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];

        f32 XPos = Position.x + Ch.Bearing.x * Scale.x;
        f32 YPos = Position.y - (Ch.Size.y - Ch.Bearing.y) * Scale.y;

        f32 W = Ch.Size.x * Scale.x;
        f32 H = Ch.Size.y * Scale.y;

        // Update VBO for each character
        f32 QuadVertices[6][3] =
        {
            { XPos,     YPos + H, 0.0f},
            { XPos,     YPos,     0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos,     YPos + H, 0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos + W, YPos + H, 0.0f}
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, Ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, Renderer->TextVertexBuffer); // Update content of Vertex buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render  Quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        Position.x += (Ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

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

    texture *BlackHole = R_CreateTexture("textures/Black Hole.png");
    texture *Bullet = R_CreateTexture("textures/Bullet.png");
    texture *Glow = R_CreateTexture("textures/Glow.png");
    texture *Laser = R_CreateTexture("textures/Laser.png");
    texture *Player = R_CreateTexture("textures/Player.png");
    texture *Pointer = R_CreateTexture("textures/Pointer.png");
    texture *Seeker = R_CreateTexture("textures/Seeker.png");
    texture *Wanderer = R_CreateTexture("textures/Wanderer.png");

    font *Arial = R_CreateFont(MainRenderer, "fonts/NovaSquare-Regular.ttf", 60, 0);

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
                    R_ResizeRenderer(MainRenderer, Window->Width, Window->Height);
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

                if(I_IsPressed(SDL_SCANCODE_SPACE) && I_IsPressed(SDL_SCANCODE_LSHIFT))
                {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }
            }

            // Update Camera Matrices
            Camera->View = glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
            Camera->Projection = glm::perspective(glm::radians(Camera->FoV), (f32)WindowWidth / (f32)WindowHeight, Camera->Near, Camera->Far);

        } // SECTION END: Update

        { // SECTION: Render

            R_BeginFrame(MainRenderer);
            DrawText2D(MainRenderer, "Score: 516", Arial, glm::vec2(30, 40), glm::vec2(1.0f), glm::vec3(0.1f, 0.0f, 1.0f));
            R_DrawTexture(MainRenderer, Camera, Pointer, PlayerPosition, glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), Angle);

            // { // Render Cube, TODO(Jorge): Create a function for this!
            //     glUseProgram(MainRenderer->CubeShader);
            //     glm::mat4 Model = glm::mat4(1.0);
            //     Model = glm::translate(Model, PlayerPosition);
            //     R_SetUniform(MainRenderer->CubeShader, "Model", Model);
            //     R_SetUniform(MainRenderer->CubeShader, "View", Camera->View);
            //     R_SetUniform(MainRenderer->CubeShader, "Projection", Camera->Projection);
            //     R_SetUniform(MainRenderer->CubeShader, "Color", glm::vec3(2.0f, 2.0f, 2.0f));
            //     glBindVertexArray(MainRenderer->CubeVAO);
            //     // Render Glowing Cube
            //     glDrawArrays(GL_TRIANGLES, 0, 36);
            //     // Render not glowing cube
            //     Model = glm::translate(Model, glm::vec3(-3.0f, 0.0f, 0.0f));
            //     Model = glm::mat4(1.0f);
            //     R_SetUniform(MainRenderer->CubeShader, "Model", Model);
            //     R_SetUniform(MainRenderer->CubeShader, "Color", glm::vec3(0.0f, 0.0f, 1.0f));
            //     glDrawArrays(GL_TRIANGLES, 0, 36);
            //     glBindVertexArray(0);
            //     glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // }

            R_EndFrame(MainRenderer);
        } // SECTION END: Render
    }

    SDL_GL_DeleteContext(Window->Handle);

    return 0;
}
