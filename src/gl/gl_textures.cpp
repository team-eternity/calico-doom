/*
  CALICO
  
  OpenGL texture management
  
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
#include "gl_framebuffer.h"

//=============================================================================
//
// Graphic resources
//

//
// Resource hive for graphics
//
static ResourceHive graphics;

//
// Abandon old texture IDs and regenerate all textures in the resource hive 
// if a resolution change occurs.
//
VALLOCATION(graphics)
{
    graphics.forEachOfType<TextureResource>([] (TextureResource *tr) {
        tr->getTexture().abandonTexture();
        tr->generate();
    });
}

extern "C" unsigned short *palette8;

//
// Convert 8-bit Jaguar graphic to 32-bit color
//
static uint32_t *GL_8bppTo32bpp(void *data, unsigned int w, unsigned int h)
{
    uint32_t *buffer = new (std::nothrow) uint32_t[w * h];

    if(buffer)
    {
        byte *src = static_cast<byte *>(data);
        for(unsigned int p = 0; p < w * h; p++)
            buffer[p] = src[p] ? CRYToRGB[palette8[src[p]]] : 0;
    }

    return buffer;
}

//
// Convert 8-bit packed Jaguar graphic to 32-bit color
//
static uint32_t *GL_8bppPackedTo32bpp(void *data,
                                      unsigned int w, unsigned int h,
                                      int palshift)
{
    uint32_t *buffer = new (std::nothrow) uint32_t[w * h];

    if(buffer)
    {
        byte *src = static_cast<byte *>(data);
        for(unsigned int p = 0; p < w * h / 2; p++)
        {
            byte pix[2];
            pix[0] = (palshift << 1) + ((src[p] & 0xF0) >> 4);
            pix[1] = (palshift << 1) + (src[p] & 0x0F);

            buffer[p * 2] = pix[0] ? CRYToRGB[palette8[pix[0]]] : 0;
            buffer[p * 2 + 1] = pix[1] ? CRYToRGB[palette8[pix[1]]] : 0;
        }
    }

    return buffer;
}

//
// Call from game code to create a texture resource from a graphic
//
void *GL_NewTextureResource(const char *name, void *data,
                            unsigned int width, unsigned int height,
                            glrestype_t restype, int palshift)
{
    TextureResource *tr;

    if(!(tr = graphics.findResourceType<TextureResource>(name)))
    {
        uint32_t *pixels = nullptr;

        switch(restype)
        {
        case RES_FRAMEBUFFER:
            pixels = new (std::nothrow) uint32_t[width * height];
            std::memset(pixels, 0, width * height * sizeof(uint32_t));
            break;
        case RES_8BIT:
            pixels = GL_8bppTo32bpp(data, width, height);
            break;
        case RES_8BIT_PACKED:
            pixels = GL_8bppPackedTo32bpp(data, width, height, palshift);
            break;
        }
        if(pixels)
        {
            tr = new (std::nothrow) TextureResource(name, pixels, width, height);
            if(tr)
            {
                tr->generate();
                graphics.addResource(tr);
            }
        }
    }

    return tr;
}

//
// Call from game code to check if a texture resource exists. If so, it will be
// returned. It will not be created if it does not.
//
void *GL_CheckForTextureResource(const char *name)
{
    return graphics.findResourceType<TextureResource>(name);
}

//
// Call from game code to draw to backing store of a texture resource
//
void GL_UpdateTextureResource(void *resource)
{
    auto tr = static_cast<TextureResource *>(resource);

    if(tr->needsUpdate())
        tr->update();
}

//
// Call from game code to get the 32-bit backing store of a texture resource
//
unsigned int *GL_GetTextureResourceStore(void *resource)
{
    return static_cast<TextureResource *>(resource)->getPixels();
}

//
// Set the indicated texture resource as needing its GL texture updated.
//
void GL_TextureResourceSetUpdated(void *resource)
{
    static_cast<TextureResource *>(resource)->setUpdated();
}

//
// Get a framebuffer as a GL resource
//
void *GL_TextureResourceGetFramebuffer(glfbwhich_t which)
{
    switch(which)
    {
    case FB_160:
        return framebuffer160;
    case FB_320:
        return framebuffer320;
    default:
        return nullptr;
    }
}

//
// Clear the backing store of a texture resource.
//
void GL_ClearTextureResource(void *resource, unsigned int clearColor)
{
    auto rez = static_cast<TextureResource *>(resource);
    if(rez)
    {
        uint32_t  *buffer = rez->getPixels();
        rbTexture &tex = rez->getTexture();
        for(unsigned int i = 0; i < tex.getWidth() * tex.getHeight(); i++)
            buffer[i] = clearColor;
        rez->setUpdated();
    }
}

// EOF
