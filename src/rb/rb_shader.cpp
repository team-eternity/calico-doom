/*
  CALICO

  OpenGL Renderer Primitives

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

#include <memory>

#include "../hal/hal_ml.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "../elib/elib.h"
#include "../elib/misc.h"
#include "rb_main.h"
#include "rb_shader.h"

static PFNGLATTACHSHADERPROC      rbglAttachShader;
static PFNGLCOMPILESHADERPROC     rbglCompileShader;
static PFNGLCREATESHADERPROC      rbglCreateShader;
static PFNGLDELETEPROGRAMPROC     rbglDeleteProgram;
static PFNGLDELETESHADERPROC      rbglDeleteShader;
static PFNGLGETPROGRAMINFOLOGPROC rbglGetProgramInfoLog;
static PFNGLGETPROGRAMIVPROC      rbglGetProgramiv;
static PFNGLGETSHADERINFOLOGPROC  rbglGetShaderInfoLog;
static PFNGLGETSHADERIVPROC       rbglGetShaderiv;
static PFNGLISPROGRAMPROC         rbglIsProgram;
static PFNGLISSHADERPROC          rbglIsShader;
static PFNGLLINKPROGRAMPROC       rbglLinkProgram;
static PFNGLSHADERSOURCEPROC      rbglShaderSource;
static PFNGLUSEPROGRAMPROC        rbglUseProgram;

//=============================================================================
// rbShader
//

//
// Output shader log info
//
void rbShader::outputLogInfo() const
{
    if(rbglIsShader(m_shaderID))
    {
        int maxLen = 0;
        rbglGetShaderiv(m_shaderID, GL_INFO_LOG_LENGTH, &maxLen);

        std::unique_ptr<char []> upBuffer { new char [maxLen] };
        GLsizei len = 0;
        rbglGetShaderInfoLog(m_shaderID, maxLen, &len, upBuffer.get());
        if(len > 0)
        {
            hal_platform.debugMsg("%s\n", upBuffer.get());
        }
    }
    else
    {
        hal_platform.debugMsg("rbShader::outputLogInfo: name %u is not a shader\n", m_shaderID);
    }
}

//
// Load shader from source string
//
bool rbShader::loadFromSource(const char *src)
{
    GLenum st;

    switch(m_type)
    {
    case TYPE_VERTEX:
        st = GL_VERTEX_SHADER;
        break;
    case TYPE_FRAGMENT:
        st = GL_FRAGMENT_SHADER;
        break;
    }

    m_shaderID = rbglCreateShader(st);
    if(!rbglIsShader(m_shaderID))
    {
        hal_platform.debugMsg("rbShader::loadFromSource: could not create GL shader object\n");
        return false;
    }

    rbglShaderSource(m_shaderID, 1, &src, nullptr);
    rbglCompileShader(m_shaderID);
    
    GLint compiled = GL_FALSE;
    rbglGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &compiled);
    if(compiled != GL_TRUE)
    {
        hal_platform.debugMsg("rbShader::loadFromSource: unable to compile shader %u\n\nSource:\n%s\n", m_shaderID, src);
        rbglDeleteShader(m_shaderID);
        m_shaderID = 0;
        return false;
    }

    return true;
}

//
// Load shader from disk file
//
bool rbShader::loadFromFile(const char *filename)
{
    EUniquePtr<char []> upSrc { M_LoadStringFromFile(filename) };
    if(!upSrc)
    {
        hal_platform.debugMsg("rbShader::loadFromFile: could not read '%s'\n", filename);
        return false;
    }

    return loadFromSource(upSrc.get());
}

//=============================================================================
// rbProgram
//

//
// Destructor
//
rbProgram::~rbProgram()
{
   // there's no need to destroy GL entities when the game is exiting;
   // the destruction of the context will take them with it.
   if(hal_medialayer.isExiting())
      return;

   deleteProgram();
}

//
// Delete the program and clear the ID
//
void rbProgram::deleteProgram()
{
    if(rbglIsProgram(m_programID))
    {
        rbglDeleteProgram(m_programID);
        m_programID = m_vtxShaderID = m_frgShaderID = 0;
    }
}

//
// Forget about the associated GL program ID
//
void rbProgram::abandonProgram()
{
    m_programID = m_vtxShaderID = m_frgShaderID = 0;
}

//
// Output program log info
//
void rbProgram::outputLogInfo() const
{
    if(rbglIsProgram(m_programID))
    {
        GLint maxLen = 0;
        rbglGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLen);

        std::unique_ptr<char []> upBuffer { new char [maxLen] };
        GLsizei len = 0;
        rbglGetProgramInfoLog(m_programID, maxLen, &len, upBuffer.get());
        if(len > 0)
        {
            hal_platform.debugMsg("%s\n", upBuffer.get());
        }
    }
    else
    {
        hal_platform.debugMsg("rbProgram::outputLogInfo: name %u is not a program\n", m_programID);
    }
}

//
// Attach a vertex shader to the program object
//
bool rbProgram::attachVertexShader(const rbShader &shader)
{
    unsigned int shaderID = shader.getShaderID();

    if(shader.getType() != rbShader::TYPE_VERTEX)
    {
        hal_platform.debugMsg("rbProgram::attachVertexShader: %u is not a vertex shader\n", shaderID);
        return false;
    }

    rbglAttachShader(m_programID, shaderID);
    m_vtxShaderID = shaderID;
    return true;
}

//
// Attach a fragment shader to the program object
//
bool rbProgram::attachFragmentShader(const rbShader &shader)
{
    unsigned int shaderID = shader.getShaderID();

    if(shader.getType() != rbShader::TYPE_FRAGMENT)
    {
        hal_platform.debugMsg("rbProgram::attachFragmentShader: %u is not a fragment shader\n", shaderID);
        return false;
    }

    rbglAttachShader(m_programID, shaderID);
    m_frgShaderID = shaderID;
    return true;
}

//
// Link the shader program
//
bool rbProgram::link()
{
    if(!rbglIsProgram(m_programID))
    {
        hal_platform.debugMsg("rbProgram::link: %u is not a program\n", m_programID);
        return false;
    }

    if(!rbglIsShader(m_vtxShaderID))
    {
        hal_platform.debugMsg("rbProgram::link: %u has invalid vertex shader %u attached\n", m_programID, m_vtxShaderID);
        return false;
    }

    if(!rbglIsShader(m_frgShaderID))
    {
        hal_platform.debugMsg("rbProgram::link: %u has invalid fragment shader %u attached\n", m_programID, m_frgShaderID);
        return false;
    }

    rbglLinkProgram(m_programID);

    GLint programSuccess = GL_TRUE;
    rbglGetProgramiv(m_programID, GL_LINK_STATUS, &programSuccess);
    if(programSuccess != GL_TRUE)
    {
        hal_platform.debugMsg("rbProgram::link: Error linking program %u\n", m_programID);
        outputLogInfo();
        rbglDeleteShader(m_vtxShaderID);
        rbglDeleteShader(m_frgShaderID);
        rbglDeleteProgram(m_programID);
        m_programID = m_vtxShaderID = m_frgShaderID = 0;
        return false;
    }

    // done with shader handles
    rbglDeleteShader(m_vtxShaderID);
    rbglDeleteShader(m_frgShaderID);
    m_vtxShaderID = m_frgShaderID = 0;

    return true;
}

//
// Bind the shader program
//
bool rbProgram::bind() const
{
    if(!rbglIsProgram(m_programID))
        return false;

    rbglUseProgram(m_programID);

    GLenum error = glGetError();
    if(error != GL_NO_ERROR)
    {
        hal_platform.debugMsg("rbProgram::bind: Error binding shader %u: %u\n", m_programID, error);
        outputLogInfo();
        return false;
    }

    return true;
}

//
// Unbind any bound shader program
//
void rbProgram::unbind() const
{
    rbglUseProgram(0);
}

//
// Load support for shaders
//
bool RB_InitShaderSupport()
{
    bool extension_ok = true;

    GETPROC(rbglAttachShader,      "glAttachShader"     );
    GETPROC(rbglCompileShader,     "glCompileShader"    );
    GETPROC(rbglCreateShader,      "glCreateShader"     );
    GETPROC(rbglDeleteProgram,     "glDeleteProgram"    );
    GETPROC(rbglGetProgramInfoLog, "glGetProgramInfoLog");
    GETPROC(rbglGetProgramiv,      "glGetProgramiv"     );
    GETPROC(rbglGetShaderInfoLog,  "glGetShaderInfoLog" );
    GETPROC(rbglGetShaderiv,       "glGetShaderiv"      );
    GETPROC(rbglIsProgram,         "glIsProgram"        );
    GETPROC(rbglIsShader,          "glIsShader"         );
    GETPROC(rbglLinkProgram,       "glLinkProgram"      );
    GETPROC(rbglShaderSource,      "glShaderSource"     );
    GETPROC(rbglUseProgram,        "glUseProgram"       );

    return extension_ok;
}

// EOF
