/* s_sound.c */

#include "elib/m_argv.h" // CALICO
#include "hal/hal_sfx.h" // CALICO
#include "s_soundfmt.h"  // CALICO
#include "doomdef.h"
#include "music.h"

#define EXTERN_BUFFER_SIZE (EXTERNALQUADS*32/2)

sfxchannel_t sfxchannels[SFXCHANNELS];

boolean channelschanged; // set by S_StartSound to signal update to remix speculative samples

int finalquad; // the last quad mixed by update.

int sfxvolume    = 128; // range 0 - 255
int musicvolume  = 128; // range 0 - 255
int oldsfxvolume = 128; // to detect transition to sound off

int soundtics;      // time spent mixing sounds
int soundstarttics; // time S_Update started

int sfxsample;      // the sample about to be output by S_WriteOutSamples

// MUSIC VARIABLES

sfx_t *instruments[256];       // pointers to all patches

channel_t music_channels[10];  // master music channel list

int            musictime;      // internal music time, follows samplecount
int            next_eventtime; // when next event will occur
unsigned char *music;          // pointer to current music data
unsigned char *music_start;    // current music start pointer
unsigned char *music_end;      // current music end pointer
unsigned char *music_memory;   // current location of cached music

int samples_per_midiclock;     // multiplier for midi clocks

int musictics = 0;

#define S_abs(x) ((x) < 0 ? -(x) : (x))

// CALICO: additional options
boolean nosfx;
boolean nomusic;

/*
==================
=
= S_Init
=
==================
*/

void S_Init(void)
{
   int i, l;
   int lump, end;
   int instnum;

   // CALICO: options to turn off sound or music
   nosfx   = M_FindArgument("-nosfx"  );
   nomusic = M_FindArgument("-nomusic");

   // CALICO: initialize low-level sound API
   if(!nosfx || !nomusic)
      hal_sound.initSound();

   // SFX
   if(!nosfx)
   {
      for(i = 1; i < NUMSFX ; i++)
      {
         l = W_CheckNumForName(S_sfx[i].name);
         if(l != -1)
         {
            S_sfx[i].md_data = W_POINTLUMPNUM(l);
            // CALICO: convert to output format
            S_sfx[i].sample  = SfxSample_LoadFromData(S_sfx[i].name, W_POINTLUMPNUM(l), W_LumpLength(l));
         }
      }
   }

   // MUSIC
   if(!nomusic)
   {
      D_memset(instruments, 0, sizeof(instruments));
      lump = W_GetNumForName("inststrt"); // get available instruments[]
      end  = W_GetNumForName("instend");
      while(lump != end)
      {
         instnum = 
            (lumpinfo[lump].name[1]-'0')*100
             + (lumpinfo[lump].name[2]-'0')*10
             + (lumpinfo[lump].name[3]-'0')
             + (lumpinfo[lump].name[0] == 'P' ? 128 : 0);
         instruments[instnum] = (sfx_t *)(W_POINTLUMPNUM(lump)); // CALICO: endianness
         // CALICO-TODO: also load as SfxSample
         lump++;
      }
 
      // hack test

      music_memory = 0;
      music = 0;
      D_memset(music_channels, 0, sizeof(music_channels));
      musictime = 0;
      next_eventtime = 0;
   }
}


/*
==================
=
= S_Clear
=
==================
*/

void S_Clear(void)
{
   int i;

   D_memset(sfxchannels, 0, sizeof(sfxchannels));

   // CALICO
   for(i = 0; i < SFXCHANNELS; i++)
      sfxchannels[i].handle = -1;

   hal_sound.stopAllChannels();
}

void S_RestartSounds(void)
{
}

/*
==================
=
= S_StartSound
=
==================
*/

void S_StartSound(mobj_t *origin, int sound_id)
{
   sfxchannel_t *channel, *newchannel;
   int        i;
   int        dist_approx;
   player_t  *player;
   int        dx, dy;
   short      vol;
   sfxinfo_t *sfx;
   float     *sampledata; // CALICO
   size_t     samplelen;  // CALICO

   if(nosfx) // CALICO
      return;

   //
   // spatialize
   //
   player = &players[consoleplayer];

   if(!origin || origin == player->mo)
      vol = 127;
   else
   {
      dx = S_abs(origin->x - player->mo->x);
      dy = S_abs(origin->y - player->mo->y);
      dist_approx = dx + dy - ((dx < dy ? dx : dy) >> 1);
      vol = dist_approx >> 20;
      if(vol > 127)
         return;        // too far away
      vol = 127 - vol;
   }

   // Get sound effect data pointer
   sfx = &S_sfx[sound_id];

   // CALICO: check for valid sample
   if(!sfx->sample)
      return;

   newchannel = NULL;

   // reject sounds started at the same instant and singular sounds
   for(channel = sfxchannels, i = 0; i < SFXCHANNELS; i++, channel++)
   {
      if(channel->sfx == sfx)
      {
         if(hal_sound.isSampleAtStart(channel->handle)) // CALICO
         {
            return;        // exact sound allready started
         }

         if(sfx->singularity)
         {
            hal_sound.stopSound(channel->handle);
            newchannel = channel;    // overlay this
            break;
         }
      }

      if(channel->origin == origin)
      {
         // cut off whatever was coming from this origin
         hal_sound.stopSound(channel->handle);
         newchannel = channel;
         break;
      }

      if(!hal_sound.isSamplePlaying(channel->handle))
      {
         newchannel = channel;    // this is a dead channel, ok to reuse
         break;
      }
   }

   // if there weren't any dead channels, try to kill an equal or lower
   // priority channel

   if(!newchannel)
   {
      for(channel = sfxchannels, i = 0; i < SFXCHANNELS; i++, channel++)
      {
         if(channel->sfx->priority >= sfx->priority)
         {
            hal_sound.stopSound(channel->handle);
            newchannel = channel;
            break;
         }
      }

      if(!newchannel)
         return;        // couldn't override a channel
   }

   //
   // fill in the new values
   //
   newchannel->sfx       = sfx;
   newchannel->origin    = origin;
   newchannel->startquad = finalquad;
   newchannel->stopquad  = finalquad + (sfx->md_data->samples / 4);
   newchannel->source    = (int *)&sfx->md_data->data;
   newchannel->volume    = vol * (short)sfxvolume;

   // CALICO: start sound through HAL
   sampledata = SfxSample_GetSamples(sfx->sample);
   samplelen  = SfxSample_GetNumSamples(sfx->sample);
   newchannel->handle = hal_sound.startSound(sampledata, samplelen, newchannel->volume / 255, HAL_FALSE);
}

/*
===================
=
= S_UpdateSounds
=
===================
*/

extern int sfx_start;
extern int music_dspcode;

void S_UpdateSounds(void)
{
   //
   // if sound was just turned off, clear out the buffer
   //
   if(!sfxvolume)
   {
      if(oldsfxvolume)
      {
         oldsfxvolume = 0;
         S_Clear();
      }
      return;
   }
   else
   {
#if 0
      // Jag-specific
      if(!oldsfxvolume)
         finalquad = (samplecount >> 3) - 100;    // don't mix lots of junk
#endif
      oldsfxvolume = sfxvolume;
   }

   // CALICO_TODO: non-portable
#if 0
   int st;

   soundstarttics = samplecount;        // for timing calculations

   //
   // run the mixing in parallel on the dsp
   //
   if(music)
   {
      if(!musictime)
         musictime = next_eventtime = samplecount + EXTERN_BUFFER_SIZE/2;
      while(samplecount - musictime > EXTERN_BUFFER_SIZE)
      {
         musictime += EXTERN_BUFFER_SIZE;
         next_eventtime += EXTERN_BUFFER_SIZE;
      }
      st = samplecount;
      DSPFunction(&music_dspcode);
      musictics = samplecount - st;
   }
   else
   {
      dspfinished = 0x1234;
      dspcodestart = (int)&sfx_start;
   }
#endif
}

void S_StartSong(int music_id, int looping)
{
   int lump;

   if(nomusic) // CALICO
      return;

   musictime = 0;
   samples_per_midiclock = 0;
   lump = W_GetNumForName(S_music[music_id].name);
   music_memory = music = (unsigned char *)(W_CacheLumpNum(lump, PU_STATIC));
   music_start  = looping ? music : 0;
   music_end    = (unsigned char *)music + BIGLONG(lumpinfo[lump].size); // CALICO: endianness

}

void S_StopSong(void)
{
   if(nomusic) // CALICO
      return;

   Z_Free(music_memory);
   music = 0; // prevent the DSP from running
   
   // CALICO: Jag-specific.
#if 0
   int i;
   int *ptr;

   ptr = soundbuffer+1; // clear music output buffer
   for(i = (EXTERNALQUADS*32) / 4; i; i-=8)
   {
      ptr[0] = 0;
      ptr[2] = 0;
      ptr[4] = 0;
      ptr[6] = 0;
      ptr += 8;
   }
#endif
}

// EOF

