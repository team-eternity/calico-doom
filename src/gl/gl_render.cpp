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
#include "../elib/bdlist.h"
#include "../elib/zone.h"
#include "../hal/hal_types.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "../renderintr/ri_interface.h"
#include "../rb/rb_common.h"
#include "../rb/rb_draw.h"
#include "../rb/rb_main.h"
#include "../rb/rb_texture.h"
#include "../rb/valloc.h"
#include "../jagcry.h"
#include "gl_render.h"
#include "gl_textures.h"
#include "gl_framebuffer.h"
#include "gl_drawcmd.h"

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
static void GL_drawRectImmediate(vtx_t v[4])
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
static void GL_drawGameRect(int gx, int gy, unsigned int gw, unsigned int gh, rbTexture &tx)
{
    vtx_t v[4];
    float sx, sy, sw, sh;

    RB_SetVertexColors(v, 4, 0xff, 0xff, 0xff, 0xff);
    RB_DefTexCoords(v, &tx);

    // bind texture
    tx.bind();

    // transform coordinates into screen space
    hal_video.transformGameCoord2f(gx, gy, &sx, &sy);

    // scale width and height into screen space
    sw = float(hal_video.transformWidth(gw));
    sh = float(hal_video.transformHeight(gh));

    GL_initVtxCoords(v, sx, sy, sw, sh);

    GL_drawRectImmediate(v);
}

//=============================================================================
//
// Refresh
//

static void GL_RenderFrame()
{
   glClear(GL_COLOR_BUFFER_BIT);

   GL_ExecuteDrawCommands();
   GL_ClearDrawCommands();
   hal_video.endFrame();
}

//=============================================================================
//
// Initialization
//

static void GL_InitRenderer(int w, int h)
{
    // set command function
    GL_SetDrawCommandFunc(GL_drawGameRect);

    // enable 2D texture mapping
    RB_SetState(RB_GLSTATE_TEXTURE0, true);

    // set viewport
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // clear model-view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // set projection matrix to a standard orthonormal projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (GLdouble)w, (GLdouble)h, 0.0, -1.0, 1.0);

    // disable depth buffer test
    RB_SetState(RB_GLSTATE_DEPTHTEST, false);
}

static renderintr_t gl11Renderer
{
    GL_InitRenderer,

    GL_InitFramebufferTextures,
    GL_GetFramebuffer,
    GL_UpdateFramebuffer,
    GL_ClearFramebuffer,
    GL_FramebufferSetUpdated,
    GL_AddFramebuffer,
    GL_RenderFrame,
    GL_NewTextureResource,
    GL_TextureResourceGetFramebuffer,
    GL_CheckForTextureResource,
    GL_UpdateTextureResource,
    GL_TextureResourceSetUpdated,
    GL_GetTextureResourceStore,
    GL_ClearTextureResource,

    GL_AddDrawCommand,
    GL_AddLateDrawCommand
};

//
// Select the GL 1.1 renderer
//
void GL_SelectRenderer()
{
    g_renderer = &gl11Renderer;
}

// EOF
