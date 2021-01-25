/* marsonly.c */

#include <stdint.h>
#include <stdlib.h>

#include "elib/configfile.h"
#include "elib/m_argv.h"
#include "hal/hal_init.h"
#include "hal/hal_input.h"
#include "hal/hal_platform.h"
#include "hal/hal_timer.h"
#include "hal/hal_video.h"
#include "renderintr/ri_interface.h"
#include "rb/rb_common.h"
#include "doomdef.h"
#include "jagcry.h"
#include "r_local.h"
#include "w_iwad.h"

/*

JAGUAR MEMORY MAP

0x  4000    text/data/bss
0x 80000    start heap
0x1c8000    sbarback (0x3210)
0x1cb210    sbarfront (0x3200)
0x1ce410    debugscreen (0x1bf0)
0x1d0000    screens[0]
0x1e0000    screens[1]
0x1f0000    soundbuffer
0x1f4000    stack (8 bytes of screenshade)
0x200000    end of ram

at all times,

pixel_t *workingscreen; will point to the undisplayed screens[workpage]

*/

static boolean debugscreenstate = false;

boolean debugscreenactive;

unsigned short *palette8;       // [256] for translating 8 bit source to 16 bit

void *debugscreenrez; // CALICO
uint32_t *debugscreen;
extern jagobj_t *sbar;
extern void     *sbartop;

static void *sbarrez;

int joypad[32]; 
int joystick1; 

int junk; 

char hexdigits[] = "0123456789ABCDEF";

void ReadEEProm(void);

static void I_GetFramebuffer(void);

//
// CALICO: HAL callback for getting game's opinion of whether input should be grabbed
//
static hal_bool ShouldGrabInput(void)
{
   return !gamepaused;
}

/* 
================ 
= 
= Jag68k_main  
= 
================ 
*/ 

//
// Main routine.
//
void Jag68k_main(int argc, const char *const *argv)
{
   // CALICO: initialize HAL
   if(!HAL_Init())
      hal_platform.fatalError("HAL initialization failed");

   hal_platform.debugMsg("HAL initialized\n");

   // CALICO: read global configuration
   Cfg_LoadFile();

   hal_platform.debugMsg("Read config file\n");

   // CALICO: initialize video
   hal_video.initVideo();
   CRY_BuildRGBTable();
   I_GetFramebuffer();

   hal_platform.debugMsg("Video initialized\n");

   // CALICO: initialize input
   hal_input.initInput();
   hal_appstate.gameGrabCallback = ShouldGrabInput;

   if(M_FindArgument("-devparm")) // CALICO: turn on debugging features
      debugscreenstate = true;

   debugscreenactive = debugscreenstate;
   debugscreenrez = g_renderer->NewTextureResource("debugscreen", NULL, 256, 224, RES_FRAMEBUFFER, 0);
   debugscreen    = g_renderer->GetTextureResourceStore(debugscreenrez);

   // CALICO: Jag-specific
#if 0
   /* clear bss and screens */
   D_memset(&enddata, 0, 0x1fc000 - (int)&enddata);

   /* init vars  */
   sbar        = (jagobj_t *)0x1c8000;
   sbartop     = (byte *)0x1cb210;
   debugscreen = (byte *)0x1ce410;
   framebuffer = screens[0] = (pixel_t *)0x1d0000; 
   screens[1]  = (pixel_t *)0x1e0000; 
   screenshade = (int *)0x1f4000;
#endif

   I_Print8(1, 0, "GPU_main");

   // CALICO: Jag-specific
#if 0
   // init sound hardware
   *(int *)0xf14000 = 0x100; /* JOYSTICK (unmute sound) */

   *(int *)0xf1a150 = 19;   /* SCLK (SSI Clock frequency) */
   *(int *)0xf1a154 = 0x15; /* SMODE (SSI Control) */
#endif

   I_Update();

   // load defaults
   ReadEEProm();

   // start doom
   D_DoomMain();
}

//=============================================================================
//
// DOOM INTERFACE CODE
//
//=============================================================================

byte font8[] =
{
   0,0,0,0,0,0,0,0,24,24,24,24,24,0,24,0,
   54,54,54,0,0,0,0,0,108,108,254,108,254,108,108,0,
   48,124,192,120,12,248,48,0,198,204,24,48,102,198,0,0,
   56,108,56,118,220,204,118,0,24,24,48,0,0,0,0,0,
   6,12,24,24,24,12,6,0,48,24,12,12,12,24,48,0,
   0,102,60,255,60,102,0,0,0,24,24,126,24,24,0,0,
   0,0,0,0,48,48,96,0,0,0,0,0,126,0,0,0,
   0,0,0,0,0,96,96,0,6,12,24,48,96,192,128,0,
   56,108,198,198,198,108,56,0,24,56,24,24,24,24,60,0,
   248,12,12,56,96,192,252,0,248,12,12,56,12,12,248,0,
   28,60,108,204,254,12,12,0,252,192,248,12,12,12,248,0,
   60,96,192,248,204,204,120,0,252,12,24,48,96,192,192,0,
   120,204,204,120,204,204,120,0,120,204,204,124,12,12,120,0,
   0,96,96,0,96,96,0,0,0,96,96,0,96,96,192,0,
   12,24,48,96,48,24,12,0,0,0,252,0,0,252,0,0,
   96,48,24,12,24,48,96,0,124,6,6,28,24,0,24,0,
   124,198,222,222,222,192,120,0,48,120,204,204,252,204,204,0,
   248,204,204,248,204,204,248,0,124,192,192,192,192,192,124,0,
   248,204,204,204,204,204,248,0,252,192,192,248,192,192,252,0,
   252,192,192,248,192,192,192,0,124,192,192,192,220,204,124,0,
   204,204,204,252,204,204,204,0,60,24,24,24,24,24,60,0,
   12,12,12,12,12,12,248,0,198,204,216,240,216,204,198,0,
   192,192,192,192,192,192,252,0,198,238,254,214,198,198,198,0,
   198,230,246,222,206,198,198,0,120,204,204,204,204,204,120,0,
   248,204,204,248,192,192,192,0,120,204,204,204,204,216,108,0,
   248,204,204,248,240,216,204,0,124,192,192,120,12,12,248,0,
   252,48,48,48,48,48,48,0,204,204,204,204,204,204,124,0,
   204,204,204,204,204,120,48,0,198,198,198,214,254,238,198,0,
   198,198,108,56,108,198,198,0,204,204,204,120,48,48,48,0,
   254,12,24,48,96,192,254,0,120,96,96,96,96,96,120,0,
   192,96,48,24,12,6,0,0,120,24,24,24,24,24,120,0,
   24,60,102,66,0,0,0,0,0,0,0,0,0,0,255,0,
   192,192,96,0,0,0,0,0,0,0,120,12,124,204,124,0,
   192,192,248,204,204,204,248,0,0,0,124,192,192,192,124,0,
   12,12,124,204,204,204,124,0,0,0,120,204,252,192,124,0,
   60,96,96,248,96,96,96,0,0,0,124,204,124,12,248,0,
   192,192,248,204,204,204,204,0,24,0,56,24,24,24,60,0,
   24,0,24,24,24,24,240,0,192,192,204,216,240,216,204,0,
   56,24,24,24,24,24,60,0,0,0,236,214,214,214,214,0,
   0,0,248,204,204,204,204,0,0,0,120,204,204,204,120,0,
   0,0,248,204,204,248,192,0,0,0,124,204,204,124,12,0,
   0,0,220,224,192,192,192,0,0,0,124,192,120,12,248,0,
   96,96,252,96,96,96,60,0,0,0,204,204,204,204,124,0,
   0,0,204,204,204,120,48,0,0,0,214,214,214,214,110,0,
   0,0,198,108,56,108,198,0,0,0,204,204,124,12,248,0,
   0,0,252,24,48,96,252,0,28,48,48,224,48,48,28,0,
   24,24,24,0,24,24,24,0,224,48,48,28,48,48,224,0,
   118,220,0,0,0,0,0,0,252,252,252,252,252,252,252,0
};

//
// Print a debug message.
// CALICO: Rewritten to expand 1-bit graphics
//
void I_Print8(int x, int y, char *string)
{
   int c;
   byte *source;
   uint32_t *dest;

   // CALICO: also output as a debug message
   hal_platform.debugMsg(string);
   
   g_renderer->TextureResourceSetUpdated(debugscreenrez);
   if(y > 224/8)
      return;

   dest = debugscreen + (y << 8) + x;

   while((c = *string++) && x < 32)
   {
      int i, b;
      uint32_t *d;

      if(c < 32 || c >= 128)
         continue;

      source = font8 + ((c - 32) << 3);

      d = dest;
      for(i = 0; i < 7; i++)
      {
         byte s = *source++;
         for(b = 0; b < 8; b++)
         {
            if(s & (1 << (7 - b)))
               *(d + b) = RB_COLOR_WHITE;
            else
               *(d + b) = RB_COLOR_CLEAR;
         }
         d += 256;
      }

      dest += 8;
      ++x;
   }
}

static char errormessage[80];

void I_Error(const char *error, ...) 
{
   va_list ap;

   va_start(ap, error);
   D_vsnprintf(errormessage, sizeof(errormessage), error, ap);
   va_end(ap);

   I_Print8(0, 25, errormessage);
   debugscreenactive = true;
   I_Update();
   hal_appstate.setGrabState(HAL_FALSE); // CALICO: ungrab input
   while(1)
   {
      // CALICO: run event loop
      hal_input.getEvents();
      hal_timer.delay(1);
   }
} 

//
// Called after all other subsystems have been started
//
void I_Init(void) 
{
   int i;

   palette8 = W_CacheLumpName("CRYPAL", PU_STATIC);
   
   // CALICO: reverse CRY for endianness
   for(i = 0; i < 14*256; i++)
   {
      palette8[i] = BIGSHORT(palette8[i]);
   }
} 

//
// Draw the status bar background into its backing buffer.
//
void I_DrawSbar(void)
{
   unsigned int width, height;
   sbar   = W_CacheLumpName("STBAR", PU_STATIC);
   width  = (unsigned int)(BIGSHORT(sbar->width));
   height = (unsigned int)(BIGSHORT(sbar->height));

   if(!sbarrez)
      sbarrez = g_renderer->NewTextureResource("STBAR", sbar->data, width, height, RES_8BIT, 0);

   // CALICO_TODO: network patch
#if 0
   jagobj_t     *frag;
   int           x, y;
   unsigned int *source, *dest;

   W_ReadLump(W_GetNumForName("STBAR"), sbar); // background
   if(netgame != gt_deathmatch)
      return;

   frag = W_CacheLumpName("STBARNET", PU_STATIC);

   dest   = (unsigned int *)((byte *)sbar->data + 240);
   source = (unsigned int *)frag->data;

   for(y = 0; y < 40; y++)
   {
      for(x = 0; x < 20; x++)
         dest[x] = source[x];
      source += 20;
      dest += 80;
   }

   Z_Free(frag);
#endif
}

boolean I_RefreshCompleted(void)
{
   // CALICO_FIXME: Jag-specific
#if 0
   return gpufinished;
#else
   return true;
#endif
}

boolean I_RefreshLatched(void)
{
   // CALICO_FIXME: Jag-specific
   return phasetime[3] != 0;
}

//
// Return a pointer to the wadfile.  In a cart environment this will
// just be a pointer to rom.  In a simulator, load the file from disk.
//
byte *I_WadBase(void)
{
   static byte *wadbase;

   // CALICO: once only
   if(!wadbase)
   {
      // CALICO: load from disk
      if(!(wadbase = W_LoadIWAD()))
         hal_platform.fatalError("I_WadBase: could not load IWAD file");
   }

   return wadbase;
}

/* 
==================== 
= 
= I_ZoneBase  
=
= Return the location and size of the heap for dynamic memory allocation
==================== 
*/ 
 
#define STARTHEAP 0x80000
#define ENDHEAP   0x1c8000

byte *I_ZoneBase(int *size)
{
   static byte *zonebase;
   
   *size = ENDHEAP - STARTHEAP; // leave 64k for stack

#if defined(CALICO_IS_X64)
   // allocate double heap size on x64
   *size *= 2;
#endif
   
   // CALICO: allocate from C heap
   if(!zonebase)
   {
      if(!(zonebase = calloc(*size, 1)))
         hal_platform.fatalError("I_ZoneBase: could not allocate %d bytes for zone", size);
   }

   return zonebase;
}

#define TICSCALE 2
 
//
// Read gamepad controls
//
int I_ReadControls(void) 
{ 
   static int oldticcount;
   int stoptic, i, cumulative;
   int ticcount = I_GetTime();

   // CALICO: run event loop
   joypad[ticcount&31] = hal_input.getEvents();

   stoptic = ticcount;
   if(stoptic - oldticcount > 4)
      oldticcount = stoptic - 4;
   if(oldticcount >= stoptic)
      oldticcount = stoptic - 1;
   cumulative = 0;
   for(i = oldticcount; i < stoptic; i++)
   {
      cumulative |= joypad[i&31];
   }

   oldticcount = stoptic;

   return cumulative;
} 

//
// CALICO: Get time in tics from HAL
//
int I_GetTime(void)
{
   return (int)(hal_timer.getTime());
}

//
// Perform a signed 16.16 by 16.16 mutliply
//
fixed_t FixedMul(fixed_t a, fixed_t b) 
{ 
   // CALICO: rewritten to use PC version
   return (fixed_t)((int64_t)a * b >> FRACBITS);
} 
 
//
// Perform a signed 16.16 by 16.16 divide (a/b)
//
fixed_t FixedDiv(fixed_t a, fixed_t b) 
{ 
   // CALICO: rewritten to use PC version
   return (D_abs(a) >> 14) >= D_abs(b) ? ((a ^ b) >> 31) ^ D_MAXINT :
      (fixed_t)(((int64_t)a << FRACBITS) / b);
} 
 
//=============================================================================
//
// TEXTURE MAPPING CODE
//
//=============================================================================

static uint32_t *framebuffer160_p, *framebuffer320_p;

//
// CALICO: Get the framebuffer pointers from the low-level graphics code
//
static void I_GetFramebuffer(void)
{
   g_renderer->InitFramebufferTextures();
   framebuffer160_p = g_renderer->GetFramebuffer(FB_160);
   framebuffer320_p = g_renderer->GetFramebuffer(FB_320);
}

extern pixel_t shadepixel;

struct cextender_s { signed int ext:4; }; // sign extender for 4-bit CRY chroma component
struct iextender_s { signed int ext:8; }; // sign extender for 8-bit CRY luminance component

//
// CALICO: Blend a CRY color with the value of the shadepixel color add object
//
static inline inpixel_t I_BlendCRY(inpixel_t in)
{
   static struct cextender_s c;
   static struct iextender_s i;

   int cc = (in & CRY_CMASK) >> CRY_CSHIFT;
   int cr = (in & CRY_RMASK) >> CRY_RSHIFT;
   int cy = (in & CRY_YMASK) >> CRY_YSHIFT;
   
   int sc = (c.ext = (shadepixel & CRY_CMASK) >> CRY_CSHIFT);
   int sr = (c.ext = (shadepixel & CRY_RMASK) >> CRY_RSHIFT);
   int sy = (i.ext = (shadepixel & CRY_YMASK) >> CRY_YSHIFT);

   cc += sc;
   cr += sr;
   cy += sy;

   cc = (cc < 0 ? 0 : (0x0f < cc ? 0x0f : cc));
   cr = (cr < 0 ? 0 : (0x0f < cr ? 0x0f : cr));
   cy = (cy < 0 ? 0 : (0xff < cy ? 0xff : cy));

   return (inpixel_t)((cc << CRY_CSHIFT) | (cr << CRY_RSHIFT) | (cy << CRY_YSHIFT));
}

// Sign extender for 24-bit CRY luminance values
struct yextender_s { signed int ext:24; };

// 
// Draw a vertical column of pixels from a projected wall texture.
// Source is the top of the column to scale.
// 
void I_DrawColumn(int dc_x, int dc_yl, int dc_yh, int light, fixed_t frac, 
                  fixed_t fracstep, inpixel_t *dc_source, int dc_texheight)
{ 
   static struct yextender_s s;

   int        count, heightmask;
   inpixel_t  cry;
   int32_t    y;
   uint32_t  *dest;

   count = dc_yh - dc_yl;
   if(count < 0)
      return;

#ifdef RANGECHECK
   if(dc_x < 0 || dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
      I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

   // CALICO: our destination framebuffer is 32-bit
   dest = framebuffer160_p + dc_yl * SCREENWIDTH + dc_x;

   heightmask = dc_texheight - 1;

   do
   {
      // CALICO: calculate CRY lighting and lookup RGB
      cry = dc_source[(frac >> FRACBITS) & heightmask];
      y = (cry & CRY_YMASK) << CRY_IINCSHIFT;
      y += (s.ext = light);
      if(y < 0)
         y = 0;
      y >>= CRY_IINCSHIFT;
      cry = (cry & CRY_COLORMASK) | (y & 0xff);
      
      // CALICO: apply screen shading if active
      if(shadepixel)
         cry = I_BlendCRY(cry);
 
      *dest = CRYToRGB[cry];
      dest += SCREENWIDTH;
      frac += fracstep;
   }
   while(count--);
} 

//
// CALICO: the Jag blitter could wrap around textures of arbitrary height, so
// we need to do the "tutti frutti" fix here. Carmack didn't bother fixing
// this for the NeXT "simulator" build of the game.
//
void I_DrawColumnNPO2(int dc_x, int dc_yl, int dc_yh, int light, fixed_t frac, 
                      fixed_t fracstep, inpixel_t *dc_source, int dc_texheight)
{
   static struct yextender_s s;

   int        count, heightmask;
   inpixel_t  cry;
   int32_t    y;
   uint32_t  *dest;

   count = dc_yh - dc_yl;
   if(count < 0)
      return;

#ifdef RANGECHECK
   if(dc_x < 0 || dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
      I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

   // CALICO: our destination framebuffer is 32-bit
   dest = framebuffer160_p + dc_yl * SCREENWIDTH + dc_x;

   heightmask = dc_texheight << FRACBITS;

   if(frac < 0)
      while((frac += heightmask) < 0);
   else
   {
      while(frac >= heightmask)
         frac -= heightmask;
   }

   do
   {
      cry = dc_source[frac >> FRACBITS];
      y = (cry & CRY_YMASK) << CRY_IINCSHIFT;
      y += (s.ext = light);
      y >>= CRY_IINCSHIFT;
      if(y < 0)
         y = 0;
      cry = (cry & CRY_COLORMASK) | (y & 0xff);

      // CALICO: apply screen shading if active
      if(shadepixel)
         cry = I_BlendCRY(cry);

      *dest = CRYToRGB[cry];
      dest += SCREENWIDTH;

      if((frac += fracstep) >= heightmask)
         frac -= heightmask;
   }
   while(count--);
}
 
void I_DrawSpan(int ds_y, int ds_x1, int ds_x2, int light, fixed_t ds_xfrac, 
                fixed_t ds_yfrac, fixed_t ds_xstep, fixed_t ds_ystep, 
                inpixel_t *ds_source) 
{ 
   static struct yextender_s s;

   fixed_t    xfrac, yfrac; 
   int        count; 
   inpixel_t  cry;
   int32_t    y;
   uint32_t  *dest;

#ifdef RANGECHECK 
   if(ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= SCREENWIDTH || ds_y < 0 || ds_y >= SCREENHEIGHT) 
      I_Error("R_DrawSpan: %i to %i at %i", ds_x1, ds_x2, ds_y); 
#endif 
   
   xfrac = ds_xfrac; 
   yfrac = ds_yfrac; 

   // CALICO: our destination framebuffer is 32-bit
   dest = framebuffer160_p + ds_y * SCREENWIDTH + ds_x1;
   count = ds_x2 - ds_x1;

   do 
   { 
      // CALICO: calculate CRY lighting and lookup RGB
      cry = ds_source[((yfrac >> (16 - 6)) & (63 * 64)) + ((xfrac >> 16) & 63)];
      y = (cry & CRY_YMASK) << CRY_IINCSHIFT;
      y += (s.ext = light);
      y >>= CRY_IINCSHIFT;
      if(y < 0)
         y = 0;
      cry = (cry & CRY_COLORMASK) | (y & 0xff);

      // CALICO: apply screen shading if active
      if(shadepixel)
         cry = I_BlendCRY(cry);

      *dest++ = CRYToRGB[cry];
      xfrac += ds_xstep; 
      yfrac += ds_ystep; 
   }
   while(count--); 
} 

//=============================================================================

#define GPULINE (BASEORGY+SCREENHEIGHT+1)
int lastticcount;
int lasttics;

//
// Display the current framebuffer
// If < 1/15th second has passed since the last display, busy wait.
// 15 fps is the maximum frame rate, and any faster displays will
// only look ragged.
//
// When displaying the automap, use full resolution, otherwise use
// wide pixels
//
void I_Update(void) 
{
   g_renderer->UpdateFramebuffer(FB_160);
   g_renderer->AddFramebuffer(FB_160);
   g_renderer->AddDrawCommand(sbarrez, 0, 2 + SCREENHEIGHT + 1, 320, 40);
   g_renderer->AddDrawCommand(sbartop, 0, 2 + SCREENHEIGHT + 1, 320, 40);
   if(debugscreenactive)
      g_renderer->AddDrawCommand(debugscreenrez, 0, 0, 256, 224);
   g_renderer->RenderFrame();
}

static byte tempbuffer[0x10000];

// 
// Return a pointer to a 64k or so temp work buffer for level setup uses
// (non-displayed frame buffer)
//
byte *I_TempBuffer(void)
{
   // CALICO: use a dedicated tempbuffer
   return tempbuffer;
}

//=============================================================================
//
// DOUBLE BUFFERED DRAWING FUNCTIONS
//
//=============================================================================

// double buffered display support functions

extern int cy;

//
// Set up the double buffered work screens
//
void DoubleBufferSetup(void)
{
   // set up new list
   while(!I_RefreshCompleted())
      ;

   g_renderer->ClearFramebuffer(FB_320, RB_COLOR_CLEAR);
   g_renderer->ClearTextureResource(debugscreenrez, RB_COLOR_CLEAR);

   cy = 4;
}

//
// Erase a block of pixels.
// CALICO: if destResource is non-null, the area will be cleared inside that
// TextureResource's backing buffer. Otherwise the 320x224 framebuffer is
// targeted.
//
void EraseBlock(int x, int y, int width, int height, void *destResource)
{
   uint32_t *base, *dest;

   if(x < 0)
   {
      width += x;
      x = 0;
   }
   if(y < 0)
   {
      height += y;
      y = 0;
   }
   if(x + width > 320)
      width = 320 - x;
   if(y + height > 244) // CALICO: adjusted to proper bound of 244
      height = 244 - y;

   if(width < 1 || height < 1)
      return;

   if(destResource)
   {
      base = g_renderer->GetTextureResourceStore(destResource);
      g_renderer->TextureResourceSetUpdated(destResource);
   }
   else
   {
      base = framebuffer320_p;
      g_renderer->FramebufferSetUpdated(FB_320);
      y += 8;
   }

   dest = base + y * CALICO_ORIG_SCREENWIDTH + x;
   for(; height; height--)
   {
      uint32_t *tdst = dest;
      int i = 0;
      for(; i < width; i++)
      {
         *tdst = 0;
         ++tdst;
      }

      dest += CALICO_ORIG_SCREENWIDTH;
   }
}

//
// Draw a jagobj_t graphic
//
void DrawJagobj(jagobj_t *jo, int x, int y, void *destResource)
{
   int srcx, srcy;
   int width, height;
   int rowsize;
   uint32_t *base, *dest;
   byte *source;

   width = rowsize = BIGSHORT(jo->width);
   height = BIGSHORT(jo->height);
   srcx = 0;
   srcy = 0;

   if(x < 0)
   {
      width += x;
      srcx = -x;
      x = 0;
   }
   if(y < 0)
   {
      srcy = -y;
      height += y;
      y = 0;
   }
   if(x + width > 320)
      width = 320 - x;
   if(y + height > 224) // CALICO: use a corrected bound of 224 (was 200)
      height = 224 - y;

   if(width < 1 || height < 1)
      return;

   if(destResource)
   {
      base = g_renderer->GetTextureResourceStore(destResource);
      g_renderer->TextureResourceSetUpdated(destResource);
   }
   else
   {
      base = framebuffer320_p;
      g_renderer->FramebufferSetUpdated(FB_320);
      y += 8;
   }

   source = jo->data + srcx + srcy*rowsize;

   dest = base + y * CALICO_ORIG_SCREENWIDTH + x;
   for(; height; height--)
   {
      uint32_t *tdst = dest;
      int i = 0;
      for(; i < width; i++)
      {
         if(source[i])
            *tdst = CRYToRGB[palette8[source[i]]];
         ++tdst;
      }

      source += rowsize;
      dest += CALICO_ORIG_SCREENWIDTH;
   }
}

void DoubleBufferObjList(void);

//
// Update the screen when drawing double-buffered 320x224 graphics
// CALICO: Rewritten for portability
//
void UpdateBuffer(void)
{
   DoubleBufferObjList();
   g_renderer->RenderFrame();
}

//
// Draw the title screen background
// CALICO: Separated out of DoubleBufferObjList
//
void DrawMTitle(void)
{
   static void *m_titleres;
   jagobj_t *backgroundpic;
   unsigned int width, height;
   int sx, sy, sw, sh;

   backgroundpic = W_POINTLUMPNUM(W_GetNumForName("M_TITLE"));
   width  = (unsigned int)(BIGSHORT(backgroundpic->width));
   height = (unsigned int)(BIGSHORT(backgroundpic->height));

   if(!m_titleres)
   {
      int   shift = BIGSHORT(backgroundpic->index);
      byte *data  = backgroundpic->data;

      if(!shift)
         shift = 40;

      m_titleres = g_renderer->NewTextureResource("M_TITLE", data, width, height, RES_8BIT_PACKED, shift);
   }

   // setup to center the full picture
   hal_video.getWindowSize(&sw, &sh);
   sx = (sw - (int)(hal_video.transformWidth(width))) / 2;
   sy = (sh - (int)(hal_video.transformHeight(height))) / 2;
   hal_video.transformFBCoord(sx, sy, &sx, &sy);

   if(hal_video.getAspectRatioType() == HAL_ASPECT_NOMINAL)
   {
      // use precise alignment observed in screenshots
      sx -= 5;
      sy += 10;
   }

   g_renderer->AddDrawCommand(m_titleres, sx, sy, width, height);
}

//
// Double-buffered refresh
//
void DoubleBufferObjList(void)
{
   DrawMTitle();
   g_renderer->UpdateFramebuffer(FB_320);
   g_renderer->AddFramebuffer(FB_320);
   if(debugscreenactive)
      g_renderer->AddDrawCommand(debugscreenrez, 0, 0, 256, 224);
}

//=============================================================================
//
// EEPROM
//
//=============================================================================

#define IDWORD (('D'<<8)+'1')

unsigned short eeread(int address);
int eewrite(int data, int address);

#define EEWORDS 8 // MUST BE EVEN!!!!!!

unsigned short eeprombuffer[EEWORDS];

void ClearEEProm(void)
{
   startskill  = sk_medium;
   startmap    = 1;
   sfxvolume   = 200;
   musicvolume = 128;
   controltype = 0;
   maxlevel    = 1;

   WriteEEProm();
}

void ReadEEProm(void)
{
   int i;
   unsigned short checksum;

   checksum = 12345;

   for(i = 0; i < EEWORDS; i++)
   {
      eeprombuffer[i] = eeread(i);
      if(i != EEWORDS-1)
         checksum += eeprombuffer[i];
   }

   if(checksum != eeprombuffer[EEWORDS-1])
   {
      // checksum failure, clear eeprom 
      ClearEEProm();
   }

   startskill = eeprombuffer[1];
   if(startskill > sk_nightmare)
      ClearEEProm();
   startmap = eeprombuffer[2];
   if(startmap > 26)
      ClearEEProm();
   sfxvolume = eeprombuffer[3];
   if(sfxvolume > 255)
      ClearEEProm();
   musicvolume = eeprombuffer[4];
   if(musicvolume > 255)
      ClearEEProm();
   controltype = eeprombuffer[5];
   if(controltype > 5)
      ClearEEProm();
   maxlevel = eeprombuffer[6];
   if(maxlevel > 25)
      ClearEEProm();
}

void WriteEEProm(void)
{
   int i;

   eeprombuffer[0] = (unsigned short)IDWORD;
   eeprombuffer[1] = (unsigned short)startskill;
   eeprombuffer[2] = (unsigned short)startmap;
   eeprombuffer[3] = (unsigned short)sfxvolume;
   eeprombuffer[4] = (unsigned short)musicvolume;
   eeprombuffer[5] = (unsigned short)controltype;
   eeprombuffer[6] = (unsigned short)maxlevel;

   eeprombuffer[EEWORDS-1] = 12345;

   for(i = 0; i < EEWORDS-1; i++)
      eeprombuffer[EEWORDS-1] += eeprombuffer[i];

   for(i = 0; i < EEWORDS; i++)
      eewrite(eeprombuffer[i], i);
}

//=============================================================================
//
// NETWORKING
//
//=============================================================================

// CALICO_FIXME: Jag-specific
#if 0
#define	ASICLK	(*(volatile unsigned short *)0xf10034)
#define ASIDATA	(*(volatile unsigned short *)0xf10030)
#define ASICTRL	(*(volatile unsigned short *)0xf10032)
#define ASISTAT	(*(volatile unsigned short *)0xf10032)

#define	PCLK		26593900
#define	UCLK_9600	((PCLK/(16*9600))-1)
#define	UCLK_19200	((PCLK/(16*19200))-1)
#define	UCLK_115200	((PCLK/(16*115200))-1)
#endif

// CALICO_FIXME: Jag-specific
int GetSerialChar(void)
{
#if 0
   unsigned int val;

reget:
   val = ASISTAT;
   if(val & (1<<15) )
   {
      /* error */
      ASICTRL = (1<<6);	/* serial control: clear error */
      goto reget;
   }
   if(!(val & (1<<7)))
      return -1; /* nothing available */

   val = ASIDATA;

   return val;
#else
   return -1;
#endif
}

int WaitGetSerialChar(void)
{
   int val;
   int vblstop;

   vblstop = I_GetTime() + 120;

   do
   {
      if(I_GetTime() >= vblstop)
         return -1; // timeout
      val = GetSerialChar();
   }
   while(val == -1);

   return val;
}

// CALICO_FIXME: Jag-specific
void PutSerialChar(int data)
{
#if 0
   unsigned int val;

   do
   {
      val = ASISTAT;
   }
   while(!(val & (1<<8))); /* wait for TBE */

   ASIDATA = data;
#endif
}

// CALICO: renamed I_Wait to avoid POSIX routine conflict
void I_Wait(int tics)
{
   int start;

   start = I_GetTime();
   do
   {
      junk = I_GetTime();
   } 
   while(junk < start + tics);
}

void Player0Setup(void)
{
   int val;
   int idbyte;
   int sendcount;

   sendcount = 0;
   idbyte = startmap + 24*startskill + 128*(starttype==2);

   I_Print8(1, 1, "waiting...");
   consoleplayer = 0;

   /* wait until we see a 0x22 from other side */
   do
   {
      joystick1 = hal_input.getEvents(); // CALICO

      if(joystick1 == JP_OPTION)
      {
         starttype = gt_single;
         return;    // abort
      }

      I_Wait(1);
      val = GetSerialChar();
      PrintHex(20,5,val);
      if(val == 0x22)
         return;    // ready to go

      PutSerialChar (idbyte);
      PrintHex(20, 6, idbyte);
      sendcount++;
      PrintHex(20, 7, sendcount);
   }
   while(1);
}

void Player1Setup(void)
{
   int val, oldval;
   
   // wait for two identical id bytes, then start game
   I_Print8(1, 1, "heard");
   oldval = 999;
   do
   {
      joystick1 = hal_input.getEvents(); // CALICO
      
      if(joystick1 == JP_OPTION)
      {
         starttype = gt_single;
         return;    // abort
      }
      val = GetSerialChar ();
      if(val == -1)
         continue;
      PrintHex(5,10,oldval);
      PrintHex(15,10,val);
      if(val == oldval)
         break;
      oldval = val;
   }
   while(1);

   if(val > 128)
   {
      starttype = 2;
      val -= 128;
   }
   else
      starttype = 1;

   startskill = val/24;
   val %= 24;
   startmap = val;

   // we are player 1.  send an acknowledge byte
   consoleplayer = 1;

   PutSerialChar(0x22);
   PutSerialChar(0x22);
}

void DrawSinglePlaque(jagobj_t *pl, const char *name);

int listen1, listen2;

void I_NetSetup(void)
{
   jagobj_t *pl;

   DoubleBufferSetup();

   pl = W_CacheLumpName("connect", PU_STATIC);
   DrawSinglePlaque(pl, "connect");
   Z_Free(pl);

   UpdateBuffer(); // CALICO: after plaque

   // CALICO_FIXME: Jag-specific
#if 0
   ASICLK = UCLK_115200;
   ASICTRL = (1<<6);
   ASICLK = UCLK_115200;
   ASICTRL = (1<<6);
#endif

   GetSerialChar();
   GetSerialChar();
   I_Wait(1);
   GetSerialChar();
   GetSerialChar();

   // wait a bit
   I_Wait(4);

   // if a character is allready waiting, we are player 1
   listen1 = GetSerialChar();
   listen2 = GetSerialChar();

   if(listen1 == -1 && listen2 == -1)
      Player0Setup();
   else
      Player1Setup();

   // wait a while and flush out the receive queue
   I_Wait(5);

   GetSerialChar();
   GetSerialChar();
   GetSerialChar();
   DoubleBufferSetup();
   UpdateBuffer();
}

void G_PlayerReborn(int player);

unsigned I_NetTransfer(unsigned int buttons)
{
   int  val;
   byte inbytes[6];
   byte outbytes[6];
   int  consistancy; // CALICO: truncation not handled properly in original code
   int  i;

   outbytes[0] = buttons>>24;
   outbytes[1] = buttons>>16;
   outbytes[2] = buttons>>8;
   outbytes[3] = buttons;

   consistancy = players[0].mo->x ^ players[0].mo->y ^ players[1].mo->x ^ players[1].mo->y;
   consistancy = (consistancy>>8) ^ consistancy ^ (consistancy>>16);

   outbytes[4] = (byte)(consistancy & 0xff);
   outbytes[5] = vblsinframe;

   if(consoleplayer)
   {
      // player 1 waits before sending
      for(i = 0; i <= 5; i++)
      {
         val = WaitGetSerialChar();
         if (val == -1)
            goto reconnect;

         inbytes[i] = val;
         PutSerialChar (outbytes[i]);
      }
      vblsinframe = inbytes[5]; // take gamevbls from other player
   }
   else
   {
      // player 0 sends first
      for(i = 0; i <= 5; i++)
      {
         PutSerialChar(outbytes[i]);
         val = WaitGetSerialChar();
         if(val == -1)
            goto reconnect;
         inbytes[i] = val;
      }
   }

   // check for consistancy error
   if(inbytes[4] != outbytes[4])
   {
      jagobj_t *pl;

      S_Clear();
      pl = W_CacheLumpName("neterror", PU_STATIC);	
      DrawPlaque(pl, "neterror");
      Z_Free (pl);

      I_Wait(200);
      goto reconnect;
   }

   val = (inbytes[0]<<24) + (inbytes[1]<<16) + (inbytes[2]<<8) + inbytes[3];

   return val;

   // reconnect
reconnect:
   S_Clear();

   if(consoleplayer)
      I_Wait(15); // let player 0 wait again

   I_NetSetup();
   if(starttype == gt_single)
      Jag68k_main(myargc, myargv);

   G_PlayerReborn(0);
   G_PlayerReborn(1);

   gameaction = ga_warped;
   ticbuttons[0] = ticbuttons[1] = oldticbuttons[0] = oldticbuttons[1] = 0;
   return 0;
}

// EOF

