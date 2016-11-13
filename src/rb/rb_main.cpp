/*
  CALICO

  OpenGL renderer main functions and utilities

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

#ifdef USE_SDL2
#include "SDL_opengl.h"
#else
#error Need include for opengl.h
#endif

#include <cstdlib>
#include <cstring>
#include "rb_main.h"
#include "rb_texture.h"
#include "valloc.h"

// Ensure AMD and NVidia cards are not rolling over to onboard graphics in the name of 
// power economy.
extern "C"
{
#if defined(__GNUC__)
__attribute__ ((visibility("default"))) int AmdPowerXpressRequestHighPerformance = 1;
__attribute__ ((visibility("default"))) unsigned long NvOptimusEnablement = 1u;
#elif defined(_MSC_VER)
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
__declspec(dllexport) unsigned long NvOptimusEnablement = 1u;
#endif
}

// GL global state cache
rbState_t rbState;

static rbTexture whiteTexture;

//
// Initialize a plain white texture
//
static void RB_InitWhiteTexture()
{
   rbbyte rgb[64]; // 4x4 RGB texture
   std::memset(rgb, 0xff, sizeof(rgb));

   whiteTexture.init(rbTexture::TCR_RGBA, 4u, 4u);
   whiteTexture.upload(rgb, rbTexture::TC_CLAMP, rbTexture::TF_NEAREST);
}

rbTexture *RB_GetWhiteTexture()
{
   if(!whiteTexture.getTextureID())
      RB_InitWhiteTexture();

   return &whiteTexture;
}

VALLOCATION(whiteTexture)
{
   whiteTexture.abandonTexture();
}

//
// Resets the OpenGL state
//
void RB_InitDefaultState()
{
   rbTexture::Unbind();
   rbState.glStateBits = 0;
   rbState.blendDest   = RB_GLDST_UNDEF;
   rbState.blendSrc    = RB_GLSRC_UNDEF;
   rbState.cullType    = RB_GLCULL_UNDEF;
   rbState.readBuffer  = RB_GLBUFFER_NONE;

   rbState.scissorRect.x = 0;
   rbState.scissorRect.y = 0;
   rbState.scissorRect.width = 0;
   rbState.scissorRect.height = 0;

   glClearDepth(1.0f);
   glClearStencil(0);
   glClearColor(0, 0, 0, 1);

   RB_ClearBuffer(RB_GLCB_ALL);
   RB_SetState(RB_GLSTATE_CULL, true);
   RB_SetCull(RB_GLCULL_BACK);
   RB_SetBlend(RB_GLSRC_SRC_ALPHA, RB_GLDST_ONE_MINUS_SRC_ALPHA);

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);
}

//
// Clear one or more elements of the framebuffer.
//
void RB_ClearBuffer(rbClearBit_e bit)
{
    GLbitfield clearBit = 0;
    
    if(bit & RB_GLCB_COLOR)
    {
       clearBit |= GL_COLOR_BUFFER_BIT;
    }
    if(bit & RB_GLCB_DEPTH)
    {
       clearBit |= GL_DEPTH_BUFFER_BIT;
    }
    if(bit & RB_GLCB_STENCIL)
    {
       clearBit |= GL_STENCIL_BUFFER_BIT;
    }
    
    glClear(clearBit);
}

//
// Set GL global state with caching, so that state changes are minimized.
//
void RB_SetState(int bits, bool bEnable)
{
   int stateFlag = 0;

   switch(bits)
   {
   case RB_GLSTATE_BLEND:
      stateFlag = GL_BLEND;
      break;

   case RB_GLSTATE_CULL:
      stateFlag = GL_CULL_FACE;
      break;

   case RB_GLSTATE_TEXTURE0:
      stateFlag = GL_TEXTURE_2D;
      break;

   case RB_GLSTATE_ALPHATEST:
      stateFlag = GL_ALPHA_TEST;
      break;

   case RB_GLSTATE_TEXGEN_S:
      stateFlag = GL_TEXTURE_GEN_S;
      break;

   case RB_GLSTATE_TEXGEN_T:
      stateFlag = GL_TEXTURE_GEN_T;
      break;

   case RB_GLSTATE_DEPTHTEST:
      stateFlag = GL_DEPTH_TEST;
      break;

   case RB_GLSTATE_FOG:
      stateFlag = GL_FOG;
      break;

   case RB_GLSTATE_STENCILTEST:
      stateFlag = GL_STENCIL_TEST;
      break;

   case RB_GLSTATE_SCISSOR:
      stateFlag = GL_SCISSOR_TEST;
      break;

   default:
      return;
   }

   // if state was already set then don't set it again
   if(bEnable && !(rbState.glStateBits & (1 << bits)))
   {
      glEnable(stateFlag);
      rbState.glStateBits |= (1 << bits);
   }
   // if state was already unset then don't unset it again
   else if(!bEnable && (rbState.glStateBits & (1 << bits)))
   {
      glDisable(stateFlag);
      rbState.glStateBits &= ~(1 << bits);
   }
}

//
// Set and cache GL blend function state
//
void RB_SetBlend(rbSrcBlend_e src, rbDstBlend_e dest)
{
   int pBlend = (rbState.blendSrc ^ src) | (rbState.blendDest ^ dest);
   int glSrc = GL_ONE;
   int glDst = GL_ONE;

   if(pBlend == 0)
      return; // already set

   switch(src)
   {
   case RB_GLSRC_ZERO:
      glSrc = GL_ZERO;
      break;

   case RB_GLSRC_ONE:
      glSrc = GL_ONE;
      break;

   case RB_GLSRC_DST_COLOR:
      glSrc = GL_DST_COLOR;
      break;

   case RB_GLSRC_ONE_MINUS_DST_COLOR:
      glSrc = GL_ONE_MINUS_DST_COLOR;
      break;

   case RB_GLSRC_SRC_ALPHA:
      glSrc = GL_SRC_ALPHA;
      break;

   case RB_GLSRC_ONE_MINUS_SRC_ALPHA:
      glSrc = GL_ONE_MINUS_SRC_ALPHA;
      break;

   case RB_GLSRC_DST_ALPHA:
      glSrc = GL_DST_ALPHA;
      break;

   case RB_GLSRC_ONE_MINUS_DST_ALPHA:
      glSrc = GL_ONE_MINUS_DST_ALPHA;
      break;

   case RB_GLSRC_ALPHA_SATURATE:
      glSrc = GL_SRC_ALPHA_SATURATE;
      break;
   }

   switch(dest) 
   {
   case RB_GLDST_ZERO:
      glDst = GL_ZERO;
      break;

   case RB_GLDST_ONE:
      glDst = GL_ONE;
      break;

   case RB_GLDST_SRC_COLOR:
      glDst = GL_SRC_COLOR;
      break;

   case RB_GLDST_ONE_MINUS_SRC_COLOR:
      glDst = GL_ONE_MINUS_SRC_COLOR;
      break;

   case RB_GLDST_SRC_ALPHA:
      glDst = GL_SRC_ALPHA;
      break;

   case RB_GLDST_ONE_MINUS_SRC_ALPHA:
      glDst = GL_ONE_MINUS_SRC_ALPHA;
      break;

   case RB_GLDST_DST_ALPHA:
      glDst = GL_DST_ALPHA;
      break;

   case RB_GLDST_ONE_MINUS_DST_ALPHA:
      glDst = GL_ONE_MINUS_DST_ALPHA;
      break;
   }

   glBlendFunc(glSrc, glDst);

   rbState.blendSrc  = src;
   rbState.blendDest = dest;
}

//
// Set GL face culling with state caching.
//
void RB_SetCull(rbCullType_e type)
{
   int pCullType = rbState.cullType ^ type;
   int cullType = 0;

   if(pCullType == 0)
      return; // already set

   switch(type)
   {
   case RB_GLCULL_FRONT:
      cullType = GL_FRONT;
      break;

   case RB_GLCULL_BACK:
      cullType = GL_BACK;
      break;

   default:
      return;
   }

   glCullFace(cullType);
   rbState.cullType = type;
}

//
// Bind read buffer
//
void RB_SetReadBuffer(rbBufferType_e state)
{
   if(rbState.readBuffer == state)
      return;

   GLenum glState = GL_NONE;
   switch(state)
   {
   case RB_GLBUFFER_FRONT:
      glState = GL_FRONT;
      break;
   case RB_GLBUFFER_BACK:
      glState = GL_BACK;
      break;
   default:
      break;
   }

   glReadBuffer(glState);
   rbState.readBuffer = state;
}

//
// Set a scissor rect if it is different from the current one.
//
void RB_SetScissorRect(const rbScissor_t &rect)
{
   if(rect.x      == rbState.scissorRect.x     &&
      rect.y      == rbState.scissorRect.y     &&
      rect.width  == rbState.scissorRect.width &&
      rect.height == rbState.scissorRect.height)
      return;

   glScissor(rect.x, rect.y, rect.width, rect.height);

   rbState.scissorRect.x      = rect.x;
   rbState.scissorRect.y      = rect.y;
   rbState.scissorRect.width  = rect.width;
   rbState.scissorRect.height = rect.height;
}

// EOF

