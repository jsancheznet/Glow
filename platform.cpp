#pragma once

#include "shared.h"
#include "platform.h"

clock *P_CreateClock()
{
    clock *Result = (clock*)Malloc(sizeof(clock)); Assert(Result);
    Result->DeltaTime = 0.0f;
    Result->PerfCounterNow = SDL_GetPerformanceCounter();
    Result->PerfCounterLast = 0;

    return Result;
}

void P_UpdateClock(clock *Clock)
{
    // NOTE(Jorge): This functions makes use of the global variable Clock
    Clock->PerfCounterLast = Clock->PerfCounterNow;
    Clock->PerfCounterNow = SDL_GetPerformanceCounter();
    Clock->DeltaTime = (f64)((Clock->PerfCounterNow - Clock->PerfCounterLast)*1000.0f / (f64)SDL_GetPerformanceFrequency() );
    Clock->DeltaTime /= 1000.0f;
    Clock->SecondsElapsed += Clock->DeltaTime;
}

void P_ToggleFullscreen(window *Window)
{
    u32 WindowFlags = SDL_GetWindowFlags(Window->Handle);

    if(WindowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP ||
       WindowFlags & SDL_WINDOW_FULLSCREEN)
    {
        SDL_SetWindowFullscreen(Window->Handle, 0);
        SDL_GL_GetDrawableSize(Window->Handle, &Window->Width, &Window->Height);
    }
    else
    {
        SDL_SetWindowFullscreen(Window->Handle, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_GL_GetDrawableSize(Window->Handle, &Window->Width, &Window->Height);
    }
}

window *P_CreateOpenGLWindow(char *Title, u32 Width, u32 Height)
{
    window *Result = NULL;
    Result = (window*)Malloc(sizeof(window));
    Result->Width = Width;
    Result->Height = Height;

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

    Result->Handle = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_OPENGL);
    if(Result->Handle == NULL)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "SDL_CreateWindow failed",
                                 Result->Handle);
        exit(-1);
    }

    Result->Context = SDL_GL_CreateContext(Result->Handle);
    if(Result->Context == NULL)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "SDL_GL_CreateContext Failed!",
                                 Result->Handle);
        exit(-2);
    }

	i32 MinorVersion;
    i32 MajorVersion;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &MajorVersion);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &MinorVersion);
    if(MajorVersion != 3 || MinorVersion != 3)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "This program needs opengl 3.3 support, update your video drivers if you see this message", Result->Handle);
        exit(-3);
    }

    // Load GL function pointers
    if(!gladLoadGL())
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "gloadLoadGL Failed", Result->Handle);
        exit(-4);
    }

    return (Result);
}
