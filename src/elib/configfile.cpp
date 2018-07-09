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

#include "elib.h"
#include <map>

#include "../hal/hal_platform.h"
#include "../hal/hal_ml.h"
#include "atexit.h"
#include "configfile.h"
#include "parser.h"
#include "qstring.h"

//=============================================================================
//
// Variable Binding
//

CfgItem *CfgItem::items[NUMCHAINS];

//
// Initialize a configuration binding completely.
//
void CfgItem::init(const char *name, itemtype_t type, void *var)
{
   m_name  = name;
   m_var   = var;
   m_type  = type;
   m_range = nullptr;

   unsigned int chain = qstring::HashCodeStatic(name) % NUMCHAINS;
   m_next = items[chain];
   items[chain] = this;
}

//
// Construct an integer config binding.
//
CfgItem::CfgItem(const char *name, int *i, cfgrange_t<int> *range)
{
   init(name, CFG_INT, i);
   m_range = range;
}
  
//
// Construct a boolean config binding.
//
CfgItem::CfgItem(const char *name, bool *b)
{
   init(name, CFG_BOOL, b);
}
   
//
// Construct a double config binding.
//
CfgItem::CfgItem(const char *name, double *d, cfgrange_t<double> *range)
{
   init(name, CFG_DOUBLE, d);
   m_range = range;
}
 
//
// Construct a string config binding.
//
CfgItem::CfgItem(const char *name, char **s)
{
   init(name, CFG_STRING, s);
}

//
// Read a config item in from input taken from file.
//
void CfgItem::readItem(const qstring &qstr)
{
   switch(m_type)
   {
   case CFG_INT:
      {
         auto i_var = static_cast<int *>(m_var);
         *i_var = qstr.toInt();
         if(m_range)
            *i_var = static_cast<cfgrange_t<int> *>(m_range)->clamp(*i_var);
      }
      break;
   case CFG_BOOL:
      *static_cast<bool *>(m_var) = !!qstr.toInt();
      break;
   case CFG_DOUBLE:
      {
         auto d_var = static_cast<double *>(m_var);
         *d_var = qstr.toDouble(nullptr);
         if(m_range)
            *d_var = static_cast<cfgrange_t<double> *>(m_range)->clamp(*d_var);
      }
      break;
   case CFG_STRING:
      {
         char **dst = static_cast<char **>(m_var);
         if(*dst)
            efree(*dst);
         *dst = qstr.duplicate();
      }
      break;
   default:
      break;
   }
}

//
// Write out the current value of a config item.
//
void CfgItem::writeItem(qstring &qstr)
{
   switch(m_type)
   {
   case CFG_INT:
      qstr << *static_cast<int *>(m_var);
      break;
   case CFG_BOOL:
      qstr << *static_cast<bool *>(m_var);
      break;
   case CFG_DOUBLE:
      qstr << *static_cast<double *>(m_var);
      break;
   case CFG_STRING:
      {
         char *src = *static_cast<char **>(m_var);
         if(src)
            qstr << src;
      }
      break;
   default:
      break;
   }
}

//
// Find a configuration binding item by name.
//
CfgItem *CfgItem::FindByName(const char *name)
{
   unsigned int chain = qstring::HashCodeStatic(name) % NUMCHAINS;
   CfgItem *item = items[chain];

   while(item && strcasecmp(name, item->m_name))
      item = item->m_next;

   return item;
}

//
// Get a variable's string representation.
//
void CfgItem::GetValueAsString(const char *name, qstring &qstr)
{
   auto item = CfgItem::FindByName(name);
   if(item)
      item->writeItem(qstr);
}

//
// Iterate over all configuration bindings and call the provided function
// for each.
//
void CfgItem::ItemIterator(void (*func)(CfgItem *, void *), void *data)
{
   for(int i = 0; i < NUMCHAINS; i++)
   {
      CfgItem *item = items[i];
      while(item)
      {
         func(item, data);
         item = item->m_next;
      }
   }
}

//=============================================================================
//
// Configuration Loading
//

//
// Configuration file parser class
//
class CfgFileParser : public Parser
{
protected:
   // state table declaration
   static bool (CfgFileParser::*States[])(Tokenizer &);

   // parser state enumeration
   enum pstate_e
   {
      STATE_EXPECTKEYWORD,
      STATE_EXPECTVALUE
   };

   // state handlers
   bool doStateExpectKeyword(Tokenizer &);
   bool doStateExpectValue(Tokenizer &);

   // parser state data
   int      m_state;
   qstring  m_key;

   // overrides
   virtual bool doToken(Tokenizer &token);
   virtual void startFile();
   virtual void initTokenizer(Tokenizer &token);

public:
   CfgFileParser(const char *filename)
      : Parser(filename), m_state(STATE_EXPECTKEYWORD), m_key()
   {
   }
};

// state table
bool (CfgFileParser::* CfgFileParser::States[])(Tokenizer &) =
{
   &CfgFileParser::doStateExpectKeyword,
   &CfgFileParser::doStateExpectValue
};

//
// Dispatch token to appropriate state handler
//
bool CfgFileParser::doToken(Tokenizer &token)
{
   return (this->*States[m_state])(token);
}

//
// Reinitialize parser at beginning of file parsing
//
void CfgFileParser::startFile()
{
   m_state = STATE_EXPECTKEYWORD;
   m_key   = "";
}

//
// Setup tokenizer state before parsing begins
//
void CfgFileParser::initTokenizer(Tokenizer &token)
{
   token.setTokenFlags(Tokenizer::TF_DEFAULT);
}

//
// Keyword state handler
//
bool CfgFileParser::doStateExpectKeyword(Tokenizer &token)
{
   int tokentype = token.getTokenType();

   switch(tokentype)
   {
   case Tokenizer::TOKEN_KEYWORD:
   case Tokenizer::TOKEN_STRING:
      // record as the current key and expect value to follow
      m_key   = token.getToken();
      m_state = STATE_EXPECTVALUE;
      break;
   default:
      // if we see anything else, keep scanning
      break;
   }

   return true;
}

//
// Value state handler
//
bool CfgFileParser::doStateExpectValue(Tokenizer &token)
{
   qstring &value = token.getToken();

   auto item = CfgItem::FindByName(m_key.constPtr());
   if(item)
      item->readItem(value);
   m_state = STATE_EXPECTKEYWORD;
   m_key   = "";

   return true;
}

//=============================================================================
//
// External Interface
//

void Cfg_LoadFile(void)
{
   qstring fn(hal_medialayer.getWriteDirectory(ELIB_APPNAME));
   fn.pathConcatenate("calico.cfg");
   CfgFileParser parser(fn.constPtr());
   parser.parseFile();

   // schedule to write config file at exit, except in case of errors
   E_AtExit(Cfg_WriteFile, false);
}

struct cfgwritedata_t
{
   std::map<qstring, CfgItem *> *itemMap;
};

static void AddItemToMap(CfgItem *item, void *data)
{
   auto cwd = static_cast<cfgwritedata_t *>(data);
   cwd->itemMap->emplace(qstring(item->getName()), item);
}

static bool WriteCfgItem(CfgItem *item, FILE *f)
{
   qstring value;
   item->writeItem(value);
   if(std::fprintf(f, "%s \"%s\"\n", item->getName(), value.constPtr()) < 0)
      return false;
   return true;
}

void Cfg_WriteFile(void)
{
   cfgwritedata_t cwd;
   FILE *f     = nullptr;
   bool  error = false;
   std::map<qstring, CfgItem *> items;
   qstring tmpName(hal_medialayer.getWriteDirectory(ELIB_APPNAME));
   qstring dstName(hal_medialayer.getWriteDirectory(ELIB_APPNAME));

   tmpName.pathConcatenate("temp.cfg");
   dstName.pathConcatenate("calico.cfg");

   cwd.itemMap = &items;
   f = std::fopen(tmpName.constPtr(), "w");
   if(!f)
   {
      hal_platform.debugMsg("Warning: could not open temp.cfg\n");
      return;
   }

   if(std::fprintf(f, "// CALICO configuration file\n") < 0)
   {
      hal_platform.debugMsg("Warning: failed write to temp.cfg\n");
      return;
   }

   // add config items to the map
   CfgItem::ItemIterator(AddItemToMap, &cwd);

   for(auto &item : items)
   {
      if(!WriteCfgItem(item.second, f))
      {
         error = true;
         break;
      }
   }

   if(error)
   {
      std::fclose(f);
      hal_platform.debugMsg("Warning: failed one or more cfg writes\n");
      return;
   }

   if(std::fclose(f) < 0)
   {
      hal_platform.debugMsg("Warning: failed to close temp.cfg\n");
      return;
   }

   std::remove(dstName.constPtr());
   if(std::rename(tmpName.constPtr(), dstName.constPtr()))
      hal_platform.debugMsg("Warning: failed to write calico.cfg\n");
}

// EOF

