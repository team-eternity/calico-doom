/*
  CALICO
  
  OpenGL rendering
  
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

#include "../elib/elib.h"
#include "../hal/hal_types.h"
#include "../hal/hal_input.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "../rb/rb_draw.h"
#include "../rb/rb_main.h"
#include "../rb/rb_texture.h"
#include "../rb/valloc.h"

//=============================================================================
//
// Primitives and utilities
//

//
// Initialize quad vertex coordinates
//
static void GL_initVtxCoords(vtx_t v[4], float sx, float sy, float sw, float sh)
{
   for(int i = 0; i < 4; i++)
      v[i].coords[VTX_Z] = 0.0f;

   v[0].coords[VTX_X] = v[2].coords[VTX_X] = sx;
   v[0].coords[VTX_Y] = v[1].coords[VTX_Y] = sy;
   v[1].coords[VTX_X] = v[3].coords[VTX_X] = sx + sw;
   v[2].coords[VTX_Y] = v[3].coords[VTX_Y] = sy + sh;
}

//
// Set default GL states, with RB caching
//
static void GL_setDefaultStates()
{
   RB_SetState(RB_GLSTATE_CULL, true);
   RB_SetCull(RB_GLCULL_FRONT);
   RB_SetState(RB_GLSTATE_DEPTHTEST, false);
   RB_SetState(RB_GLSTATE_BLEND, true);
   RB_SetState(RB_GLSTATE_ALPHATEST, true);
   RB_SetBlend(RB_GLSRC_SRC_ALPHA, RB_GLDST_ONE_MINUS_SRC_ALPHA);
}

//
// Bind vertex draw pointers and output two triangles
//
void GL_DrawRectImmediate(vtx_t v[4])
{
   // set states
   GL_setDefaultStates();

   // render
   RB_BindDrawPointers(v);
   RB_AddTriangle(0, 1, 2);
   RB_AddTriangle(3, 2, 1);
   RB_DrawElements(GL_TRIANGLES);
   RB_ResetElements();
}

//
// Draw a rect from game coordinates (gx, gy) to translated framebuffer
// coordinates with the provided information.
//
void GL_DrawGameRect(int gx, int gy, unsigned int gw, unsigned int gh,
                     rbTexture *tx, vtx_t v[4])
{
   float sx, sy, sw, sh;

   // bind texture
   tx->bind();

   // transform coordinates into screen space
   hal_video.transformGameCoord2f(gx, gy, &sx, &sy);

   // scale width and height into screen space
   sw = float(hal_video.transformWidth(gw));
   sh = float(hal_video.transformHeight(gh));

   GL_initVtxCoords(v, sx, sy, sw, sh);

   GL_DrawRectImmediate(v);
}

//=============================================================================
//
// Software Framebuffer
//

uint32_t framebuffer[CALICO_ORIG_SCREENWIDTH * CALICO_ORIG_SCREENHEIGHT];

// EOF

