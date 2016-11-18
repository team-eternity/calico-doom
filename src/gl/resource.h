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

#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "../elib/dllist.h"
#include "../elib/qstring.h"

//
// Inherit from this class to participate in resource management.
//
class Resource
{
protected:
   friend class ResourceHive;

   qstring m_tag;
   DLListItem<Resource> m_links;

public:
   Resource(const char *tag)
      : m_tag(tag), m_links()
   {
   }

   virtual ~Resource() 
   {
      if(m_links.dllPrev)
         m_links.remove();
   }

   const qstring &getTag()   const { return m_tag; }
   unsigned int   hashCode() const { return m_tag.hashCode(); }
};

//
// Manages resources.
//
class ResourceHive
{
protected:
   enum chaincount_e { NUMCHAINS = 257 };
   DLList<Resource, &Resource::m_links> chains[NUMCHAINS];

public:
   ResourceHive();
   ~ResourceHive();

   void      addResource(Resource *res);
   Resource *findResource(const qstring &tag);
   Resource *findResource(const char *tag);

   //
   // Find a resource of a given type by its tag name.
   // Returns null if there is no such resource loaded both of the name and
   // the proper type.
   // T: The type of object, as checked by dynamic_cast. Must be a 
   //    descendant of Resource in order for this to work.
   // P: Parameter type; this can be type inferred.
   //
   template<typename T, typename P> T *findResourceType(const P &tag)
   {
      return dynamic_cast<T *>(findResource(tag));
   }

   void purgeResource(Resource *);
   bool purgeResource(const qstring &tag);
   bool purgeResource(const char *tag);
   
   bool purgeAllResourceNamed(const char *tag);
   void purgeAll();

   typedef void (*foreachfn_t)(Resource *);
   void forEach(foreachfn_t fn);   

   template<typename T>
   void forEachOfType(void (*fn)(T *))
   {
      for(int i = 0; i < NUMCHAINS; i++)
      {
         auto item = chains[i].head;

         while(item)
         {
            auto next = item->dllNext;
            T *asT;
            if((asT = dynamic_cast<T *>(item->dllObject)))
               fn(asT);
            item = next;
         }
      }
   }
};

#endif

// EOF

