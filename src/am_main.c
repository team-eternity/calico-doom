/* am_main.c -- automap */

#include "gl/gl_render.h"
#include "rb/rb_common.h"
#include "jagcry.h"
#include "doomdef.h"
#include "p_local.h"

#define STEPVALUE   0x800000

#define CRY_RED     0xd260
#define CRY_BLUE    0x3080
#define CRY_GREEN   0x6f80
#define CRY_BROWN   0xba20
#define CRY_YELLOW  0xff80
#define CRY_GREY    0x9826
#define CRY_AQUA    0x6aa0

int     blink;
int     pause;
#define MAXSCALES 5
int     scale;
int     scalex[MAXSCALES] = { 18, 19, 20, 21, 22 };
int     scaley[MAXSCALES] = { 17, 18, 19, 20, 21 };
#define NOSELENGTH 0x200000 // PLAYER'S TRIANGLE
#define MOBJLENGTH 0x100000

fixed_t oldplayerx;
fixed_t oldplayery;
int     blockx;
int     blocky;

// CHEATING STUFF
typedef enum
{
   ch_allmap,
   ch_things,
   ch_maxcheats
} cheat_e;

char cheatstrings[][11] = // order should mirror cheat_e
{
   "8002545465", // allmap cheat
   "8005778788"  // show things cheat
};

char currentcheat[11] = "0000000000";
int  showAllThings; // CHEAT VARS
int  showAllLines;

static uint32_t *framebuffer; // CALICO: framebuffer pointer

//=================================================================
//
// Start up Automap
//
//=================================================================
void AM_Start(void)
{
   scale = 3;
   showAllThings = showAllLines = 0;
   players[consoleplayer].automapflags &= ~AF_ACTIVE;
   
   // CALICO: get framebuffer pointer
   framebuffer = GL_GetFramebuffer(FB_160);
}

//=================================================================
//
// Check for cheat codes for automap fun stuff!
//
//=================================================================
cheat_e AM_CheckCheat(int buttons,int oldbuttons)
{
   // CALICO: Fixed to actually work (was missing entry for BT_0)
   int  codes[10] = { BT_0, BT_1, BT_2, BT_3, BT_4, BT_5, BT_6 ,BT_7, BT_8, BT_9 };
   char chars[10] = "0123456789";
   char c;
   int  i;

   // CONVERT BUTTON PRESS TO A CHARACTER
   c = 'z';
   for(i = 0; i < 10; i++)
   {
      if((buttons & codes[i]) && !(oldbuttons & codes[i]))
      {
         c = chars[i];
         break;
      }
   }

   if(c == 'z') // NO 1-9 BUTTON PRESSED
      return -1;

   // SHIFT STRING LEFT A CHARACTER
   for(i = 1; i < 10; i++)
      currentcheat[i-1] = currentcheat[i];
   currentcheat[9] = c;
   currentcheat[10] = 0;

   // SEE IF STRING MATCHES A CHEATSTRING
   for(i = 0; i < ch_maxcheats; i++)
   {
      if(!D_strncasecmp(currentcheat, cheatstrings[i], 10))
         return i;
   }

   return -1;
}

//
// Draw an automap line
// CALICO: Rewritten for portability
//
void DrawLine(pixel_t color, int x1, int y1, int x2, int y2)
{
   int dx, dy;
   int adx, ady;
   uint32_t quadcolor;
   fixed_t xstep, ystep;
   int count = 1;
   fixed_t fx, fy;
   int x, y;
   uint32_t *a1ptr;

   quadcolor = CRYToRGB[color];

   dx  = x2 - x1;
   adx = dx < 0 ? -dx : dx;
   dy  = y2 - y1;
   ady = dy < 0 ? -dy : dy;

   if(!dx)
   {
      xstep = 0;
      ystep = dy > 0 ? FRACUNIT : -FRACUNIT;
      count = ady; // count
   }
   else if(!dy)
   {
      ystep = 0;
      xstep = dx > 0 ? FRACUNIT : -FRACUNIT;
      count = adx; // count
   }
   else if(adx > ady)
   {
      if(dx > 0)
         xstep = FRACUNIT;
      else
         xstep = -FRACUNIT;

      ystep = (dy << FRACBITS) / adx;
      count = adx; // count 
   }
   else
   {
      if(dy > 0)
         ystep = FRACUNIT;
      else
         ystep = -FRACUNIT;

      xstep = (dx << FRACBITS) / ady;
      count = ady; // count
   }

   fx = x1 << FRACBITS;
   fy = y1 << FRACBITS;
   do
   {
      x = fx / FRACUNIT;
      y = fy / FRACUNIT;
      if(x >= 0 && x < SCREENWIDTH && y >= 0 && y < SCREENHEIGHT)
      {
         a1ptr = framebuffer + y * SCREENWIDTH + x;
         *a1ptr = quadcolor;
      }
      fx += xstep;
      fy += ystep;
   }
   while(count--);
}

/*
==================
=
= AM_Control
=
= Called by P_PlayerThink before any other player processing
=
= Button bits can be eaten by clearing them in ticbuttons[playernum]
==================
*/

void AM_Control(player_t *player)
{
   int     buttons, oldbuttons, step;
   cheat_e cheatcode;

   buttons    = ticbuttons[playernum];
   oldbuttons = oldticbuttons[playernum];

   if((buttons & BT_9) && !(oldbuttons & BT_9))
   {
      player->automapflags ^= AF_ACTIVE;
      player->automapx = player->mo->x;
      player->automapy = player->mo->y;
      blockx = 80;
      blocky = 90;
   }

   if(!(player->automapflags & AF_ACTIVE))
      return;

   oldplayerx = player->automapx;
   oldplayery = player->automapy;

   blink = (blink++)&7; // BLINK PLAYER'S BOX 
   pause++;             // PAUSE BETWEEN SCALINGS

   step = STEPVALUE;
   if(buttons & BT_A)
      step *= 2;

   cheatcode = AM_CheckCheat(buttons,oldbuttons);
   switch(cheatcode)
   {
   case ch_allmap:
      showAllLines ^= 1;
      break;
   case ch_things:
      showAllThings ^= 1;
   default:
      break;
   }

   if(buttons & BT_C) // IF 'C' IS HELD DOWN, MOVE AROUND
   {
      ticbuttons[playernum] &= ~BT_C;
      oldticbuttons[playernum] &= ~BT_C;
      return;
   }

   if(buttons & BT_RIGHT)
   {
      if(buttons & BT_B)
      {
         if(pause > 5)
         {
            pause = 0;
            scale--;
            if(scale < 0)
               scale = 0;
         }
      }
      else
         player->automapx+=step;
   }

   if(buttons & BT_LEFT)
   {
      if(buttons & BT_B)
      {
         if(pause > 5)
         {
            pause = 0;
            scale++;
            if(scale == MAXSCALES)
               scale--;
         }
      }
      else
         player->automapx-=step;
   }

   if(buttons & BT_UP)
   {
      if(buttons & BT_B)
      {
         if(pause > 5)
         {
            pause = 0;
            scale--;
            if(scale < 0)
               scale = 0;
         }
      }
      else
         player->automapy+=step;
   }

   if(buttons & BT_DOWN)
   {
      if(buttons & BT_B)
      {
         if(pause > 5)
         {
            pause = 0;
            scale++;
            if(scale == MAXSCALES)
               scale--;
         }
      }
      else
         player->automapy-=step;
   }

   ticbuttons[playernum] &= ~(BT_B|BT_LEFT|BT_RIGHT|BT_UP|BT_DOWN);
}

/*
==================
=
= AM_Drawer
=
= Draws the current frame to workingscreen
==================
*/

void AM_Drawer(void)
{
   int       i;
   player_t *p;
   line_t   *line;
   int       x1,y1;
   int       x2,y2;
   int       ox,oy;
   int       outcode;
   int       outcode2;
   int       color;
   int       xshift;
   int       yshift;
   int       drawn; // HOW MANY LINES DRAWN?

   // CALICO: Clear playfield framebuffer
   GL_ClearFramebuffer(FB_160, RB_COLOR_BLACK);

   p = &players[consoleplayer];
   ox = p->automapx;
   oy = p->automapy;

   xshift = scalex[scale];
   yshift = scaley[scale];

   line = lines;
   drawn = 0;
   for(i = 0; i < numlines; i++, line++)
   {
      if((!(line->flags & ML_MAPPED) || // IF NOT MAPPED OR DON'T DRAW
         line->flags & ML_DONTDRAW) &&
         (!(p->powers[pw_allmap] + showAllLines)))
         continue;

      x1 = line->v1->x;
      y1 = line->v1->y;
      x2 = line->v2->x;
      y2 = line->v2->y;

      x1 -= ox;
      x2 -= ox;
      y1 -= oy;
      y2 -= oy;
      x1 >>= xshift;
      x2 >>= xshift;
      y1 >>= yshift;
      y2 >>= yshift;

      outcode = (y1 > 90) << 1;
      outcode |= (y1 < -90);
      outcode2 = (y2 > 90) << 1;
      outcode2 |= (y2 < -90);
      if(outcode & outcode2) 
         continue;

      outcode = (x1 > 80) << 1;
      outcode |= (x1 < -80);
      outcode2 = (x2 > 80) << 1;
      outcode2 |= (x2 < -80);
      if(outcode & outcode2)
         continue;

      //
      // Figure out color
      //
      color = CRY_BROWN;
      if((p->powers[pw_allmap] +
         showAllLines) && // IF COMPMAP && !MAPPED YET
         !(line->flags & ML_MAPPED))
         color = CRY_GREY;
      else if (!(line->flags & ML_TWOSIDED)) // ONE-SIDED LINE
         color = CRY_RED;
      else if (line->special == 97 || // TELEPORT LINE
         line->special == 39)
         color = CRY_GREEN;
      else if (line->flags & ML_SECRET)
         color = CRY_RED;
      else if (line->special)
         color = CRY_BLUE; // SPECIAL LINE
      else if (line->frontsector->floorheight != line->backsector->floorheight)
         color = CRY_YELLOW;
      else if (line->frontsector->ceilingheight != line->backsector->ceilingheight)
         color = CRY_BROWN;

      DrawLine(color, 80 + x1, 90 - y1, 80 + x2, 90 - y2);
      drawn++;
   }

   // IF <5 LINES DRAWN, MOVE TO LAST POSITION!
   if(drawn < 5)
   {
      p->automapx = oldplayerx;
      p->automapy = oldplayery;
   }

   if(blink > 2)
   {
      fixed_t c;
      fixed_t s;
      fixed_t x1,y1;
      fixed_t nx1,ny1;
      fixed_t nx2,ny2;
      fixed_t nx3,ny3;
      angle_t angle;
      player_t *pl;

      for(i = 0; i < MAXPLAYERS; i++)
      {
         if((i != consoleplayer) && (netgame != gt_coop))
            continue;

         pl = &players[i];
         x1 = (pl->mo->x - players[consoleplayer].automapx);
         y1 = (pl->mo->y - players[consoleplayer].automapy);
         angle = pl->mo->angle;
         color = !i ? CRY_GREEN : CRY_YELLOW;
         c = finecosine[angle >> ANGLETOFINESHIFT];
         s = finesine[angle >> ANGLETOFINESHIFT];
         nx1 = FixedMul(c,NOSELENGTH) + x1;
         ny1 = FixedMul(s,NOSELENGTH) + y1;

         c = finecosine[((angle - ANG90) - ANG45) >> ANGLETOFINESHIFT];
         s = finesine[((angle - ANG90) - ANG45) >> ANGLETOFINESHIFT];
         nx2 = FixedMul(c, NOSELENGTH) + x1;
         ny2 = FixedMul(s, NOSELENGTH) + y1;

         c = finecosine[((angle + ANG90) + ANG45) >> ANGLETOFINESHIFT];
         s = finesine[((angle + ANG90) + ANG45) >> ANGLETOFINESHIFT];
         nx3 = FixedMul(c, NOSELENGTH) + x1;
         ny3 = FixedMul(s, NOSELENGTH) + y1;

         nx1 >>= xshift;
         ny1 >>= yshift;
         nx2 >>= xshift;
         ny2 >>= yshift;
         nx3 >>= xshift;
         ny3 >>= yshift;

         DrawLine(color, 80 + nx1, 90 - ny1, 80 + nx2, 90 - ny2);
         DrawLine(color, 80 + nx2, 90 - ny2, 80 + nx3, 90 - ny3);
         DrawLine(color, 80 + nx1, 90 - ny1, 80 + nx3, 90 - ny3);
      }
   }

   // SHOW ALL MAP THINGS (CHEAT)
   if(showAllThings)
   {
      fixed_t  x1,y1;
      fixed_t  nx1,ny1;
      fixed_t  nx2,ny2;
      fixed_t  nx3,ny3;
      mobj_t  *mo;
      mobj_t  *next;

      for(mo = mobjhead.next; mo != &mobjhead; mo = next)
      {
         // CALICO: skip removed mobjs
         next = mo->next;
         if(mo == p->mo || mo->latecall == P_RemoveMobjDeferred)
            continue;

         x1 = mo->x - p->automapx;
         y1 = mo->y - p->automapy;

         nx1 = x1 >> xshift;
         ny1 = (y1 - MOBJLENGTH) >> yshift;
         nx2 = (x1 - MOBJLENGTH) >> xshift;
         ny2 = (y1 + MOBJLENGTH) >> yshift;
         nx3 = (x1 + MOBJLENGTH) >> xshift;
         ny3 = ny2;

         DrawLine(CRY_AQUA, 80 + nx1, 90 - ny1, 80 + nx2, 90 - ny2);
         DrawLine(CRY_AQUA, 80 + nx2, 90 - ny2, 80 + nx3, 90 - ny3);
         DrawLine(CRY_AQUA, 80 + nx1, 90 - ny1, 80 + nx3, 90 - ny3);
      }
   }
}

// EOF

