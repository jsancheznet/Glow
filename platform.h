#pragma once

struct window
{
    SDL_Window *Handle;
    SDL_GLContext Context;

    i32 Width;
    i32 Height;
};

struct clock
{
    u64 PerfCounterNow;
    u64 PerfCounterLast;
    f64 DeltaTime;
    f64 SecondsElapsed;
};
