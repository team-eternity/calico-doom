/*
  CALICO
  
  OpenGL framebuffer management
  
  The MIT License (MIT)
  
  Copyright (C) 2021 James Haley
  
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
#include "gl_textures.h"

//=============================================================================
//
// Software Framebuffers
//

//
// Global framebuffer resources
//
TextureResource *framebuffer160;
TextureResource *framebuffer320;

//
// Create the GL texture handle for the framebuffer texture
//
void GL_InitFramebufferTextures()
{
    // create 160x180 playfield texture
    framebuffer160 = static_cast<TextureResource *>(
        GL_NewTextureResource(
            "framebuffer",
            nullptr,
            CALICO_ORIG_GAMESCREENWIDTH,
            CALICO_ORIG_GAMESCREENHEIGHT,
            RES_FRAMEBUFFER,
            0
        )
    );
    if(!framebuffer160)
        hal_platform.fatalError("Could not create 160x180 framebuffer texture");

    // create 320x224 screen texture
    framebuffer320 = static_cast<TextureResource *>(
        GL_NewTextureResource(
            "framebuffer320",
            nullptr,
            CALICO_ORIG_SCREENWIDTH,
            CALICO_ORIG_SCREENHEIGHT,
            RES_FRAMEBUFFER,
            0
        )
    );
    if(!framebuffer320)
        hal_platform.fatalError("Could not create 320x224 framebuffer texture");
}

//
// Return the pointer to the local 32-bit framebuffer
//
void *GL_GetFramebuffer(glfbwhich_t which)
{
    switch(which)
    {
    case FB_160:
        return framebuffer160->getPixels();
    case FB_320:
        return framebuffer320->getPixels();
    }

    return nullptr;
}

void GL_UpdateFramebuffer(glfbwhich_t which)
{
    TextureResource *fb;

    switch(which)
    {
    case FB_160:
        fb = framebuffer160;
        break;
    case FB_320:
        fb = framebuffer320;
        break;
    default:
        return;
    }

    if(fb->needsUpdate())
        fb->update();
}

void GL_ClearFramebuffer(glfbwhich_t which, unsigned int clearColor)
{
    TextureResource *fb;

    switch(which)
    {
    case FB_160:
        fb = framebuffer160;
        break;
    case FB_320:
        fb = framebuffer320;
        break;
    default:
        return;
    }

    GL_ClearTextureResource(fb, clearColor);
}

void GL_FramebufferSetUpdated(glfbwhich_t which)
{
    switch(which)
    {
    case FB_160:
        framebuffer160->setUpdated();
        break;
    case FB_320:
        framebuffer320->setUpdated();
        break;
    default:
        break;
    }
}

void GL_AddFramebuffer(glfbwhich_t which)
{
    TextureResource *fb;

    switch(which)
    {
    case FB_160:
        fb = framebuffer160;
        g_renderer->AddDrawCommand(fb, 0, 2, CALICO_ORIG_SCREENWIDTH, CALICO_ORIG_GAMESCREENHEIGHT);
        break;
    case FB_320:
        fb = framebuffer320;
        g_renderer->AddDrawCommand(fb, 0, 0, CALICO_ORIG_SCREENWIDTH, CALICO_ORIG_SCREENHEIGHT);
        break;
    default:
        return;
    }
}

// EOF
