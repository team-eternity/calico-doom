/*
  CALICO
  
  HAL Media Layer interface

  The MIT License (MIT)
  
  Copyright (C) 2016 James Haley
  
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

#ifndef HAL_ML_H__
#define HAL_ML_H__

#include "hal_types.h"

typedef struct hal_medialayer_s
{
   hal_bool    (*init)(void);
   void        (*exit)(void);
   void        (*error)(void);
   int         (*msgbox)(const char *title, const char *msg, hal_bool isError);
   hal_bool    (*isExiting)(void);
   const char *(*getBaseDirectory)(void);
   const char *(*getWriteDirectory)(const char *app);
} hal_medialayer_t;

#ifdef __cplusplus
extern "C" {
#endif

extern hal_medialayer_t hal_medialayer;

#ifdef __cplusplus
}
#endif

#endif

// EOF

