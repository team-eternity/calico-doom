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

#ifndef GL_RENDER_H__
#define GL_RENDER_H__

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

#ifdef __cplusplus
extern "C" {
#endif

void  GL_InitFramebufferTextures(void);
void *GL_GetFramebuffer(glfbwhich_t which);
void  GL_UpdateFramebuffer(glfbwhich_t which);
void  GL_ClearFramebuffer(glfbwhich_t which, unsigned int clearColor);
void  GL_FramebufferSetUpdated(glfbwhich_t which);
void  GL_AddFramebuffer(glfbwhich_t which);
void  GL_RenderFrame(void);

void *GL_NewTextureResource(const char *lumpname, void *data,
                            unsigned int width, unsigned int height,
                            glrestype_t restype, int palshift);
void *GL_TextureResourceGetFramebuffer(glfbwhich_t which);
void *GL_CheckForTextureResource(const char *name);

void          GL_UpdateTextureResource(void *resource);
void          GL_TextureResourceSetUpdated(void *resource);
unsigned int *GL_GetTextureResourceStore(void *resource);
void          GL_ClearTextureResource(void *resource, unsigned int clearColor);
void          GL_AddDrawCommand(void *res, int x, int y, unsigned int w, unsigned int h);
void          GL_AddLateDrawCommand(void *res, int x, int y, unsigned int w, unsigned int h);

#ifdef __cplusplus
}
#endif

#endif

// EOF

