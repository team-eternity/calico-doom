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
#include "../elib/elib.h"
#include "../elib/configfile.h"
#include "../hal/hal_types.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_video.h"
#include "../jagpad.h"
#include "sdl_input.h"

//=============================================================================
//
// App State Maintenance
//

static hal_bool windowFocused;
static hal_bool screenVisible;
static hal_bool shouldGrabInput;

//
// Set application input grabbing state
//
void SDL2_SetGrabState(hal_bool state)
{
   shouldGrabInput = state;
}

//
// Determine whether or not mouse should be grabbed
//
hal_bool SDL2_MouseShouldBeGrabbed(void)
{
   return (hal_bool)(windowFocused && shouldGrabInput);
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
// Keyboard code
//

enum kbjoykeys_e
{
   KBJK_A,
   KBJK_B,
   KBJK_C,
   KBJK_UP,
   KBJK_DOWN,
   KBJK_LEFT,
   KBJK_RIGHT,
   KBJK_OPTION,
   KBJK_PAUSE,
   KBJK_NUM,
   KBJK_STAR,
   KBJK_0,
   KBJK_1,
   KBJK_2,
   KBJK_3,
   KBJK_4,
   KBJK_5,
   KBJK_6,
   KBJK_7,
   KBJK_8,
   KBJK_9,
   KBJK_MAX
};

static char *kbKeyNames[KBJK_MAX];
static SDL_Keycode kbKeyCodes[KBJK_MAX] =
{
   SDLK_RSHIFT,
   SDLK_RCTRL,
   SDLK_RALT,
   SDLK_UP,
   SDLK_DOWN,
   SDLK_LEFT,
   SDLK_RIGHT,
   SDLK_ESCAPE,
   SDLK_PAUSE,
   SDLK_KP_DIVIDE,
   SDLK_KP_MULTIPLY,
   SDLK_0,
   SDLK_1,
   SDLK_2,
   SDLK_3,
   SDLK_4,
   SDLK_5,
   SDLK_6,
   SDLK_7,
   SDLK_8,
   SDLK_9
};
static bool kbKeyDown[KBJK_MAX];
static int kbKeyToJagButton[KBJK_MAX] =
{
   JP_A,
   JP_B,
   JP_C,
   JP_UP,
   JP_DOWN,
   JP_LEFT,
   JP_RIGHT,
   JP_OPTION,
   JP_PAUSE,
   JP_NUM,
   JP_STAR,
   JP_0,
   JP_1,
   JP_2,
   JP_3,
   JP_4,
   JP_5,
   JP_6,
   JP_7,
   JP_8,
   JP_9
};

static CfgItem cfgKeyNameA("kb_key_a",      &kbKeyNames[KBJK_A]);
static CfgItem cfgKeyNameB("kb_key_b",      &kbKeyNames[KBJK_B]);
static CfgItem cfgKeyNameC("kb_key_c",      &kbKeyNames[KBJK_C]);
static CfgItem cfgKeyNameU("kb_key_up",     &kbKeyNames[KBJK_UP]);
static CfgItem cfgKeyNameD("kb_key_down",   &kbKeyNames[KBJK_DOWN]);
static CfgItem cfgKeyNameL("kb_key_left",   &kbKeyNames[KBJK_LEFT]);
static CfgItem cfgKeyNameR("kb_key_right",  &kbKeyNames[KBJK_RIGHT]);
static CfgItem cfgKeyNameO("kb_key_option", &kbKeyNames[KBJK_OPTION]);
static CfgItem cfgKeyNameP("kb_key_pause",  &kbKeyNames[KBJK_PAUSE]);
static CfgItem cfgKeyNameN("kb_key_num",    &kbKeyNames[KBJK_NUM]);
static CfgItem cfgKeyNameS("kb_key_star",   &kbKeyNames[KBJK_STAR]);
static CfgItem cfgKeyName0("kb_key_0",      &kbKeyNames[KBJK_0]);
static CfgItem cfgKeyName1("kb_key_1",      &kbKeyNames[KBJK_1]);
static CfgItem cfgKeyName2("kb_key_2",      &kbKeyNames[KBJK_2]);
static CfgItem cfgKeyName3("kb_key_3",      &kbKeyNames[KBJK_3]);
static CfgItem cfgKeyName4("kb_key_4",      &kbKeyNames[KBJK_4]);
static CfgItem cfgKeyName5("kb_key_5",      &kbKeyNames[KBJK_5]);
static CfgItem cfgKeyName6("kb_key_6",      &kbKeyNames[KBJK_6]);
static CfgItem cfgKeyName7("kb_key_7",      &kbKeyNames[KBJK_7]);
static CfgItem cfgKeyName8("kb_key_8",      &kbKeyNames[KBJK_8]);
static CfgItem cfgKeyName9("kk_key_9",      &kbKeyNames[KBJK_9]);

//#define IS_CTRL(code)  (code == SDLK_LCTRL  || code == SDLK_RCTRL )
//#define IS_ALT(code)   (code == SDLK_LALT   || code == SDLK_RALT  )
//#define IS_SHIFT(code) (code == SDLK_LSHIFT || code == SDLK_RSHIFT)

/*
static void SDL2_keyEvent(SDL_Keycode code, bool isDown)
{
   for(size_t i = 0; i < KBJK_MAX; i++)
   {
      if(kbKeyCodes[i] == code ||
         (IS_CTRL(code)  && IS_CTRL(kbKeyCodes[i]))  ||
         (IS_ALT(code)   && IS_ALT(kbKeyCodes[i]))   ||
         (IS_SHIFT(code) && IS_SHIFT(kbKeyCodes[i])))
      {
         kbKeyDown[i] = isDown;
      }
   }
}
*/

#define KEYISCTRL(code)  (code == SDLK_LCTRL  || code == SDLK_RCTRL )
#define KEYISALT(code)   (code == SDLK_LALT   || code == SDLK_RALT  )
#define KEYISSHIFT(code) (code == SDLK_LSHIFT || code == SDLK_RSHIFT)

//
// Handle key down events
//
static void SDL2_processKeyboard()
{
   const Uint8 *state = SDL_GetKeyboardState(nullptr);
   const Uint8 *state1, *state2;

   for(size_t i = 0; i < KBJK_MAX; i++)
   {
      SDL_Scancode sc = SDL_GetScancodeFromKey(kbKeyCodes[i]);
      if(sc == SDL_SCANCODE_LCTRL || sc == SDL_SCANCODE_RCTRL)
      {
         state1 = &state[SDL_SCANCODE_LCTRL];
         state2 = &state[SDL_SCANCODE_RCTRL];
      }
      else if(sc == SDL_SCANCODE_LSHIFT || sc == SDL_SCANCODE_RSHIFT)
      {
         state1 = &state[SDL_SCANCODE_LSHIFT];
         state2 = &state[SDL_SCANCODE_RSHIFT];
      }
      else if(sc == SDL_SCANCODE_LALT || sc == SDL_SCANCODE_RALT)
      {
         state1 = &state[SDL_SCANCODE_LALT];
         state2 = &state[SDL_SCANCODE_RALT];
      }
      else
      {
         state1 = &state[sc];
         state2 = nullptr;
      }

      kbKeyDown[i] = !!*state1 || (state2 ? !!*state2 : false);
   }
}

//=============================================================================
//
// Main Interface
//

//
// Initialize input subsystem
//
void SDL2_InitInput(void)
{
   // initialize keyboard key bindings
   for(size_t i = 0; i < KBJK_MAX; i++)
   {
      if(estrempty(kbKeyNames[i]))
         kbKeyNames[i] = estrdup(SDL_GetKeyName(kbKeyCodes[i]));
      else
         kbKeyCodes[i] = SDL_GetKeyFromName(kbKeyNames[i]);
   }
}

//
// Primary input processing function
//
int SDL2_GetEvents(void)
{
   SDL_Event evt;

   SDL2_UpdateFocus();
   SDL2_UpdateGrab();

   if(!windowFocused)
   {
      // make sure keys aren't stuck
      SDL2_ResetInput();
   }

   while(SDL_PollEvent(&evt))
   {
      switch(evt.type)
      {
         /*
      case SDL_KEYDOWN:
      case SDL_KEYUP:
         SDL2_keyEvent(evt.key.keysym.sym, evt.type == SDL_KEYDOWN);
         break;
         */
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

   SDL2_processKeyboard();

   int buttons = 0;

   for(size_t i = 0; i < KBJK_MAX; i++)
   {
      if(kbKeyDown[i])
         buttons |= kbKeyToJagButton[i];
   }

   return buttons;
}

//
// Reset input states
//
void SDL2_ResetInput(void)
{
   for(size_t i = 0; i < KBJK_MAX; i++)
      kbKeyDown[i] = false;
}

#endif

// EOF

