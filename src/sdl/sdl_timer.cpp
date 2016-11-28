/*
  CALICO
  
  SDL 2 Timer Functions
  
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
#include "../hal/hal_timer.h"
#include "sdl_timer.h"

//
// Delay for at least ms milliseconds.
//
void SDL2_Delay(unsigned int ms)
{
   SDL_Delay(ms);
}

static Uint32 basetime = 0;

//
// Get time in frames at a rate of 15 FPS
//
unsigned int SDL2_GetTime(void)
{
   Uint32 ticks = SDL_GetTicks();

   if(basetime == 0)
      basetime = ticks;

   ticks -= basetime;

   return (ticks * CALICO_GLOBAL_FPS) / 1000;
}

//
// Get time in milliseconds
//
unsigned int SDL2_GetTimeMS(void)
{
   Uint32 ticks = SDL_GetTicks();

   if(basetime == 0)
      basetime = ticks;

   return ticks - basetime;
}

#endif

// EOF

