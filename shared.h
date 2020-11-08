/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <jsanchez@monoinfinito.net> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Jorge Sï¿½nchez
 * ----------------------------------------------------------------------------
 */

#pragma once

#pragma warning(disable: 4189)
#pragma warning(disable: 4201)
#pragma warning(disable: 4100)

#pragma warning(disable:4127) // GLM fails to compile if this warning is turned on!
#pragma warning(disable: 4201) // GLM warning
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp> // glm::rotate vector

#include <stdint.h>
typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t    i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;
typedef i32      b32;

#define global static
#define internal_variable static

#define INLINE __forceinline
#include <assert.h>
#define Assert(Expr) assert(Expr)

#define Kilobytes(Expr) ((Expr) * 1024)
#define Megabytes(Expr) (Kilobytes(Expr) * 1024)
#define Gigabytes(Expr) (Megabytes(Expr) * 1024)

#define PI32 3.14159265358979323846

global u32 AllocationCount = 0;
global size_t AllocationSize = 0;
void *Malloc(size_t Size)
{
    AllocationCount += 1;
    return calloc(1, Size);
}

void Free(void *Ptr)
{
    AllocationCount -= 1;
    free(Ptr);
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

f32 Normalize(f32 Input, f32 Minimum, f32 Maximum)
{
    return (Input - Minimum) / (Maximum - Minimum);
}

f32 MapRange(f32 Input, f32 InputStart, f32 InputEnd, f32 OutputStart, f32 OutputEnd)
{
    return (Input - InputStart) / (InputEnd - InputStart) * (OutputEnd - OutputStart) + OutputStart;
}

f32 Abs(f32 Input)
{
    return fabs(Input);
}

f32 GetRotationAngle(f32 x, f32 y)
{
    return (( (f32)atan2(y, x) * (f32)180.0f) / 3.14159265359f);
}

f32 Distance(glm::vec3 A, glm::vec3 B)
{
    return glm::distance(A, B);
}

glm::vec3 Normalize(glm::vec3 A)
{
    return glm::normalize(A);
}

f32 Atan2(f32 y, f32 x)
{
    return (f32)atan2(y, x);
}

f32 EaseOutBounce(float Input)
{
    const f32 n1 = 7.5625f;
    const f32 d1 = 2.75f;

    if (Input < 1 / d1)
    {
        return n1 * Input * Input;
    }
    else if (Input < 2.0f / d1)
    {
        return n1 * (Input -= 1.5f / d1) * Input + 0.75f;
    }
    else if (Input < 2.5f / d1)
    {
        return n1 * (Input -= 2.25f / d1) * Input + 0.9375f;
    }
    else
    {
        return n1 * (Input -= 2.625f / d1) * Input + 0.984375f;
    }
}

// This small code makes the program run on dedicated gpu's if there is one.
#ifdef _WIN32
extern "C"
{
    // http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
    __declspec( dllexport ) unsigned long int NvOptimusEnablement = 0x00000001;
    // https://gpuopen.com/amdpowerxpressrequesthighperformance/
    __declspec( dllexport ) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

//
// Error Checking Macro's
//

#define GLERR                                                                                               \
    do {                                                                                                    \
        GLuint glerr;                                                                                       \
        while((glerr = glGetError()) != GL_NO_ERROR)                                                        \
            fprintf(stderr, "%s:%d glGetError() = 0x%04x\n", __FILE__, __LINE__, glerr);                    \
    } while(0)
