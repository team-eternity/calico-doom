/* m_main.c -- main menu */

#include "hal/hal_input.h"
#include "hal/hal_ml.h"
#include "doomdef.h"
#include "g_options.h" // CALICO
	
#define MOVEWAIT   3
#define CURSORX    50
#define STARTY     40
#define CURSORY(y) (STARTY*(y))
#define	NUMLCHARS  64	

typedef enum
{
   gamemode,
   level,
   difficulty,
   quit,
   NUMMENUITEMS
} menu_t;

static int nummenuitems; // CALICO

typedef enum
{
   single,
   coop,
   dmatch,
   NUMMODES
} playmode_t;

jagobj_t *m_doom, *m_skull1, *m_skull2, *m_gamemode, *m_level, *m_difficulty,
         *m_single, *m_coop, *m_deathmatch;

jagobj_t *nums[10];
jagobj_t *m_skill[5];
jagobj_t *m_playmode[NUMMODES];

int cursorframe, cursorcount;
int movecount;
int playermap;

playmode_t currentplaymode;
menu_t     cursorpos;
skill_t    playerskill;

void M_Start(void)
{
   int i,l;

   nummenuitems = g_allowexit ? NUMMENUITEMS : NUMMENUITEMS - 1; // CALICO: adjustable bounds

   // cache all needed graphics
   m_doom       = W_CacheLumpName("M_DOOM",   PU_STATIC);
   m_skull1     = W_CacheLumpName("M_SKULL1", PU_STATIC);
   m_skull2     = W_CacheLumpName("M_SKULL2", PU_STATIC);
   m_gamemode   = W_CacheLumpName("M_GAMMOD", PU_STATIC);
   m_level      = W_CacheLumpName("M_LEVEL",  PU_STATIC);
   m_difficulty = W_CacheLumpName("M_DIFF",   PU_STATIC);

   l = W_GetNumForName("M_SINGLE");
   for(i = 0; i < NUMMODES; i++)
      m_playmode[i] = W_CacheLumpNum (l+i, PU_STATIC);

   l = W_GetNumForName("SKILL0");
   for(i = 0; i < 5; i++)
      m_skill[i] = W_CacheLumpNum (l+i, PU_STATIC);

   l = W_GetNumForName("NUM_0");
   for(i = 0; i < 10; i++)
      nums[i] = W_CacheLumpNum(l+i, PU_STATIC);

   cursorcount = 0;
   cursorframe = 0;
   cursorpos   = 0;
   playerskill = startskill;
   playermap   = startmap;

   DoubleBufferSetup();

   hal_appstate.setGrabState(HAL_FALSE); // CALICO: don't grab input
}

void M_Stop(void)
{
   WriteEEProm();
}

/*
=================
=
= M_Ticker
=
=================
*/

int M_Ticker(void)
{
   int buttons;
   const int onquit = (g_allowexit && cursorpos == quit); // CALICO: allow quit option

   buttons = ticbuttons[consoleplayer];

   // exit menu if button press
   // CALICO: added extra custom input actions
   if(ticon > 10 && (buttons & (JP_A|JP_B|JP_C|JP_ATTACK|JP_USE|JP_STRAFE|JP_SPEED)) && !onquit)
   {
      startmap   = playermap;       // set map number
      startskill = playerskill;     // set skill level
      starttype  = currentplaymode; // set play type
      return 1;                     // done with menu
   }

   // animate skull
   if(++cursorcount == 4)
   {
      cursorframe ^= 1;
      cursorcount = 0;
   }

   // check for movement
   if(!(buttons & (JP_UP|JP_DOWN|JP_LEFT|JP_RIGHT|JP_SLEFT|JP_SRIGHT))) // CALICO: allow SLEFT/SRIGHT
   {
      movecount = 0; // move immediately on next press

      // CALICO: allow quit option
      if(onquit && (buttons & (BT_ATTACK|JP_ATTACK)))
          hal_medialayer.exit();
   }
   else
   {
      if(cursorpos == level && movecount == 3)
         movecount = 0; // fast level select
      if(movecount == 6)
         movecount = 0; // slower everything else
      if(++movecount == 1)
      {
         if(buttons & JP_DOWN)
         {
            cursorpos++;
            if(cursorpos == nummenuitems)
               cursorpos = 0;
         }

         if(buttons & JP_UP)
         {
            cursorpos--;
            if(cursorpos == -1)
               cursorpos = nummenuitems-1;
         }

         switch(cursorpos)
         {
         case gamemode:
            if(buttons & (JP_RIGHT|JP_SRIGHT)) // CALICO: allow SRIGHT
            {
               currentplaymode++;
               if(currentplaymode == NUMMODES)
                  currentplaymode--;
            }
            if(buttons & (JP_LEFT|JP_SLEFT)) // CALICO: allow SLEFT
            {
               currentplaymode--;
               if (currentplaymode == -1)
                  currentplaymode++;
            }
            break;
         case level:
            if(buttons & (JP_RIGHT|JP_SRIGHT))
            {			
               playermap++;
               if(playermap == maxlevel+1)
                  playermap--;
            }	
            if(buttons & (JP_LEFT|JP_SLEFT))
            {
               playermap--;
               if(playermap == 0)
                  playermap++;
            }
            break;
         case difficulty:
            if(buttons & (JP_RIGHT|JP_SRIGHT))
            {
               playerskill++;
               if(playerskill > sk_nightmare)
                  playerskill--;
            }
            if(buttons & (JP_LEFT|JP_SLEFT))
            {
               playerskill--;
               if(playerskill == -1)
                  playerskill++;
            }
            break;
         default:
            break;
         }
      }
   }

   return 0;
}

extern void print(int x, int y, char *string); // CALICO

/*
=================
=
= M_Drawer
=
=================
*/

void M_Drawer(void)
{
   int leveltens, levelones;
   int m_doomheight = BIGSHORT(m_doom->height); // CALICO: needs endianness correction

   // Draw main menu 
   DrawJagobj(m_doom, 100, 2, NULL);

   // erase old skulls
   EraseBlock(CURSORX, 0, BIGSHORT(m_skull1->width), 240, NULL);

   // draw new skull
   if(cursorframe)
      DrawJagobj(m_skull2, CURSORX, CURSORY(cursorpos) + m_doomheight, NULL);
   else
      DrawJagobj(m_skull1, CURSORX, CURSORY(cursorpos) + m_doomheight, NULL);

   // draw menu items

   // draw game mode information
   DrawJagobj(m_gamemode, 74, BIGSHORT(m_doom->height)+2, NULL);
   EraseBlock(90, m_doomheight + 22, 320-90, 240 - m_doomheight + 22, NULL);
   DrawJagobj(m_playmode[currentplaymode], 90, m_doomheight + 22, NULL);

   // draw start level information
   DrawJagobj(m_level, 74 ,CURSORY(1) + m_doomheight + 2, NULL); 
   leveltens = playermap / 10;
   levelones = playermap % 10;
   EraseBlock(90, m_doomheight + 61, 320-90, 200 - m_doomheight + 62, NULL);
   if(leveltens)
   {
      DrawJagobj(nums[leveltens],  90, m_doomheight + 62, NULL);
      DrawJagobj(nums[levelones], 104, m_doomheight + 62, NULL);
   }
   else
      DrawJagobj(nums[levelones], 90, m_doomheight + 62, NULL);

   // draw difficulty information
   DrawJagobj(m_difficulty, CURSORX + 24, CURSORY(2) + m_doomheight + 2, NULL); 
   EraseBlock(92, m_doomheight + 102, 320-92, 240 - m_doomheight + 102, NULL);
   DrawJagobj(m_skill[playerskill], 92, m_doomheight + 102, NULL);

   // CALICO: test
   if(g_allowexit)
       print(CURSORX + 24, CURSORY(3) + m_doomheight + 2, "Quit Game");

   UpdateBuffer();
}

// EOF

