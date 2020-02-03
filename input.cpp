#pragma once

#include "shared.h"
#include "input.h"

global keyboard *__Keyboard = NULL; // Variable to be used as shortcut inside this file
global mouse *__Mouse = NULL;

void I_Init(keyboard *Keyboard, mouse *Mouse)
{
    Keyboard->State = SDL_GetKeyboardState(&Keyboard->Numkeys);
    Keyboard->CurrentState = (u8*)Malloc(sizeof(u8) * Keyboard->Numkeys);
    Keyboard->PrevState = (u8*)Malloc(sizeof(u8) * Keyboard->Numkeys);
    Assert(Keyboard->PrevState);
    Assert(Keyboard->CurrentState);
    *Keyboard->CurrentState = {};
    *Keyboard->PrevState = {};

    __Keyboard = Keyboard;

    Mouse->ButtonState = 0;
    Mouse->Sensitivity = 0.3f;
    Mouse->FirstMouse = 1;
    Mouse->RelX = 0;
    Mouse->RelY = 0;
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void I_ResetMouse(mouse *Mouse)
{
    Mouse->ButtonState = 0;
    Mouse->Sensitivity = 0.3f;
    Mouse->FirstMouse = 1;
    Mouse->RelX = 0;
    Mouse->RelY = 0;
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void I_UpdateMouse(mouse *Mouse)
{
    Mouse->ButtonState = SDL_GetRelativeMouseState(&Mouse->RelX, &Mouse->RelY);
}

void I_UpdateKeyboard(keyboard *Keyboard)
{
    memcpy((void*)Keyboard->PrevState, (void*)Keyboard->CurrentState, sizeof(u8) * Keyboard->Numkeys);
    Keyboard->State = SDL_GetKeyboardState(NULL);
    memcpy((void*)Keyboard->CurrentState, (void*)Keyboard->State, sizeof(u8) * Keyboard->Numkeys);
    Keyboard->ModState = SDL_GetModState();
}

b32 I_IsPressed(SDL_Scancode Scancode)
{
    return __Keyboard->State[Scancode];
}

b32 I_IsNotPressed(SDL_Scancode Scancode)
{
    return !__Keyboard->State[Scancode];
}

b32 I_IsReleased(SDL_Scancode Scancode)
{
    return (!__Keyboard->State[Scancode] && __Keyboard->PrevState[Scancode]);
}
