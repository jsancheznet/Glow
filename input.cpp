#pragma once

#include "shared.h"
#include "input.h"

global keyboard *__Keyboard = NULL; // Variable to be used as shortcut inside this file

void InitKeyboard(keyboard *Keyboard)
{
    Keyboard->State = SDL_GetKeyboardState(&Keyboard->Numkeys);
    Keyboard->CurrentState = (u8*)Malloc(sizeof(u8) * Keyboard->Numkeys);
    Keyboard->PrevState = (u8*)Malloc(sizeof(u8) * Keyboard->Numkeys);
    Assert(Keyboard->PrevState);
    Assert(Keyboard->CurrentState);
    *Keyboard->CurrentState = {};
    *Keyboard->PrevState = {};

    __Keyboard = Keyboard;
}

void InitMouse(mouse *Mouse)
{
    Mouse->ButtonState = 0;
    Mouse->Sensitivity = 0.3f;
    Mouse->FirstMouse = 1;
    Mouse->RelX = 0;
    Mouse->RelY = 0;
}

void UpdateMouse(mouse *Mouse)
{
    Mouse->ButtonState = SDL_GetRelativeMouseState(&Mouse->RelX, &Mouse->RelY);
}

void UpdateKeyboard(keyboard *Keyboard)
{
    memcpy((void*)Keyboard->PrevState, (void*)Keyboard->CurrentState, sizeof(u8) * Keyboard->Numkeys);
    Keyboard->State = SDL_GetKeyboardState(NULL);
    memcpy((void*)Keyboard->CurrentState, (void*)Keyboard->State, sizeof(u8) * Keyboard->Numkeys);
    Keyboard->ModState = SDL_GetModState();
}

b32 IsPressed(SDL_Scancode Scancode)
{
    return __Keyboard->State[Scancode];
}

b32 IsNotPressed(SDL_Scancode Scancode)
{
    return !__Keyboard->State[Scancode];
}

b32 IsReleased(SDL_Scancode Scancode)
{
    return (!__Keyboard->State[Scancode] && __Keyboard->PrevState[Scancode]);
}
