/*
  CALICO
  
  OpenGL 4 rendering
  
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

#include "glm/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "../elib/elib.h"
#include "../elib/configfile.h"
#include "../elib/m_argv.h"
#include "../elib/misc.h"
#include "../hal/hal_types.h"
#include "../hal/hal_input.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "../rb/rb_draw.h"
#include "../rb/rb_main.h"
#include "../rb/rb_texture.h"
#include "../rb/valloc.h"
#include "../rb/rb_shader.h"
#include "../rb/rb_vao.h"
#include "../renderintr/ri_interface.h"
#include "../gl/gl_framebuffer.h"
#include "../gl/gl_textures.h"
#include "../gl/gl_drawcmd.h"

using uint = unsigned int;

//=============================================================================
//
// Shaders
//

static rbProgram defaultshader;

VALLOCATION(defaultshader)
{
    defaultshader.abandonProgram();
}

//
// Allow specification of the folder path from which to get shaders
//
static qstring GetShaderPath()
{
    int shaderarg;
    if((shaderarg = M_GetArgParameters("-shaderpath", 1)) != 0)
        return qstring(myargv[shaderarg]);
    else
        return qstring(hal_medialayer.getBaseDirectory()) / "shaders";
}

//
// Get a user-specified shader file name, or use the default.
//
static qstring GetShaderName()
{
    int shaderarg;
    if((shaderarg = M_GetArgParameters("-shader", 1)) != 0)
        return qstring(myargv[shaderarg]);
    else
        return qstring("default");
}

//
// Get proper file extension for the specified type of shader
//
static inline constexpr const char *ExtForShaderType(const rbShader::type_e type)
{
    return (type == rbShader::TYPE_FRAGMENT) ? ".frag" : ".vert";
}

//
// Figure out the path and name of a shader file to use with possible
// customization by the user. If a .vert or .frag file isn't found with
// the given name, the default will be tried instead.
//
static qstring GetShaderFile(rbShader::type_e type)
{
    const qstring     path = GetShaderPath();
    const qstring     name = GetShaderName();
    const char *const ext  = ExtForShaderType(type);

    qstring fullfn = path / (name + ext);

    if(!hal_platform.fileExists(fullfn.constPtr()))
        fullfn = path / (qstring("default") + ext);

    return fullfn;
}

//
// Load shaders
//
static void GL4_LoadDefaultShaders()
{
    if(!RB_InitShaderSupport())
        hal_platform.fatalError("Could not initialize shader support");

    rbShader frg { rbShader::TYPE_FRAGMENT };
    rbShader vtx { rbShader::TYPE_VERTEX   };

    const qstring fragShaderName = GetShaderFile(rbShader::TYPE_FRAGMENT);
    const qstring vertShaderName = GetShaderFile(rbShader::TYPE_VERTEX);

    // load shaders
    if(!frg.loadFromFile(fragShaderName.constPtr()))
        hal_platform.fatalError("Could not load or compile fragment shader %s", fragShaderName.constPtr());
    if(!vtx.loadFromFile(vertShaderName.constPtr()))
        hal_platform.fatalError("Could not load or compile vertex shader %s", vertShaderName.constPtr());

    // create program
    defaultshader.createProgram();

    // attach shaders
    if(!defaultshader.attachFragmentShader(std::move(frg)))
        hal_platform.fatalError("Could not attach fragment shader to program");
    if(!defaultshader.attachVertexShader(std::move(vtx)))
        hal_platform.fatalError("Could not attach vertex shader to program");

    // link
    if(!defaultshader.link())
        hal_platform.fatalError("Failed to link shader program");
}

//=============================================================================
//
// Refresh
//

#define MAX_CMD_TRANSFORMS 128

static float vertices_xy_uv[4 * 2] = 
{ 
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f
};

static rbVAO vao;
static rbVBO vertexVBO;
static int modelMatrixLoc;

VALLOCATION(vao)
{
    vao.abandonVAO();
    vertexVBO.abandonVBO();
    modelMatrixLoc = -1;
}

static glm::mat4 projectionMatrix;
static glm::mat4 modelMatrix;

//
// Draw a rect from game coordinates (gx, gy) to translated framebuffer
// coordinates with the provided information.
//
static void GL4_ExecuteDrawCommand(int gx, int gy, unsigned int gw, unsigned int gh, rbTexture &tx)
{
    float sx, sy, sw, sh;

    // transform coordinates into screen space
    hal_video.transformGameCoord2f(gx, gy, &sx, &sy);

    // scale width and height into screen space
    sw = float(hal_video.transformWidth(gw));
    sh = float(hal_video.transformHeight(gh));

    modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::translate(modelMatrix, { sx, sy, 0.0f });
    modelMatrix = glm::scale(modelMatrix, { sw, sh, 1.0f });

    if(modelMatrixLoc < 0)
        modelMatrixLoc = defaultshader.getUniformLocation("modelMatrix");
    if(modelMatrixLoc >= 0)
        rbProgram::UniformMatrix4fv(modelMatrixLoc, 1, false, &modelMatrix[0][0]);

    tx.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void GL4_RenderFrame()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // bind default shader
    defaultshader.bind();

    // execute draw commands
    GL_ExecuteDrawCommands();
    GL_ClearDrawCommands();

    // flip/refresh display
    hal_video.endFrame();
}

//=============================================================================
//
// Initialization
//

static void GL4_InitRenderer(int w, int h)
{
    // set draw command callback
    GL_SetDrawCommandFunc(GL4_ExecuteDrawCommand);

    // set viewport
    glViewport(0, 0, GLsizei(w), GLsizei(h));

    // load VAO/VBO support
    if(!RB_InitVAOSupport())
        hal_platform.fatalError("Could not initialize VAO/VBO support");

    // load default shaders
    GL4_LoadDefaultShaders();

    // bind default shader
    defaultshader.bind();

    // compute orthonormal projection matrix
    projectionMatrix = glm::ortho(0.0f, float(w), float(h), 0.0f, -1.0f, 1.0f);
    const int projMatLocation = defaultshader.getUniformLocation("projectionMatrix");
    if(projMatLocation >= 0)
        rbProgram::UniformMatrix4fv(projMatLocation, 1, false, &projectionMatrix[0][0]);

    // init VAO
    vao.generate();
    vao.bind();

    // init vertex VBO
    vertexVBO.generate();
    vertexVBO.bind();
    vertexVBO.bufferData(sizeof(vertices_xy_uv), vertices_xy_uv);
    const int verticesLoc = defaultshader.getAttribLocation("position");
    if(verticesLoc >= 0)
    {
        const uint uvl = uint(verticesLoc);
        vao.enableVertexAttribArray(uvl);
        vertexVBO.vertexAttribPointer(uvl, 2, rbVBO::FLOAT, false, 0, nullptr);
    }
    vertexVBO.unbind();

    // disable depth buffer test
    RB_SetState(RB_GLSTATE_DEPTHTEST, false);
}

static renderintr_t gl4Renderer
{
    GL4_InitRenderer,

    GL_InitFramebufferTextures,
    GL_GetFramebuffer,
    GL_UpdateFramebuffer,
    GL_ClearFramebuffer,
    GL_FramebufferSetUpdated,
    GL_AddFramebuffer,

    GL4_RenderFrame,

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

void GL4_SelectRenderer()
{
    g_renderer = &gl4Renderer;
}

// EOF
