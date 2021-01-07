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

static PFNGLATTACHSHADERPROC       rbglAttachShader;
static PFNGLBINDATTRIBLOCATIONPROC rbglBindAttribLocation;
static PFNGLCOMPILESHADERPROC      rbglCompileShader;
static PFNGLCREATESHADERPROC       rbglCreateShader;
static PFNGLCREATEPROGRAMPROC      rbglCreateProgram;
static PFNGLDELETEPROGRAMPROC      rbglDeleteProgram;
static PFNGLDELETESHADERPROC       rbglDeleteShader;
static PFNGLGETATTRIBLOCATIONPROC  rbglGetAttribLocation;
static PFNGLGETPROGRAMINFOLOGPROC  rbglGetProgramInfoLog;
static PFNGLGETPROGRAMIVPROC       rbglGetProgramiv;
static PFNGLGETSHADERINFOLOGPROC   rbglGetShaderInfoLog;
static PFNGLGETSHADERIVPROC        rbglGetShaderiv;
static PFNGLGETUNIFORMLOCATIONPROC rbglGetUniformLocation;
static PFNGLISPROGRAMPROC          rbglIsProgram;
static PFNGLISSHADERPROC           rbglIsShader;
static PFNGLLINKPROGRAMPROC        rbglLinkProgram;
static PFNGLSHADERSOURCEPROC       rbglShaderSource;
static PFNGLUNIFORMMATRIX4FVPROC   rbglUniformMatrix4fv;
static PFNGLUSEPROGRAMPROC         rbglUseProgram;

//=============================================================================
// rbShader
//

//
// Destructor
//
rbShader::~rbShader()
{
    if(!hal_medialayer.isExiting())
        deleteShader();
}

//
// Delete the shader object
//
void rbShader::deleteShader()
{
    if(rbglIsShader(m_shaderID))
    {
        rbglDeleteShader(m_shaderID);
        m_shaderID = 0;
    }
}

//
// Forget the attached shader name
//
void rbShader::abandonShader()
{
    m_shaderID = 0;
}

//
// Output shader log info
//
void rbShader::outputLogInfo() const
{
    if(rbglIsShader(m_shaderID))
    {
        GLint maxLen = 0;
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
        deleteShader();
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

static int currentProgramID;

//
// Destructor
//
rbProgram::~rbProgram()
{
    // there's no need to destroy GL entities when the game is exiting;
    // the destruction of the context will take them with it.
    if(!hal_medialayer.isExiting())
        deleteProgram();
}

//
// Create a program
//
void rbProgram::createProgram()
{
    m_programID = rbglCreateProgram();
    m_attributeLocations.clear();
    m_uniformLocations.clear();
}

//
// Delete the program and clear the ID
//
void rbProgram::deleteProgram()
{
    m_vtxShader.deleteShader();
    m_frgShader.deleteShader();
    m_attributeLocations.clear();
    m_uniformLocations.clear();

    if(rbglIsProgram(m_programID))
        rbglDeleteProgram(m_programID);
    if(currentProgramID == m_programID)
        currentProgramID = 0;
    m_programID = 0;
}

//
// Forget about the associated GL program ID
//
void rbProgram::abandonProgram()
{
    m_vtxShader.abandonShader();
    m_frgShader.abandonShader();
    m_attributeLocations.clear();
    m_uniformLocations.clear();

    if(currentProgramID == m_programID)
        currentProgramID = 0;

    m_programID = 0;
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
bool rbProgram::attachVertexShader(rbShader &&shader)
{
    unsigned int shaderID = shader.getShaderID();

    if(shader.getType() != rbShader::TYPE_VERTEX)
    {
        hal_platform.debugMsg("rbProgram::attachVertexShader: %u is not a vertex shader\n", shaderID);
        return false;
    }

    rbglAttachShader(m_programID, shaderID);
    m_vtxShader = std::move(shader);
    return true;
}

//
// Attach a fragment shader to the program object
//
bool rbProgram::attachFragmentShader(rbShader &&shader)
{
    unsigned int shaderID = shader.getShaderID();

    if(shader.getType() != rbShader::TYPE_FRAGMENT)
    {
        hal_platform.debugMsg("rbProgram::attachFragmentShader: %u is not a fragment shader\n", shaderID);
        return false;
    }

    rbglAttachShader(m_programID, shaderID);
    m_frgShader = std::move(shader);
    return true;
}

//
// Bind an attribute location
//
bool rbProgram::bindAttribLocation(int idx, const char *name)
{
    if(!rbglIsProgram(m_programID))
    {
        hal_platform.debugMsg("rbProgram::bindAttribLocation: %u is not a program\n", m_programID);
        return false;
    }

    if(m_linked)
    {
        hal_platform.debugMsg("rbProgram::bindAttribLocation: Shader program %u already linked\n", m_programID);
        return false;
    }

    qstring qstrName { name };
    const AttribLocationMap::const_iterator citr = m_attributeLocations.find(qstrName);
    if(citr != m_attributeLocations.cend())
    {
        if(idx == citr->second)
            return true; // already mapped at same location; ignore (though, wtf are you doing?)
        else
        {
            hal_platform.debugMsg(
                "rbProgram::bindAttribLocation: attribute %s intended for index %d already bound to index %d\n",
                name, idx, citr->second
            );
            return false; // this is not good
        }
    }
    else
    {
        rbglBindAttribLocation(m_programID, static_cast<GLuint>(idx), name);
        m_attributeLocations[std::move(qstrName)] = idx;
        return true;
    }
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

    if(!rbglIsShader(m_vtxShader.getShaderID()))
    {
        hal_platform.debugMsg("rbProgram::link: %u has invalid vertex shader %u attached\n", m_programID, m_vtxShader.getShaderID());
        return false;
    }

    if(!rbglIsShader(m_frgShader.getShaderID()))
    {
        hal_platform.debugMsg("rbProgram::link: %u has invalid fragment shader %u attached\n", m_programID, m_frgShader.getShaderID());
        return false;
    }

    m_linked = false;

    rbglLinkProgram(m_programID);

    GLint programSuccess = GL_TRUE;
    rbglGetProgramiv(m_programID, GL_LINK_STATUS, &programSuccess);
    if(programSuccess != GL_TRUE)
    {
        hal_platform.debugMsg("rbProgram::link: Error linking program %u\n", m_programID);
        outputLogInfo();

        m_vtxShader.deleteShader();
        m_frgShader.deleteShader();
        deleteProgram();
        return false;
    }

    // done with shader handles
    m_vtxShader.deleteShader();
    m_frgShader.deleteShader();

    m_linked = true;

    return true;
}

//
// Bind the shader program
//
bool rbProgram::bind() const
{
    if(!rbglIsProgram(m_programID))
        return false;

    if(currentProgramID == m_programID)
        return true; // already in use

    glGetError();

    rbglUseProgram(m_programID);

    GLenum error = glGetError();
    if(error != GL_NO_ERROR)
    {
        hal_platform.debugMsg("rbProgram::bind: Error binding shader %u: %u\n", m_programID, error);
        outputLogInfo();
        return false;
    }

    currentProgramID = m_programID;
    return true;
}

//
// Unbind any bound shader program
//
void rbProgram::unbind() const
{
    currentProgramID = 0;
    rbglUseProgram(0);
}

//
// Look up an attribute location, with caching
//
int rbProgram::getAttribLocation(const char *name)
{
    if(!m_linked)
    {
        hal_platform.debugMsg("rbProgram::getAttribLocation: Shader program %u has not been linked\n", m_programID);
        return -1;
    }

    qstring qstrName { name };
    const AttribLocationMap::const_iterator citr = m_attributeLocations.find(qstrName);
    if(citr != m_attributeLocations.cend())
    {
        return citr->second; 
    }
    else
    {
        int id = rbglGetAttribLocation(m_programID, name);
        if(id >= 0)
        {
            m_attributeLocations[std::move(qstrName)] = id;
            return id;
        }
        else
        {
            hal_platform.debugMsg("rbProgram::getAttribLocation: Invalid name '%s' for program %u\n", name, m_programID);
            return -1;
        }
    }
}

//
// Look up a uniform location, with caching
//
int rbProgram::getUniformLocation(const char *name)
{
    if(!m_linked)
    {
        hal_platform.debugMsg("rbProgram::getUniformLocation: Shader program %u has not been linked\n", m_programID);
        return -1;
    }

    qstring qstrName { name };
    UniformLocationMap::const_iterator citr;
    if((citr = m_uniformLocations.find(qstrName)) != m_uniformLocations.cend())
    {
        return citr->second; 
    }
    else
    {
        int id = rbglGetUniformLocation(m_programID, name);
        if(id >= 0)
        {
            m_uniformLocations[std::move(qstrName)] = id;
            return id;
        }
        else
        {
            hal_platform.debugMsg("rbProgram::getUniformLocation: Invalid name '%s' for program %u\n", name, m_programID);
            return -1;
        }
    }
}

//
// Set a uniform matrix value
//
void rbProgram::UniformMatrix4fv(int location, int size, bool transpose, const float *value)
{
    rbglUniformMatrix4fv(location, size, transpose, value);
}

//
// Load support for shaders
//
bool RB_InitShaderSupport()
{
    bool extension_ok = true;

    GETPROC(rbglAttachShader,       "glAttachShader"      );
    GETPROC(rbglBindAttribLocation, "glBindAttribLocation");
    GETPROC(rbglCompileShader,      "glCompileShader"     );
    GETPROC(rbglCreateProgram,      "glCreateProgram"     );
    GETPROC(rbglCreateShader,       "glCreateShader"      );
    GETPROC(rbglDeleteProgram,      "glDeleteProgram"     );
    GETPROC(rbglDeleteShader,       "glDeleteShader"      );
    GETPROC(rbglGetAttribLocation,  "glGetAttribLocation" );
    GETPROC(rbglGetProgramInfoLog,  "glGetProgramInfoLog" );
    GETPROC(rbglGetProgramiv,       "glGetProgramiv"      );
    GETPROC(rbglGetShaderInfoLog,   "glGetShaderInfoLog"  );
    GETPROC(rbglGetShaderiv,        "glGetShaderiv"       );
    GETPROC(rbglGetUniformLocation, "glGetUniformLocation");
    GETPROC(rbglIsProgram,          "glIsProgram"         );
    GETPROC(rbglIsShader,           "glIsShader"          );
    GETPROC(rbglLinkProgram,        "glLinkProgram"       );
    GETPROC(rbglShaderSource,       "glShaderSource"      );
    GETPROC(rbglUniformMatrix4fv,   "glUniformMatrix4fv"  );
    GETPROC(rbglUseProgram,         "glUseProgram"        );

    return extension_ok;
}

// EOF
