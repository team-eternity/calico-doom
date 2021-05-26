/*
  CALICO
  
  SDL2 Audio Implementation
  
  The MIT License (MIT)
  
  Copyright (c) 2021 James Haley, Max Waine
  
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

#ifdef USE_SDL2

#include <type_traits>

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_thread.h"

#include "sdl_sound.h"

#include "../hal/hal_platform.h"
#include "../elib/elib.h"
#include "../elib/atexit.h"
#include "../elib/compare.h"
#include "../elib/configfile.h"

// CALICO-TODO: allow more sound channels as an option?
#define MAXCHANNELS 4

// channel structure
struct channelinfo_t
{
   unsigned int  step;               // channel step amount
   unsigned int  stepremainder;      // remainder of last step
   float        *data;               // current location
   float        *startdata;          // starting location
   float        *enddata;            // end of sample
   float         leftvol, rightvol;  // stereo volume levels
   bool          loop;               // looping?
   SDL_sem      *sem;                // semaphore
   bool          shouldstop;         // flagged to stop by game engine
};

// sound channels
static channelinfo_t channels[MAXCHANNELS];

// track whether or not sound was successfully initialized
static bool sndInit;

// sound mixing buffer
static float *mixbuffer;
static Uint32 mixbufferSize;

// MaxW: 2019/08/24: float audio if true else Sint16
bool float_samples = false;

// MaxW: 2019/08/24: Audiospec we actually got
SDL_AudioSpec audio_spec = {};

//=============================================================================
//
// Channel Management
//

//
// Add the effect to the already determined free channel.
//
static bool SDL2Sfx_addSfx(float *data, size_t numsamples, int volume, bool loop, int channel)
{
   // critical section
   if(SDL_SemWait(channels[channel].sem) == 0)
   {
      channels[channel].data      = data;
      channels[channel].enddata   = data + numsamples - 1;
      channels[channel].startdata = channels[channel].data;

      channels[channel].stepremainder = 0;
      channels[channel].loop = loop;
      channels[channel].shouldstop = false;

      // CALICO-TODO: allow stereo separation as option?
      channels[channel].leftvol  = (float)(eclamp((double)volume / 191.0, 0.0, 1.0));
      channels[channel].rightvol = channels[channel].leftvol;

      channels[channel].step = 1 << 16;

      SDL_SemPost(channels[channel].sem);
      return true;
   }
   else
      return false;
}

//
// Start a sound effect. Returns a channel handle if successful, or -1 if failed.
//
int SDL2Sfx_StartSound(float *data, size_t numsamples, int volume, hal_bool loop)
{
   int handle;

   if(!sndInit)
      return -1;

   for(handle = 0; handle < MAXCHANNELS; handle++)
   {
      if(channels[handle].data == nullptr || channels[handle].shouldstop)
         break;
   }

   if(handle == MAXCHANNELS)
      return -1;

   return SDL2Sfx_addSfx(data, numsamples, volume, loop == HAL_TRUE, handle) ? handle : -1;
}

//
// Stop a sound effect by channel handle.
//
void SDL2Sfx_StopSound(int handle)
{
   if(!sndInit || handle < 0 || handle >= MAXCHANNELS)
      return;

   channels[handle].shouldstop = true;
}

//
// Check if a channel is still playing.
//
hal_bool SDL2Sfx_IsSamplePlaying(int handle)
{
   if(!sndInit || handle < 0 || handle >= MAXCHANNELS)
      return HAL_FALSE;

   return hal_bool(!channels[handle].shouldstop && channels[handle].data);
}

//
// Test if a channel is sitting at the start of the sample data.
//
hal_bool SDL2Sfx_IsSampleAtStart(int handle)
{
   hal_bool atStart = HAL_FALSE;

   if(!sndInit || handle < 0 || handle >= MAXCHANNELS)
      return HAL_FALSE;

   if(SDL_SemWait(channels[handle].sem) == 0)
   {
      atStart = hal_bool(channels[handle].data == channels[handle].startdata);
      SDL_SemPost(channels[handle].sem);
   }

   return atStart;
}


//
// Stop all active channels.
//
void SDL2Sfx_StopAllChannels()
{
   if(!sndInit)
      return;

   for(int i = 0; i < MAXCHANNELS; i++)
      channels[i].shouldstop = true;
}

//=============================================================================
//
// Three-Band Equalization
//

static double s_lowfreq   = 880.0;
static double s_highfreq  = 5000.0;
static double s_preampmul = 0.93896;
static double s_lowgain   = 1.2;
static double s_midgain   = 1.0;
static double s_highgain  = 0.8;

static cfgrange_t<double> gainRange   = { 0.0, 3.0 };
static cfgrange_t<double> preAmpRange = { 0.0, 1.0 };

static CfgItem cfgSLowFreq  ("s_lowfreq",   &s_lowfreq);
static CfgItem cfgSHighFreq ("s_highfreq",  &s_highfreq);
static CfgItem cfgSPreampMul("s_preampmul", &s_preampmul, &preAmpRange);
static CfgItem cfgSLowGain  ("s_lowgain",   &s_lowgain,   &gainRange);
static CfgItem cfgSMidGain  ("s_midgain",   &s_midgain,   &gainRange);
static CfgItem cfgSHighGain ("s_highgain",  &s_highgain,  &gainRange);

struct EQSTATE
{
   // Filter #1 (Low band)

   double  lf;       // Frequency
   double  f1p0;     // Poles ...
   double  f1p1;    
   double  f1p2;
   double  f1p3;

   // Filter #2 (High band)

   double  hf;       // Frequency
   double  f2p0;     // Poles ...
   double  f2p1;
   double  f2p2;
   double  f2p3;

   // Sample history buffer

   double  sdm1;     // Sample data minus 1
   double  sdm2;     //                   2
   double  sdm3;     //                   3

                     // Gain Controls

   double  lg;       // low  gain
   double  mg;       // mid  gain
   double  hg;       // high gain

};  

// equalizers for each stereo channel
static EQSTATE eqstate[2];

//
// This is a rational function to approximate a tanh-like soft clipper. It is
// based on the pade-approximation of the tanh function with tweaked 
// coefficients.
// The function is in the range x=-3..3 and outputs the range y=-1..1. Beyond
// this range the output must be clamped to -1..1.
// The first two derivatives of the function vanish at -3 and 3, so the 
// transition to the hard clipped region is C2-continuous.
//
static inline double rational_tanh(double x)
{
   if(x < -3)
      return -1;
   else if(x > 3)
      return 1;
   else
      return x * (27 + x * x) / (27 + 9 * x * x);
}

//
// do_3band
//
// EQ.C - Main Source file for 3 band EQ
// http://www.musicdsp.org/showone.php?id=236
//
// (c) Neil C / Etanza Systems / 2K6
// Shouts / Loves / Moans = etanza at lycos dot co dot uk
//
// This work is hereby placed in the public domain for all purposes, including
// use in commercial applications.
// The author assumes NO RESPONSIBILITY for any problems caused by the use of
// this software.
//
template<typename T>
static void do_3band(const float *stream, const float *const end, T *dest)
{
   int esnum = 0;

   static const double vsa = (1.0 / 4294967295.0);
   double l, m, h;    // Low / Mid / High - Sample Values

   while(stream != end)
   {
      EQSTATE *const es = &eqstate[esnum];
      esnum ^= 1; // haleyjd: toggle between equalizer channels

      const double sample = *stream++ * s_preampmul;

      // Filter #1 (lowpass)
      es->f1p0  += (es->lf * (sample   - es->f1p0)) + vsa;
      es->f1p1  += (es->lf * (es->f1p0 - es->f1p1));
      es->f1p2  += (es->lf * (es->f1p1 - es->f1p2));
      es->f1p3  += (es->lf * (es->f1p2 - es->f1p3));

      l          = es->f1p3;

      // Filter #2 (highpass)
      es->f2p0  += (es->hf * (sample   - es->f2p0)) + vsa;
      es->f2p1  += (es->hf * (es->f2p0 - es->f2p1));
      es->f2p2  += (es->hf * (es->f2p1 - es->f2p2));
      es->f2p3  += (es->hf * (es->f2p2 - es->f2p3));

      h          = es->sdm3 - es->f2p3;

      // Calculate midrange (signal - (low + high))
      m          = es->sdm3 - (h + l);

      // Scale, Combine and store
      l         *= es->lg;
      m         *= es->mg;
      h         *= es->hg;

      // Shuffle history buffer
      es->sdm3   = es->sdm2;
      es->sdm2   = es->sdm1;
      es->sdm1   = sample;                

      // Return result
      // haleyjd: use rational_tanh for soft clipping
      if /*constexpr*/(std::is_same<T, Sint16>::value)
         *dest = static_cast<T>(rational_tanh(l + m + h) * 32767.0);
      else if /*constexpr*/(std::is_same<T, float>::value)
         *dest = static_cast<T>(rational_tanh(l + m + h));
      static_assert(std::is_same<T, Sint16>::value || std::is_same<T, float>::value,
                    "do_3band called with incompatible template parameter");
      dest++;
   }
}

//=============================================================================
//
// Audiospec Callback
//

// size of a single sample
static int sample_size;

// step to next stereo sample pair (prooobably 2 samples)
static int step;

//
// Convert the input buffer to floating point
//
template<typename T>
static void inline I_SDLConvertSoundBuffer(Uint8 *stream, int len)
{
   // Pointers in audio stream, left, right, end.
   T *leftout = reinterpret_cast<T *>(stream);

   // Determine end, for left channel only
   //  (right channel is implicit).
   const T *const leftend = reinterpret_cast<T *>(stream + len);

   // convert input to mixbuffer
   float *bptr0 = mixbuffer;
   while(leftout != leftend)
   {
      if /*constexpr*/(std::is_same<T, Sint16>::value)
      {
         *(bptr0 + 0) = static_cast<float>(*(leftout + 0)) * (1.0f / 32768.0f); // L
         *(bptr0 + 1) = static_cast<float>(*(leftout + 1)) * (1.0f / 32768.0f); // R
      }
      else if /*constexpr*/(std::is_same<T, float>::value)
      {
         *(bptr0 + 0) = *(leftout + 0); // L
         *(bptr0 + 1) = *(leftout + 1); // R
      }
      static_assert(std::is_same<T, Sint16>::value || std::is_same<T, float>::value,
                    "I_SDLConvertSoundBuffer called with incompatible template parameter");

      leftout += step;
      bptr0   += step;
   }
}

//
// SDL_mixer postmix callback routine, dispatched asynchronously. We do
// our own mixing on up to 32 digital sound channels.
//
template<typename T>
static void SDL2Sfx_updateSoundCB(void *userdata, Uint8 *stream, int len)
{
   // convert input samples to floating point
   I_SDLConvertSoundBuffer<T>(stream, len);

   float *leftout = mixbuffer;
   const float *const leftend = mixbuffer + (len / sample_size);

   for(channelinfo_t *chan = channels; chan != &channels[MAXCHANNELS]; chan++)
   {
      // fast rejection before semaphore lock
      if(chan->shouldstop || !chan->data)
         continue;

      // try to acquire semaphore, but do not block; if the main thread is using
      // this channel we'll skip it for now, this is safer and faster.
      if(SDL_SemTryWait(chan->sem) != 0)
         continue;

      // lost before semaphore lock?
      if(chan->shouldstop || !chan->data)
      {
         SDL_SemPost(chan->sem);
         continue;
      }

      while(leftout != leftend)
      {
         float sample = *chan->data;
         *(leftout + 0) = *(leftout + 0) + sample * chan->leftvol;
         *(leftout + 1) = *(leftout + 1) + sample * chan->rightvol;

         // increment current pointers in stream
         leftout += step;

         // increment index
         chan->stepremainder += chan->step;

         // MSB is next sample
         chan->data += chan->stepremainder >> 16;

         // limit to LSB
         chan->stepremainder &= 0xffff;

         // check if done
         if(chan->data >= chan->enddata)
         {
            if(chan->loop) // TODO: stop looping while game is paused or minimized
            {
               chan->data = chan->startdata;
               chan->stepremainder = 0;
            }
            else
            {
               // flag the channel to be stopped by the main thread ASAP
               chan->data = nullptr;
               break;
            }
         }
      }

      // release semaphore and move on to next channel
      SDL_SemPost(chan->sem);
      leftout = mixbuffer;
   }

   // equalization output pass
   do_3band(mixbuffer, leftend, reinterpret_cast<T *>(stream));
}

//=============================================================================
//
// Initialization
//

#define SND_PI 3.14159265

//
// Update equalization filter parameters
//
void SDL2Sfx_UpdateEQParams()
{
   std::memset(eqstate, 0, sizeof(eqstate));

   eqstate[0].lg = eqstate[1].lg = s_lowgain;
   eqstate[0].mg = eqstate[1].mg = s_midgain;
   eqstate[0].hg = eqstate[1].hg = s_highgain;

   eqstate[0].lf = eqstate[1].lf = 2 * std::sin(SND_PI * (s_lowfreq  / (double)SAMPLERATE));
   eqstate[0].hf = eqstate[1].hf = 2 * std::sin(SND_PI * (s_highfreq / (double)SAMPLERATE));
}

//
// Initialize the channel structures
//
static void SDL2Sfx_initChannels()
{
   // allocate mixing buffer
   mixbuffer = ecalloc(float, 2*mixbufferSize, sizeof(float));

   // create semaphores
   for(int i = 0; i < MAXCHANNELS; i++)
   {
      if(!(channels[i].sem = SDL_CreateSemaphore(1)))
         hal_platform.fatalError("Could not initialize sound channel semaphore");
   }

   SDL2Sfx_UpdateEQParams();
}

// Dummy callback for audiospec during buffer size test
static void SDL2Sfx_dummyCallback(void *, Uint8 *, int) {}

//
// Figure out required size of the sound buffers. Due to lousy API design, 
// this requires audio to be init'd and then closed.
//
static Uint32 SDL2Sfx_testSoundBufferSize()
{
   Uint32 ret = 0;
   SDL_AudioSpec want;
   audio_spec = {};

   want.freq     = SAMPLERATE;
   want.format   = MIX_DEFAULT_FORMAT;
   want.channels = 2;
   want.samples  = 2048;
   want.callback = SDL2Sfx_dummyCallback;

   if(SDL_OpenAudio(&want, &audio_spec) >= 0)
   {
      ret = audio_spec.size;
      SDL_CloseAudio();
   }

   return ret;
}

//
// At-exit handler to shutdown SDL_mixer
//
static void SDL2Sfx_Shutdown()
{
   Mix_CloseAudio();
}

//
// Check if sound is initialized
//
hal_bool SDL2Sfx_IsInit()
{
   return hal_bool(sndInit);
}

//
// Initialize SDL_mixer for sound effects and music
//
hal_bool SDL2Sfx_MixerInit()
{
   // fixure out mix buffer size
   mixbufferSize = SDL2Sfx_testSoundBufferSize();
   if(!mixbufferSize)
   {
      hal_platform.debugMsg("Failed to set mixbufferSize\n");
      return HAL_FALSE;
   }
   
   sample_size   = SDL_AUDIO_BITSIZE(audio_spec.format) / 8; // bits to bytes
   float_samples = !!SDL_AUDIO_ISFLOAT(audio_spec.format);
   step          = audio_spec.channels;
   
   if(Mix_OpenAudio(audio_spec.freq, audio_spec.format, audio_spec.channels, audio_spec.samples) != 0)
   {
      hal_platform.debugMsg("Mix_OpenAudio failed\n");
      return HAL_FALSE;
   }

   sndInit = true;
   E_AtExit(SDL2Sfx_Shutdown, 1);

   SDL2Sfx_initChannels();

   // setup postmix
   Mix_SetPostMix(float_samples ? SDL2Sfx_updateSoundCB<float> : SDL2Sfx_updateSoundCB<Sint16>, nullptr);

   return HAL_TRUE;
}

//
// Return the sample rate for digital sound mixing.
//
int SDL2Sfx_GetSampleRate()
{
   return audio_spec.freq;
}

#endif

// EOF

