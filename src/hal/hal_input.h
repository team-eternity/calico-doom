/*
  CALICO
  
  HAL Input Interface
  
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

#ifndef HAL_INPUT_H__
#define HAL_INPUT_H__

#include "hal_types.h"

typedef struct hal_appstate_s
{
   hal_bool (*mouseShouldBeGrabbed)(void);
   void     (*updateGrab)(void);
   void     (*updateFocus)(void);
   void     (*setGrabState)(hal_bool state);
   hal_bool (*gameGrabCallback)(void);
} hal_appstate_t;

typedef struct hal_input_s
{
   void (*initInput)(void);
   int  (*getEvents)(void);
   void (*resetInput)(void);
   void (*getMouseMotion)(int *x, int *y);
} hal_input_t;

#ifdef __cplusplus
extern "C" {
#endif

extern hal_appstate_t  hal_appstate;
extern hal_input_t     hal_input;

#ifdef __cplusplus
}
#endif

#endif

// EOF

