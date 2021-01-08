/*
  CALICO
 
  Jaguar EEPROM Emulation

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

#include <stdio.h>
#include "elib/elib.h"
#include "elib/misc.h"
#include "hal/hal_ml.h"
#include "hal/hal_platform.h"

static FILE *eepromFile;

//
// Open a file to use as the Jaguar EEPROM store.
//
static void J_OpenEEPROM(const char *mode)
{
    char *const fullpath = M_SafeFilePath(hal_medialayer.getWriteDirectory(ELIB_APPNAME), "eeprom.cal");
    if(eepromFile)
    {
        fclose(eepromFile);
        eepromFile = NULL;
    }
    eepromFile = hal_platform.fileOpen(fullpath, mode);
    efree(fullpath);
}

//
// Read from emulated EEPROM
//
unsigned short eeread(int address)
{
   unsigned short temp, value = 0;

   if(address == 0)
      J_OpenEEPROM("rb");

   if(eepromFile)
   {
      if(fread(&temp, sizeof(temp), 1, eepromFile) == 1)
         value = temp;

      if(address == 7)
      {
         fclose(eepromFile);
         eepromFile = NULL;
      }
   }

   return value;
}

//
// Write to emulated EEPROM
//
int eewrite(int data, int address)
{
   unsigned short store = (unsigned short)data;

   if(address == 0)
      J_OpenEEPROM("wb");

   if(eepromFile)
   {
      fwrite(&store, sizeof(store), 1, eepromFile);
      
      if(address == 7)
      {
         fclose(eepromFile);
         eepromFile = NULL;
      }
   }

   return -1; // always successful (we currently lie if it wasn't...)
}

// EOF

