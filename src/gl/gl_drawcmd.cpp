/*
  CALICO
  
  OpenGL draw command system
  
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
#include "gl_textures.h"
#include "gl_drawcmd.h"

//=============================================================================
//
// Draw Command List
//

struct drawcommand_t
{
    BDListItem<drawcommand_t> links; // list links
    TextureResource *res;            // source graphics
    int x, y;                        // where to put it (in 320x224 coord space)
    unsigned int w, h;               // size (in 320x224 coord space)
};

static BDList<drawcommand_t, &drawcommand_t::links> drawCommands;
static BDList<drawcommand_t, &drawcommand_t::links> lateDrawCommands;

static cmdfunc_t cmdfunc;

//
// Set the callback routine for executing draw commands
//
void GL_SetDrawCommandFunc(cmdfunc_t fn)
{
    cmdfunc = fn;
}

//
// Add a texture resource to the draw command list. If the resource needs its
// GL texture updated, it will be done now.
//
void GL_AddDrawCommand(void *res, int x, int y, unsigned int w, unsigned int h)
{
    if(!res)
        return;

    const auto dc = estructalloc(drawcommand_t, 1);

    dc->res = static_cast<TextureResource *>(res);
    dc->x = x;
    dc->y = y;
    dc->w = w;
    dc->h = h;

    drawCommands.insert(dc);

    if(dc->res->needsUpdate())
        dc->res->update();
}

//
// Add a late draw command, which will draw after everything else.
//
void GL_AddLateDrawCommand(void *res, int x, int y, unsigned int w, unsigned int h)
{
    if(!res)
        return;

    const auto dc = estructalloc(drawcommand_t, 1);

    dc->res = static_cast<TextureResource *>(res);
    dc->x = x;
    dc->y = y;
    dc->w = w;
    dc->h = h;

    lateDrawCommands.insert(dc);

    if(dc->res->needsUpdate())
        dc->res->update();
}

void GL_ClearDrawCommands()
{
    while(!drawCommands.empty())
    {
        drawcommand_t *const cmd = drawCommands.first()->bdObject;
        drawCommands.remove(cmd);
        efree(cmd);
    }
}

void GL_ExecuteDrawCommands()
{
    const BDListItem<drawcommand_t> *item;

    // fold in the late draw commands now
    while((item = lateDrawCommands.first()) != &lateDrawCommands.head)
    {
        lateDrawCommands.remove(item->bdObject);
        drawCommands.insert(item->bdObject);
    }

    for(item = drawCommands.first(); item != &drawCommands.head; item = item->bdNext)
    {
        const drawcommand_t *const cmd = item->bdObject;
        cmdfunc(cmd->x, cmd->y, cmd->w, cmd->h, cmd->res->getTexture());
    }
}

// EOF
