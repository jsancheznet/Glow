#pragma once

struct mouse
{
    u32 ButtonState;
    u32 PrevButtonState;

    // Use by the camera system, relative to last mouse position
    i32 RelX;
    i32 RelY;

    // Mouse position inside the window
    i32 X;
    i32 Y;

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
