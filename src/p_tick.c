
#include "hal/hal_input.h"
#include "renderintr/ri_interface.h"
#include "rb/rb_common.h"
#include "doomdef.h"
#include "jagcry.h"
#include "p_local.h"

int playertics, thinkertics, sighttics, basetics, latetics;
int tictics;

boolean   gamepaused;
jagobj_t *pausepic;

/*
===============================================================================

THINKERS

All thinkers should be allocated by Z_Malloc so they can be operated on uniformly.
The actual structures will vary in size, but the first element must be thinker_t.

Mobjs are similar to thinkers, but kept seperate for more optimal list
processing
===============================================================================
*/

thinker_t thinkercap;     // both the head and tail of the thinker list
mobj_t    mobjhead;       // head and tail of mobj list
int       activethinkers; // debug count
int       activemobjs;    // debug count

/*
===============
=
= P_InitThinkers
=
===============
*/

void P_InitThinkers(void)
{
   thinkercap.prev = thinkercap.next = &thinkercap;
   mobjhead.next   = mobjhead.prev   = &mobjhead;
}

/*
===============
=
= P_AddThinker
=
= Adds a new thinker at the end of the list
=
===============
*/

void P_AddThinker(thinker_t *thinker)
{
   thinkercap.prev->next = thinker;
   thinker->next = &thinkercap;
   thinker->prev = thinkercap.prev;
   thinkercap.prev = thinker;
}

/*
===============
=
= P_RemoveThinker
=
= Deallocation is lazy -- it will not actually be freed until its
= thinking turn comes up
=
===============
*/

void P_RemoveThinker(thinker_t *thinker)
{
   // CALICO_TODO: non-portable
   thinker->function = (think_t)-1;
}

/*
===============
=
= P_RunThinkers
=
===============
*/

void P_RunThinkers(void)
{
   thinker_t *currentthinker;

   activethinkers = 0;

   currentthinker = thinkercap.next;
   while(currentthinker != &thinkercap)
   {
      if(currentthinker->function == (think_t)-1) // CALICO_FIXME: non-portable
      {
         // time to remove it
         currentthinker->next->prev = currentthinker->prev;
         currentthinker->prev->next = currentthinker->next;
         Z_Free(currentthinker);
      }
      else
      {
         if(currentthinker->function)
         {
            currentthinker->function(currentthinker);
         }
         activethinkers++;
      }
      currentthinker = currentthinker->next;
   }
}

//=============================================================================

/*
===============
=
= P_CheckSights
=
= Check sights of all mobj thinkers that are going to change state this
= tic and have MF_COUNTKILL set
===============
*/

void P_CheckSights2(void);

void P_CheckSights(void)
{
   P_CheckSights2();
}

//=============================================================================

/* 
=================== 
= 
= P_RunMobjBase  
=
= Run stuff that doesn't happen every tick
=================== 
*/ 

void P_RunMobjBase(void)
{
   P_RunMobjBase2();
}

//=============================================================================

/* 
=================== 
= 
= P_RunMobjLate  
=
= Run stuff that doesn't happen every tick
=================== 
*/ 

void P_RunMobjLate(void)
{
   P_RunMobjExtra();
}

//=============================================================================

/*
==============
=
= P_CheckCheats
=
==============
*/
 
void P_CheckCheats(void)
{
   int buttons, oldbuttons;
   int warpmap;
   int i;
   player_t *p;

   for(i = 0; i < MAXPLAYERS; i++)
   {
      if(!playeringame[i])
         continue;
      buttons    = ticbuttons[i];
      oldbuttons = oldticbuttons[i];

      if((buttons & BT_PAUSE) && !(oldbuttons & BT_PAUSE))
         gamepaused ^= 1;
   }

   if(netgame)
      return;

   buttons    = ticbuttons[0];
   oldbuttons = oldticbuttons[0];

   if((oldbuttons & JP_PAUSE) || !(buttons & JP_PAUSE))
      return;

   if(buttons & JP_NUM)
   {
      // free stuff
      p = &players[0];
      for(i = 0; i < NUMCARDS; i++)
         p->cards[i] = true;
      p->armorpoints = 200;
      p->armortype = 2;
      for(i = 0; i <NUMWEAPONS; i++)
         p->weaponowned[i] = true;
      for(i = 0; i < NUMAMMO; i++)
         p->ammo[i] = p->maxammo[i] = 500;
   }

   if(buttons & JP_STAR)
   {
      // godmode
      players[0].cheats ^= CF_GODMODE;
   }
   warpmap = 0;
   if(buttons & JP_1) warpmap = 1;
   if(buttons & JP_2) warpmap = 2;
   if(buttons & JP_3) warpmap = 3;
   if(buttons & JP_4) warpmap = 4;
   if(buttons & JP_5) warpmap = 5;
   if(buttons & JP_6) warpmap = 6;
   if(buttons & JP_7) warpmap = 7;
   if(buttons & JP_8) warpmap = 8;
   if(buttons & JP_9) warpmap = 9;
   if(buttons & JP_A) warpmap += 10;
   else if(buttons & JP_B) warpmap += 20;

   // CALICO_FIXME: limit incorrect for warp cheat
   if(warpmap > 0 && warpmap < 27)
   {
      gamemap = warpmap;
      gameaction = ga_warped;
   }
}

int playernum;

void G_DoReborn(int playernum); 

/*
=================
=
= P_Ticker
=
=================
*/

int ticphase;

int P_Ticker(void)
{
   //int       start;
   //int       ticstart;
   player_t *pl;

   //ticstart = samplecount;

   while(!I_RefreshLatched())
      ; // wait for refresh to latch all needed data before running the next tick

   gameaction = ga_nothing;

   gametic++;
 
   //
   // check for pause and cheats
   //
   P_CheckCheats();

   //
   // do option screen processing
   //
   for(playernum = 0, pl = players; playernum < MAXPLAYERS; playernum++, pl++)
   {
      if(playeringame[playernum])
         O_Control(pl);
   }

   if(gamepaused)
      return 0;

   //
   // run player actions
   //
   //start = samplecount;
   for(playernum = 0, pl = players; playernum < MAXPLAYERS; playernum++, pl++)
   {
      if(playeringame[playernum])
      {
         if(pl->playerstate == PST_REBORN) 
            G_DoReborn(playernum); 
         AM_Control(pl);
         P_PlayerThink(pl);
      }
   }

   //playertics = samplecount - start;

   //start = samplecount;
   P_RunThinkers();
   //thinkertics = samplecount - start;

   //start = samplecount;
   P_CheckSights();
   //sighttics = samplecount - start;

   //start = samplecount;
   P_RunMobjBase();
   //basetics = samplecount - start;

   //start = samplecount;
   P_RunMobjLate();
   //latetics = samplecount - start;

   P_UpdateSpecials();

   P_RespawnSpecials();

   ST_Ticker(); // update status bar

   //tictics = samplecount - ticstart;

   return gameaction; // may have been set to ga_died, ga_completed, or ga_secretexit
}

/* 
============= 
= 
= DrawPlaque 
= 
============= 
*/

void DrawPlaque(jagobj_t *pl, const char *name)
{
   int       x, y ,w, h;
   byte     *source;
   void     *rez;   // CALICO
   uint32_t *bdest; // CALICO

   while(!I_RefreshCompleted())
      ;

   source = pl->data;
   w      = BIGSHORT(pl->width);
   h      = BIGSHORT(pl->height);

   // CALICO: create or retrieve a texture resource for this plaque
   if(!(rez = g_renderer->CheckForTextureResource(name)))
   {
      if(!(rez = g_renderer->NewTextureResource(name, NULL, w, h, RES_FRAMEBUFFER, 0)))
         return;
      
      g_renderer->TextureResourceSetUpdated(rez);
      bdest = g_renderer->GetTextureResourceStore(rez);

      for(y = 0; y < h; y++)
      {
         for(x = 0; x < w; x++)
         {
            bdest[x] = CRYToRGB[palette8[*source++]];
         }
         bdest += w;
      }
   }

   g_renderer->AddLateDrawCommand(rez, 160 - w, 80, w*2, h);
}

/* 
=================== 
= 
= DrawSinglePlaque 
= 
=================== 
*/ 
 
void DrawSinglePlaque(jagobj_t *pl, const char *name)
{
   int       x, y ,w, h;
   byte     *source;
   void     *rez;   // CALICO
   uint32_t *bdest; // CALICO

   while(!I_RefreshCompleted())
      ;

   source = pl->data;
   w      = BIGSHORT(pl->width);
   h      = BIGSHORT(pl->height);

   // CALICO: create or retrieve a texture resource for this plaque
   if(!(rez = g_renderer->CheckForTextureResource(name)))
   {
      if(!(rez = g_renderer->NewTextureResource(name, NULL, w, h, RES_FRAMEBUFFER, 0)))
         return;

      g_renderer->TextureResourceSetUpdated(rez);
      bdest = g_renderer->GetTextureResourceStore(rez);

      for(y = 0; y < h; y++)
      {
         for(x = 0; x < w; x++)
         {
            bdest[x] = CRYToRGB[palette8[*source++]];
         }
         bdest += w;
      }
   }

   g_renderer->AddLateDrawCommand(rez, 160 - w / 2, 80, w, h);
}

/* 
============= 
= 
= P_Drawer 
= 
= draw current display 
============= 
*/ 
 
void P_Drawer(void) 
{
   static boolean refreshdrawn;

   if(players[consoleplayer].automapflags & AF_OPTIONSACTIVE)
   {
      O_Drawer();
      refreshdrawn = false;
   }
   else if(players[consoleplayer].automapflags & AF_ACTIVE)
   {
      ST_Drawer();
      AM_Drawer();
      I_Update();
      refreshdrawn = true;
   }
   else
   {
      if(gamepaused && refreshdrawn)     // CALICO: do this here
         DrawPlaque(pausepic, "paused");
      ST_Drawer();
      R_RenderPlayerView();
      refreshdrawn = true;
      // assume part of the refresh is now running parallel with main code
   }
} 
 
extern int ticremainder[2];

void P_Start(void)
{
   AM_Start();
   S_RestartSounds();

   players[0].automapflags = 0;
   players[1].automapflags = 0;
   ticremainder[0] = ticremainder[1] = 0;
   M_ClearRandom();
}

void P_Stop(void)
{
   Z_FreeTags(mainzone);
}

// EOF

