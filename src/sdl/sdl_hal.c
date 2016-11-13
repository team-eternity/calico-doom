/*
  CALICO
  
  SDL 2 HAL setup
  
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
#include "../hal/hal_types.h"
#include "../hal/hal_input.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_video.h"
#include "sdl_init.h"
#include "sdl_input.h"
#include "sdl_video.h"

//
// Initialize the HAL layer with SDL2 implementation function pointers
//
void SDL2_InitHAL(void)
{
   // Basic interface
   hal_medialayer.init      = SDL2_Init;
   hal_medialayer.exit      = SDL2_Exit;
   hal_medialayer.error     = SDL2_Error;
   hal_medialayer.msgbox    = SDL2_MsgBox;
   hal_medialayer.isExiting = SDL2_IsExiting;

   // Video functions
   hal_video.initVideo            = SDL2_InitVideo;
   hal_video.setNewVideoMode      = SDL2_SetNewVideoMode;
   hal_video.getWindowSize        = SDL2_GetWindowSize;
   hal_video.getGLProcAddress     = SDL2_GetGLProcAddress;
   hal_video.transformFBCoord     = SDL2_TransformFBCoord;
   hal_video.transformGameCoord2i = SDL2_TransformGameCoord2i;
   hal_video.transformGameCoord2f = SDL2_TransformGameCoord2f;
   hal_video.transformWidth       = SDL2_TransformWidth;
   hal_video.transformHeight      = SDL2_TransformHeight;
   hal_video.toggleGLSwap         = SDL2_ToggleGLSwap;
   hal_video.isFullScreen         = SDL2_IsFullScreen;
   hal_video.getCurrentDisplay    = SDL2_GetCurrentDisplay;
   hal_video.getWindowFlags       = SDL2_GetWindowFlags;
   hal_video.setGrab              = SDL2_SetGrab;
   hal_video.warpMouse            = SDL2_WarpMouse;

   // App state maintenance
   hal_appstate.mouseShouldBeGrabbed = SDL2_MouseShouldBeGrabbed;
   hal_appstate.updateGrab           = SDL2_UpdateGrab;
   hal_appstate.updateFocus          = SDL2_UpdateFocus;

   // Input
   hal_input.getEvents = SDL2_GetEvents;
}

#endif

// EOF

