/*
  CALICO

  Management of resources that need to change in response to a video
  mode/resolution change.

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

#ifndef VALLOC_H__
#define VALLOC_H__

#include "../elib/dllist.h"

class VAllocItem
{
public:
   typedef void (*allocfn_t)();

protected:
   static DLListItem<VAllocItem> *gAllocList;

   DLListItem<VAllocItem> m_links;
   allocfn_t m_allocator;

public:
   VAllocItem(allocfn_t allocator)
      : m_links(), m_allocator(allocator)
   {
      m_links.insert(this, &gAllocList);
   }

   static void ModeChanging();
};

#define VALLOCFNNAME(name) VAllocFn_ ## name
#define VALLOCFNSIG(name)  static void VALLOCFNNAME(name) ()
#define VALLOCDECL(name)   static VAllocItem VAllocItem_ ## name (VALLOCFNNAME(name))
#define VALLOCFNDEF(name)  static void VALLOCFNNAME(name) ()

#define VALLOCATION(name) \
   VALLOCFNSIG(name);     \
   VALLOCDECL(name);      \
   VALLOCFNDEF(name)

#endif

// EOF

