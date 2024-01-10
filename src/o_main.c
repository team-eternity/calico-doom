/* o_main.c -- options menu */

#include "hal/hal_input.h"
#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"
#include "g_options.h" // CALICO
#include "hal/hal_sfx.h" // CALICO

#define MOVEWAIT   5
#define ITEMSPACE  40
#define SLIDEWIDTH 90

extern int cx, cy;
extern int sfxvolume;   // range from 0 to 255
extern int controltype; // 0 to 5

extern void print(int x, int y, char *string);
extern void IN_DrawValue(int x, int y, int value);

// action buttons can be set to BT_A, BT_B, or BT_C
// strafe and use should be set to the same thing

extern unsigned int BT_ATTACK;
extern unsigned int BT_USE;
extern unsigned int BT_STRAFE;
extern unsigned int BT_SPEED;

typedef enum
{
   SFU,
   SUF,
   FSU,
   FUS,
   USF,
   UFS,
   NUMCONTROLOPTIONS
} control_t;

typedef enum
{
   soundvol,
   musicvol, // CALICO:
   controls,
   mainmenu, // CALICO: only if g_allowexit is true
   NUMMENUITEMS
} menupos_t;

menupos_t cursorpos;
static int nummenuitems; // CALICO

typedef struct
{
   int     x;
   int     y;
   boolean hasslider;
   char    name[20];
} menuitem_t;

menuitem_t menuitem[NUMMENUITEMS];
 
typedef struct
{
   int curval;
   int maxval;
} slider_t;

slider_t slider[2];

int cursorframe, cursorcount;
int movecount;

jagobj_t *uchar[52];

jagobj_t *o_cursor1, *o_cursor2;
jagobj_t *o_slider, *o_slidertrack;

char buttona[NUMCONTROLOPTIONS][8] = { "Speed", "Speed", "Fire",  "Fire",  "Use",   "Use"   };
char buttonb[NUMCONTROLOPTIONS][8] = { "Fire",  "Use",   "Speed", "Use",   "Speed", "Fire"  };
char buttonc[NUMCONTROLOPTIONS][8] = { "Use",   "Fire",  "Use",   "Speed", "Fire",  "Speed" };

unsigned int configuration[NUMCONTROLOPTIONS][3] =
{
   { BT_A, BT_B, BT_C },
   { BT_A, BT_C, BT_B },
   { BT_B, BT_A, BT_C },
   { BT_C, BT_A, BT_B },
   { BT_B, BT_C, BT_A },
   { BT_C, BT_B, BT_A } 
};

void O_SetButtonsFromControltype(void)
{
   BT_SPEED  = configuration[controltype][0];
   BT_ATTACK = configuration[controltype][1];
   BT_USE    = configuration[controltype][2];
   BT_STRAFE = configuration[controltype][2];
}

//
// Draw control value
//
void O_DrawControl(void)
{
   EraseBlock(menuitem[controls].x + 40, menuitem[controls].y + 20, 90, !g_allowexit ? 80 : 60, NULL);
   print(menuitem[controls].x + 40, menuitem[controls].y + 20, buttona[controltype]);
   print(menuitem[controls].x + 40, menuitem[controls].y + 40, buttonb[controltype]);
   print(menuitem[controls].x + 40, menuitem[controls].y + 60, buttonc[controltype]);

   O_SetButtonsFromControltype();
}

/*
===============
=
= O_Init
=
===============
*/

void O_Init(void)
{
   int i, l;

   // the eeprom has set controltype, so set buttons from that
   O_SetButtonsFromControltype();

   // cache all needed graphics
   o_cursor1     = W_CacheLumpName("M_SKULL1", PU_STATIC);
   o_cursor2     = W_CacheLumpName("M_SKULL2", PU_STATIC);
   o_slider      = W_CacheLumpName("O_SLIDER", PU_STATIC);
   o_slidertrack = W_CacheLumpName("O_STRACK", PU_STATIC);

   l = W_GetNumForName("CHAR_065");
   for(i = 0; i < 52; i++)
      uchar[i] = W_CacheLumpNum(l+i, PU_STATIC);

   // initialize variables

   cursorcount = 0;
   cursorframe = 0;
   cursorpos   = 0;

   D_strncpy(menuitem[soundvol].name, !g_allowmusicvolume ? "  Volume": "  Sound Volume", 16); // Fixed CEF
   menuitem[soundvol].x = 95 - (!g_allowmusicvolume ? 0 : 40);
   menuitem[soundvol].y = !g_allowexit ? 50 - (!g_allowmusicvolume ? 0 : 15) : 42 - (!g_allowmusicvolume ? 0 : 15);
   menuitem[soundvol].hasslider = true;

   slider[soundvol].maxval = 16;
   slider[soundvol].curval = 16*(sfxvolume + 1)/255; // [GEC] Fixes a small error in the position of the slider track. "sfxvolume -> (sfxvolume + 1)"

   if (g_allowmusicvolume) { // CALICO: add an option music volume
       D_strncpy(menuitem[musicvol].name, "  Music Volume", 16); // Fixed CEF
       menuitem[musicvol].x = 95 - 32;
       menuitem[musicvol].y = !g_allowexit ? 60 + (!g_allowmusicvolume ? 0 : 25) : 42 + (!g_allowmusicvolume ? 0 : 25);
       menuitem[musicvol].hasslider = true;

       slider[musicvol].maxval = 16;
       slider[musicvol].curval = 16 * (musicvolume + 1) / 255; // [GEC] Fixes a small error in the position of the slider track. "sfxvolume -> (sfxvolume + 1)"
   }

   D_strncpy(menuitem[controls].name, "Controls", 8); // Fixed CEF
   menuitem[controls].x = 95;
   menuitem[controls].y = !g_allowexit ? 110 + (!g_allowmusicvolume ? 0 : 20) : 90 + (!g_allowmusicvolume ? 0 : 20);
   menuitem[controls].hasslider = false;

   nummenuitems = 3;

   if(g_allowexit) // CALICO: add an option to return to the main menu
   {
       D_strncpy(menuitem[mainmenu].name, "Main Menu", 9);
       menuitem[mainmenu].x = 95;
       menuitem[mainmenu].y = 180 + (!g_allowmusicvolume ? 0 : 15);
       menuitem[mainmenu].hasslider = false;
       nummenuitems = 4;
   }
}

/*
==================
=
= O_Control
=
= Button bits can be eaten by clearing them in ticbuttons[playernum]
==================
*/

void O_Control(player_t *player)
{
   int buttons, oldbuttons;

   buttons    = ticbuttons[playernum];
   oldbuttons = oldticbuttons[playernum];

   if((buttons & BT_OPTION) && !(oldbuttons & BT_OPTION))
   {
      cursorpos = 0;
      player->automapflags ^= AF_OPTIONSACTIVE;
      if(player->automapflags & AF_OPTIONSACTIVE)
      {
         hal_appstate.setGrabState(HAL_FALSE); // CALICO: don't grab input
         DoubleBufferSetup();
      }
      else
      {
         hal_appstate.setGrabState(HAL_TRUE); // CALICO: grab input
         WriteEEProm(); // save new settings
      }
   }
   if(!(player->automapflags & AF_OPTIONSACTIVE))
      return;

   // clear buttons so game player isn't moving aroung
   ticbuttons[playernum] &= BT_OPTION; // leave option status alone

   if (playernum != consoleplayer)
      return;

   // animate skull
   if(++cursorcount == 4)
   {
      cursorframe ^= 1;
      cursorcount = 0;
   }

   // check for movement
   if(!(buttons & (JP_UP|JP_DOWN|JP_LEFT|JP_RIGHT|JP_SLEFT|JP_SRIGHT))) // CALICO: added SLEFT/SRIGHT
   {
      movecount = 0; // move immediately on next press

      // CALICO: allow exit option
      if(g_allowexit && cursorpos == mainmenu)
      {
          if(buttons & (BT_ATTACK|JP_ATTACK))
          {
              WriteEEProm(); // save new settings
              gameaction = ga_exitdemo;
              return;
          }
      }
   }
   else
   {
      if(buttons & (JP_RIGHT | JP_SRIGHT)) // CALICO: allow strafe-right input
      {
         if(menuitem[cursorpos].hasslider)
         {
            slider[cursorpos].curval++;
            if(slider[cursorpos].curval > slider[cursorpos].maxval)
               slider[cursorpos].curval = slider[cursorpos].maxval;
            if(cursorpos == soundvol)
            {
               sfxvolume = 255*slider[cursorpos].curval / slider[cursorpos].maxval;
               S_StartSound(NULL, sfx_pistol);

               hal_sound.setMasterVolume(sfxvolume, g_allowmusicvolume ? musicvolume : sfxvolume); // CALICO
            }

            if (cursorpos == musicvol && g_allowmusicvolume)
            {
                musicvolume = 255 * slider[cursorpos].curval / slider[cursorpos].maxval;
                S_StartSound(NULL, sfx_pistol);

                hal_sound.setMasterVolume(sfxvolume, musicvolume); // CALICO
            }
         }
      }
      if(buttons & (JP_LEFT | JP_SLEFT)) // CALICO: allow strafe-left input
      {
         if(menuitem[cursorpos].hasslider)
         {
            slider[cursorpos].curval--;
            if(slider[cursorpos].curval < 0)
               slider[cursorpos].curval = 0;
            if(cursorpos == soundvol)
            {
               sfxvolume = 255*slider[cursorpos].curval / slider[cursorpos].maxval;
               S_StartSound(NULL, sfx_pistol);

               hal_sound.setMasterVolume(sfxvolume, g_allowmusicvolume ? musicvolume: sfxvolume); // CALICO
            }

            if (cursorpos == musicvol && g_allowmusicvolume)
            {
                musicvolume = 255 * slider[cursorpos].curval / slider[cursorpos].maxval;
                S_StartSound(NULL, sfx_pistol);

                hal_sound.setMasterVolume(sfxvolume, musicvolume); // CALICO
            }
         }
      }

      if(movecount == MOVEWAIT)
         movecount = 0; // repeat move
      if(++movecount == 1)
      {
         if(buttons & JP_DOWN)
         {
            cursorpos++;
            if (!g_allowmusicvolume) { // CALICO: skip music volume option
                if (cursorpos == musicvol) {
                    cursorpos++;
                }
            }
            if(cursorpos == nummenuitems)
               cursorpos = 0;
         }

         if(buttons & JP_UP)
         {
            cursorpos--;
            if (!g_allowmusicvolume) { // CALICO: skip music volume option
                if (cursorpos == musicvol) {
                    cursorpos--;
                }
            }
            if(cursorpos == -1)
               cursorpos = nummenuitems-1;
         }
         if(buttons & (JP_RIGHT | JP_SRIGHT)) // CALICO: allow strafe-right input
         {
            if(cursorpos == controls)
            {
               controltype++;
               if(controltype == NUMCONTROLOPTIONS)
                  controltype = (NUMCONTROLOPTIONS - 1); 
            }
         }
         if(buttons & (JP_LEFT | JP_SLEFT)) // CALICO: allow strafe-left input
         {
            if(cursorpos == controls)
            {
               controltype--;
               if(controltype == -1)
                  controltype = 0; 
            }
         }
      }
   }
}

void O_Drawer(void)
{
   int i;
   int offset;

   // Erase old and Draw new cursor frame
   EraseBlock(56 - (!g_allowmusicvolume ? 0 : 40), 40 - (!g_allowmusicvolume ? 0 : 20), o_cursor1->width, 200, NULL);

   int cursorposX = 0;

   if (g_allowmusicvolume) {
       if (cursorpos == soundvol) {
           cursorposX = 28;
       }

       if (cursorpos == musicvol) {
           cursorposX = 20;
       }
   }

   if(cursorframe)
      DrawJagobj(o_cursor1, 60 - cursorposX, menuitem[cursorpos].y - 2, NULL);
   else
      DrawJagobj(o_cursor2, 60 - cursorposX, menuitem[cursorpos].y - 2, NULL);

   // Draw menu

   print(104, 10 - (!g_allowmusicvolume ? 0 : 5), "Options");

   for(i = 0; i < nummenuitems; i++)
   {
      print(menuitem[i].x, menuitem[i].y, menuitem[i].name);

      if(menuitem[i].hasslider == true)
      {
         int sliderX = !g_allowmusicvolume ? menuitem[i].x : (160 - (BIGSHORT(o_slidertrack->width) / 2)) - 2;
         DrawJagobj(o_slidertrack, sliderX + 2, menuitem[i].y + 20, NULL);
         offset = (slider[i].curval * SLIDEWIDTH) / slider[i].maxval;
         DrawJagobj(o_slider, sliderX + 7 + offset, menuitem[i].y + 20, NULL);
      }
   }

   // Draw control info

   print(menuitem[controls].x + 10, menuitem[controls].y + 20, "A");
   print(menuitem[controls].x + 10, menuitem[controls].y + 40, "B");
   print(menuitem[controls].x + 10, menuitem[controls].y + 60, "C");

   O_DrawControl();

   UpdateBuffer();
}

// EOF

