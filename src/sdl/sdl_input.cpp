/*
  CALICO
  
  SDL2 Input Implementation
  
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
#include "../elib/configfile.h"
#include "../hal/hal_types.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_video.h"
#include "sdl_input.h"

//=============================================================================
//
// App State Maintenance
//

static hal_bool windowFocused;
static hal_bool screenVisible;

//
// Determine whether or not mouse should be grabbed
//
hal_bool SDL2_MouseShouldBeGrabbed(void)
{
   // CALICO_TODO: invoke a game callback to get game status;
   // ie., don't grab when playing a demo, paused, etc.
   return windowFocused;
}

//
// Update input grabbing status
//
void SDL2_UpdateGrab(void)
{
   static hal_bool currentlyGrabbed = HAL_FALSE;
   hal_bool grab;

   grab = SDL2_MouseShouldBeGrabbed();

   if(grab && !currentlyGrabbed)
   {
      SDL_ShowCursor(SDL_DISABLE);
      hal_video.setGrab(HAL_TRUE);
   }

   if(!grab && currentlyGrabbed)
   {
      SDL_ShowCursor(SDL_ENABLE);
      hal_video.setGrab(HAL_FALSE);
   }

   currentlyGrabbed = grab;
}

//
// Update window visibility and input focus status
//
void SDL2_UpdateFocus(void)
{
   unsigned int state;
   hal_bool active, focused;

   SDL_PumpEvents();

   state   = hal_video.getWindowFlags();
   active  = (hal_bool)((state & SDL_WINDOW_SHOWN) && !(state & SDL_WINDOW_MINIMIZED));
   focused = (hal_bool)((state & (SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS)) != 0);

   screenVisible = active;
   windowFocused = (hal_bool)(active && focused);
}

//=============================================================================
//
// Mouse code
//

//
// Config
//
static double gamepadSensitivityX = 1.0; // multiplier for gamepad X axis motion
static double gamepadSensitivityY = 1.0; // multiplier for gamepad Y axis motion

static CfgItem cfgGamepadSensitivityX("gamepad_sens_x", &gamepadSensitivityX);
static CfgItem cfgGamepadSensitivityY("gamepad_sens_y", &gamepadSensitivityY);

void SDL2_GetRawMousePos(int *x, int *y)
{
   SDL_GetMouseState(x, y);
}

//
// Get mouse position translated into game coordinates
//
void SDL2_GetTxMousePos(int *x, int *y)
{
   SDL_GetMouseState(x, y);
   hal_video.transformFBCoord(*x, *y, x, y);
}

//
// Warp the mouse to a position in game coordinates
//
void SDL2_WarpTxMousePos(int x, int y)
{
   int scrx, scry;
   hal_video.transformGameCoord2i(x, y, &scrx, &scry);
   hal_video.warpMouse(scrx, scry);
}

//=============================================================================
//
// Main Interface
//

//
// Primary input processing function
//
void SDL2_GetEvents(void)
{
   SDL_Event evt;

   SDL2_UpdateFocus();
   SDL2_UpdateGrab();

   if(!windowFocused)
   {
      // CALICO_TODO: make sure keys aren't stuck
   }

   while(SDL_PollEvent(&evt))
   {
      switch(evt.type)
      {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
         // CALICO_TODO: keyboard
         break;
      case SDL_MOUSEMOTION:
         // CALICO_TODO: mouse motion
         break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
         // CALICO_TODO: mouse buttons
         break;
      case SDL_MOUSEWHEEL:
         // CALICO_TODO: mouse wheel
         break;
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP:
         // CALICO_TODO: game controller buttons
         break;
      case SDL_QUIT:
         // CALICO_TODO: notify game loop?
         hal_medialayer.exit();
         break;
      case SDL_WINDOWEVENT:
         if(evt.window.event != SDL_WINDOWEVENT_CLOSE)
         {
            // update all app status when any window event occurs
            SDL2_UpdateFocus();
            SDL2_UpdateGrab();
         }
         break;
      default:
         break;
      }
   }
}

#endif

// EOF

