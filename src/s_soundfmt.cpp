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

#include "elib/elib.h"
#include "elib/binary.h"
#include "elib/compare.h"
#include "gl/resource.h"
#include "hal/hal_sfx.h"

#include "s_soundfmt.h"

//=============================================================================
//
// SfxSample class
//

class SfxSample : public Resource
{
protected:
   size_t m_numsamples;                 // length of sample array
   std::unique_ptr<float []> m_samples; // sample data

public:
   SfxSample(const char *tag, size_t numsamples, float *data)
      : Resource(tag), m_numsamples(numsamples), m_samples(data)
   {
   }

   size_t  getNumSamples() const { return m_numsamples;    }
   float  *getSamples()    const { return m_samples.get(); }
};

//=============================================================================
//
// Sample Manager
//

static ResourceHive gSoundManager;

//=============================================================================
//
// Sample Loading
//

// sample formats
enum sampleformat_e
{
   S_FMT_U8
};

struct sounddata_t
{
   unsigned int    samplerate;
   size_t          samplecount;
   size_t          alen;
   byte           *samplestart;
   float          *data;
   sampleformat_e  fmt;
};

// original size of binary sfx_t structure (see soundst.h)
#define SFX_T_ORIG_SIZEOF 28

//
// Jaguar DOOM sound effects and instrument patches are 8-bit unsigned PCM with a custom header.
// Verify JagDoom-format sample when loading:
// * Must be larger than the 28-byte header size
// * Number of samples must match the lump size minus the header length
// * Number of samples must be at least four (the original sound driver wrote out four at a time)
// * Loop properties either need to be valid, or be a recognized percussion instrument pattern
//
static bool S_isJaguarSample(byte *data, size_t len, sounddata_t &sd)
{
   // minimum size check
   if(len <= SFX_T_ORIG_SIZEOF)
      return false;

   byte *rover = data;

   uint32_t samples   = GetBinaryUDWordBE(&rover);
   uint32_t loopstart = GetBinaryUDWordBE(&rover);
   uint32_t loopend   = GetBinaryUDWordBE(&rover);

   // check sample count
   if(samples != len - SFX_T_ORIG_SIZEOF || samples < 4)
      return false;

   // check loop properties
   if((loopstart < samples && loopend <= samples && loopstart <= loopend) || // normal sample
      (loopstart == 0xffffffff && (loopend == samples + 1 || loopend == 0))) // percussion patch
   {
      sd.samplerate  = 11025u;
      sd.fmt         = S_FMT_U8;
      sd.samplecount = static_cast<size_t>(samples);
      sd.samplestart = data + SFX_T_ORIG_SIZEOF;
      return true;
   }

   return false;
}

//
// PCM Conversion
//

//
// Calculate the "actual" sample length for a digital sound effect after
// conversion from its native samplerate to the samplerate used for output.
//
static size_t S_alenForSample(const sounddata_t &sd, int targetsamplerate)
{
   return (size_t)(((uint64_t)sd.samplecount * targetsamplerate) / sd.samplerate);
}

//
// Convert unsigned 8-bit PCM to floating point.
//
static void S_convertPCMU8(sounddata_t &sd, int targetsamplerate)
{
   sd.alen = S_alenForSample(sd, targetsamplerate);
   sd.data = new float [sd.alen];

   if(sd.alen != sd.samplecount)
   {
      size_t i;
      float *dest = sd.data;
      byte  *src  = sd.samplestart;

      unsigned int step = (sd.samplerate << 16) / targetsamplerate;
      unsigned int stepremainder = 0, j = 0;

      // do linear filtering operation
      for(i = 0; i < sd.alen && j < sd.samplecount - 1; i++)
      {
         double d = (((unsigned int)src[j  ] * (0x10000 - stepremainder)) +
            ((unsigned int)src[j+1] * stepremainder));
         d /= 65536.0;
         dest[i] = static_cast<float>(eclamp(d * 2.0 / 255.0 - 1.0, -1.0, 1.0));

         stepremainder += step;
         j += (stepremainder >> 16);

         stepremainder &= 0xffff;
      }
      // fill remainder (if any) with final sample byte
      for(; i < sd.alen; i++)
         dest[i] = static_cast<float>(eclamp(src[j] * 2.0 / 255.0 - 1.0, -1.0, 1.0));
   }
   else
   {
      // sound is already at target sample rate, just convert to float
      float *dest = sd.data;
      byte  *src  = sd.samplestart;

      for(size_t i = 0; i < sd.alen; i++)
         dest[i] = static_cast<float>(eclamp(src[i] * 2.0 / 255.0 - 1.0, -1.0, 1.0));
   }
}

//
// Internal conversion routine
//
static bool S_LoadFromDataInt(sounddata_t &sd, byte *data, size_t len)
{
   // check format
   if(!S_isJaguarSample(data, len, sd))
      return false;

   // load based on returned sample format
   switch(sd.fmt)
   {
   case S_FMT_U8:
      S_convertPCMU8(sd, hal_sound.getSampleRate());
      break;
   default: // unsupported PCM format
      return false;
   }

   return true;
}

//=============================================================================
//
// External Interface
//

PSFXSAMPLE SfxSample_LoadFromData(const char *tag, void *data, size_t len)
{
   SfxSample *ret = nullptr;

   if(!(ret = gSoundManager.findResourceType<SfxSample>(tag)))
   {
      edefstructvar(sounddata_t, sd);

      if(S_LoadFromDataInt(sd, static_cast<byte *>(data), len))
      {
         ret = new SfxSample(tag, sd.alen, sd.data);
         gSoundManager.addResource(ret);
      }
   }

   return ret;
}

PSFXSAMPLE SfxSample_FindByTag(const char *tag)
{
   return gSoundManager.findResourceType<SfxSample>(tag);
}

size_t SfxSample_GetNumSamples(PCSFXSAMPLE sfx)
{
   return sfx->getNumSamples();
}

float *SfxSample_GetSamples(PCSFXSAMPLE sfx)
{
   return sfx->getSamples();
}

// EOF

