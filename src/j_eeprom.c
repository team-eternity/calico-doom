/*
  CALICO
 
  Jaguar EEPROM Emulation
*/

#include <stdio.h>

static FILE *eepromFile;

//
// Open a file to use as the Jaguar EEPROM store.
//
static void J_OpenEEPROM(const char *mode)
{
   if(eepromFile)
   {
      fclose(eepromFile);
      eepromFile = NULL;
   }
   eepromFile = fopen("eeprom.cal", mode);
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

