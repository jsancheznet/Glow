#define DEBUG 1

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

#pragma warning(disable:4127) // GLM fails to compile if this warning is turned on!
#pragma warning(disable: 4201) // GLM warning
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <ft2build.h>
#include FT_FREETYPE_H

// STB Libs
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

// NOTE: http://casual-effects.com/data/index.html <--- Cool site for test models
#include "shared.h"

// TODO(Jorge): Shader Hot reloading: http://antongerdelan.net/opengl/shader_hot_reload.html

// TODO(Jorge): Visualize the different opengl buffers, depth buffer, stencil, color buffer, etc...
// TODO(Jorge): Create a lot of bullets and render them using instanced rendering
// TODO(Jorge): Colors are different while rendering with nVidia card, and Intel card
// TODO(Jorge): Load Sphere model
// TODO(Jorge): Load Tetrahedron model
// TODO(Jorge): Load a Cube!
// TODO(Joreg): HDR Rendering!
// TODO(Jorge): Bloom!
// TODO(Jorge): Draw Tetrahedron correctly
// TODO(Jorge): Add another shape! maybe Torus or Sphere
// TODO(Jorge): Add a uniform Color to renderable_objects
// TODO(Jorge): Add Normals to objects
// TODO(Jorge): Add License to all files
// TODO(Jorge): Changing focus to the console window clicking on it and switching to the main game window crashes the program. Is this a bug?

// IDEAS: Load variables from a file, such as SetSwapInterval
// IDEAS: Stop using relative mouse mode, use regular so nSight can work correctly, and we can have mouse navigatable debug in game menus

struct mouse
{
    u32 ButtonState;
    i32 RelX;
    i32 RelY;
    f32 Sensitivity;
    b32 FirstMouse; // FirstMouse is used only on the first mouse/frame input to avoid a camera jump
};

struct keyboard
{
    const u8 *State;
    u8 *CurrentState;
    u8 *PrevState;
    i32 Numkeys;
    SDL_Keymod ModState;
};

struct clock
{
    u64 PerfCounterNow;
    u64 PerfCounterLast;
    f64 DeltaTime;
    f64 SecondsElapsed;
};

struct camera
{
    // Camera implemented using: https://learnopengl.com/Getting-started/Camera
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;

    f32 Speed;
    f32 Yaw;
    f32 Pitch;

    f32 FoV;
    f32 Near;
    f32 Far;

    glm::mat4 View;
    glm::mat4 Projection;
    glm::mat4 Ortho;
};

struct textured_cube
{
    u32 VAO;
    // u32 EBO;
    u32 VertexBuffer;
    u32 TexCoordsBuffer;
    u32 TextureHandle;
    u32 Shader;
};

struct textured_quad
{
    u32 VAO;
    u32 EBO;
    u32 VertexBuffer;
    u32 TexCoordsBuffer;
    u32 TextureHandle;
    u32 Shader;
};

struct skybox
{
    u32 TextureID;
    u32 VAO;
    u32 VBO;
    u32 Shader;
};


struct character
{
    u32 TextureID;
    glm::ivec2 Size; // Size of glyph
    glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
    u32 Advance; // Offset to advance to next glyph
};

struct font
{
    char *Filename;
    u32 Shader;
    i32 CharacterWidth;
    i32 CharacterHeight;

    character Characters[256];

    u32 VAO;
    u32 VertexBuffer;
    u32 TexCoordsBuffer;
};

struct render_target
{
    u32 Framebuffer; // The main opengl container for attachments
    u32 TextureColorBuffer;
    u32 DepthStencilRenderbuffer;

    // Size of the offscreen buffer
    i32 Width;
    i32 Height;

    // The final screen sized Quad which is used to render to the window
    u32 ScreenQuadVAO;
    u32 ScreenQuadVBO;
    u32 ScreenQuadShader;
};


//
// Globals
//
global SDL_Window *Window = NULL;
global i32 WindowWidth = 1366;
global i32 WindowHeight = 768;
global b32 IsRunning = 1;
global clock Clock;
global mouse Mouse;
global keyboard Keyboard;
global camera Camera;

// These variables correspond to the FPS counter, TODO: make them not global
global f32 AverageFPS;
global f32 AverageMillisecondsPerFrame;
global f32 FPSTimerSecondsElapsed = 0.0f;
global f32 FPSCounter = 0.0f;

void InitMouse()
{
    Mouse.ButtonState = 0;
    Mouse.Sensitivity = 0.3f;
    Mouse.FirstMouse = 1;
    Mouse.RelX = 0;
    Mouse.RelY = 0;
}

void UpdateMouse()
{
    Mouse.ButtonState = SDL_GetRelativeMouseState(&Mouse.RelX, &Mouse.RelY);
}

void InitKeyboard()
{
    Keyboard.State = SDL_GetKeyboardState(&Keyboard.Numkeys);
    Keyboard.CurrentState = (u8*)Malloc(sizeof(u8) * Keyboard.Numkeys);
    Keyboard.PrevState = (u8*)Malloc(sizeof(u8) * Keyboard.Numkeys);
    Assert(Keyboard.PrevState);
    Assert(Keyboard.CurrentState);
    *Keyboard.CurrentState = {};
    *Keyboard.PrevState = {};
}

void UpdateKeyboard()
{
    memcpy((void*)Keyboard.PrevState, (void*)Keyboard.CurrentState, sizeof(u8) * Keyboard.Numkeys);
    Keyboard.State = SDL_GetKeyboardState(NULL);
    memcpy((void*)Keyboard.CurrentState, (void*)Keyboard.State, sizeof(u8) * Keyboard.Numkeys);
    Keyboard.ModState = SDL_GetModState();
}

void InitCamera()
{
    // NOTE(Jorge): This functions makes use of the global_variable camera Camera;
    Camera.Position = glm::vec3(0.0f, 0.0f, 2.0f);
    Camera.Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Camera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    Camera.Speed = 1.5f;
    Camera.FoV = 90.0f;
    Camera.Near = 0.1f;
    Camera.Far = 1500.0f;
    Camera.View = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
    Camera.Projection = glm::perspective(glm::radians(Camera.FoV), (f32)WindowWidth / (f32)WindowHeight, Camera.Near, Camera.Far);
    Camera.Ortho = glm::ortho(0.0f, (f32)WindowWidth, 0.0f, (f32)WindowHeight);
}

void InitClock()
{
    // NOTE(Jorge): This function makes use of the global_variable clock Clock;
    Clock.DeltaTime = 0.0f;
    Clock.PerfCounterNow = SDL_GetPerformanceCounter();
    Clock.PerfCounterLast = 0;
}

void UpdateClock()
{
    // NOTE(Jorge): This functions makes use of the global variable Clock
    Clock.PerfCounterLast = Clock.PerfCounterNow;
    Clock.PerfCounterNow = SDL_GetPerformanceCounter();
    Clock.DeltaTime = (f64)((Clock.PerfCounterNow - Clock.PerfCounterLast)*1000.0f / (f64)SDL_GetPerformanceFrequency() );
    Clock.DeltaTime /= 1000.0f;
    Clock.SecondsElapsed += Clock.DeltaTime;
}

b32 IsPressed(SDL_Scancode Scancode)
{
    return Keyboard.State[Scancode];
}

b32 IsNotPressed(SDL_Scancode Scancode)
{
    return !Keyboard.State[Scancode];
}

b32 IsReleased(SDL_Scancode Scancode)
{
    return (!Keyboard.State[Scancode] && Keyboard.PrevState[Scancode]);
}

char *ReadTextFile(char *Filename)
{
    // IMPORTANT(Jorge): The caller of this function needs to free the allocated pointer!
    Assert(Filename);

    SDL_RWops *RWops = SDL_RWFromFile(Filename, "rb");
    if (RWops == NULL)
    {
        return NULL;
    }

    size_t FileSize = SDL_RWsize(RWops);
    char* Result = (char*)Malloc(FileSize + 1);
    char* Buffer = Result;

    size_t BytesReadTotal = 0, BytesRead = 1;
    while (BytesReadTotal < FileSize && BytesRead != 0)
    {
        BytesRead = SDL_RWread(RWops, Buffer, 1, (FileSize - BytesReadTotal));
        BytesReadTotal += BytesRead;
        Buffer += BytesRead;
    }

    SDL_RWclose(RWops);
    if (BytesReadTotal != FileSize)
    {
        Free(Result);
        return NULL;
    }

    Result[BytesReadTotal] = '\0';

    return Result;
}

void ToggleFullscreen(SDL_Window *WindowIn)
{
    // TODO: Recreate the framebuffer attachments so it corresponds to the current resolution
    Assert(WindowIn);

    u32 WindowFlags = SDL_GetWindowFlags(WindowIn);

    if(WindowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP ||
       WindowFlags & SDL_WINDOW_FULLSCREEN)
    {
        SDL_SetWindowFullscreen(WindowIn, 0);
        SDL_GL_GetDrawableSize(WindowIn, &WindowWidth, &WindowHeight);
        fprintf(stdout, "WINDOWED: Drawable Width: %d\nDrawable Height: %d\n", WindowWidth, WindowHeight);
    }
    else
    {
        SDL_SetWindowFullscreen(WindowIn, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_GL_GetDrawableSize(WindowIn, &WindowWidth, &WindowHeight);
        fprintf(stdout, "FULLSCREEN: Drawable Width: %d\nDrawable Height: %d\n", WindowWidth, WindowHeight);
    }
    glViewport(0, 0, WindowWidth, WindowHeight);
}

u32 CompileShaderObject(const char *Source, GLenum ShaderType)
{
    Assert(Source);

    u32 Shader;
    Shader = glCreateShader(ShaderType);

    if(Shader)
    {
        glShaderSource(Shader, 1, &Source, NULL);
        glCompileShader(Shader);

        i32 Compiled;
        glGetShaderiv(Shader, GL_COMPILE_STATUS, &Compiled);
        if (Compiled != GL_TRUE)
        {
            i32 LogLength = 0;
            char ErrorMessage[1024];
            glGetShaderInfoLog(Shader, 1024, &LogLength, ErrorMessage);

            if(ShaderType == GL_VERTEX_SHADER)
            {
                fprintf(stderr, "Error compiling VERTEX SHADER\n");
            }
            else if(ShaderType == GL_FRAGMENT_SHADER)
            {
                fprintf(stderr, "Error compiling FRAGMENT SHADER\n");
            }
            else if(ShaderType == GL_GEOMETRY_SHADER)
            {
                fprintf(stderr, "Error compiling GEOMETRY SHADER\n");
            }
            fprintf(stderr, "%s\n", ErrorMessage);

            glDeleteShader(Shader);
            Shader = 0;
        }
    }
    else
    {
        fprintf(stderr, "glCreateShader Failed!\n");
    }

    return (Shader);
}

u32 CreateShaderProgram(char *VertexFile, char *FragmentFile)
{
    Assert(VertexFile);
    Assert(FragmentFile);

    char *VertexSource = ReadTextFile(VertexFile);
    Assert(VertexSource);
    char *FragmentSource = ReadTextFile(FragmentFile);
    Assert(FragmentSource);

    u32 Result;
    u32 VertexShader;
    u32 FragmentShader;
    VertexShader = CompileShaderObject(VertexSource, GL_VERTEX_SHADER);
    FragmentShader = CompileShaderObject(FragmentSource, GL_FRAGMENT_SHADER);

    // This function returns 0 if an error occurs creating the program object.
    Result = glCreateProgram();
    if(Result)
    {
        glAttachShader(Result, VertexShader);
        glAttachShader(Result, FragmentShader);
        glLinkProgram(Result);

        i32 IsLinked = 0;
        glGetProgramiv(Result, GL_LINK_STATUS, (GLint *)&IsLinked);
        if (IsLinked == GL_FALSE)
        {
            i32 MaxLogLength = 2048;
            // The maxLength includes the NULL character
            char InfoLog[2048] = {0};
            glGetProgramInfoLog(Result, MaxLogLength, &MaxLogLength, &InfoLog[0]);

            // We don't need the program anymore.
            glDeleteProgram(Result);

            // Use the infoLog as you see fit.
            printf("SHADER PROGRAM FAILED TO COMPILE\\LINK\n");
            printf("%s\n", InfoLog);
        }

    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    Free(VertexSource);
    Free(FragmentSource);

    return (Result);
}

u32 CreateShaderProgram(char *Filename)
{
    Assert(Filename);

    u32 Result;
    char *SourceFile = ReadTextFile(Filename);

    // Compile Vertex Shader
    u32 VertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    char *VertexSource[2] = {"#version 330 core\n#define VERTEX_SHADER\n", SourceFile};
    glShaderSource(VertexShaderObject, 2, VertexSource, NULL);
    glCompileShader(VertexShaderObject);
    i32 Compiled;
    glGetShaderiv(VertexShaderObject, GL_COMPILE_STATUS, &Compiled);
    if (Compiled != GL_TRUE)
    {
        i32 LogLength = 0;
        char ErrorMessage[1024];
        glGetShaderInfoLog(VertexShaderObject, 1024, &LogLength, ErrorMessage);
        fprintf(stderr, "%s-%s\n", Filename, ErrorMessage);
        VertexShaderObject = 0;
    }

    // Compile Fragment Shader
    u32 FragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    char *FragmentSource[2] = {"#version 330 core\n#define FRAGMENT_SHADER\n", SourceFile};
    glShaderSource(FragmentShaderObject, 2, FragmentSource, NULL);
    glCompileShader(FragmentShaderObject);
    // i32 Compiled;
    glGetShaderiv(FragmentShaderObject, GL_COMPILE_STATUS, &Compiled);
    if (Compiled != GL_TRUE)
    {
        i32 LogLength = 0;
        char ErrorMessage[1024];
        glGetShaderInfoLog(FragmentShaderObject, 1024, &LogLength, ErrorMessage);
        fprintf(stderr, "%s-%s\n", Filename, ErrorMessage);
        FragmentShaderObject = 0;
    }

    // Link program
    Result = glCreateProgram();
    glAttachShader(Result, VertexShaderObject);
    glAttachShader(Result, FragmentShaderObject);
    glLinkProgram(Result);
    i32 IsLinked = 0;
    glGetProgramiv(Result, GL_LINK_STATUS, (GLint *)&IsLinked);
    if (IsLinked == GL_FALSE)
    {
        i32 MaxLogLength = 1024;
        char InfoLog[1024] = {0};
        glGetProgramInfoLog(Result, MaxLogLength, &MaxLogLength, &InfoLog[0]);
        printf("%s: SHADER PROGRAM FAILED TO COMPILE/LINK\n", Filename);
        printf("%s\n", InfoLog);
        glDeleteProgram(Result);
        Result = 0;
    }

    glDeleteShader(VertexShaderObject);
    glDeleteShader(FragmentShaderObject);
    Free(SourceFile);

    return Result;
}

void SetShaderUniform(u32 Shader, char *Name, i32 Value)
{
    Assert(Name);
    glUseProgram(Shader);
    glUniform1i(glGetUniformLocation(Shader, Name), Value);
}

void SetShaderUniform(u32 Shader, char *Name, f32 Value)
{
    Assert(Name);
    glUseProgram(Shader);
    glUniform1f(glGetUniformLocation(Shader, Name), Value);
}

void SetShaderUniform(u32 Shader, char *Name, glm::mat4 *Value)
{
    Assert(Name);
    glUseProgram(Shader);
    glUniformMatrix4fv(glGetUniformLocation(Shader, Name), 1, GL_FALSE, glm::value_ptr(*Value));
}

void SetShaderUniform(u32 Shader, char *Name, glm::mat4 Value)
{
    Assert(Name);
    glUseProgram(Shader);
    glUniformMatrix4fv(glGetUniformLocation(Shader, Name), 1, GL_FALSE, glm::value_ptr(Value));
}

void SetShaderUniform(u32 Shader, char *Name, f32 X, f32 Y, f32 Z)
{
    Assert(Name);
    glUseProgram(Shader);
    glUniform3f(glGetUniformLocation(Shader, Name), X, Y, Z);
}

void SetShaderUniform(u32 Shader, char *Name, glm::vec3 Value)
{
    Assert(Name);
    glUseProgram(Shader);
    glUniform3f(glGetUniformLocation(Shader, Name), Value.x, Value.y, Value.z);
}

u32 CreateOpenGLTexture(char *Filename)
{
    Assert(Filename);
    u32 Result = 0;

    // Load Texture
    i32 Width, Height, ChannelCount;
    GLenum InternalFormat, Format;
    stbi_set_flip_vertically_on_load(1);
    i32 RequestedChannelCount = 0;
    u8 *Data = stbi_load(Filename, &Width, &Height, &ChannelCount, RequestedChannelCount);
    if(Data)
    {
        glGenTextures(1, &Result);
        glBindTexture(GL_TEXTURE_2D, Result);
        // Texture Wrapping Parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Texture Filtering Parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if(ChannelCount == 3)
        {
            Format = GL_RGB;
            InternalFormat = GL_SRGB;
        }
        else if(ChannelCount == 4)
        {
            Format = GL_RGBA;
            InternalFormat = GL_SRGB_ALPHA;
        }
        else
        {
            fprintf(stderr, "%s Texture format is not GL_RGB or GL_RGBA\n", Filename);
            stbi_image_free(Data);
            return 0;
        }

        i32 MipMapDetailLevel = 0;
        // REMINDER: Textures to be used for data should not be uploaded as GL_SRGB!
        // NOTE: InternalFormat is the format we want to store the data, Format is the input format
        glTexImage2D(GL_TEXTURE_2D, MipMapDetailLevel, InternalFormat, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
        // NOTE(Jorge): Set custom MipMaps filtering values here!
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(Data);
    }
    else
    {
        printf("Could not load image file: %s\n", Filename);
    }

    return Result;
}

textured_quad *CreateTexturedQuad(char *Filename, u32 Shader)
{
    Assert(Filename);

    if(glIsProgram(Shader) == GL_FALSE)
    {
        printf("Function CreateTexturedQuad: Shader parameter \"Shader\" is invalid in %s:%d\n", __FILE__, __LINE__);
        return NULL;
    }

    textured_quad *Result = (textured_quad*)Malloc(sizeof(textured_quad));
    Assert(Result);
    Result->TextureHandle = CreateOpenGLTexture(Filename);
    Result->Shader = Shader;

    f32 Vertices[] =
    {
        -0.5f, -0.5f, 0.0f, // Lower Left
        0.5f, -0.5f, 0.0f, // Lower Right
        0.5f, 0.5f, 0.0f, // Upper Right
        -0.5f, 0.5f, 0.0f, // Upper Left
    };

    u32 Indices[] =
    {
        0, 1, 2, // First Triangle
        2, 3, 0 // Second Triangle
    };

    f32 TexCoords[] =
    {
        0.0f, 0.0f, // Lower Left
        1.0f, 0.0f, // Lower Right
        1.0f, 1.0f, // Upper Right
        0.0f, 1.0f, // Upper Left
    };

    // VAO
    glGenVertexArrays(1, &Result->VAO);
    glBindVertexArray(Result->VAO);

    // Vertex Buffer
    glGenBuffers(1, &Result->VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Result->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    // glNamedBufferData(Result->VertexBuffer, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Texture Coordinates
    glGenBuffers(1, &Result->TexCoordsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Result->TexCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords), TexCoords, GL_STATIC_DRAW);
    // glNamedBufferData(Result->TexCoordsBuffer, sizeof(TexCoords), TexCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // EBO
    glGenBuffers(1, &Result->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Result->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
    // glNamedBufferData(Result->EBO, sizeof(Indices), Indices, GL_STATIC_DRAW);
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // Tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    SetShaderUniform(Result->Shader, "Image", 0);

    return Result;
}

void DrawTexturedQuad(textured_quad *Quad, glm::vec3 Position, glm::vec3 Size, glm::vec3 RotationAxis, f32 RotationAngle)
{
    // NOTE: Uses global variable Camera

    glUseProgram(Quad->Shader);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(Position));
    Model = glm::scale(Model, Size);
    Model = glm::rotate(Model, glm::radians(RotationAngle), RotationAxis); // NOTE: Do we need to normalize the RotationAngle?
    SetShaderUniform(Quad->Shader, "Model", Model);
    SetShaderUniform(Quad->Shader, "View", Camera.View);
    SetShaderUniform(Quad->Shader, "Projection", Camera.Projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Quad->TextureHandle);
    glUseProgram(Quad->Shader);
    glBindVertexArray(Quad->VAO);
    glDrawElements(GL_TRIANGLES,6, GL_UNSIGNED_INT, 0);
}


void DrawTexturedCube(textured_cube *Cube, glm::vec3 Position, glm::vec3 Scale, glm::vec3 RotationAxis, f32 RotationAngle)
{
    glUseProgram(Cube->Shader);
    glBindVertexArray(Cube->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Cube->TextureHandle);
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position);
    Model = glm::scale(Model, Scale);
    Model = glm::rotate(Model, glm::radians(RotationAngle), RotationAxis);
    SetShaderUniform(Cube->Shader, "Model", &Model);
    SetShaderUniform(Cube->Shader, "View", &Camera.View);
    SetShaderUniform(Cube->Shader, "Projection", &Camera.Projection);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}


textured_cube *CreateTexturedCube(u32 Shader, char *TextureFilename)
{
    Assert(TextureFilename);

    textured_cube *Result = (textured_cube*)Malloc(sizeof(textured_cube));
    Result->Shader = Shader;
    Result->TextureHandle = CreateOpenGLTexture(TextureFilename);

    f32 CubeVertices[] =
    {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &Result->VAO);
    glGenBuffers(1, &Result->VertexBuffer);
    glBindVertexArray(Result->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Result->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), &CubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glUseProgram(Result->Shader);
    SetShaderUniform(Result->Shader, "Texture1", 0);

    return Result;
}

skybox *CreateSkybox(u32 SkyboxShader, char *Right, char *Left, char *Top, char *Bottom, char *Front, char *Back)
{
    Assert(Right);
    Assert(Left);
    Assert(Top);
    Assert(Bottom);
    Assert(Front);
    Assert(Back);

    skybox *Result = NULL;
    Result = (skybox*)Malloc(sizeof(skybox));
    if(Result)
    {
        if(glIsProgram(SkyboxShader))
        {
            Result->Shader = SkyboxShader;
        }
        else
        {
            // Shader Parameter is invalid
            Free(Result);
            return NULL;
        }

        // VAO, VBO, Upload Vertex data
        f32 SkyboxVertices[] =
        {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
        };
        glGenVertexArrays(1, &Result->VAO);
        glGenBuffers(1, &Result->VBO);
        glBindVertexArray(Result->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, Result->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices), &SkyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // Load images, Create cubemap texture
        glGenTextures(1, &Result->TextureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Result->TextureID);

        char *Filenames[] =
        {
            Right,
            Left,
            Top,
            Bottom,
            Front,
            Back,
        };

        i32 CubeFaceCount = 6;
        for(i32 i = 0; i < CubeFaceCount; i++)
        {
            i32 RequestedChannelCount = 0;
            i32 Width, Height, ChannelCount;
            GLenum InputFormat, OutputFormat;
            stbi_set_flip_vertically_on_load(0);
            u8 *Data = stbi_load(Filenames[i], &Width, &Height, &ChannelCount, RequestedChannelCount);
            if(Data)
            {
                if(ChannelCount == 3)
                {
                    InputFormat = GL_SRGB;
                    OutputFormat = GL_RGB;
                }
                else if(ChannelCount == 4)
                {
                    InputFormat = GL_SRGB_ALPHA;
                    OutputFormat = GL_RGBA;
                }
                else
                {
                    fprintf(stderr, "%s Texture format is not GL_RGB or GL_RGBA\n", Filenames[i]);
                    free(Result);
                    stbi_image_free(Data);
                    return NULL;
                }
                // REMINDER: Textures used for data should not be upload with SRGB Format
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, InputFormat, Width, Height, 0, OutputFormat, GL_UNSIGNED_BYTE, Data);
                stbi_image_free(Data);
            }
            else
            {
                printf("CreateSkybox: failed to load %s in %s:%d\n", Filenames[i], __FILE__, __LINE__);
                Free(Result);
                stbi_image_free(Data);
                return NULL;
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    else
    {
        // Failed to allocate memory
        return NULL;
    }

    // NOTE: Shader Configuration
    SetShaderUniform(SkyboxShader, "Skybox", 0);

    return Result;

}

void DrawSkybox(skybox *Skybox)
{
    // TODO(Jorge): Optimize the skybox rendering by doing the section
    // "An Optimization":
    // https://learnopengl.com/Advanced-OpenGL/Cubemaps
    glDepthMask(GL_FALSE);
    glUseProgram(Skybox->Shader);
    glm::mat4 View = glm::mat4(glm::mat3(Camera.View)); // Take the translation part of the mat4 by casting converting it to mat3
    SetShaderUniform(Skybox->Shader, "View", View);
    SetShaderUniform(Skybox->Shader, "Projection", Camera.Projection);
    // skybox cube
    glBindVertexArray(Skybox->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox->TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

font *CreateFont(char *Filename, u32 Shader, i32 Width, i32 Height)
{
    Assert(Filename);
    Assert(Width >= 0);
    Assert(Height >= 0);

    font *Result = (font*)Malloc(sizeof(font));
    Result->Filename = Filename;
    Result->CharacterWidth = Width;
    Result->CharacterHeight = Height;
    Result->Shader = Shader;

    FT_Library FT;
    FT_Face Face;
    if(FT_Init_FreeType(&FT) == 0)
    {
        if(FT_New_Face(FT, Result->Filename, 0, &Face) == 0)
        {
            // Once we've loaded the face, we should define the font size we'd like to extract from this face
            FT_Set_Pixel_Sizes(Face, Result->CharacterWidth, Result->CharacterHeight);

            // Disable byte-alignment restriction
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // NOTE: Do we need to set it back to default?

            for(u8 CurrentChar = 0; CurrentChar < 255; CurrentChar++)
            {
                // Load character glyph
                if(FT_Load_Char(Face, CurrentChar, FT_LOAD_RENDER) == 0)
                {
                    // Generate Texture
                    u32 Texture;
                    glGenTextures(1, &Texture);
                    glBindTexture(GL_TEXTURE_2D, Texture);
                    glTexImage2D(GL_TEXTURE_2D,
                                 0,
                                 GL_RED,
                                 Face->glyph->bitmap.width,
                                 Face->glyph->bitmap.rows,
                                 0,
                                 GL_RED,
                                 GL_UNSIGNED_BYTE,
                                 Face->glyph->bitmap.buffer);

                    // Set Texture Options
                    // NOTE: What's better? clamp to edge or clam to border?
                    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    // Now store character for later use
                    character Character =
                    {
                        Texture,
                        glm::ivec2(Face->glyph->bitmap.width, Face->glyph->bitmap.rows),
                        glm::ivec2(Face->glyph->bitmap_left, Face->glyph->bitmap_top),
                        (u32)Face->glyph->advance.x,
                    };

                    Result->Characters[CurrentChar] = Character;
                    // Result->Characters.insert(std::pair<char, character>(CurrentChar, Character));
                }
                else
                {
                    printf("FT_Load_Char: Error, Freetype Failed to load Glyph %c\n", CurrentChar);
                }
            }
        }
        else
        {
            printf("FT_New_Face failed miserably while loading font %s\n", Result->Filename);
            Free(Result);
            return NULL;
        }
    }
    else
    {
        printf("FT_Init_FreeType failed miserably, could not init FreeType Library\n");
        Free(Result);
        return NULL;
    }
    // Destroy a given FreeType library object and all of its children, including resources, drivers, faces, sizes, etc.
    FT_Done_FreeType(FT);
    // glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // NOTE: Do we need to reset it?

    glGenVertexArrays(1, &Result->VAO);
    // Upload Vertex Data
    glGenBuffers(1, &Result->VertexBuffer);
    glBindVertexArray(Result->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Result->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 3, NULL, GL_DYNAMIC_DRAW); // 6 Vertices, 3 floats each
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), 0);

    // Upload TexCoords
    f32 TexCoords[6][2] =
    {
        {0.0, 0.0f},
        {0.0f, 1.0f},
        {1.0, 1.0},
        {0.0, 0.0},
        {1.0, 1.0},
        {1.0, 0.0},
    };
    glGenBuffers(1, &Result->TexCoordsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Result->TexCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 2, TexCoords, GL_DYNAMIC_DRAW); // 6 Vertices, 2 floats(UV) each
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return Result;
}

void
DrawText2DCentered(char *Text, font *Font, glm::vec2 Position, glm::vec2 Scale, glm::vec3 Color)
{
    Assert(Text);
    Assert(Font);

    glUseProgram(Font->Shader);
    glm::mat4 Identity = glm::mat4(1.0f);
    SetShaderUniform(Font->Shader, "Model", Identity);
    SetShaderUniform(Font->Shader, "View", Identity);
    SetShaderUniform(Font->Shader, "Projection", Camera.Ortho);
    SetShaderUniform(Font->Shader, "TextColor", Color);

    glActiveTexture(GL_TEXTURE0); // TODO: Read why do we need to activate textures! NOTE: read this https://community.khronos.org/t/when-to-use-glactivetexture/64913
    glBindVertexArray(Font->VAO);

    f32 TotalWidth = 0;
    f32 TotalHeight = 0;
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];
        TotalWidth += (Ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64)

        if((f32)Ch.Size.y > TotalHeight)
        {
            TotalHeight = (f32)Ch.Size.y;
        }
    }
    f32 HalfWidth = TotalWidth / 2.0f;
    f32 HalfHeight = TotalHeight / 2.0f;

    // Iterate through all the characters in string
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];

        // f32 XPos = Position.x + Ch.Bearing.x * Scale.x;
        // f32 YPos = Position.y - (Ch.Size.y - Ch.Bearing.y) * Scale.y;

        f32 XPos = Position.x - HalfWidth + Ch.Bearing.x * Scale.x;
        f32 YPos = Position.y - HalfHeight - (Ch.Size.y - Ch.Bearing.y) * Scale.y;

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
        glBindBuffer(GL_ARRAY_BUFFER, Font->VertexBuffer); // Update content of Vertex buffer
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

void
DrawText2D(char *Text, font *Font, glm::vec2 Position, glm::vec2 Scale, glm::vec3 Color)
{
    Assert(Text);
    Assert(Font);

    glUseProgram(Font->Shader);
    glm::mat4 Identity = glm::mat4(1.0f);
    SetShaderUniform(Font->Shader, "Model", Identity);
    SetShaderUniform(Font->Shader, "View", Identity);
    SetShaderUniform(Font->Shader, "Projection", Camera.Ortho);
    SetShaderUniform(Font->Shader, "TextColor", Color);

    glActiveTexture(GL_TEXTURE0); // TODO: Read why do we need to activate textures! NOTE: read this https://community.khronos.org/t/when-to-use-glactivetexture/64913
    glBindVertexArray(Font->VAO);

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
        glBindBuffer(GL_ARRAY_BUFFER, Font->VertexBuffer); // Update content of Vertex buffer
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

void
DrawText3D(char *Text, font *Font, glm::vec3 Position, glm::vec3 Scale, glm::vec3 RotationAxis, f32 RotationAngle, glm::vec3 Color)
{
    Assert(Text);
    Assert(Font);

    // Rotate first, scale second, then translate
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::rotate(Model, glm::radians(RotationAngle), RotationAxis);
    Model = glm::scale(Model, Scale);
    Model = glm::translate(Model, Position);

    glUseProgram(Font->Shader);
    SetShaderUniform(Font->Shader, "Model", Model);
    SetShaderUniform(Font->Shader, "View", Camera.View);
    SetShaderUniform(Font->Shader, "Projection", Camera.Projection);
    SetShaderUniform(Font->Shader, "TextColor", Color);

    glActiveTexture(GL_TEXTURE0); // TODO: Read why do we need to activate textures! NOTE: read this https://community.khronos.org/t/when-to-use-glactivetexture/64913
    glBindVertexArray(Font->VAO);

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
        glBindBuffer(GL_ARRAY_BUFFER, Font->VertexBuffer); // Update content of Vertex buffer
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

void
DrawText3DCentered(char *Text, font *Font, glm::vec3 Position, glm::vec3 Scale, glm::vec3 RotationAxis, f32 RotationAngle, glm::vec3 Color)
{
    Assert(Text);
    Assert(Font);

    // Rotate first, scale second, then translate
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::rotate(Model, glm::radians(RotationAngle), RotationAxis);
    Model = glm::scale(Model, Scale);
    Model = glm::translate(Model, Position);

    glUseProgram(Font->Shader);
    SetShaderUniform(Font->Shader, "Model", Model);
    SetShaderUniform(Font->Shader, "View", Camera.View);
    SetShaderUniform(Font->Shader, "Projection", Camera.Projection);
    SetShaderUniform(Font->Shader, "TextColor", Color);

    glActiveTexture(GL_TEXTURE0); // TODO: Read why do we need to activate textures! NOTE: read this https://community.khronos.org/t/when-to-use-glactivetexture/64913
    glBindVertexArray(Font->VAO);

    f32 TotalWidth = 0;
    f32 TotalHeight = 0;
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];
        TotalWidth += (Ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64)

        if((f32)Ch.Size.y > TotalHeight)
        {
            TotalHeight = (f32)Ch.Size.y * Scale.y;
        }
    }
    f32 HalfWidth = TotalWidth / 2.0f;
    f32 HalfHeight = TotalHeight / 2.0f;

    // Iterate through all the characters in string
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];

        f32 XPos = Position.x - HalfWidth + Ch.Bearing.x * Scale.x;
        f32 YPos = Position.y - HalfHeight - (Ch.Size.y - Ch.Bearing.y) * Scale.y;

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
        glBindBuffer(GL_ARRAY_BUFFER, Font->VertexBuffer); // Update content of Vertex buffer
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

render_target *CreateRenderTarget(char *ShaderFilename, i32 Width, i32 Height)
{
    // NOTE: If switching to fullscreen and back is slow, we can
    // create two separate textures and renderbuffers at rendertarget
    // creation, one for each window resolution. One fullscreen, the
    // other windowed. It may solve the speed issue.

    // A Framebuffer is a collection of buffers that can be used
    // as a destination for offscreen rendering. OpenGL has two
    // kinds of framebuffers: the Default Framebuffer, which is
    // provided by the OpenGL Context; and user-created
    // framebuffers called Framebuffer Objects (FBOs). The buffers
    // for default framebuffers are part of the context and
    // usually represent a window or display device. The buffers
    // for FBOs reference images from either Textures or
    // Renderbuffers; they are never directly visible.

    // Renderbuffer attachments can be Textures or Renderbuffer
    // objects. Renderbuffer objects are used when we are not
    // going to sample/read from the renderbuffer, they are stored
    // in opengl's internal format and are hard/inefficient to
    // sample/read from.

    // Frambuffer has multiple color buffers attachments + depth + stencil attachments.

    // When the window is created and opengl is initialized,
    // OpenGL automatically creates a framebuffer. We can also
    // create our own framebuffers, this will enable us to do
    // postprocessig and other cool effects.

    Assert(ShaderFilename);
    Assert(Width > 0);
    Assert(Height > 0);

    render_target *Result = (render_target*)Malloc(sizeof(render_target));
    Result->ScreenQuadShader = CreateShaderProgram(ShaderFilename);
    Result->Width = Width;
    Result->Height = Height;

    // Create the main framebuffer
    glGenFramebuffers(1, &Result->Framebuffer);
    // GL_FRAMEBUFFER binds the framebuffer to both, read + write.
    glBindFramebuffer(GL_FRAMEBUFFER, Result->Framebuffer);

    // Create a texture as a color attachment for the framebuffer
    glGenTextures(1, &Result->TextureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, Result->TextureColorBuffer);
    // TODO: Edit here when changing internalformat to float to support HDR
    i32 LevelOfDetail = 0;
    i32 InternalFormat = GL_RGB;
    GLenum FormatOfPixelData = GL_RGB;
    GLenum PixelDataType = GL_UNSIGNED_BYTE;
    glTexImage2D(GL_TEXTURE_2D, LevelOfDetail, InternalFormat, Width, Height, 0, FormatOfPixelData, PixelDataType, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Attach the texture buffer to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Result->TextureColorBuffer, 0);

    // Create a RenderBuffer object for depth and stencil attachments.
    glGenRenderbuffers(1, &Result->DepthStencilRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, Result->DepthStencilRenderbuffer);
    // Use a single renderbuffer object for both, stencil+depth
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);
    // Attach it
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Result->DepthStencilRenderbuffer);

    // Now that all attachments are added, we check if the framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        // Framebuffer is not complete, free everything, return NULL;
        printf("ERROR: Framebuffer is not complete! - %s:%d\n", __FILE__, __LINE__);
        glDeleteFramebuffers(1, &Result->Framebuffer);
        glDeleteTextures(1, &Result->TextureColorBuffer);
        glDeleteRenderbuffers(1, &Result->DepthStencilRenderbuffer);
        Free(Result);
        return NULL;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // VAO and Vertex Data for displaying the off-screen buffer to the main window
    // VAO for the final screen render quad
    f32 QuadVertices[] =
    { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &Result->ScreenQuadVAO);
    glGenBuffers(1, &Result->ScreenQuadVBO);
    glBindVertexArray(Result->ScreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, Result->ScreenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), &QuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));
    SetShaderUniform(Result->ScreenQuadShader, "ScreenTexture", 0);

    return Result;
}

void SetActiveRenderTarget(render_target *RenderTarget)
{
    // Cannot assert, RenderTarget is allowed to be NULL/0
    if(RenderTarget == NULL)
    {
        // Set to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, RenderTarget->Framebuffer);
    }
}

void DisplayRenderTarget(render_target *RenderTarget)
{
    glDisable(GL_DEPTH_TEST);
    glUseProgram(RenderTarget->ScreenQuadShader);
    glBindVertexArray(RenderTarget->ScreenQuadVAO);
    glBindTexture(GL_TEXTURE_2D, RenderTarget->TextureColorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST); // Enable depth testing (it's disabled for rendering screen space quad)
}

void ResizeRenderTarget(render_target *RenderTarget, i32 Width, i32 Height)
{
    // TODO: Maybe we can create 2 targets at RenderTarget
    // initialization and switch between them. It's probably faster!

    Assert(RenderTarget);
    Assert(Width > 0);
    Assert(Height > 0);

    // Delete old texture and depth+stencil renderbuffer
    glDeleteTextures(1, &RenderTarget->TextureColorBuffer);
    glDeleteRenderbuffers(1, &RenderTarget->DepthStencilRenderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, RenderTarget->Framebuffer);

    // Color texture attachment

    // Create a texture as a color attachment for the framebuffer
    glGenTextures(1, &RenderTarget->TextureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, RenderTarget->TextureColorBuffer);
    // TODO: Edit here when changing internalformat to float to support HDR
    i32 LevelOfDetail = 0;
    i32 InternalFormat = GL_RGB;
    GLenum FormatOfPixelData = GL_RGB;
    GLenum PixelDataType = GL_UNSIGNED_BYTE;
    glTexImage2D(GL_TEXTURE_2D, LevelOfDetail, InternalFormat, Width, Height, 0, FormatOfPixelData, PixelDataType, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Attach the texture buffer to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTarget->TextureColorBuffer, 0);


    // RenderBuffer
    // Create a RenderBuffer object for depth and stencil attachments.
    glGenRenderbuffers(1, &RenderTarget->DepthStencilRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, RenderTarget->DepthStencilRenderbuffer);
    // Use a single renderbuffer object for both, stencil+depth
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);
    // Attach it
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderTarget->DepthStencilRenderbuffer);

    // Now that all attachments are added, we check if the framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Framebuffer is not complete!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int Argc, char **Argv)
{
    Argc; Argv;

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS | SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
#if DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

   Window = SDL_CreateWindow("Untitled", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowWidth, WindowHeight, SDL_WINDOW_OPENGL);
    if(Window == NULL)
    {
        printf("SDL_CreateWindow Failed!\n");
        return -1;
    }

    SDL_GLContext GLContext = SDL_GL_CreateContext(Window);
    if(GLContext == NULL)
    {
        printf("SDL_GL_CreateContext Failed, maybe the version of opengl requested is not supported?\n");
        printf("SDL_GetError(): %s\n", SDL_GetError());
    }

	i32 MinorVersion;
    i32 MajorVersion;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &MajorVersion);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &MinorVersion);
    if(MajorVersion != 3 || MinorVersion != 3)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "This program needs opengl 3.3 support, update your video drivers if you see this message", Window);
        return -1;
    }

    // Load GL function pointers
    if(!gladLoadGL())
    {
        printf("gladLoadGL failed!\n");
        return -1;
    }

    printf("GPU VENDOR: %s\n", glGetString(GL_VENDOR));
    printf("GPU: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    if(SDL_GL_SetSwapInterval(0) != 0)
    {
        printf("Something went wrong in SDL_GL_SetSwapInterval");
        return -2;
    }

    glEnable(GL_FRAMEBUFFER_SRGB);
    glFrontFace(GL_CCW);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST); // NOTE: Disabling the depth test makes the red border cube look fine
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, WindowWidth, WindowHeight);

    if(SDL_SetRelativeMouseMode(SDL_TRUE) != 0)
    {
        printf("Error setting relative mouse mode!\n");
    }

    InitMouse();
    InitKeyboard();
    InitCamera();
    InitClock();

    u32 TexturedCubeShader   = CreateShaderProgram("shaders/TexturedCube.glsl");
    u32 TextShader           = CreateShaderProgram("shaders/Text.glsl");
    u32 TexturedQuadShader   = CreateShaderProgram("shaders/TexturedQuad.glsl");
    u32 SkyboxShader         = CreateShaderProgram("shaders/Skybox.glsl");
    u32 ScreenQuadShader     = CreateShaderProgram("shaders/ScreenQuad.glsl");

    font *Arial              = CreateFont("fonts/arial.ttf", TextShader, 0, 100);
    font *DebugInfoFont      = CreateFont("fonts/arial.ttf", TextShader, 0, 22);

    textured_cube *MyCube    = CreateTexturedCube(TexturedCubeShader, "textures/Border.png");
    textured_quad *RedWindow = CreateTexturedQuad("textures/blending_transparent_window.png", TexturedQuadShader);
    textured_quad *Joker     = CreateTexturedQuad("textures/joker.png", TexturedQuadShader);
    skybox *MainSkybox       = CreateSkybox(SkyboxShader,
                                      "textures/skybox/right.jpg",
                                      "textures/skybox/left.jpg",
                                      "textures/skybox/top.jpg",
                                      "textures/skybox/bottom.jpg",
                                      "textures/skybox/front.jpg",
                                      "textures/skybox/back.jpg");

    render_target *MainRenderTarget = CreateRenderTarget("shaders/ScreenQuad.glsl", WindowWidth, WindowHeight);

    glm::vec3 PlayerPosition = glm::vec3(0,0, 1.0f);


    f32 Angle = 0.0f;
    SDL_Event Event;
    while(IsRunning)
    {
        // TODO: REMOVE
        Angle += 0.2f * (f32)Clock.DeltaTime * 999;

        UpdateClock();

        // Compute Average FPS - Average Milliseconds Per Frame
        {
            f32 FramesPerSecondToShow = 2; // How many times per second to calculate fps
            if(FPSTimerSecondsElapsed > (1.0f / FramesPerSecondToShow))
            {
                AverageFPS = FPSCounter / FPSTimerSecondsElapsed;
                AverageMillisecondsPerFrame = (FPSTimerSecondsElapsed / FPSCounter) * 1000.0f;
                FPSCounter = 0;
                FPSTimerSecondsElapsed = 0.0f;
                char Title[60] = {};
                sprintf_s(Title, sizeof(Title),"Average FPS: %2.2f - Average Ms per frame: %2.2f", AverageFPS, AverageMillisecondsPerFrame);
                SDL_SetWindowTitle(Window, Title);
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
            UpdateKeyboard();
            UpdateMouse();

            if(IsPressed(SDL_SCANCODE_LSHIFT))
            {
                // Camera Stuff
                if(Mouse.FirstMouse)
                {
                    Camera.Yaw = -90.0f; // Set the Yaw to -90 so the mouse faces to 0, 0, 0 in the first frame X
                    Camera.Pitch = 0.0f;
                    Mouse.FirstMouse = false;
                }
                Camera.Yaw += Mouse.RelX * Mouse.Sensitivity;
                Camera.Pitch += -Mouse.RelY *Mouse.Sensitivity; // reversed since y-coordinates range from bottom to top
                if(Camera.Pitch > 89.0f)
                {
                    Camera.Pitch =  89.0f;
                }
                else if(Camera.Pitch < -89.0f)
                {
                    Camera.Pitch = -89.0f;
                }
                glm::vec3 Front;
                Front.x = cos(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
                Front.y = sin(glm::radians(Camera.Pitch));
                Front.z = sin(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
                Camera.Front = glm::normalize(Front);
            }

            // Handle Window input stuff
            if(IsPressed(SDL_SCANCODE_ESCAPE))
            {
                IsRunning = false;
            }
            if(IsReleased(SDL_SCANCODE_RETURN) && IsPressed(SDL_SCANCODE_LALT))
            {
                ToggleFullscreen(Window);
                ResizeRenderTarget(MainRenderTarget, WindowWidth, WindowHeight);
            }

            // Handle Camera Input
            if(IsPressed(SDL_SCANCODE_W) && IsPressed(SDL_SCANCODE_LSHIFT))
            {
                Camera.Position += Camera.Front * Camera.Speed * (f32)Clock.DeltaTime;
            }
            if(IsPressed(SDL_SCANCODE_S) && IsPressed(SDL_SCANCODE_LSHIFT))
            {
                Camera.Position -= Camera.Speed * Camera.Front * (f32)Clock.DeltaTime;
            }
            if(IsPressed(SDL_SCANCODE_A) && IsPressed(SDL_SCANCODE_LSHIFT))
            {
                Camera.Position -= glm::normalize(glm::cross(Camera.Front, Camera.Up)) * Camera.Speed * (f32)Clock.DeltaTime;
            }
            if(IsPressed(SDL_SCANCODE_D) && IsPressed(SDL_SCANCODE_LSHIFT))
            {
                Camera.Position += glm::normalize(glm::cross(Camera.Front, Camera.Up)) * Camera.Speed * (f32)Clock.DeltaTime;
            }
            if(IsPressed(SDL_SCANCODE_SPACE) && IsPressed(SDL_SCANCODE_LSHIFT))
            {
                // Camera.Position.y += Camera.Speed * (f32)Clock.DeltaTime;
                InitCamera();
                InitMouse();
            }

            // Handle Player Input
            f32 PlayerSpeed = 0.01f;
            if(IsPressed(SDL_SCANCODE_W) && IsNotPressed(SDL_SCANCODE_LSHIFT))
            {
                PlayerPosition.y += PlayerSpeed;
            }
            if(IsPressed(SDL_SCANCODE_S) && IsNotPressed(SDL_SCANCODE_LSHIFT))
            {
                PlayerPosition.y -= PlayerSpeed;
            }
            if(IsPressed(SDL_SCANCODE_A) && IsNotPressed(SDL_SCANCODE_LSHIFT))
            {
                PlayerPosition.x -= PlayerSpeed;
            }
            if(IsPressed(SDL_SCANCODE_D) && IsNotPressed(SDL_SCANCODE_LSHIFT))
            {
                PlayerPosition.x += PlayerSpeed;
            }
            if(IsPressed(SDL_SCANCODE_SPACE) && IsNotPressed(SDL_SCANCODE_LSHIFT))
            {
                // Jump?!?!?
            }

            // Play with FoV
            if(IsPressed(SDL_SCANCODE_UP))
            {
                Camera.FoV += 0.1f;
            }
            if(IsPressed(SDL_SCANCODE_DOWN))
            {
                Camera.FoV -= 0.1f;
            }

        }

        // Update Camera Matrices
        Camera.View = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
        Camera.Projection = glm::perspective(glm::radians(Camera.FoV), (f32)WindowWidth / (f32)WindowHeight, Camera.Near, Camera.Far);

        if(glIsEnabled(GL_CULL_FACE))
        {
            printf("Culling enabled!\n");
        }


        // Render //
        // TODO(Jorge): HDR PIPELINE REQUIRES HDR DATA!, GET SOME HDR DATA NOW!
        SetActiveRenderTarget(MainRenderTarget);

        glClearColor(0.2f, 0.2f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        DrawSkybox(MainSkybox);
        DrawTexturedQuad(Joker, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);

        glDisable(GL_DEPTH_TEST); // NOTE: Disabling the depth test makes the red border cube look fine
        DrawTexturedCube(MyCube, PlayerPosition, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), Angle);
        glEnable(GL_DEPTH_TEST); // NOTE: Disabling the depth test makes the red border cube look fine

        // DrawTexturedQuad(RedWindow, PlayerPosition, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f), 0.0f);


        // DrawText3DCentered("This is some CENTERED 3D Text", Arial,
        //                    glm::vec3(0.0f, 0.0f, 0.0f),
        //                    glm::vec3(0.3f, 0.3f, 0.3f),
        //                    glm::vec3(1.0f, 1.0f, 1.0f), Angle,
        //                    glm::vec3(1.0f, 0.1f, 1.0f));

        // DrawText2DCentered("This is some 2D Text", DebugInfoFont,
        //                    glm::vec2(WindowWidth / 2.0f, WindowHeight / 2.0f), glm::vec2(2.0f, 2.0f), glm::vec3(1,1,1));

        // Shader Hotloading
        // DrawShape;
        // Game hotloading?
        // CheckForCollisionsAgainstFloor;
        // AABBvsAABB;
        // DrawDebugLine();
        // DrawLine();
        // Test Tweening Functions!

        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        SetActiveRenderTarget(0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        DisplayRenderTarget(MainRenderTarget);

        // printf("AllocationCount: %d\n", AllocationCount);

        SDL_GL_SwapWindow(Window);
    }

    SDL_GL_DeleteContext(GLContext);

    return 0;
}
