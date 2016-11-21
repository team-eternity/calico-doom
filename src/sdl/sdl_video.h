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

#ifndef SDL_VIDEO_H__
#define SDL_VIDEO_H__

#ifdef USE_SDL2

#include "../hal/hal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern SDL_Window *mainwindow;

void          SDL2_GetWindowSize(int *width, int *height);
void         *SDL2_GetGLProcAddress(const char *proc);
void          SDL2_InitVideo(void);
hal_bool      SDL2_SetNewVideoMode(int w, int h, int fs, int mnum);
void          SDL2_TransformFBCoord(int x, int y, int *tx, int *ty);
void          SDL2_TransformGameCoord2i(int x, int y, int *tx, int *ty);
void          SDL2_TransformGameCoord2f(int x, int y, float *tx, float *ty);
unsigned int  SDL2_TransformWidth(unsigned int w);
unsigned int  SDL2_TransformHeight(unsigned int h);
int           SDL2_ToggleGLSwap(hal_bool swap);
int           SDL2_IsFullScreen(void);
int           SDL2_GetCurrentDisplay(void);
unsigned int  SDL2_GetWindowFlags(void);
void          SDL2_SetGrab(hal_bool grab);
void          SDL2_WarpMouse(int x, int y);
void          SDL2_EndFrame(void);
void         *SDL2_GetWindowHandle(void);

#ifdef __cplusplus
}
#endif

#endif

#endif

// EOF


