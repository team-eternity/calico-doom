/*
  CALICO
  
  IWAD Location
  
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keywords.h"
#include "m_argv.h"
#include "w_iwad.h"

typedef enum wfiletype_e
{
   WFT_ERROR = -1, // error
   WFT_UNKNOWN,    // unknown
   WFT_WAD,        // id WAD file
   WFT_ROM         // Jaguar ROM image
} wfiletype_e;

//
// Check for the -iwad command line parameter
//
static FILE *W_checkForIWADArg(void)
{
   int i;

   if((i = M_GetArgParameters("-iwad", 1)) != 0)
      return fopen(myargv[i], "rb");

   return NULL;
}

//
// Check if jagdoom.wad is visible
//
static FILE *W_haveJagDoomWAD(void)
{
   return fopen("jagdoom.wad", "rb");
}

//
// Test whether the loaded file is an IWAD
//
static wfiletype_e W_isIWAD(FILE *f, long *offset)
{
   char id[4];

   if(fread(id, 1, earrlen(id), f) != earrlen(id))
      return WFT_ERROR;

   if(!strncmp(id, "IWAD", earrlen(id)))
   {
      *offset = 0;
      return WFT_WAD;
   }

   return WFT_UNKNOWN;
}

#define WBUFFERSIZE 0x400

//
// Test whether the loaded file is a Jaguar ROM that contains an IWAD
//
// unzip.h -- IO for uncompress .zip files using zlib
// Version 0.15 beta, Mar 19th, 1998,
//
// Copyright (C) 1998 Gilles Vollant
//
// I WAIT FEEDBACK at mail info@winimage.com
// Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
static wfiletype_e W_isROM(FILE *f, long *offset)
{
   uint8_t buffer[WBUFFERSIZE + 4];
   long fileSize = 0;
   long backRead = 4;

   if(fseek(f, 0, SEEK_END))
      return WFT_ERROR;

   fileSize = ftell(f);
   while(backRead < fileSize)
   {
      long i, readSize, readPos;
      size_t sReadSize;

      if(backRead + WBUFFERSIZE > fileSize)
         backRead = fileSize;
      else
         backRead += WBUFFERSIZE;

      readPos  = fileSize - backRead;
      readSize = emin(earrlen(buffer), fileSize - readPos);

      if(fseek(f, readPos, SEEK_SET))
         return WFT_ERROR;

      sReadSize = (size_t)readSize;
      if(fread(buffer, 1, sReadSize, f) != sReadSize)
         return WFT_ERROR;

      for(i = readSize - 3; (i--) > 0; )
      {
         if(buffer[i] == 'I' && buffer[i + 1] == 'W' && buffer[i + 2] == 'A' && buffer[i + 3] == 'D')
         {
            *offset = readPos + i;
            return WFT_ROM;
         }
      }
   }
   
   return WFT_UNKNOWN;
}

//
// Check file extension of the path to determine format
//
static wfiletype_e W_checkFileFormat(FILE *f, long *offset)
{
   wfiletype_e ret;

   // check if it is an IWAD   
   switch((ret = W_isIWAD(f, offset)))
   {
   case WFT_WAD:
   case WFT_ERROR:
      return ret;
   default:
      // check if it is a ROM file
      return W_isROM(f, offset);
   }
}

//
// Read the WAD file into memory
//
static byte *W_cacheWADFile(FILE *f, long offset)
{
   long length;
   byte *buffer;

   if(fseek(f, 0, SEEK_END))
      return NULL;
   length = ftell(f);
   if(fseek(f, 0, SEEK_SET))
      return NULL;

   if(!(buffer = malloc(length)))
      return NULL;

   if(fread(buffer, 1, length, f) != length)
   {
      free(buffer);
      return NULL;
   }

   return buffer + offset;
}

//
// Load the IWAD
//
byte *W_LoadIWAD(void)
{
   FILE *f = NULL;
   wfiletype_e type;
   long offset = 0;
   byte *data = NULL;

   // check for -iwad specification on command line
   if(!(f = W_checkForIWADArg()))
   {
      // check if jagdoom.wad is available
      if(!(f = W_haveJagDoomWAD()))
         return NULL;
   }

   // check format of opened file
   type = W_checkFileFormat(f, &offset);
   if(type == WFT_WAD || type == WFT_ROM)
      data = W_cacheWADFile(f, offset);

   fclose(f);
   return data;
}

// EOF

