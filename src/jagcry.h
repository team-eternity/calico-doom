/*
  CALICO

  Jaguar CRY Color Space Utilities

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

#ifndef JAGCRY_H__
#define JAGCRY_H__

#include <stdint.h>

#define CRY_CMASK 0xF000
#define CRY_RMASK 0x0F00
#define CRY_YMASK 0x00FF

#define CRY_CSHIFT 12
#define CRY_RSHIFT 8
#define CRY_YSHIFT 0

// Main CRY translation table
extern uint32_t CRYToRGB[0x10000];

void CRY_BuildRGBTable(void);

#endif

// EOF

