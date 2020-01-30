#pragma once

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
