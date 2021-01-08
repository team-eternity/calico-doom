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

#ifndef SDL_INPUT_H__
#define SDL_INPUT_H__

#ifdef USE_SDL2

#ifdef __cplusplus
extern "C" {
#endif

void     SDL2_SetGrabState(hal_bool state);
hal_bool SDL2_MouseShouldBeGrabbed(void);
void     SDL2_UpdateGrab(void);
void     SDL2_UpdateFocus(void);
void     SDL2_InitInput(void);
int      SDL2_GetEvents(void);
void     SDL2_ResetInput(void);
void     SDL2_GetMouseMotion(int *x, int *y);

#ifdef __cplusplus
}
#endif

#endif

#endif

// EOF

