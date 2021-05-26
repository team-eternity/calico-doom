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
#include "sdl_video.h"

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

static void SetShowCursor(bool show)
{
    // When the cursor is hidden, grab the input.
    // Relative mode implicitly hides the cursor.
    SDL_SetRelativeMouseMode(!show ? SDL_TRUE : SDL_FALSE);
    SDL_GetRelativeMouseState(nullptr, nullptr);
}

//
// Update input grabbing status
//
void SDL2_UpdateGrab(void)
{
   static hal_bool currentlyGrabbed = HAL_FALSE;
   const hal_bool grab = SDL2_MouseShouldBeGrabbed();

   if(grab && !currentlyGrabbed)
   {
      SetShowCursor(false);
      hal_video.setGrab(HAL_TRUE);
   }

   if(!grab && currentlyGrabbed)
   {
      SetShowCursor(true);
      hal_video.setGrab(HAL_FALSE);

      int w, h;
      SDL_GetWindowSize(mainwindow, &w, &h);
      SDL_WarpMouseInWindow(mainwindow, w / 2, h / 2);
      SDL_GetRelativeMouseState(nullptr, nullptr);
   }

   currentlyGrabbed = grab;
}

//
// Update window visibility and input focus status
//
void SDL2_UpdateFocus()
{
   SDL_PumpEvents();

   const unsigned int state   = hal_video.getWindowFlags();
   const hal_bool     active  = (hal_bool)((state & SDL_WINDOW_SHOWN) && !(state & SDL_WINDOW_MINIMIZED));
   const hal_bool     focused = (hal_bool)((state & (SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS)) != 0);

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

static const char *kbJoyKeyJagNames[KBJK_MAX] =
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

// config settings
static char *kbKeyNames[KBJK_MAX];

// default bindings
static SDL_Keycode kbKeyCodes[KBJK_MAX] =
{
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_w,
   SDLK_s,
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
   SDLK_TAB,
   SDLK_RALT,
   SDLK_a,
   SDLK_d,
   SDLK_SPACE,
   SDLK_LEFTBRACKET,
   SDLK_RIGHTBRACKET,
   SDLK_RCTRL,
   SDLK_LSHIFT
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
   const Uint8 *const state = SDL_GetKeyboardState(nullptr);
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

static bool  useGamepad = true;
static char *gamepadDevice;
static bool  gamepadInvertY;
static int   gamepadThreshold = 8000;
static int   gamepadTriggerThreshold = 3000;
static int   gamepadLTriggerAction = GPT_NONE;
static int   gamepadRTriggerAction = GPT_ATTACK;

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

static CfgItem cfgGamepadButtonA      { "gamepad_button_a",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_A            ].name };
static CfgItem cfgGamepadButtonB      { "gamepad_button_b",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_B            ].name };
static CfgItem cfgGamepadButtonX      { "gamepad_button_x",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_X            ].name };
static CfgItem cfgGamepadButtonY      { "gamepad_button_y",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_Y            ].name };
static CfgItem cfgGamepadButtonBack   { "gamepad_button_back",   &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_BACK         ].name };
static CfgItem cfgGamepadButtonGuide  { "gamepad_button_guide",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_GUIDE        ].name };
static CfgItem cfgGamepadButtonStart  { "gamepad_button_start",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_START        ].name };
static CfgItem cfgGamepadButtonLStick { "gamepad_button_lstick", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSTICK    ].name };
static CfgItem cfgGamepadButtonRStick { "gamepad_button_rstick", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK   ].name };
static CfgItem cfgGamepadButtonLShldr { "gamepad_button_lshldr", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER ].name };
static CfgItem cfgGamepadButtonRShldr { "gamepad_button_rshldr", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].name };
static CfgItem cfgGamepadButtonUp     { "gamepad_button_up",     &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_UP      ].name };
static CfgItem cfgGamepadButtonDown   { "gamepad_button_down",   &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN    ].name };
static CfgItem cfgGamepadButtonLeft   { "gamepad_button_left",   &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_LEFT    ].name };
static CfgItem cfgGamepadButtonRight  { "gamepad_button_right",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT   ].name };

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
      const SDL_JoystickGUID currentGUID = SDL_JoystickGetGUIDFromString(gamepadDevice);
      for(int i = 0; i < SDL_NumJoysticks(); i++)
      {
         if(SDL_IsGameController(i))
         {
            const SDL_JoystickGUID thisGUID = SDL_JoystickGetDeviceGUID(i);
            if(!memcmp(&thisGUID, &currentGUID, sizeof(SDL_JoystickGUID)))
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
      char *const oldGamePad = gamepadDevice;
      char newGUIDStr[33];

      SDL_JoystickGUID newGUID = SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(pCurController));
      SDL_JoystickGetGUIDString(newGUID, newGUIDStr, earrlen(newGUIDStr));

      gamepadDevice = estrdup(newGUIDStr);
      if(oldGamePad)
         efree(oldGamePad);
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
   const int *actionVar;

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

   const gptriggeraction_e action = static_cast<gptriggeraction_e>(*actionVar);
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
// Mouse Input
//

static bool   useMouse          = true;
static double mouseAcceleration = 2.0;
static int    mouseThreshold    = 10;
static int    mouseSensitivity  = 5;
static int    mousex;
static int    mousey;

static cfgrange_t<double> mouseAccelRng  { 1.0, 5.0 };
static cfgrange_t<int>    mouseThreshRng { 0,   32  };
static cfgrange_t<int>    mouseSenseRng  { 1,   256 };

static CfgItem cfgUseMouse         { "useMouse",          &useMouse };
static CfgItem cfgMouseAccel       { "mouseAcceleration", &mouseAcceleration, &mouseAccelRng  };
static CfgItem cfgMouseThreshold   { "mouseThreshold",    &mouseThreshold,    &mouseThreshRng };
static CfgItem cfgMouseSensitivity { "mouseSensitivity",  &mouseSensitivity,  &mouseSenseRng  };

struct mbutton_t
{
   Uint8  button;   // SDL controller button for this slot
   int    kbButton; // KBJ button to which the SDL controller button is assigned
   char  *name;     // name of Jag controller button for config
   bool   down;     // if true, button is down
};

enum calico_mb_e
{
    CMB_LEFT,
    CMB_MIDDLE,
    CMB_RIGHT,
    CMB_X1,
    CMB_X2,
    NUM_CALICO_MBS
};

static const Uint8 calicoMBToSDLMB[NUM_CALICO_MBS] =
{
    SDL_BUTTON_LEFT,
    SDL_BUTTON_MIDDLE,
    SDL_BUTTON_RIGHT,
    SDL_BUTTON_X1,
    SDL_BUTTON_X2
};

static inline calico_mb_e SDLMBToCalicoMB(Uint8 button)
{
    for(size_t i = 0; i < NUM_CALICO_MBS; i++)
    {
        if(calicoMBToSDLMB[i] == button)
            return static_cast<calico_mb_e>(i);
    }
    return NUM_CALICO_MBS;
}

static mbutton_t sdlMouseToJagButton[NUM_CALICO_MBS] =
{
   { SDL_BUTTON_LEFT,   KBJK_ATTACK, nullptr, false }, // default: attack
   { SDL_BUTTON_MIDDLE, KBJK_STRAFE, nullptr, false }, // default: strafe
   { SDL_BUTTON_RIGHT,  KBJK_USE,    nullptr, false }, // default: use
   { SDL_BUTTON_X1,     KBJK_9,      nullptr, false }, // default: toggle map
   { SDL_BUTTON_X2,     KBJK_PAUSE,  nullptr, false }, // default: pause
};

static CfgItem cfgMouseButtonLeft   { "mouse_button_left",   &sdlMouseToJagButton[CMB_LEFT  ].name };
static CfgItem cfgMouseButtonMiddle { "mouse_button_middle", &sdlMouseToJagButton[CMB_MIDDLE].name };
static CfgItem cfgMouseButtonRight  { "mouse_button_right",  &sdlMouseToJagButton[CMB_RIGHT ].name };
static CfgItem cfgMouseButtonX1     { "mouse_button_x1",     &sdlMouseToJagButton[CMB_X1    ].name };
static CfgItem cfgMouseButtonX2     { "mouse_button_x2",     &sdlMouseToJagButton[CMB_X2    ].name };

static int AccelerateMouse(int val)
{
    if(val < 0)
        return -AccelerateMouse(-val);

    if(val > mouseThreshold)
        return int((val - mouseThreshold) * mouseAcceleration + mouseThreshold);
    else
        return val;
}

static void SDL2_ReadMouse()
{
    int x = 0, y = 0;
    SDL_GetRelativeMouseState(&x, &y);

    if(x != 0 || y != 0)
    {
        mousex = AccelerateMouse(x) * (mouseSensitivity + 5) / 10;
        mousey = AccelerateMouse(y) * (mouseSensitivity + 5) / 10;
    }
    else
        mousex = mousey = 0;
}

void SDL2_GetMouseMotion(int *x, int *y)
{
    if(useMouse && windowFocused)
        SDL2_ReadMouse();

    if(x)
        *x = mousex;
    if(y)
        *y = mousey;
}

//
// Process a mouse button event
//
static void SDL2_processMouseButton(const SDL_MouseButtonEvent &mbe)
{
   if(!useMouse)
      return;

   const calico_mb_e button = SDLMBToCalicoMB(mbe.button);
   if(button != NUM_CALICO_MBS)
      sdlMouseToJagButton[button].down = (mbe.type == SDL_MOUSEBUTTONDOWN);
}

//=============================================================================
//
// Main Interface
//

//
// Initialize input subsystem
//
void SDL2_InitInput()
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

   // intialize mouse button bindings
   for(mbutton_t &mbutton : sdlMouseToJagButton)
   {
      if(estrempty(mbutton.name))
      {
         // unconfigured name
         if(mbutton.kbButton >= 0)
            mbutton.name = estrdup(kbJoyKeyJagNames[mbutton.kbButton]);
         else
            mbutton.name = estrdup("unbound");
      }
      else
      {
         // name is configured; find the corresponding KBJK enum value and setup the button correspondence
         bool found = false;

         for(size_t j = 0; j < KBJK_MAX; j++)
         {
            if(!strcasecmp(kbJoyKeyJagNames[j], mbutton.name))
            {
               mbutton.kbButton = j;
               found = true;
               break;
            }
         }

         // must be unbound
         if(!found)
            mbutton.kbButton = -1;
      }
   }

   // init gamepad device
   SDL2_PadInit();
}

//
// Primary input processing function
//
int SDL2_GetEvents()
{
   SDL_Event evt;

   SDL2_UpdateFocus();
   SDL2_UpdateGrab();

   if(!windowFocused)
   {
      // make sure keys aren't stuck
      SDL2_ResetInput();
   }

   // turn input into Jaguar gamepad buttons
   int buttons = 0;

   while(SDL_PollEvent(&evt))
   {
      switch(evt.type)
      {
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
         SDL2_processMouseButton(evt.button);
         break;
      case SDL_MOUSEWHEEL:
         if(useMouse && !(buttons & (JP_NWEAPN|JP_PWEAPN)))
         {
            if(evt.wheel.y > 0)
                buttons |= JP_NWEAPN;
            else if(evt.wheel.y < 0)
                buttons |= JP_PWEAPN;                          
         }
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

   // mouse buttons
   if(useMouse)
   {
       for(const mbutton_t &mbutton : sdlMouseToJagButton)
       {
           if(mbutton.down && mbutton.kbButton >= 0)
               buttons |= kbKeyToJagButton[mbutton.kbButton];
       }
   }

   // gamepad axes
   SDL2_processGamepadAxes(buttons);

   return buttons;
}

//
// Reset input states
//
void SDL2_ResetInput()
{
    for(bool &kd : kbKeyDown)
        kd = false;

    for(gpbutton_t &gp : sdlButtonToJagButton)
        gp.down = false;

    for(mbutton_t &mb : sdlMouseToJagButton)
        mb.down = false;
}

#endif

// EOF

