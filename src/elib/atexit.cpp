/*
  CALICO
  
  Safe atexit functions with error condition handling
  
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

#include "elib.h"
#include "atexit.h"

static atexit_listentry_t *exitFuncs = nullptr;

//
// atexit substitute with error condition control
//
void E_AtExit(atexit_func_t func, int runOnError)
{
   auto entry = estructalloc(atexit_listentry_t, 1);

   entry->func       = func;
   entry->runOnError = runOnError;
   entry->next       = exitFuncs;
   exitFuncs         = entry;
}

void E_RunAtExitFuncs(int isError)
{
   auto entry = exitFuncs;

   while(entry)
   {
      if(!isError || entry->runOnError)
         entry->func();
      entry = entry->next;
   }

   // do not run them more than once.
   exitFuncs = nullptr;
}

// EOF

