/*
  CALICO

  OpenGL renderer texture functions

  The MIT License (MIT)

  Copyright (C) 2007-2014 Samuel Villarreal
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

#ifndef RB_TEXTURE_H__
#define RB_TEXTURE_H__

#include "rb_types.h"

unsigned int RB_MakeTextureDimension(unsigned int i);

//
// Texture class
//
class rbTexture
{
public:
   typedef unsigned int dtexture;

   typedef enum
   {
      TC_CLAMP    = 0,
      TC_CLAMP_BORDER,
      TC_REPEAT,
      TC_MIRRORED
   } texClampMode_t;

   typedef enum
   {
      TF_LINEAR   = 0,
      TF_NEAREST,
      TF_AUTO
   } texFilterMode_t;

   typedef enum {
      TCR_RGB     = 0,
      TCR_RGBA,
      TCR_BGRA
   } texColorMode_t;

protected:
   friend class TexTmpBuffer;

   unsigned int    width;
   unsigned int    height;
   texClampMode_t  clampMode;
   texFilterMode_t filterMode;
   texColorMode_t  colorMode;
   dtexture        texid;
   bool            streaming;
   dtexture        pboid;
   bool            dirtypbo;

   void deletePBO();

public:
   rbTexture();
   rbTexture(rbTexture &&other);
   ~rbTexture();

   void init(texColorMode_t colorMode, unsigned int w, unsigned int h);
   void initStreaming(unsigned int w, unsigned int h);
   void setTexParameters();
   void changeTexParameters(texClampMode_t clamp, texFilterMode_t filter);
   void bind(bool forDraw = true);
   void deleteTexture();
   void abandonTexture();
   void upload(rbbyte *data, texClampMode_t clamp, texFilterMode_t filter);
   void update(rbbyte *data);
   void fromFrameBuffer();

   // CALICO_TODO: screenshot function
#if 0
   bool writePNG(const char *filename);
#endif

   static void Unbind();

   unsigned int    getWidth()      const { return width;      }
   unsigned int    getHeight()     const { return height;     }
   texClampMode_t  getClampMode()  const { return clampMode;  }
   texFilterMode_t getFilterMode() const { return filterMode; }
   texColorMode_t  getColorMode()  const { return colorMode;  }
   dtexture        getTextureID()  const { return texid;      }
};

extern bool rb_linear_filtering;

#endif

// EOF

