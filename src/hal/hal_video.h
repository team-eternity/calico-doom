/*
  CALICO
  
  HAL Video Interface
  
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

#ifndef HAL_VIDEO_H__
#define HAL_VIDEO_H__

#include "hal_types.h"

#define CALICO_ORIG_SCREENWIDTH  320
#define CALICO_ORIG_SCREENHEIGHT 224

#define CALICO_ORIG_GAMESCREENWIDTH  160
#define CALICO_ORIG_GAMESCREENHEIGHT 180

typedef enum hal_aspect_e
{
   HAL_ASPECT_NOMINAL,
   HAL_ASPECT_WIDE,
   HAL_ASPECT_NARROW
} hal_aspect_t;

typedef struct hal_video_s
{
   void          (*initVideo)(void);
   hal_bool      (*setNewVideoMode)(int w, int h, int fs, int mnum);
   void         *(*getGLProcAddress)(const char *proc);
   void          (*getWindowSize)(int *width, int *height);
   void          (*transformFBCoord)(int x, int y, int *tx, int *ty);
   void          (*transformGameCoord2i)(int x, int y, int *tx, int *ty);
   void          (*transformGameCoord2f)(int x, int y, float *tx, float *ty);
   unsigned int  (*transformWidth)(unsigned int w);
   unsigned int  (*transformHeight)(unsigned int h);
   int           (*toggleGLSwap)(hal_bool swap);
   void          (*endFrame)(void);
   int           (*isFullScreen)(void);
   int           (*getCurrentDisplay)(void);
   unsigned int  (*getWindowFlags)(void);
   void          (*setGrab)(hal_bool grab);
   void          (*warpMouse)(int x, int y);
   void         *(*getWindowHandle)(void);
   hal_aspect_t  (*getAspectRatioType)(void);
   void          (*getSubscreenExtents)(int *x, int *y, int *w, int *h);
} hal_video_t;

#ifdef __cplusplus
extern "C" {
#endif

extern hal_video_t hal_video;

#ifdef __cplusplus
}
#endif

#endif

// EOF

