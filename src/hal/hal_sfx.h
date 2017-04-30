/*
  CALICO
  
  HAL Digital Sound Interface
  
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

#ifndef HAL_SFX_H__
#define HAL_SFX_H__

#ifndef __cplusplus
// needed in game code for the definition of size_t
#include <stdlib.h>
#endif

#include "hal_types.h"

typedef struct hal_sound_s
{
   hal_bool (*initSound)(void);
   hal_bool (*isInit)(void);
   int      (*startSound)(float *data, size_t numsamples, int volume, hal_bool loop);
   void     (*stopSound)(int handle);
   hal_bool (*isSamplePlaying)(int handle);
   hal_bool (*isSampleAtStart)(int handle);
   void     (*stopAllChannels)(void);
   void     (*updateEQParams)(void);
   int      (*getSampleRate)(void);
} hal_sound_t;

#ifdef __cplusplus
extern "C" {
#endif

extern hal_sound_t hal_sound;

#ifdef __cplusplus
}
#endif

#endif

// EOF

