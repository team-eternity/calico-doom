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
#include "../hal/hal_input.h"
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
   hal_bool gameWants = HAL_TRUE;
   
   // get game's opinion if available
   if(hal_appstate.gameGrabCallback)
      gameWants = hal_appstate.gameGrabCallback();
   
   return (hal_bool)(windowFocused && shouldGrabInput && gameWants);
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
   KBJK_STRAFE,
   KBJK_SLEFT,
   KBJK_SRIGHT,
   KBJK_USE,
   KBJK_PWEAPN,
   KBJK_NWEAPN,
   KBJK_ATTACK,
   KBJK_SPEED,
   KBJK_MAX
};

static char *kbJoyKeyJagNames[KBJK_MAX] =
{
   "a",
   "b",
   "c",
   "up",
   "down",
   "left",
   "right",
   "option",
   "pause",
   "num",
   "star",
   "0",
   "1",
   "2",
   "3",
   "4",
   "5",
   "6",
   "7",
   "8",
   "9",
   "strafeon",
   "strafeleft",
   "straferight",
   "use",
   "prevweapon",
   "nextweapon",
   "attack",
   "speed"
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
   SDLK_9,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN
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
   JP_9,
   JP_STRAFE,
   JP_SLEFT,
   JP_SRIGHT,
   JP_USE,
   JP_PWEAPN,
   JP_NWEAPN,
   JP_ATTACK,
   JP_SPEED
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
static CfgItem cfgKeyName9("kb_key_9",      &kbKeyNames[KBJK_9]);

// CALICO: custom inputs for enhanced control
static CfgItem cfgKeyNameStrafeOn   ("kb_key_strafeon",    &kbKeyNames[KBJK_STRAFE]);
static CfgItem cfgKeyNameStrafeLeft ("kb_key_strafeleft",  &kbKeyNames[KBJK_SLEFT ]);
static CfgItem cfgKeyNameStrafeRight("kb_key_straferight", &kbKeyNames[KBJK_SRIGHT]);
static CfgItem cfgKeyNameUse        ("kb_key_use",         &kbKeyNames[KBJK_USE   ]);
static CfgItem cfgKeyNamePrevWeapon ("kb_key_prevweapon",  &kbKeyNames[KBJK_PWEAPN]);
static CfgItem cfgKeyNameNextWeapon ("kb_key_nextweapon",  &kbKeyNames[KBJK_NWEAPN]);
static CfgItem cfgKeyNameAttack     ("kb_key_attack",      &kbKeyNames[KBJK_ATTACK]);
static CfgItem cfgKeyNameSpeed      ("kb_key_speed",       &kbKeyNames[KBJK_SPEED ]);

//
// Handle key down events
//
static void SDL2_processKeyboard()
{
   const Uint8 *state = SDL_GetKeyboardState(nullptr);
   const Uint8 *state1, *state2;

   for(size_t i = 0; i < KBJK_MAX; i++)
   {
      if(kbKeyCodes[i] == SDLK_UNKNOWN) // unbound?
         continue;

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
// Gamepad code
//

//
// Config
//

enum gptriggeraction_e
{
   GPT_NONE,
   GPT_LSTRAFE,
   GPT_RSTRAFE,
   GPT_PREVWEAPON,
   GPT_NEXTWEAPON,
   GPT_ATTACK,
   GPT_USE,
   GPT_MAX
};

static bool  useGamepad;
static char *gamepadDevice;
static bool  gamepadInvertY;
static int   gamepadThreshold = 8000;
static int   gamepadTriggerThreshold = 3000;
static int   gamepadLTriggerAction = GPT_ATTACK;
static int   gamepadRTriggerAction = GPT_NONE;

static cfgrange_t<int> triggerRange = { GPT_NONE, GPT_MAX - 1 };

static CfgItem cfgUseGamepad      ("use_gamepad",              &useGamepad             );
static CfgItem cfgGamepad         ("gamepad",                  &gamepadDevice          );
static CfgItem cfgGamepadInvertY  ("gamepad_inverty",          &gamepadInvertY         );
static CfgItem cfgGamepadThreshold("gamepad_xythreshold",      &gamepadThreshold       );
static CfgItem cfgGamepadTThresh  ("gamepad_triggerthreshold", &gamepadTriggerThreshold);

static CfgItem cfgGamepadLTrigger("gamepad_ltrigger", &gamepadLTriggerAction, &triggerRange);
static CfgItem cfgGamepadRTrigger("gamepad_rtrigger", &gamepadRTriggerAction, &triggerRange);

static SDL_GameController *pCurController;

struct gpbutton_t
{
   SDL_GameControllerButton  button;   // SDL controller button for this slot
   int                       kbButton; // KBJ button to which the SDL controller button is assigned
   char                     *name;     // name of Jag controller button for config
   bool                      down;     // if true, button is down
};

static gpbutton_t sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_MAX] =
{
   { SDL_CONTROLLER_BUTTON_A,             KBJK_USE,    nullptr, false }, // default: use
   { SDL_CONTROLLER_BUTTON_B,             KBJK_SPEED,  nullptr, false }, // default: speed-on
   { SDL_CONTROLLER_BUTTON_X,             KBJK_STRAFE, nullptr, false }, // default: strafe-on
   { SDL_CONTROLLER_BUTTON_Y,             KBJK_9,      nullptr, false }, // default: toggle map
   { SDL_CONTROLLER_BUTTON_BACK,          KBJK_PAUSE,  nullptr, false }, // default: pause
   { SDL_CONTROLLER_BUTTON_GUIDE,         -1,          nullptr, false }, // default: unassigned (not usable on Windows)
   { SDL_CONTROLLER_BUTTON_START,         KBJK_OPTION, nullptr, false }, // default: toggle menu
   { SDL_CONTROLLER_BUTTON_LEFTSTICK,     KBJK_STAR,   nullptr, false }, // default: jag * key
   { SDL_CONTROLLER_BUTTON_RIGHTSTICK,    KBJK_NUM,    nullptr, false }, // default: jag # key
   { SDL_CONTROLLER_BUTTON_LEFTSHOULDER,  KBJK_PWEAPN, nullptr, false }, // default: previous weapon
   { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, KBJK_NWEAPN, nullptr, false }, // default: next weapon
   { SDL_CONTROLLER_BUTTON_DPAD_UP,       KBJK_UP,     nullptr, false }, // default: move up
   { SDL_CONTROLLER_BUTTON_DPAD_DOWN,     KBJK_DOWN,   nullptr, false }, // default: move down
   { SDL_CONTROLLER_BUTTON_DPAD_LEFT,     KBJK_LEFT,   nullptr, false }, // default: move left
   { SDL_CONTROLLER_BUTTON_DPAD_RIGHT,    KBJK_RIGHT,  nullptr, false }, // default: move right
};

static CfgItem cfgGamepadButtonA     ("gamepad_button_a",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_A            ].name);
static CfgItem cfgGamepadButtonB     ("gamepad_button_b",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_B            ].name);
static CfgItem cfgGamepadButtonX     ("gamepad_button_x",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_X            ].name);
static CfgItem cfgGamepadButtonY     ("gamepad_button_y",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_Y            ].name);
static CfgItem cfgGamepadButtonBack  ("gamepad_button_back",   &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_BACK         ].name);
static CfgItem cfgGamepadButtonGuide ("gamepad_button_guide",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_GUIDE        ].name);
static CfgItem cfgGamepadButtonStart ("gamepad_button_start",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_START        ].name);
static CfgItem cfgGamepadButtonLStick("gamepad_button_lstick", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSTICK    ].name);
static CfgItem cfgGamepadButtonRStick("gamepad_button_rstick", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK   ].name);
static CfgItem cfgGamepadButtonLShldr("gamepad_button_lshldr", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER ].name);
static CfgItem cfgGamepadButtonRShldr("gamepad_button_rshldr", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].name);
static CfgItem cfgGamepadButtonUp    ("gamepad_button_up",     &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_UP      ].name);
static CfgItem cfgGamepadButtonDown  ("gamepad_button_down",   &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN    ].name);
static CfgItem cfgGamepadButtonLeft  ("gamepad_button_left",   &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_LEFT    ].name);
static CfgItem cfgGamepadButtonRight ("gamepad_button_right",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT   ].name);

//
// Initialize the game controller
//
static void SDL2_PadInit()
{
   if(pCurController)
   {
      SDL_GameControllerClose(pCurController);
      pCurController = nullptr;
   }

   // not using gamepads?
   if(!useGamepad)
      return;

   // try to find the device named in the configuration file
   if(!estrempty(gamepadDevice))
   {
      for(int i = 0; i < SDL_NumJoysticks(); i++)
      {
         if(SDL_IsGameController(i))
         {
            const char *sdlName = SDL_GameControllerNameForIndex(i);
            if(sdlName && !strcasecmp(gamepadDevice, sdlName))
            {
               if((pCurController = SDL_GameControllerOpen(i)))
                  break;
            }
         }
      }
   }
   else // no device is configured, but we want to use one...
   {
      // ...so open the first supported controller that will respond
      for(int i = 0; i < SDL_NumJoysticks(); i++)
      {
         if(SDL_IsGameController(i))
         {
            if((pCurController = SDL_GameControllerOpen(i)))
               break;
         }
      }
   }

   // save name of chosen device for config file
   if(pCurController)
   {
      char *oldGamePad = gamepadDevice;
      const char *newGamePad = SDL_GameControllerName(pCurController);

      if(pCurController)
      {
         gamepadDevice = estrdup(newGamePad);
         if(oldGamePad)
            efree(oldGamePad);
      }
   }
}

//
// Process a gamepad button event
//
static void SDL2_processGamepadButton(const SDL_ControllerButtonEvent &cbe)
{
   if(!useGamepad)
      return;

   if(cbe.button < SDL_CONTROLLER_BUTTON_MAX)
      sdlButtonToJagButton[cbe.button].down = (cbe.type == SDL_CONTROLLERBUTTONDOWN);
}

//
// Process a trigger axis
// 
static void SDL2_processTrigger(int &buttons, SDL_GameControllerAxis which, Sint16 value)
{
   int *actionVar;
   gptriggeraction_e action;

   switch(which)
   {
   case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
      actionVar = &gamepadLTriggerAction;
      break;
   case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
      actionVar = &gamepadRTriggerAction;
      break;
   default:
      return;
   }

   action = static_cast<gptriggeraction_e>(*actionVar);
   switch(action)
   {
   case GPT_USE:
      buttons |= JP_USE;
      break;
   case GPT_ATTACK:
      buttons |= JP_B;
      break;
   case GPT_LSTRAFE:
      buttons |= JP_SLEFT;
      break;
   case GPT_RSTRAFE:
      buttons |= JP_SRIGHT;
      break;
   case GPT_PREVWEAPON:
      buttons |= JP_PWEAPN;
      break;
   case GPT_NEXTWEAPON:
      buttons |= JP_NWEAPN;
      break;
   default:
      break;
   }
}

//
// Process asserted gamepad axes
//
static void SDL2_processGamepadAxes(int &buttons)
{
   if(!useGamepad || !pCurController)
      return;

   Sint16 lx = SDL_GameControllerGetAxis(pCurController, SDL_CONTROLLER_AXIS_LEFTX);
   Sint16 ly = SDL_GameControllerGetAxis(pCurController, SDL_CONTROLLER_AXIS_LEFTY);
   Sint16 rx = SDL_GameControllerGetAxis(pCurController, SDL_CONTROLLER_AXIS_RIGHTX);
   //Sint16 ry = SDL_GameControllerGetAxis(pCurController, SDL_CONTROLLER_AXIS_RIGHTY);
   Sint16 lt = SDL_GameControllerGetAxis(pCurController, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
   Sint16 rt = SDL_GameControllerGetAxis(pCurController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

   // invert y axes?
   if(gamepadInvertY)
   {
      ly = -ly;
      //ry = -ry;
   }

   // left stick vertical - up/down movement
   if(abs(ly) > gamepadThreshold)
   {
      if(ly < 0)
         buttons |= JP_UP;
      else
         buttons |= JP_DOWN;
   }

   // left stick horizontal - left/right strafe
   if(abs(lx) > gamepadThreshold)
   {
      if(lx < 0)
         buttons |= JP_SLEFT;
      else
         buttons |= JP_SRIGHT;
   }

   // right stick horizontal - turn left/right
   if(abs(rx) > gamepadThreshold)
   {
      if(rx < 0)
         buttons |= JP_LEFT;
      else
         buttons |= JP_RIGHT;
   }

   // left trigger
   if(abs(lt) > gamepadTriggerThreshold)
      SDL2_processTrigger(buttons, SDL_CONTROLLER_AXIS_TRIGGERLEFT, lt);

   // right trigger
   if(abs(rt) > gamepadTriggerThreshold)
      SDL2_processTrigger(buttons, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, rt);
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

   // intialize gamepad button bindings
   for(gpbutton_t &gpbutton : sdlButtonToJagButton)
   {
      if(estrempty(gpbutton.name))
      {
         // unconfigured name
         if(gpbutton.kbButton >= 0)
            gpbutton.name = estrdup(kbJoyKeyJagNames[gpbutton.kbButton]);
         else
            gpbutton.name = estrdup("unbound");
      }
      else
      {
         // name is configured; find the corresponding KBJK enum value and setup the button correspondence
         bool found = false;

         for(size_t j = 0; j < KBJK_MAX; j++)
         {
            if(!strcasecmp(kbJoyKeyJagNames[j], gpbutton.name))
            {
               gpbutton.kbButton = j;
               found = true;
               break;
            }
         }

         // must be unbound
         if(!found)
            gpbutton.kbButton = -1;
      }
   }

   // init gamepad device
   SDL2_PadInit();
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
         SDL2_processGamepadButton(evt.cbutton);
         break;
      case SDL_QUIT:
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

   // process keyboard state
   SDL2_processKeyboard();

   // turn input into Jaguar gamepad buttons
   int buttons = 0;

   // keyboard
   for(size_t i = 0; i < KBJK_MAX; i++)
   {
      if(kbKeyDown[i])
         buttons |= kbKeyToJagButton[i];
   }

   // gamepad buttons
   if(useGamepad)
   {
      for(const gpbutton_t &gpbutton : sdlButtonToJagButton)
      {
         if(gpbutton.down && gpbutton.kbButton >= 0)
            buttons |= kbKeyToJagButton[gpbutton.kbButton];
      }
   }

   // gamepad axes
   SDL2_processGamepadAxes(buttons);

   return buttons;
}

//
// Reset input states
//
void SDL2_ResetInput(void)
{
   for(bool &kd : kbKeyDown)
      kd = false;

   for(gpbutton_t &gp : sdlButtonToJagButton)
      gp.down = false;
}

#endif

// EOF

