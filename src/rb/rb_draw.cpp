/*
  CALICO

  OpenGL Renderer Primitives

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

#include "../hal/hal_platform.h"
#include "rb_draw.h"

//=============================================================================
//
// Locals
//

#define MAXINDICES 0x10000

// draw indices
static uint16_t indexcnt = 0;
static uint16_t drawIndices[MAXINDICES];

//=============================================================================
//
// Code
//

//
// Sets the pointer to the vertex data on the
// client side. A better approach would to use
// vertex buffer objects instead so everything
// can be done on the GPU side.
//
// Having witness numerous issues with ATI cards
// and seeing how vertex data will always be
// changing/moving, I just don't see it being worth
// while to support it
//
void RB_BindDrawPointers(vtx_t *vtx)
{
   static vtx_t *prevpointer = nullptr;

   if(prevpointer == vtx)
      return;

   prevpointer = vtx;

   glTexCoordPointer(2, GL_FLOAT,         sizeof(vtx_t), vtx->txcoords);
   glVertexPointer  (3, GL_FLOAT,         sizeof(vtx_t), vtx->coords);
   glColorPointer   (4, GL_UNSIGNED_BYTE, sizeof(vtx_t), vtx->colors);
}

//
// Add a triangle to the "draw indices" list
//
void RB_AddTriangle(uint16_t v0, uint16_t v1, uint16_t v2)
{
   if(indexcnt + 3 >= MAXINDICES)
   {
      hal_platform.debugMsg("RB_AddTriangle: Triangle index overflow\n");
      return;
   }

   drawIndices[indexcnt++] = v0;
   drawIndices[indexcnt++] = v1;
   drawIndices[indexcnt++] = v2;
}

//
// Add a line to the "draw indices" list
//
void RB_AddLine(uint16_t v0, uint16_t v1)
{
   if(indexcnt + 2 >= MAXINDICES)
   {
      hal_platform.debugMsg("RB_AddLine: index overflow\n");
      return;
   }

   drawIndices[indexcnt++] = v0;
   drawIndices[indexcnt++] = v1;
}

//
// Execute an element draw list
//
void RB_DrawElements(int mode)
{
   glDrawElements(mode, indexcnt, GL_UNSIGNED_SHORT, drawIndices);
}

//
// Reset the element draw list
//
void RB_ResetElements()
{
   indexcnt = 0;
}

// EOF

