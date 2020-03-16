#pragma once

#include "shared.h"
#include "input.h"

// TODO: Avoid the use of hacky shortcuts
global keyboard *__Keyboard = NULL; // Variable to be used as shortcut inside this file
global mouse *__Mouse = NULL;

keyboard *I_CreateKeyboard()
{
    keyboard *Result = (keyboard*)Malloc(sizeof(keyboard)); Assert(Result);
    Result->State = SDL_GetKeyboardState(&Result->Numkeys);
    Result->CurrentState = (u8*)Malloc(sizeof(u8) * Result->Numkeys); Assert(Result->CurrentState);
    Result->PrevState = (u8*)Malloc(sizeof(u8) * Result->Numkeys); Assert(Result->PrevState);
    *Result->CurrentState = {};
    *Result->PrevState = {};

    __Keyboard = Result;

    return Result;
}

mouse *I_CreateMouse()
{
    mouse *Result = (mouse*)Malloc(sizeof(mouse)); Assert(Result);

    Result->ButtonState = 0;
    Result->Sensitivity = 0.3f;
    Result->FirstMouse = 1;
    Result->RelX = 0;
    Result->RelY = 0;
    // SDL_SetRelativeMouseMode(SDL_TRUE);

    __Mouse = Result;

    return Result;
}

void I_ResetMouse(mouse *Mouse)
{
    Mouse->ButtonState = 0;
    Mouse->Sensitivity = 0.3f;
    Mouse->FirstMouse = 1;
    Mouse->RelX = 0;
    Mouse->RelY = 0;
    // SDL_SetRelativeMouseMode(SDL_TRUE);
}

void I_UpdateMouse(mouse *Mouse)
{
    Mouse->PrevButtonState = Mouse->ButtonState;
    Mouse->ButtonState = SDL_GetRelativeMouseState(&Mouse->RelX, &Mouse->RelY);
    Mouse->ButtonState = SDL_GetMouseState(&Mouse->X, &Mouse->Y);
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

b32 I_WasNotPressed(SDL_Scancode Scancode)
{
    return !__Keyboard->PrevState[Scancode];
}

b32 I_IsReleased(SDL_Scancode Scancode)
{
    return (!__Keyboard->State[Scancode] && __Keyboard->PrevState[Scancode]);
}

b32 I_IsMouseButtonPressed(i32 Input)
{
    return __Mouse->ButtonState & SDL_BUTTON(Input);
}

b32 I_IsMouseButtonNotPressed(i32 Input)
{
    return !__Mouse->ButtonState & SDL_BUTTON(Input);
}
