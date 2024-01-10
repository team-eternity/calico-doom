/*
  CALICO
  
  Sound effects loading
  
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

#ifndef S_SOUNDFMT_H__
#define S_SOUNDFMT_H__

#ifdef __cplusplus

typedef class SfxSample       *PSFXSAMPLE;
typedef class SfxSample const *PCSFXSAMPLE;

#else

typedef struct SfxSample       *PSFXSAMPLE;
typedef struct SfxSample const *PCSFXSAMPLE;

#endif

#ifdef __cplusplus
extern "C" {
#endif

PSFXSAMPLE SfxSample_LoadFromData(const char *tag, void *data, size_t len);
PSFXSAMPLE SfxSample_FindByTag(const char *tag);
size_t     SfxSample_GetNumSamples(PCSFXSAMPLE sfx);
float     *SfxSample_GetSamples(PCSFXSAMPLE sfx);
size_t	   SfxSample_GetLoopOffset(PCSFXSAMPLE sfx);
hal_bool   SfxSample_GetLoop(PCSFXSAMPLE sfx);

#ifdef __cplusplus
}
#endif

#endif

// EOF

