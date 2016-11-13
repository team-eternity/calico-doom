/*
  CALICO
  
  Configuration file
  
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

#ifndef CONFIG_H__
#define CONFIG_H__

#ifdef __cplusplus

#include "compare.h"

class qstring;

template<typename T>
struct cfgrange_t
{
   T min;
   T max;
   T clamp(const T &value)
   {
      return eclamp<T>(value, min, max);
   }
};

class CfgItem
{
public:
   enum itemtype_t
   {
      CFG_BOOL,
      CFG_INT,
      CFG_DOUBLE,
      CFG_STRING
   };

   enum chains_e { NUMCHAINS = 257 };

protected:
   static CfgItem *items[NUMCHAINS];
   const char *m_name;
   CfgItem    *m_next;
   itemtype_t  m_type;
   void       *m_var;
   void       *m_range;

   void init(const char *name, itemtype_t type, void *var);

public:
   CfgItem(const char *name, int     *i, cfgrange_t<int> *range = nullptr);
   CfgItem(const char *name, bool    *b);
   CfgItem(const char *name, double  *d, cfgrange_t<double> *range = nullptr);
   CfgItem(const char *name, char   **s);

   void readItem(const qstring &qstr);
   void writeItem(qstring &qstr);

   itemtype_t  getType() const { return m_type; }
   const char *getName() const { return m_name; }

   static CfgItem *FindByName(const char *name);
   static void GetValueAsString(const char *name, qstring &qstr);
   static void ItemIterator(void (*func)(CfgItem *, void *), void *data);
};

extern "C" {
#endif

void Cfg_LoadFile();
void Cfg_WriteFile();

#ifdef __cplusplus
}
#endif

#endif

// EOF

