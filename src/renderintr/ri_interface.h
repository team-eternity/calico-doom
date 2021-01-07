/*
  CALICO
  
  Renderer Interface
  
  The MIT License (MIT)
  
  Copyright (c) 2021 James Haley
  
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

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum glrestype_e
{
   RES_FRAMEBUFFER, // a raw framebuffer of specified dimensions
   RES_8BIT,        // 8-bit Jag resource
   RES_8BIT_PACKED  // 8-bit packed Jag resource
} glrestype_t;

typedef enum glfbwhich_e
{
   FB_160,
   FB_320
} glfbwhich_t;

typedef struct renderintr_t
{
    // initialization
    void  (*InitRenderer)(int w, int h);

    // framebuffers
    void  (*InitFramebufferTextures)(void);
    void *(*GetFramebuffer)(glfbwhich_t which);
    void  (*UpdateFramebuffer)(glfbwhich_t which);
    void  (*ClearFramebuffer)(glfbwhich_t which, unsigned int clearColor);
    void  (*FramebufferSetUpdated)(glfbwhich_t which);
    void  (*AddFramebuffer)(glfbwhich_t which);

    // refresh
    void  (*RenderFrame)(void);

    // textures
    void *(*NewTextureResource)(
        const char *lumpname, void *data,
        unsigned int width, unsigned int height,
        glrestype_t restype, int palshift
    );
    void *(*TextureResourceGetFramebuffer)(glfbwhich_t which);
    void *(*CheckForTextureResource)(const char *name);
    void  (*UpdateTextureResource)(void *resource);
    void  (*TextureResourceSetUpdated)(void *resource);
    unsigned int *(*GetTextureResourceStore)(void *resource);
    void  (*ClearTextureResource)(void *resource, unsigned int clearColor);
    
    // draw commands
    void  (*AddDrawCommand)(void *res, int x, int y, unsigned int w, unsigned int h);
    void  (*AddLateDrawCommand)(void *res, int x, int y, unsigned int w, unsigned int h);
} renderintr_t;

extern renderintr_t *g_renderer;

#if defined(__cplusplus)
}
#endif

// EOF
