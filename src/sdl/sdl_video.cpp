/*
  CALICO
  
  SDL 2 Video
  
  The MIT License (MIT)
  
  Copyright (c) 2016 James Haley
  
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

#include "SDL.h"
#include "SDL_syswm.h"

#include "sdl_video.h"
#include "../elib/configfile.h"
#include "../hal/hal_types.h"
#include "../hal/hal_input.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "../rb/rb_main.h"
#include "../rb/valloc.h"
#include "../renderintr/ri_interface.h"
#include "../gl/gl_render.h"
#include "../gl4/gl4_render.h"

//=============================================================================
//
// Globals
// 

// Main window object
SDL_Window    *mainwindow;

// GL context
SDL_GLContext  glcontext;

//=============================================================================
//
// Configuration options
//

enum renderer_e
{
    RENDERER_GL1_1, // GL 1.1 renderer
    RENDERER_GL4,   // GL 4 renderer

    RENDERER_MIN = RENDERER_GL1_1,
    RENDERER_MAX = RENDERER_GL4
};

static int screenwidth     = CALICO_ORIG_SCREENWIDTH;
static int screenheight    = CALICO_ORIG_SCREENHEIGHT;
static int fullscreen      = 0;
static int monitornum      = 0;
static hal_aspect_t aspect = HAL_ASPECT_NOMINAL;
static int aspectNum       = 4;
static int aspectDenom     = 3;
static int renderer        = RENDERER_GL1_1;

static cfgrange_t<int> swRange = { 320, 32768 };
static cfgrange_t<int> shRange = { 224, 32768 };
static cfgrange_t<int> fsRange = { -1,  1     };
static cfgrange_t<int> anRange = {  3,  100   };
static cfgrange_t<int> adRange = {  2,  100   };

static cfgrange_t<int> rnRange = { RENDERER_MIN, RENDERER_MAX };

static CfgItem cfgScreenWidth  { "screenwidth",  &screenwidth,  &swRange };
static CfgItem cfgScreenHeight { "screenheight", &screenheight, &shRange };
static CfgItem cfgFullScreen   { "fullscreen",   &fullscreen,   &fsRange };
static CfgItem cfgMonitorNum   { "monitornum",   &monitornum };
static CfgItem cfgAspectNum    { "aspectnum",    &aspectNum,    &anRange };
static CfgItem cfgAspectDenom  { "aspectdenom",  &aspectDenom,  &adRange };
static CfgItem cfgRenderer     { "renderer",     &renderer,     &rnRange };

//=============================================================================
//
// Current state
//

// window geometry
static int curscreenwidth;
static int curscreenheight;
static int curfullscreen;
static int curmonitornum;

// 4:3 subscreen
static rbScissor_t subscreen;

// scale factors
static float screenxscale;
static float screenyscale;
static float screenxiscale;
static float screenyiscale;

//=============================================================================
//
// Mode setting
//

//
// Select the proper renderer
//
static void SDL2_setRenderer()
{
    switch(renderer)
    {
    case RENDERER_GL1_1:
        GL_SelectRenderer();
        break;
    case RENDERER_GL4:
        GL4_SelectRenderer();
        break;
    default:
        hal_platform.fatalError("Unknown value for renderer (%d)", renderer);
    }
    g_renderer->InitRenderer(curscreenwidth, curscreenheight);
}

//
// Calculate the game area's subrect on the framebuffer
//
static void SDL2_calcSubRect()
{
   const rbfixed aspectRatio    = curscreenwidth * RBFRACUNIT / curscreenheight;
   const rbfixed nomAspectRatio = aspectNum * RBFRACUNIT / aspectDenom;

   if(aspectRatio == nomAspectRatio) // nominal
   {
      subscreen.width  = curscreenwidth;
      subscreen.height = curscreenheight;
      subscreen.x      = 0;
      subscreen.y      = 0;
      aspect           = HAL_ASPECT_NOMINAL;
   }
   else if(aspectRatio > nomAspectRatio) // widescreen
   {
      subscreen.width  = curscreenheight * aspectNum / aspectDenom;
      subscreen.height = curscreenheight;
      subscreen.x      = (curscreenwidth - subscreen.width) / 2;
      subscreen.y      = 0;
      aspect           = HAL_ASPECT_WIDE;
   }
   else // narrow
   {
      subscreen.width  = curscreenwidth;
      subscreen.height = curscreenwidth * aspectDenom / aspectNum;
      subscreen.x      = 0;
      subscreen.y      = (curscreenheight - subscreen.height) / 2;
      aspect           = HAL_ASPECT_NARROW;
   }

   screenxscale  = float(subscreen.width ) / CALICO_ORIG_SCREENWIDTH;
   screenyscale  = float(subscreen.height) / CALICO_ORIG_SCREENHEIGHT;
   screenxiscale = 1.0f / screenxscale;
   screenyiscale = 1.0f / screenyscale;
}

//
// Set video mode
//
hal_bool SDL2_SetVideoMode(int width, int height, int fs, int mnum)
{
   if(glcontext)
   {
      SDL_GL_DeleteContext(glcontext);
      glcontext = nullptr;
   }
   if(mainwindow)
   {
      SDL_DestroyWindow(mainwindow);
      mainwindow = nullptr;
   }

   Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_SHOWN;
   int    x     = SDL_WINDOWPOS_CENTERED_DISPLAY(mnum);
   int    y     = SDL_WINDOWPOS_CENTERED_DISPLAY(mnum);

   if(fs == 1)
      flags |= SDL_WINDOW_FULLSCREEN;

   // if dimensions are zero, then use fullscreen desktop
   if(!(width && height) || fs == -1)
   {
      x = SDL_WINDOWPOS_UNDEFINED_DISPLAY(mnum);
      y = SDL_WINDOWPOS_UNDEFINED_DISPLAY(mnum);
      flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
      fs = -1;
   }

   // set GL attributes
   SDL_GL_SetAttribute(SDL_GL_RED_SIZE,      8);
   SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,    8);
   SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,     8);
   SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,    8);
   SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,  32);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,  1);

   if(renderer == RENDERER_GL4)
   {
       SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
       SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
       SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
   }

   if(!(mainwindow = SDL_CreateWindow("Calico", x, y, width, height, flags)))
   {
      // Try resetting previous mode
      if(width != curscreenwidth || height != curscreenheight || fs != curfullscreen ||
         mnum != curmonitornum)
      {
         return SDL2_SetVideoMode(curscreenwidth, curscreenheight, curfullscreen, curmonitornum);
      }
      else
      {
         hal_platform.fatalError("Failed to set video mode %dx%d (%s)", width, height, 
                                 SDL_GetError());
      }
   }

   // set window icon
   hal_platform.setIcon();

   // create GL context
   if(!(glcontext = SDL_GL_CreateContext(mainwindow)))
   {
      hal_platform.fatalError("Failed to create GL context (%s)", SDL_GetError());
   }

   // remember set state
   SDL_GetWindowSize(mainwindow, &curscreenwidth, &curscreenheight);
   curfullscreen = fs;
   curmonitornum = mnum;

   // calculate game subscreen geometry
   SDL2_calcSubRect();

   // make current and set swap
   SDL_GL_MakeCurrent(mainwindow, glcontext);
   SDL2_ToggleGLSwap(HAL_TRUE);

   // wake up RB system
   RB_InitDefaultState(renderer == RENDERER_GL4 ? 4 : 1);

   // update appstate maintenance
   hal_appstate.updateFocus();
   hal_appstate.updateGrab();

   return HAL_TRUE;
}

//
// Remember the current video mode in the configuration file
//
static void SDL2_saveVideoMode()
{
   screenwidth  = curscreenwidth;
   screenheight = curscreenheight;
   fullscreen   = curfullscreen;
   monitornum   = curmonitornum;
}

//
// Set initial video mode at startup using configuration file settings
//
void SDL2_InitVideo()
{
   SDL2_SetVideoMode(screenwidth, screenheight, fullscreen, monitornum);
   SDL2_saveVideoMode(); // remember settings

   // initialize renderer
   SDL2_setRenderer();
}

//
// Change video mode at runtime
//
hal_bool SDL2_SetNewVideoMode(int w, int h, int fs, int mnum)
{
   hal_bool res;

   if((res = SDL2_SetVideoMode(w, h, fs, mnum)))
      SDL2_saveVideoMode(); // remember settings

   // notify all registered game objects and subsystems of the change
   VAllocItem::ModeChanging();

   // initialize renderer
   SDL2_setRenderer();

   return res;
}

//=============================================================================
//
// Coordinate transforms
//

//
// Transform a framebuffer or client window coordinate into the game's 
// 640x480 subscreen space.
//
void SDL2_TransformFBCoord(int x, int y, int *tx, int *ty)
{
   *tx = int((x - subscreen.x) * screenxiscale);
   *ty = int((y - subscreen.y) * screenyiscale);
}

//
// Transform a coordinate in the game's 640x480 subscreen space into
// framebuffer space.
//
void SDL2_TransformGameCoord2i(int x, int y, int *tx, int *ty)
{
   *tx = int(x * screenxscale + subscreen.x);
   *ty = int(y * screenyscale + subscreen.y);
}

//
// Float version of TransformGameCoord
//
void SDL2_TransformGameCoord2f(int x, int y, float *tx, float *ty)
{
   *tx = x * screenxscale + subscreen.x;
   *ty = y * screenyscale + subscreen.y;
}

//
// Scale up a width value
//
unsigned int SDL2_TransformWidth(unsigned int w)
{
   return static_cast<unsigned int>(w * screenxscale);
}

//
// Scale up a height value
//
unsigned int SDL2_TransformHeight(unsigned int h)
{
   return static_cast<unsigned int>(h * screenyscale);
}

//=============================================================================
//
// State retrieval
//

//
// Get the size of the video window
//
void SDL2_GetWindowSize(int *width, int *height)
{
   if(mainwindow)
      SDL_GetWindowSize(mainwindow, width, height);
}

//
// Check if application is running in fullscreen
//
int SDL2_IsFullScreen()
{
   return curfullscreen;
}

//
// Get current display number
//
int SDL2_GetCurrentDisplay()
{
   return curmonitornum;
}

//
// Get main window's state flags
//
unsigned int SDL2_GetWindowFlags()
{
   return (mainwindow ? SDL_GetWindowFlags(mainwindow) : 0);
}

//
// Toggle mouse pointer grabbing
//
void SDL2_SetGrab(hal_bool grab)
{
   if(mainwindow)
      SDL_SetWindowGrab(mainwindow, grab ? SDL_TRUE : SDL_FALSE);
}

//
// Warp mouse
//
void SDL2_WarpMouse(int x, int y)
{
   if(mainwindow)
      SDL_WarpMouseInWindow(mainwindow, x, y);
}

//
// Get a GL function pointer
//
void *SDL2_GetGLProcAddress(const char *proc)
{
   return SDL_GL_GetProcAddress(proc);
}

//
// Toggle swapping
//
int SDL2_ToggleGLSwap(hal_bool swap)
{
   static hal_bool swapState;

   if(swap == swapState)
      return 0;
   swapState = swap;

   return SDL_GL_SetSwapInterval(!!swap);
}

//
// End frame and swap buffers
//
void SDL2_EndFrame()
{
   if(mainwindow)
      SDL_GL_SwapWindow(mainwindow);
}

//
// Get platform-specific window handle
//
void *SDL2_GetWindowHandle()
{
#ifdef _WIN32
   SDL_SysWMinfo info;
   SDL_VERSION(&info.version);

   if(mainwindow && SDL_GetWindowWMInfo(mainwindow, &info))
      return info.info.win.window;
#endif

   return nullptr;
}

//
// Get aspect ratio type
//
hal_aspect_t SDL2_GetAspectRatioType()
{
   return aspect;
}

//
// Get location and size of subscreen.
//
void SDL2_GetSubscreenExtents(int *x, int *y, int *w, int *h)
{
   if(x)
      *x = subscreen.x;
   if(y)
      *y = subscreen.y;
   if(w)
      *w = subscreen.width;
   if(h)
      *h = subscreen.height;
}

#endif

// EOF

