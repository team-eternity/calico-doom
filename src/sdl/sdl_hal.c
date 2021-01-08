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
#include "../hal/hal_sfx.h"
#include "../hal/hal_timer.h"
#include "../hal/hal_video.h"
#include "sdl_init.h"
#include "sdl_input.h"
#include "sdl_sound.h"
#include "sdl_timer.h"
#include "sdl_video.h"

//
// Initialize the HAL layer with SDL2 implementation function pointers
//
void SDL2_InitHAL(void)
{
   // Basic interface
   hal_medialayer.init              = SDL2_Init;
   hal_medialayer.exit              = SDL2_Exit;
   hal_medialayer.error             = SDL2_Error;
   hal_medialayer.msgbox            = SDL2_MsgBox;
   hal_medialayer.isExiting         = SDL2_IsExiting;
   hal_medialayer.getBaseDirectory  = SDL2_GetBaseDirectory;
   hal_medialayer.getWriteDirectory = SDL2_GetWriteDirectory;

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
   hal_video.endFrame             = SDL2_EndFrame;
   hal_video.isFullScreen         = SDL2_IsFullScreen;
   hal_video.getCurrentDisplay    = SDL2_GetCurrentDisplay;
   hal_video.getWindowFlags       = SDL2_GetWindowFlags;
   hal_video.setGrab              = SDL2_SetGrab;
   hal_video.warpMouse            = SDL2_WarpMouse;
   hal_video.getWindowHandle      = SDL2_GetWindowHandle;
   hal_video.getAspectRatioType   = SDL2_GetAspectRatioType;
   hal_video.getSubscreenExtents  = SDL2_GetSubscreenExtents;

   // App state maintenance
   hal_appstate.mouseShouldBeGrabbed = SDL2_MouseShouldBeGrabbed;
   hal_appstate.updateGrab           = SDL2_UpdateGrab;
   hal_appstate.updateFocus          = SDL2_UpdateFocus;
   hal_appstate.setGrabState         = SDL2_SetGrabState;

   // Input
   hal_input.initInput      = SDL2_InitInput;
   hal_input.getEvents      = SDL2_GetEvents;
   hal_input.resetInput     = SDL2_ResetInput;
   hal_input.getMouseMotion = SDL2_GetMouseMotion;

   // Sound
   hal_sound.initSound       = SDL2Sfx_MixerInit;
   hal_sound.isInit          = SDL2Sfx_IsInit;
   hal_sound.startSound      = SDL2Sfx_StartSound;
   hal_sound.stopSound       = SDL2Sfx_StopSound;
   hal_sound.isSamplePlaying = SDL2Sfx_IsSamplePlaying;
   hal_sound.isSampleAtStart = SDL2Sfx_IsSampleAtStart;
   hal_sound.stopAllChannels = SDL2Sfx_StopAllChannels;
   hal_sound.updateEQParams  = SDL2Sfx_UpdateEQParams;
   hal_sound.getSampleRate   = SDL2Sfx_GetSampleRate;

   // Timer
   hal_timer.delay     = SDL2_Delay;
   hal_timer.getTime   = SDL2_GetTime;
   hal_timer.getTimeMS = SDL2_GetTimeMS;
}

#endif

// EOF

