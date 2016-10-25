/*

  CALICO

  String utilites

*/

#include <stdio.h>
#include "doomdef.h"

/* prints number of characters printed. */

int mystrlen(char *string)
{
   int rc = 0;
   
   if(string) 
   {
      while(*(string++)) 
         rc++;
   }
   else 
      rc = -1;
   
   return rc;
}

//
// CALICO: Replaced with call to C library function to eliminate non-portable
// argument-passing idiom.
//
int D_vsnprintf(char *str, size_t nmax, const char *format, va_list ap)
{
   int result;

   if(nmax < 1)
   {
      return 0;
   }

   // vsnprintf() in Windows (and possibly other OSes) doesn't always
   // append a trailing \0. We have the responsibility of making this
   // safe by writing into a buffer that is one byte shorter ourselves.
   result = vsnprintf(str, nmax, format, ap);

   // If truncated, change the final char in the buffer to a \0.
   // In Windows, a negative result indicates a truncated buffer.
   if(result < 0 || (size_t)result >= nmax)
   {
      str[nmax - 1] = '\0';
      result = (int)(nmax - 1);
   }

   return result;
}

// EOF

