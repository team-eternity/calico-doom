/*
  CALICO

  OpenGL renderer VAO/VBO support

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

#pragma once

class rbVBO
{
public:
    ~rbVBO();

    enum drawtype_e
    {
        STATIC,
        DYNAMIC
    };

    enum attribtype_e
    {
        BYTE,
        UNSIGNED_BYTE,
        SHORT,
        UNSIGNED_SHORT,
        INT,
        UNSIGNED_INT,
        HALF_FLOAT,
        FLOAT,
        DOUBLE,
        FIXED
    };

    void generate();
    bool isValid() const;
    void bind()    const;
    void unbind()  const;
    void bufferData(ptrdiff_t size, const void *data, drawtype_e type = STATIC) const;
    void vertexAttribPointer(unsigned int index, int size, attribtype_e type, bool normalized, int stride, const void *ptr) const;
    void vertexAttribDivisor(unsigned int index, unsigned int divisor) const;

    void abandonVBO();

private:
    static unsigned int currentBoundVBO;
    unsigned int m_id = 0;

    void deleteVBO();
};

class rbVAO
{
public:
    ~rbVAO();

    void generate();
    bool isValid() const;
    void bind()    const;
    void unbind()  const;
    void enableVertexAttribArray(unsigned int idx) const;
    void vertexAttribPointer(
        const rbVBO &vbo,
        unsigned int index, 
        int size, rbVBO::attribtype_e type, bool normalized, int stride
    ) const;

    void abandonVAO();

private:
    static unsigned int currentBoundVAO;
    unsigned int m_id = 0;

    void deleteVAO();    
};

bool RB_InitVAOSupport();

// EOF

