/*
  CALICO

  Inline functions for endian-independent number handling

  The MIT License (MIT)

  Copyright (C) 2017 James Haley

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

#ifndef SWAP_H__
#define SWAP_H__

#include "elib.h"

//=============================================================================
//
// Swap 16bit, that is, MSB and LSB byte.
//

inline static int16_t SwapShort(int16_t x)
{
   return (((uint8_t *) &x)[1] << 8) |
           ((uint8_t *) &x)[0];
}

inline static uint16_t SwapUShort(uint16_t x)
{
   return (((uint8_t *) &x)[1] << 8) |
           ((uint8_t *) &x)[0];
}

// haleyjd: same routine but for big-endian input

inline static int16_t SwapBigShort(int16_t x)
{
   return (((uint8_t *) &x)[0] << 8) |
           ((uint8_t *) &x)[1];
}

inline static uint16_t SwapBigUShort(uint16_t x)
{
   return (((uint8_t *) &x)[0] << 8) |
           ((uint8_t *) &x)[1];
}

//=============================================================================
//
// Swapping 32bit.
//

inline static int32_t SwapLong(int32_t x)
{
   return (((uint8_t *) &x)[3] << 24) |
          (((uint8_t *) &x)[2] << 16) |
          (((uint8_t *) &x)[1] <<  8) |
           ((uint8_t *) &x)[0];
}

inline static uint32_t SwapULong(uint32_t x)
{
   return (((uint8_t *) &x)[3] << 24) |
          (((uint8_t *) &x)[2] << 16) |
          (((uint8_t *) &x)[1] <<  8) |
           ((uint8_t *) &x)[0];
}

// haleyjd: same routine but for big-endian input

inline static int32_t SwapBigLong(int32_t x)
{
   return (((uint8_t *) &x)[0] << 24) |
          (((uint8_t *) &x)[1] << 16) |
          (((uint8_t *) &x)[2] <<  8) |
           ((uint8_t *) &x)[3];
}

inline static uint32_t SwapBigULong(uint32_t x)
{
   return (((uint8_t *) &x)[0] << 24) |
          (((uint8_t *) &x)[1] << 16) |
          (((uint8_t *) &x)[2] <<  8) |
           ((uint8_t *) &x)[3];
}

#endif

// EOF

