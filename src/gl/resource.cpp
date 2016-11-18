/*
  CALICO
  
  Resource management
  
  The MIT License (MIT)
  
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

#include "../elib/elib.h"
#include "resource.h"

//=============================================================================
//
// ResourceHive
//

//
// Constructor
//
ResourceHive::ResourceHive()
{
   for(int i = 0; i < NUMCHAINS; i++)
      chains[i].head = nullptr;
}

//
// Destructor
//
ResourceHive::~ResourceHive()
{
   purgeAll();
}

//
// Add a new resource. It will be head-inserted into its hash chain, so if 
// there are multiple resources of the same name, the newest one added wins.
//
void ResourceHive::addResource(Resource *res)
{
   unsigned int hc = res->hashCode() % NUMCHAINS;
   chains[hc].insert(res);
}

//
// Find a resource by its tag name. Returns null if there is no such 
// resource.
//
Resource *ResourceHive::findResource(const char *tag)
{
   unsigned int hc = qstring::HashCodeStatic(tag) % NUMCHAINS;
   DLListItem<Resource> *res = chains[hc].head;

   while(res && res->dllObject->getTag() != tag)
      res = res->dllNext;
   
   return res ? res->dllObject : nullptr;
}

//
// Convenience method for loading a resource given a qstring instance.
//
Resource *ResourceHive::findResource(const qstring &tag)
{
   return findResource(tag.constPtr());
}

//
// Free a resource
//
void ResourceHive::purgeResource(Resource *res)
{
   delete res;
}

//
// Find a resource by name and free it.
//
bool ResourceHive::purgeResource(const char *tag)
{
   Resource *res;
   bool      didfree = false;

   if((res = findResource(tag)))
   {
      purgeResource(res);
      didfree = true;
   }
   
   return didfree;
}

//
// Convenience overload for qstring.
//
bool ResourceHive::purgeResource(const qstring &tag)
{
   return purgeResource(tag.constPtr());
}

//
// Find all resources with the given name and delete them.
//
bool ResourceHive::purgeAllResourceNamed(const char *tag)
{
   Resource *res;
   bool      didfree = false;

   while((res = findResource(tag)))
   {
      purgeResource(res);
      didfree = true;
   }

   return didfree;
}

//
// Delete all resources in the hive.
//
void ResourceHive::purgeAll()
{
   for(int i = 0; i < NUMCHAINS; i++)
   {
      auto item = chains[i].head;

      while(item)
      {
         DLListItem<Resource> *next = item->dllNext;
         delete item->dllObject;
         item = next;
      }
   }
}

//
// Execute a callback for all resources in the hive.
//
void ResourceHive::forEach(foreachfn_t fn)
{
   for(int i = 0; i < NUMCHAINS; i++)
   {
      auto item = chains[i].head;

      while(item)
      {
         DLListItem<Resource> *next = item->dllNext;
         fn(item->dllObject);
         item = next;
      }
   }
}

// EOF

