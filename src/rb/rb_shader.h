/*
  CALICO

  OpenGL renderer texture functions

  The MIT License (MIT)

  Copyright (C) 2019 James Haley

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

#ifndef RB_SHADER_H__
#define RB_SHADER_H__

class rbShader
{
public:
    enum type_e
    {
        TYPE_VERTEX,
        TYPE_FRAGMENT
    };

    rbShader(type_e type) : m_type(type) {}
    rbShader(const rbShader &other) = delete;
    rbShader(rbShader &&other)
        : m_type(other.m_type), m_shaderID(other.m_shaderID)
    {
        other.abandonShader();
    }

    ~rbShader();

    bool loadFromSource(const char *src);
    bool loadFromFile(const char *filename);

    type_e       getType()     const { return m_type;     }
    unsigned int getShaderID() const { return m_shaderID; }

    rbShader &operator = (const rbShader &other) = delete;
    rbShader &operator = (rbShader &&other)
    {
        deleteShader();
        m_type     = other.m_type;
        m_shaderID = other.m_shaderID;
        other.abandonShader();
        return *this;
    }

protected:
    friend class rbProgram;

    type_e       m_type;
    unsigned int m_shaderID = 0;

    void deleteShader();
    void abandonShader();
    void outputLogInfo() const;
};

class rbProgram
{
public:
    rbProgram() 
        : m_vtxShader(rbShader::TYPE_VERTEX), m_frgShader(rbShader::TYPE_FRAGMENT) 
    {
    }

    ~rbProgram();

    bool attachVertexShader(rbShader &&shader);
    bool attachFragmentShader(rbShader &&shader);
    bool link();

    bool bind() const;
    void unbind() const;
    
    unsigned int getProgramID() const { return m_programID; }

protected:
    unsigned int m_programID = 0;

    rbShader m_vtxShader;
    rbShader m_frgShader;

    void deleteProgram();
    void abandonProgram();
    void outputLogInfo() const;
};

bool RB_InitShaderSupport();

#endif

// EOF
