#pragma once

#include "shared.h"

typedef Mix_Music sound_music;
typedef Mix_Music sound_test;
typedef Mix_Chunk sound_effect;

struct sound_system
{
    // NOTE: Max Volume in SDL_Mixer is 128/MIX_MAX_VOLUME
    i32 MusicVolume;
    i32 EffectsVolume;
};
