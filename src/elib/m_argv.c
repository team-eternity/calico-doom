/*
  CALICO

  Command line argument management
  
  The MIT License (MIT)

  Copyright (c) 2016 James Haley

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

#include <string.h>
#include "m_argv.h"

// Global arguments
int myargc;
const char *const *myargv;

//
// Returns true if the specified argument was passed to the program,
// and false otherwise.
//
hal_bool M_FindArgument(const char *arg)
{
    for(int i = 1; i < myargc; i++)
    {
        if(!strcmp(myargv[i], arg))
            return HAL_TRUE;
    }

    return HAL_FALSE;
}

//
// Returns the index of the first parameter to the specified argument, if
// the argument was passed and can have at least "count" required arguments
// after it on the command line. Returns 0 if either the parameter was not
// passed at all or it is too near the end of the command line.
//
int M_GetArgParameters(const char *arg, int count)
{
   for(int i = 1; i < myargc; i++)
   {
      if(!strcmp(myargv[i], arg))
      {
         if(i < myargc - count)
            return i + 1;
         else
            return 0; // not enough arguments available
      }
   }

   return 0; // not found
}

// EOF

