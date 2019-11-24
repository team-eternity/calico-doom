/*
  CALICO

  OpenGL renderer main functions and utilities

  The MIT License (MIT)

  Copyright (C) 2007-2014 Samuel Villarreal
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

#ifndef RB_MAIN_H__
#define RB_MAIN_H__

#include "rb_types.h"

class rbTexture;

enum rbvtx_e
{
   // coords
   VTX_X = 0,
   VTX_Y,
   VTX_Z,

   // texture coords
   VTX_U = 0,
   VTX_V,

   // color components
   VTX_R = 0,
   VTX_G,
   VTX_B,
   VTX_A
};

// Vertex type
struct vtx_t
{
   float  coords[3];
   float  txcoords[2];
   rbbyte colors[4];
};

struct rbScissor_t
{
   int x;
   int y;
   int width;
   int height;
};

// GL state caching enumeration
typedef enum
{
    RB_GLSTATE_BLEND   = 0,
    RB_GLSTATE_CULL,
    RB_GLSTATE_TEXTURE0,
    RB_GLSTATE_TEXTURE1,
    RB_GLSTATE_TEXTURE2,
    RB_GLSTATE_TEXTURE3,
    RB_GLSTATE_DEPTHTEST,
    RB_GLSTATE_STENCILTEST,
    RB_GLSTATE_SCISSOR,
    RB_GLSTATE_ALPHATEST,
    RB_GLSTATE_TEXGEN_S,
    RB_GLSTATE_TEXGEN_T,
    RB_GLSTATE_FOG,
    RB_NUMGLSTATES
} rbState_e;

// GL clear buffer flags
typedef enum
{
   RB_GLCB_COLOR      = 0x01,
   RB_GLCB_DEPTH      = 0x02,
   RB_GLCB_STENCIL    = 0x04,
   RB_GLCB_ALL        = (RB_GLCB_COLOR|RB_GLCB_DEPTH|RB_GLCB_STENCIL)
} rbClearBit_e;

typedef enum
{
   RB_GLSRC_UNDEF     = -1,
   RB_GLSRC_ZERO      = 0,
   RB_GLSRC_ONE,
   RB_GLSRC_DST_COLOR,
   RB_GLSRC_ONE_MINUS_DST_COLOR,
   RB_GLSRC_SRC_ALPHA,
   RB_GLSRC_ONE_MINUS_SRC_ALPHA,
   RB_GLSRC_DST_ALPHA,
   RB_GLSRC_ONE_MINUS_DST_ALPHA,
   RB_GLSRC_ALPHA_SATURATE,
} rbSrcBlend_e;

typedef enum
{
   RB_GLDST_UNDEF     = -1,
   RB_GLDST_ZERO      = 0,
   RB_GLDST_ONE,
   RB_GLDST_SRC_COLOR,
   RB_GLDST_ONE_MINUS_SRC_COLOR,
   RB_GLDST_SRC_ALPHA,
   RB_GLDST_ONE_MINUS_SRC_ALPHA,
   RB_GLDST_DST_ALPHA,
   RB_GLDST_ONE_MINUS_DST_ALPHA,
} rbDstBlend_e;

typedef enum {
   RB_GLCULL_UNDEF    = -1,
   RB_GLCULL_FRONT    = 0,
   RB_GLCULL_BACK
} rbCullType_e;

typedef enum {
   RB_GLBUFFER_NONE,
   RB_GLBUFFER_BACK,
   RB_GLBUFFER_FRONT
} rbBufferType_e;

//
// GL state caching structure
//
struct rbState_t
{
   unsigned int glStateBits;
   unsigned int currentTexture;
   rbSrcBlend_e blendSrc;
   rbDstBlend_e blendDest;
   rbCullType_e cullType;
   unsigned int readBuffer;
   rbScissor_t  scissorRect;
};

extern rbState_t rbState;

void RB_InitDefaultState();

rbTexture *RB_GetWhiteTexture();

void RB_ClearBuffer(rbClearBit_e bit);
void RB_SetState(int bits, bool bEnable);
void RB_SetBlend(rbSrcBlend_e src, rbDstBlend_e dest);
void RB_SetCull(rbCullType_e type);
void RB_SetReadBuffer(rbBufferType_e state);
void RB_SetScissorRect(const rbScissor_t &rect);

#define GETPROC(ptr, name) \
   ptr = reinterpret_cast<decltype(ptr)>(hal_video.getGLProcAddress(name)); \
   extension_ok = (extension_ok && ptr != nullptr)

#endif

// EOF

