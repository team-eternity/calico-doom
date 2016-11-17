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

#include <stdint.h>
#include "rb/rb_common.h"
#include "jagcry.h"

// Main CRY translation table
uint32_t CRYToRGB[0x10000];

//
// Red components for CRY to RGB conversion
//
static const uint8_t cryred[16][16] = 
{
   { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
   { 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 19, 0  },
   { 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 64, 43, 21, 0  },
   { 102,102,102,102,102,102,102,102,102,102,102,95, 71, 47, 23, 0  },
   { 135,135,135,135,135,135,135,135,135,135,130,104,78, 52, 26, 0  },
   { 169,169,169,169,169,169,169,169,169,170,141,113,85, 56, 28, 0  },
   { 203,203,203,203,203,203,203,203,203,183,153,122,91, 61, 30, 0  },
   { 237,237,237,237,237,237,237,237,230,197,164,131,98, 65, 32, 0  },
   { 255,255,255,255,255,255,255,255,247,214,181,148,15, 82, 49, 7  },
   { 255,255,255,255,255,255,255,255,255,235,204,173,143,112,81, 51 },
   { 255,255,255,255,255,255,255,255,255,255,227,198,170,141,113,85 },
   { 255,255,255,255,255,255,255,255,255,255,249,223,197,171,145,119},
   { 255,255,255,255,255,255,255,255,255,255,255,248,224,200,177,153},
   { 255,255,255,255,255,255,255,255,255,255,255,255,252,230,208,187},
   { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,221},
   { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255} 
};

//
// Green components for CRY to RGB conversion
//
static const uint8_t crygreen[16][16] =
{
   { 0,  17, 34, 51,68, 85, 102,119,136,153,170,187,204,221,238,255 },
   { 0,  19, 38, 57,77, 96, 115,134,154,173,192,211,231,250,255,255 },
   { 0,  21, 43, 64,86, 107,129,150,172,193,215,236,255,255,255,255 },
   { 0,  23, 47, 71,95, 119,142,166,190,214,238,255,255,255,255,255 },
   { 0,  26, 52, 78,104,130,156,182,208,234,255,255,255,255,255,255 },
   { 0,  28, 56, 85,113,141,170,198,226,255,255,255,255,255,255,255 },
   { 0,  30, 61, 91,122,153,183,214,244,255,255,255,255,255,255,255 },
   { 0,  32, 65, 98,131,164,197,230,255,255,255,255,255,255,255,255 },
   { 0,  32, 65, 98,131,164,197,230,255,255,255,255,255,255,255,255 },
   { 0,  30, 61, 91,122,153,183,214,244,255,255,255,255,255,255,255 },
   { 0,  28, 56, 85,113,141,170,198,226,255,255,255,255,255,255,255 },
   { 0,  26, 52, 78,104,130,156,182,208,234,255,255,255,255,255,255 },
   { 0,  23, 47, 71,95, 119,142,166,190,214,238,255,255,255,255,255 },
   { 0,  21, 43, 64,86, 107,129,150,172,193,215,236,255,255,255,255 },
   { 0,  19, 38, 57,77, 96, 115,134,154,173,192,211,231,250,255,255 },
   { 0,  17, 34, 51,68, 85, 102,119,136,153,170,187,204,221,238,255 }
};

//
// Blue components for CRY to RGB conversion
//
static const uint8_t cryblue[16][16] = 
{
   { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
   { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,221 },
   { 255,255,255,255,255,255,255,255,255,255,255,255,252,230,208,187 },
   { 255,255,255,255,255,255,255,255,255,255,255,248,224,200,177,153 },
   { 255,255,255,255,255,255,255,255,255,255,249,223,197,171,145,119 },
   { 255,255,255,255,255,255,255,255,255,255,227,198,170,141,113,85  },
   { 255,255,255,255,255,255,255,255,255,235,204,173,143,112,81, 51  },
   { 255,255,255,255,255,255,255,255,247,214,181,148,115,82, 49, 17  },
   { 237,237,237,237,237,237,237,237,230,197,164,131,98, 65, 32, 0   },
   { 203,203,203,203,203,203,203,203,203,183,153,122,91, 61, 30, 0   },
   { 169,169,169,169,169,169,169,169,169,170,141,113,85, 56, 28, 0   },
   { 135,135,135,135,135,135,135,135,135,135,130,104,78, 52, 26, 0   },
   { 102,102,102,102,102,102,102,102,102,102,102,95, 71, 47, 23, 0   },
   { 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 64, 43, 21, 0   },
   { 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 19, 0   },
   { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   }
};

void CRY_BuildRGBTable(void)
{
   uint32_t i;

   for(i = 0; i < 0x10000; i++)
   {
      uint32_t cyan = (i & CRY_CMASK) >> CRY_CSHIFT;
      uint32_t red  = (i & CRY_RMASK) >> CRY_RSHIFT;
      uint32_t intensity = (i & CRY_YMASK);

      uint32_t r = (((uint32_t)cryred  [cyan][red]) * intensity) >> 8;
      uint32_t g = (((uint32_t)crygreen[cyan][red]) * intensity) >> 8;
      uint32_t b = (((uint32_t)cryblue [cyan][red]) * intensity) >> 8;

      CRYToRGB[i] = D_RGBA(r, g, b, 0xff);
   }
}

// EOF

