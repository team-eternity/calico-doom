/*
  CALICO
  
  SDL2 Audio Implementation
  
  The MIT License (MIT)
  
  Copyright (c) 2017 James Haley
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef SDL_SOUND_H__
#define SDL_SOUND_H__

#ifdef USE_SDL2

#include "../hal/hal_types.h"

#define SAMPLERATE 44100

#ifdef __cplusplus
extern "C" {
#endif

int      SDL2Sfx_StartSound(float *data, size_t numsamples, int volume, hal_bool loop);
void     SDL2Sfx_StopSound(int handle);
hal_bool SDL2Sfx_IsSamplePlaying(int handle);
hal_bool SDL2Sfx_IsSampleAtStart(int handle);
void     SDL2Sfx_StopAllChannels(void);
void     SDL2Sfx_UpdateEQParams(void);
hal_bool SDL2Sfx_IsInit(void);
hal_bool SDL2Sfx_MixerInit(void);
int      SDL2Sfx_GetSampleRate(void);

void SDL2Sfx_SetMasterVolume(int sfxvolume, int musvolume);
void SDL2Sfx_StartMusic(unsigned char* musicPtr, unsigned char* music_startPtr, unsigned char* music_endPtr);
void SDL2Sfx_addMusicSample(int instnum, float* data, size_t len, size_t loopoffset, hal_bool loop);
void SDL2Sfx_StopMusic(void);

#ifdef __cplusplus
}
#endif

#endif

#endif

// EOF

