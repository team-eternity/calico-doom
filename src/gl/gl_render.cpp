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
#include "../rb/rb_common.h"
#include "../rb/rb_draw.h"
#include "../rb/rb_main.h"
#include "../rb/rb_texture.h"
#include "../rb/valloc.h"
#include "../jagcry.h"
#include "gl_render.h"
#include "resource.h"

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
static void GL_drawGameRect(int gx, int gy, 
                            unsigned int gw, unsigned int gh,
                            rbTexture &tx, vtx_t v[4])
{
   float sx, sy, sw, sh;
   
   RB_SetVertexColors(v, 4, 0xff, 0xff, 0xff, 0xff);
   RB_DefTexCoords(v, &tx);
   
   // bind texture
   tx.bind();

   // transform coordinates into screen space
   hal_video.transformGameCoord2f(gx, gy, &sx, &sy);

   // scale width and height into screen space
   sw = float(hal_video.transformWidth(int(gw)));
   sh = float(hal_video.transformHeight(int(gh)));

   GL_initVtxCoords(v, sx, sy, sw, sh);

   GL_drawRectImmediate(v);
}

//=============================================================================
//
// Graphic resources
//

//
// Resource hive for graphics
//
static ResourceHive graphics;

//
// Texture resource class
//
class TextureResource : public Resource
{
protected:
   rbTexture    m_tex;
   unsigned int m_width;
   unsigned int m_height;
   std::unique_ptr<uint32_t []> m_data;

public:
   TextureResource(const char *tag, uint32_t *pixels, unsigned int w, unsigned int h)
      : Resource(tag), m_tex(), m_data(pixels), m_width(w), m_height(h)
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
   }

   rbTexture &getTexture() { return m_tex;        }
   uint32_t  *getPixels()  { return m_data.get(); }
};

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
// Convert 8-bit packed Jaguar graphic to 32-bit color
//
static uint32_t *GL_8bppPackedTo32bpp(void *data, 
                                      unsigned int w, unsigned int h,
                                      int palshift)
{
   uint32_t *buffer = new (std::nothrow) uint32_t [w * h];

   if(buffer)
   {
      byte *src = static_cast<byte *>(data);
      for(unsigned int p = 0; p < w * h / 2; p++)
      {
         byte pix[2];
         pix[0] = (palshift << 1) + ((src[p] & 0xF0) >> 4);
         pix[1] = (palshift << 1) +  (src[p] & 0x0F);

         buffer[p*2  ] = pix[0] ? CRYToRGB[palette8[pix[0]]] : 0;
         buffer[p*2+1] = pix[1] ? CRYToRGB[palette8[pix[1]]] : 0;
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
         pixels = new (std::nothrow) uint32_t [width * height];
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
// Call from game code to draw to backing store of a texture resource
//
void GL_UpdateTextureResource(void *resource)
{
   static_cast<TextureResource *>(resource)->update();
}

//
// Call from game code to get the 32-bit backing store of a texture resource
//
unsigned int *GL_GetTextureResourceStore(void *resource)
{
   return static_cast<TextureResource *>(resource)->getPixels();
}

//=============================================================================
//
// Draw Command List
//

struct drawcommand_t
{
   BDListItem<drawcommand_t> links; // list links
   TextureResource *res;            // source graphics
   int x, y;                        // where to put it (in 640x480 coord space)
   unsigned int w, h;               // size (in 640x480 coord space)
};

static BDList<drawcommand_t, &drawcommand_t::links> drawCommands;

void GL_AddDrawCommand(void *res, int x, int y, unsigned int w, unsigned int h)
{
   if(!res)
      return;
   
   auto dc = estructalloc(drawcommand_t, 1);

   dc->res = static_cast<TextureResource *>(res);
   dc->x   = x;
   dc->y   = y;
   dc->w   = w;
   dc->h   = h;

   drawCommands.insert(dc);
}

void GL_ClearDrawCommands(void)
{
   while(!drawCommands.empty())
   {
      drawcommand_t *cmd = drawCommands.first()->bdObject;
      drawCommands.remove(cmd);
      efree(cmd);
   }
}

static void GL_executeDrawCommands(void)
{
   BDListItem<drawcommand_t> *item;
   vtx_t v[4];

   for(item = drawCommands.first(); item != &drawCommands.head; item = item->bdNext)
   {
      drawcommand_t *cmd = item->bdObject;
      GL_drawGameRect(cmd->x, cmd->y, cmd->w, cmd->h, cmd->res->getTexture(), v);
   }
}

//=============================================================================
//
// Software Framebuffer
//

static TextureResource *framebuffer;

//
// Create the GL texture handle for the framebuffer texture
//
void GL_InitFramebufferTexture(void)
{
   framebuffer = static_cast<TextureResource *>(
      GL_NewTextureResource(
         "framebuffer",
         nullptr,
         CALICO_ORIG_SCREENWIDTH,
         CALICO_ORIG_SCREENHEIGHT,
         RES_FRAMEBUFFER,
         0
      )
   );
   if(!framebuffer)
      hal_platform.fatalError("Could not create 640x480 framebuffer texture");
}

//
// Return the pointer to the local 32-bit framebuffer
//
void *GL_GetFramebuffer(void)
{
   return framebuffer->getPixels();
}

void GL_UpdateFramebuffer(void)
{
   framebuffer->update();
}

//=============================================================================
//
// Refresh
//

void GL_RenderFrame(void)
{
   GL_AddDrawCommand(framebuffer, 0, 0, CALICO_ORIG_SCREENWIDTH, CALICO_ORIG_SCREENHEIGHT);
   GL_executeDrawCommands();
   GL_ClearDrawCommands();
   hal_video.endFrame();

   // CALICO_TODO: TEMP: clear framebuffer until everything is drawing
   //for(int i = 0; i < CALICO_ORIG_SCREENWIDTH*CALICO_ORIG_SCREENHEIGHT; i++)
    //  framebuffer->getPixels()[i] = D_RGBA(0x60, 0x60, 0x60, 0xff);
}

// EOF

