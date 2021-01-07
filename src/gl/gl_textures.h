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

#pragma once

#include "../rb/rb_texture.h"
#include "../renderintr/ri_interface.h"
#include "resource.h"


//
// Texture resource class
//
class TextureResource : public Resource
{
protected:
    rbTexture    m_tex;
    unsigned int m_width;
    unsigned int m_height;
    bool         m_needUpdate;
    std::unique_ptr<uint32_t[]> m_data;

public:
    TextureResource(const char *tag, uint32_t *pixels, unsigned int w, unsigned int h)
        : Resource(tag), m_tex(), m_width(w), m_height(h), m_needUpdate(false),
        m_data(pixels)
    {
    }

    void generate()
    {
        m_tex.init(rbTexture::TCR_RGBA, m_width, m_height);
        m_tex.upload(m_data.get(), rbTexture::TC_CLAMP, rbTexture::TF_AUTO);
    }

    void update()
    {
        m_tex.update(m_data.get());
        m_needUpdate = false;
    }

    rbTexture  &getTexture() { return m_tex; }
    uint32_t   *getPixels() { return m_data.get(); }
    bool needsUpdate() const { return m_needUpdate; }
    void setUpdated() { m_needUpdate = true; }
};

void *GL_NewTextureResource(
    const char *name, void *data,
    unsigned int width, unsigned int height,
    glrestype_t restype, int palshift
);

void         *GL_TextureResourceGetFramebuffer(glfbwhich_t which);
void         *GL_CheckForTextureResource(const char *name);
void          GL_UpdateTextureResource(void *resource);
void          GL_TextureResourceSetUpdated(void *resource);
unsigned int *GL_GetTextureResourceStore(void *resource);
void          GL_ClearTextureResource(void *resource, unsigned int clearColor);

// EOF
