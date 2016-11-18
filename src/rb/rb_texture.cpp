/*
  CALICO

  OpenGL renderer texture functions

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

#include <cstring>

#include "../hal/hal_ml.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "rb_main.h"
#include "rb_texture.h"
#include "valloc.h"

//
// Config Vars
//
bool rb_linear_filtering = false;

// CALICO_TODO: config
#if 0
static CfgItem cfgLinearFiltering("linear_filtering", &rb_linear_filtering);
#endif

//
// PBOs
//

// PBO extension function pointers
static bool use_arb_pbo;
static bool arb_pbo_loaded;
static PFNGLGENBUFFERSARBPROC    pglGenBuffersARB    = nullptr;
static PFNGLDELETEBUFFERSARBPROC pglDeleteBuffersARB = nullptr;
static PFNGLBINDBUFFERARBPROC    pglBindBufferARB    = nullptr;
static PFNGLBUFFERDATAARBPROC    pglBufferDataARB    = nullptr;
static PFNGLMAPBUFFERARBPROC     pglMapBufferARB     = nullptr;
static PFNGLUNMAPBUFFERARBPROC   pglUnmapBufferARB   = nullptr;

#define GETPROC(ptr, name) \
   ptr = reinterpret_cast<decltype(ptr)>(hal_video.getGLProcAddress(name)); \
   extension_ok = (extension_ok && ptr != nullptr)

//
// Must reload extension pointers if game video mode changes
//
VALLOCATION(arb_pbo_loaded)
{
   arb_pbo_loaded = false;
}

static void RB_loadPBOExtension()
{
   if(arb_pbo_loaded)
      return;

   auto extensions   = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
   bool extension_ok = true;
   bool have_arb_pbo = (std::strstr(extensions, "GL_ARB_pixel_buffer_object") != nullptr);
   
   if(have_arb_pbo)
   {
      GETPROC(pglGenBuffersARB,    "glGenBuffersARB");
      GETPROC(pglDeleteBuffersARB, "glDeleteBuffersARB");
      GETPROC(pglBindBufferARB,    "glBindBufferARB");
      GETPROC(pglBufferDataARB,    "glBufferDataARB");
      GETPROC(pglMapBufferARB,     "glMapBufferARB");
      GETPROC(pglUnmapBufferARB,   "glUnmapBufferARB");

      // use the extension if all procedures were found
      use_arb_pbo = extension_ok;
      if(use_arb_pbo)
         hal_platform.debugMsg("Successfully loaded GL_ARB_pixel_buffer_object\n");
   }
   else
      use_arb_pbo = false;

   // even if load fails, still set this, so we don't try to load it constantly
   // on a machine where it is not supported.
   arb_pbo_loaded = true;
}


//
// Get next higher power of two, which will be a suitable texture
// dimension for standard OpenGL textures.
//
unsigned int RB_MakeTextureDimension(unsigned int i)
{
   if(i)
   {
      --i;
      i |= i >> 1;
      i |= i >> 2;
      i |= i >> 4;
      i |= i >> 8;
      i |= i >> 16;
      ++i;
   }

   return i;
}

//
// Default constructor
//
rbTexture::rbTexture()
   : width(0), height(0), clampMode(TC_CLAMP), filterMode(TF_NEAREST), 
     colorMode(TCR_RGBA), texid(0), streaming(false), pboid(0), 
     dirtypbo(false)
{
}

//
// Move constructor
//
rbTexture::rbTexture(rbTexture &&other)
{
   // copy basic properties
   width      = other.width;
   height     = other.height;
   clampMode  = other.clampMode;
   filterMode = other.filterMode;
   colorMode  = other.colorMode;
   streaming  = other.streaming;
   dirtypbo   = other.dirtypbo;

   // move texture and pbo IDs to the target object
   texid = other.texid;
   pboid = other.pboid;
   other.texid = 0;
   other.pboid = 0;
}

//
// Destructor
//
rbTexture::~rbTexture()
{
   // there's no need to destroy GL entities when the game is exiting;
   // the destruction of the context will take them with it.
   if(hal_medialayer.isExiting())
      return;

   deleteTexture();
   deletePBO();
}

//
// Setup color mode and width/height.
//
void rbTexture::init(texColorMode_t color, unsigned int w, unsigned int h)
{
   colorMode  = color;
   width      = w;
   height     = h;
}

//
// Setup width/height and PBO for streaming texture upload
//
void rbTexture::initStreaming(unsigned int w, unsigned int h)
{
   // make sure PBO extension is loaded
   RB_loadPBOExtension();

   colorMode  = TCR_BGRA;
   width      = w;
   height     = h;
   streaming  = true;
}

//
// Set GL texture parameters for clamping and filtering
//
void rbTexture::setTexParameters()
{
   unsigned int clamp;
   unsigned int filter;

   switch(this->clampMode)
   {
   case TC_CLAMP:
      clamp = GL_CLAMP_TO_EDGE;
      break;

   case TC_CLAMP_BORDER:
      clamp = GL_CLAMP_TO_BORDER;
      break;

   case TC_REPEAT:
      clamp = GL_REPEAT;
      break;

   case TC_MIRRORED:
      clamp = GL_MIRRORED_REPEAT;
      break;

   default:
      return;
   }

   switch(this->filterMode)
   {
   case TF_LINEAR:
      filter = GL_LINEAR;
      break;

   case TF_NEAREST:
      filter = GL_NEAREST;
      break;

   default:
      return;
   }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     clamp);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     clamp);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}

//
// Change GL texture parameters for clamping and filtering.
//
void rbTexture::changeTexParameters(texClampMode_t clamp, texFilterMode_t filter)
{
   if(this->clampMode == clamp && this->filterMode == filter)
      return;

   this->clampMode = clamp;
   this->filterMode = filter;

   setTexParameters();
}

//
// Bind this texture if it is not the current globally bound texture.
//
void rbTexture::bind(bool forDraw)
{
   dtexture tid;
   dtexture currentTexture;
   bool needsUpdate = (forDraw && use_arb_pbo && pboid && dirtypbo);

   tid = this->texid;
   currentTexture = rbState.currentTexture;

   if(tid == currentTexture && !needsUpdate)
      return;

   glBindTexture(GL_TEXTURE_2D, tid);

   // streaming texture may need to update from its PBO
   if(needsUpdate)
   {
      pglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboid);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
      dirtypbo = false;
      pglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   }

   rbState.currentTexture = tid;
}

//
// Static routine; unbind any currently bound texture and update global state
// to reflect that no texture is currently bound.
//
void rbTexture::Unbind()
{
   rbState.currentTexture = 0;
   glBindTexture(GL_TEXTURE_2D, 0);
}

//
// Delete the owned OpenGL texture, if one exists.
//
void rbTexture::deleteTexture()
{
   if(this->texid == 0)
      return;

   if(this->texid == rbState.currentTexture)
      Unbind();

   glDeleteTextures(1, &this->texid);
   this->texid = 0;
}

//
// Delete the owned OpenGL pixel buffer object, if one exists.
//
void rbTexture::deletePBO()
{
   if(!use_arb_pbo || this->pboid == 0)
      return;

   pglDeleteBuffersARB(1, &this->pboid);
   this->pboid = 0;
}

//
// Forget the GL texture ID belonging to this rbTexture object. This
// is appropriate only in a context where the texture was destroyed in another
// fashion (such as death of the owning GL context)
//
void rbTexture::abandonTexture()
{
   this->texid = 0;
   this->pboid = 0;
}

static inline GLenum RB_glTexForTCR(rbTexture::texColorMode_t tcm)
{
   return 
      ((tcm == rbTexture::TCR_BGRA) ? GL_BGRA :
       (tcm == rbTexture::TCR_RGBA) ? GL_RGBA : 
       GL_RGB);
}

//
// Create a GL texture and upload image data to it.
//
void rbTexture::upload(void *data, texClampMode_t clamp, texFilterMode_t filter)
{
   // delete any previous created resources
   deleteTexture();
   deletePBO();

   if(filter == TF_AUTO) // use global filtering mode?
      filter = (rb_linear_filtering ? TF_LINEAR : TF_NEAREST);

   this->clampMode  = clamp;
   this->filterMode = filter;

   glGenTextures(1, &this->texid);

   // if streaming, create PBO
   if(streaming)
   {
      // ensure extension is loaded
      RB_loadPBOExtension();
      if(use_arb_pbo)
         pglGenBuffersARB(1, &this->pboid);
   }

   if(this->texid == 0)
   {
      // renderer is not initialized yet
      return;
   }

   bind(false);

   if(data)
   {
      void *src = data;

      if(use_arb_pbo && pboid)
      {
         // streaming textures should upload to PBO
         GLvoid *ptr;
         pglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboid);
         pglBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, width*height*4, 0, GL_STREAM_DRAW_ARB);
         if((ptr = pglMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB)))
         {
            std::memcpy(ptr, src, width*height*4);
            dirtypbo = true;
            pglUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
         }
         pglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
      }
      else
      {
         glTexImage2D(
            GL_TEXTURE_2D,
            0,
            (this->colorMode == TCR_RGB) ? GL_RGB8 : GL_RGBA8,
            this->width,
            this->height,
            0,
            RB_glTexForTCR(this->colorMode),
            GL_UNSIGNED_BYTE,
            src
         );
      }
   }

   glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

   setTexParameters();
   Unbind();
}

//
// Upload new contents to an existing texture.
//
void rbTexture::update(void *data)
{
   if(this->texid == 0)
      return;

   bind(false);

   void *src = data;

   if(use_arb_pbo && pboid)
   {
      // streaming textures should update to PBO
      GLvoid *ptr;
      pglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboid);
      pglBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, width*height*4, 0, GL_STREAM_DRAW_ARB);
      if((ptr = pglMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB)))
      {
         std::memcpy(ptr, src, width*height*4);
         dirtypbo = true;
         pglUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
      }
      pglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   }
   else
   {
      glTexSubImage2D(
         GL_TEXTURE_2D,
         0,
         0,
         0,
         this->width,
         this->height,
         RB_glTexForTCR(this->colorMode),
         GL_UNSIGNED_BYTE,
         src
      );
   }
}

//
// Populate an rbTexture instance with data from the framebuffer.
//
void rbTexture::fromFrameBuffer()
{
   int sw = 0, sh = 0;
   unsigned int usw, ush;
   hal_video.getWindowSize(&sw, &sh);

   usw = static_cast<unsigned int>(sw);
   ush = static_cast<unsigned int>(sh);

   if(this->colorMode != TCR_RGBA || this->width != usw || this->height != ush)
   {
      deleteTexture();
      deletePBO();

      if(this->streaming)
         this->streaming = false;

      this->colorMode  = TCR_RGBA;
      this->clampMode  = TC_CLAMP;
      this->filterMode = TF_NEAREST;

      this->width  = usw;
      this->height = ush;
   }
   if(!this->texid)
      glGenTextures(1, &this->texid);
   
   bind(false);
   RB_SetReadBuffer(RB_GLBUFFER_BACK);
   glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, usw, ush, 0);
   setTexParameters();
}

// CALICO_TODO: screenshot routine
#if 0
//
// Write out the texture as a PNG file.
//
bool rbTexture::writePNG(const char *filename)
{
   GLint width, height, intfmt;
   GLint numbytes = 0;
   rbbyte *pixels;
   bool res;

   if(this->texid == 0)
      return false;

   bind();
   glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &intfmt);
   glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,      &width);
   glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT,     &height);

   switch(intfmt)
   {
   case GL_RGB8:
      numbytes = width * height * 3;
      break;
   case GL_RGBA8:
      numbytes = width * height * 4;
      break;
   default: // not supported for PNG write
      return false;
   }

   if(!numbytes)
      return false;

   pixels = emalloc(rbbyte, numbytes);
   glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

   res = WritePNGFile(filename, reinterpret_cast<uint32_t *>(pixels), width, height);

   efree(pixels);

   return res;
}
#endif

// EOF

