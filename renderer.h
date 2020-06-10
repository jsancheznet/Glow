#pragma once

#include "shared.h"
#include "platform.h"

struct renderer
{
    window *Window;

    // Settings
    f32 Exposure;

    // OpenGL
    const u8 *HardwareVendor = NULL;
    const u8 *HardwareModel = NULL;
    const u8 *OpenGLVersion = NULL;
    const u8 *GLSLVersion = NULL;

    glm::vec4 BackgroundColor;

    u32 DrawableWidth;
    u32 DrawableHeight;

    u32 QuadVAO;
    u32 QuadVBO;
    u32 TextVAO;
    u32 TextVertexBuffer;
    u32 TextTexCoordsBuffer;
    u32 UnitQuadVAO;
    u32 UnitQuadVBO;

    struct Shaders
    {
        u32 Blur; // Does not use Uniform Buffer object for Camera
        u32 Bloom; // Does not use Uniform Buffer object for Camera
        u32 Hdr; // Does not use Uniform Buffer object for Camera
        u32 Texture;
        u32 Text;
    } Shaders;

    u32 Framebuffer;
    u32 ColorBuffer;
    u32 BrightnessBuffer;
    u32 DepthStencilRenderbuffer;
    u32 UniformCameraBuffer;
    u32 Attachments[2];
    u32 PingPongFBO[2];
    u32 PingPongBuffer[2];

    // These variables correspond to the FPS counter
    f32 FPS; // AverageFPS
    f32 AverageMsPerFrame;
    f32 SecondsElapsed = 0.0f; // Timer, seconds elapsed since last FPS calculation, by default it computes FPS twice a second.
    f32 FrameCounter = 0.0f; // Counts the number of frames rendered, resetted every half_second by default.

    u32 PreviousDrawCallsPerFrame;
    u32 CurrentDrawCallsPerFrame;
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
    i32 Width;
    i32 Height;
    character Characters[256];
};

f32 UnitQuadVertices__[] =
{
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

#if 1
f32 QuadVertices__[] =
{
    // positions        // texture Coords
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
};
#endif

f32 TextTexCoords__[6][2] =
{
    {0.0, 0.0f},
    {0.0f, 1.0f},
    {1.0, 1.0},
    {0.0, 0.0},
    {1.0, 1.0},
    {1.0, 0.0},
};
