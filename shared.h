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

#include <stdint.h>
typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
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

#define Malloc(Expr) Malloc__((Expr))
#define Free(Expr) Free__((Expr))
global u32 AllocationCount = 0;
global size_t AllocationSize = 0;
void *Malloc__(size_t Size)
{
    AllocationCount += 1;
    return calloc(1, Size);
}

void Free__(void *Ptr)
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

#define ZeroVec3 glm::vec3(0.0f)

//
// Error Checking Macro's
//

#define NULL_CHECK(Expr)                                                                                    \
    if(!Expr)                                                                                               \
    {                                                                                                       \
        DWORD Err = GetLastError();                                                                         \
        printf("NULL_CHECK TRIGGERED, MSDN ErrorCode: %ld, File: %s Line: %d \n", Err, __FILE__, __LINE__); \
        return NULL;                                                                                        \
    }

#define GLERR                                                                                               \
    do {                                                                                                    \
        GLuint glerr;                                                                                       \
        while((glerr = glGetError()) != GL_NO_ERROR)                                                        \
            fprintf(stderr, "%s:%d glGetError() = 0x%04x\n", __FILE__, __LINE__, glerr);                    \
    } while(0)
