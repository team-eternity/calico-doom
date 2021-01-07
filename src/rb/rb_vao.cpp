/*
  CALICO

  OpenGL renderer VAO/VBO support

  The MIT License (MIT)

  Copyright (C) 2007-2014 Samuel Villarreal
  Copyright (C) 2020 James Haley

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

#include "../hal/hal_ml.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "../elib/elib.h"
#include "../elib/misc.h"
#include "rb_main.h"
#include "rb_vao.h"

static PFNGLBINDBUFFERPROC              rbglBindBuffer;
static PFNGLBINDVERTEXARRAYPROC         rbglBindVertexArray;
static PFNGLBUFFERDATAPROC              rbglBufferData;
static PFNGLDELETEBUFFERSPROC           rbglDeleteBuffers;
static PFNGLDELETEVERTEXARRAYSPROC      rbglDeleteVertexArrays;
static PFNGLDRAWARRAYSEXTPROC           rbglDrawArrays;
static PFNGLENABLEVERTEXATTRIBARRAYPROC rbglEnableVertexAttribArray;
static PFNGLGENBUFFERSPROC              rbglGenBuffers;
static PFNGLGENVERTEXARRAYSPROC         rbglGenVertexArrays;
static PFNGLISBUFFERPROC                rbglIsBuffer;
static PFNGLISVERTEXARRAYPROC           rbglIsVertexArray;
static PFNGLVERTEXATTRIBDIVISORPROC     rbglVertexAttribDivisor;
static PFNGLVERTEXATTRIBPOINTERPROC     rbglVertexAttribPointer;

// local utils
namespace {

inline GLenum GetGLDataType(rbVBO::attribtype_e rbtype)
{
    switch(rbtype)
    {
    case rbVBO::BYTE:           return GL_BYTE;
    case rbVBO::UNSIGNED_BYTE:  return GL_UNSIGNED_BYTE;
    case rbVBO::SHORT:          return GL_SHORT;
    case rbVBO::UNSIGNED_SHORT: return GL_UNSIGNED_SHORT;
    case rbVBO::INT:            return GL_INT;
    case rbVBO::UNSIGNED_INT:   return GL_UNSIGNED_INT;
    case rbVBO::HALF_FLOAT:     return GL_HALF_FLOAT;    
    case rbVBO::DOUBLE:         return GL_DOUBLE;
    case rbVBO::FIXED:          return GL_FIXED;
    default:                    return GL_FLOAT;
    }
}

}

//=============================================================================
// 
// rbVBO
//
//=============================================================================

unsigned int rbVBO::currentBoundVBO;

//
// Destructor
//
rbVBO::~rbVBO()
{
    if(!hal_medialayer.isExiting())
        deleteVBO();
}

//
// Generate a GL vertex buffer object
//
void rbVBO::generate()
{
    rbglGenBuffers(1, &m_id);
}

//
// Test if the vertex buffer object is valid
//
bool rbVBO::isValid() const
{
    return (rbglIsBuffer(m_id) == GL_TRUE);
}

//
// Delete the vertex buffer object
//
void rbVBO::deleteVBO()
{
    if(rbglIsBuffer(m_id))
        rbglDeleteBuffers(1, &m_id);
    if(currentBoundVBO == m_id)
        currentBoundVBO = 0;
    m_id = 0;
}

//
// Abandon the vertex buffer object
//
void rbVBO::abandonVBO()
{
    if(currentBoundVBO == m_id)
        currentBoundVBO = 0;
    m_id = 0;
}

//
// Bind the buffer object as a vertex attribute array
//
void rbVBO::bind() const
{
    if(currentBoundVBO != m_id)
    {
        rbglBindBuffer(GL_ARRAY_BUFFER, m_id);
        currentBoundVBO = m_id;
    }
}

//
// Unbind any buffer object bound as vertex attributes
//
void rbVBO::unbind() const
{
    currentBoundVBO = 0;
    rbglBindBuffer(GL_ARRAY_BUFFER, 0);
}

//
// Set vertex attribute data
//
void rbVBO::bufferData(ptrdiff_t size, const void *data, drawtype_e type) const
{
    rbglBufferData(GL_ARRAY_BUFFER, size, data, type == DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

//
// Specify the location and data format of the array of generic vertex attributes at index
//
void rbVBO::vertexAttribPointer(unsigned int index, int size, attribtype_e type, bool normalized, int stride, const void *ptr) const
{
    rbglVertexAttribPointer(index, size, GetGLDataType(type), normalized, stride, ptr);
}

void rbVBO::vertexAttribDivisor(unsigned int index, unsigned int divisor) const
{
    rbglVertexAttribDivisor(index, divisor);
}

//=============================================================================
// 
// rbVAO
//
//=============================================================================

unsigned int rbVAO::currentBoundVAO;

//
// Destructor
//
rbVAO::~rbVAO()
{
    if(!hal_medialayer.isExiting())
        deleteVAO();
}

//
// Generate a vertex array object
//
void rbVAO::generate()
{
    rbglGenVertexArrays(1, &m_id);
}

//
// Test if the vertex array object is valid
//
bool rbVAO::isValid() const
{
    return (rbglIsVertexArray(m_id) == GL_TRUE);
}

//
// Delete the vertex array object
//
void rbVAO::deleteVAO()
{
    if(rbglIsVertexArray(m_id))
        rbglDeleteVertexArrays(1, &m_id);
    if(currentBoundVAO == m_id)
        currentBoundVAO = 0;
    m_id = 0;
}

//
// Abandon the vertex array object
//
void rbVAO::abandonVAO()
{
    if(currentBoundVAO == m_id)
        currentBoundVAO = 0;
    m_id = 0;
}

//
// Bind the VAO
//
void rbVAO::bind() const
{
    if(currentBoundVAO != m_id)
    {
        rbglBindVertexArray(m_id);
        currentBoundVAO = m_id;
    }
}

//
// Unbind any bound VAO
//
void rbVAO::unbind() const
{
    currentBoundVAO = 0;
    rbglBindVertexArray(0);
}

//
// Enable vertex attribute array for the currently bound VAO at index
//
void rbVAO::enableVertexAttribArray(unsigned int idx) const
{
    rbglEnableVertexAttribArray(idx);
}

//
// Combined operation to bind the VAO and VBO, set attribute data, and enable the attribute
//
void rbVAO::vertexAttribPointer(
    const rbVBO &vbo,
    unsigned int index, 
    int size, rbVBO::attribtype_e type, bool normalized, int stride
) const
{
    bind();     // ensure self is bound as the active VAO
    vbo.bind(); // bind vbo
    vbo.vertexAttribPointer(index, size, type, normalized, stride, nullptr); // set attrib data
    enableVertexAttribArray(index); // enable attribute
}


// ============================================================================

//
// Load support for vertex array object pipeline
//
bool RB_InitVAOSupport()
{
    bool extension_ok = true;

    GETPROC(rbglBindBuffer,              "glBindBuffer");
    GETPROC(rbglBindVertexArray,         "glBindVertexArray");
    GETPROC(rbglBufferData,              "glBufferData");
    GETPROC(rbglDeleteBuffers,           "glDeleteBuffers");
    GETPROC(rbglDeleteVertexArrays,      "glDeleteVertexArrays");
    GETPROC(rbglDrawArrays,              "glDrawArrays");
    GETPROC(rbglEnableVertexAttribArray, "glEnableVertexAttribArray");
    GETPROC(rbglGenBuffers,              "glGenBuffers");
    GETPROC(rbglGenVertexArrays,         "glGenVertexArrays");
    GETPROC(rbglIsBuffer,                "glIsBuffer");
    GETPROC(rbglIsVertexArray,           "glIsVertexArray");
    GETPROC(rbglVertexAttribDivisor,     "glVertexAttribDivisor");
    GETPROC(rbglVertexAttribPointer,     "glVertexAttribPointer");

    return extension_ok;
}

// EOF
