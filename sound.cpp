#pragma once

#include "sound.h"

void S_SetMusicVolume(sound_system *System, i32 Input)
{
    Assert(System);
    if(Input > MIX_MAX_VOLUME)
    {
        System->MusicVolume = MIX_MAX_VOLUME;
    }
    else if(Input < 0)
    {
        System->MusicVolume = 0;
    }
    else
    {
        System->MusicVolume = Input;
    }

    Mix_VolumeMusic(System->MusicVolume);
}

void S_SetEffectsVolume(sound_system* System, i32 Input)
{
    Assert(System);

    if(Input > MIX_MAX_VOLUME)
    {
        System->EffectsVolume = MIX_MAX_VOLUME;
    }
    else if(Input < 0)
    {
        System->EffectsVolume = 0;
    }
    else
    {
        System->EffectsVolume = Input;
    }

    Mix_Volume(-1, System->EffectsVolume);
}

sound_system *S_CreateSoundSystem()
{
    sound_system *Result = (sound_system*)Malloc(sizeof(sound_system)); Assert(Result);

    // if(Mix_Init(MIX_INIT_MP3) != MIX_INIT_MP3)
    // {
    //     printf("Error Initializing Mixer: %s\n", Mix_GetError());
    //     Free(Result);
    //     return NULL;
    // }

    //Initialize SDL_mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        Free(Result);
        return NULL;
    }

    // NOTE: Set volume to 64, half of the max volume
    i32 InitialVolume = 2;
    S_SetMusicVolume(Result, InitialVolume);
    S_SetEffectsVolume(Result, InitialVolume);

    return Result;
}

sound_effect *S_CreateSoundEffect(char *Filename)
{
    Assert(Filename);

    sound_effect *Result = Mix_LoadWAV(Filename);
    if(Result == NULL)
    {
        printf("Sound Effect %s failed to load\n", Filename);
        Free(Result);
        Result = NULL;
    }

    return (Result);
}

sound_music *S_CreateMusic(char *Filename)
{
    Assert(Filename);

    sound_music *Result = Mix_LoadMUS(Filename);
    if(Result == NULL)
    {
        printf("S_CreateMusic failed to load %s\n", Filename);
        Free(Result);
        Result = NULL;
    }

    return Result;
}

void S_PlaySoundEffect(sound_effect *Fx)
{
    /*
      Arg 1, Channel to play on, or -1 for the first free unreserved channel.
      Arg 2, Sample to play.
      Arg 3, Number of loops, -1 is infinite loops. Passing one here plays the sample twice (1 loop).
    */
    Mix_PlayChannel( -1, Fx, 0 );
}

void S_PlayMusic(sound_music *Song)
{
    /*

      Arg 1, Pointer to Mix_Music to play.
      Arg 2, loops number of times to play through the music. 0 plays the music zero times... -1 plays the music forever (or as close as it can get to that)
     */
    if(Mix_PlayMusic(Song, -1) == -1)
    {
        printf("Mix_PlayMusic: %s\n", Mix_GetError());
    }
}
