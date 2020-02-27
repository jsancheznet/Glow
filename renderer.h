#pragma once

#include "shared.h"
#include "platform.h"

struct renderer
{
    SDL_Window *Window;

    // Settings
    f32 Exposure;

    // OpenGL
    const u8 *HardwareVendor = NULL;
    const u8 *HardwareModel = NULL;
    const u8 *OpenGLVersion = NULL;
    const u8 *GLSLVersion = NULL;

    u32 DrawableWidth;
    u32 DrawableHeight;

    u32 QuadVAO;
    u32 QuadVBO;
    u32 CubeVAO;
    u32 CubeVBO;
    u32 TextVAO;
    u32 TextVertexBuffer;
    u32 TextTexCoordsBuffer;

    struct Shaders
    {
        u32 Blur;
        u32 Bloom;
        u32 Cube; // TODO(Jorge): Are we using this? delete if anwser is no
        u32 Hdr;
        u32 Texture;
        u32 Text;
    } Shaders;

    // TODO:(Jorge): Rename and comment what each buffer/variable is for
    u32 Framebuffer;
    u32 ColorBuffer;
    u32 BrightnessBuffer;
    u32 DepthStencilRenderbuffer;
    u32 Attachments[2];
    u32 PingPongFBO[2];
    u32 PingPongBuffer[2];

    // These variables correspond to the FPS counter
    f32 FPS; // AverageFPS
    f32 AverageMsPerFrame;
    f32 SecondsElapsed = 0.0f; // Timer, seconds elapsed since last FPS calculation, by default it computes FPS twice a second.
    f32 FrameCounter = 0.0f; // Counts the number of frames rendered, resetted every half_second by default.
};

struct camera // TODO: This might need to be in something like entities.cpp
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

struct texture
{
    u32 Handle;
    i32 Width;
    i32 Height;
    i32 ChannelCount;
    GLenum InternalFormat;
    GLenum Format;
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
    i32 CharacterWidth;
    i32 CharacterHeight;
    character Characters[256];
};

//
// Model data for the renderer
//

f32 CubeVertices__[] =
{
    // layout (location = 0) in vec3 aPos;
    // layout (location = 1) in vec3 aNormal;
    // layout (location = 2) in vec2 aTexCoords;
    // back face
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
    1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
    // front face
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
    1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
    // left face
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
    // right face
    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
    1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
    1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
    // bottom face
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
    1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
    // top face
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
    1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
    1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
    1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
};

f32 QuadVertices__[] =
{
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

f32 TextTexCoords__[6][2] =
{
    {0.0, 0.0f},
    {0.0f, 1.0f},
    {1.0, 1.0},
    {0.0, 0.0},
    {1.0, 1.0},
    {1.0, 0.0},
};
