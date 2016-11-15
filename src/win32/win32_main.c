/*
  CALICO

  Win32 Main Routine

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

#ifdef _WIN32

#include <windows.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

extern void Jag68k_main(int argc, const char *const *argv);

//
// Remove slash characters from escaped quotation marks
//
static void Win32_unEscapeQuotes(char *arg)
{
   char *last = NULL;

   while(*arg)
   {
      if(*arg == '"' && (last != NULL && *last == '\\'))
      {
         char *c_curr = arg;
         char *c_last = last;

         while(*c_curr)
         {
            *c_last = *c_curr;
            c_last = c_curr;
            ++c_curr;
         }
         *c_last = '\0';
      }
      last = arg;
      ++arg;
   }
}

//
// Parse the command line buffer into arguments
//
static int Win32_parseCommandLine(char *cmdline, char **argv)
{
   char *bufp;
   char *lastp = NULL;
   int argc, last_argc;

   argc = last_argc = 0;
   for(bufp = cmdline; *bufp; )
   {
      // skip leading whitespace
      while(isspace((int)*bufp))
         ++bufp;

      // skip over argument
      if(*bufp == '"')
      {
         ++bufp;
         if(*bufp)
         {
            if(argv)
               argv[argc] = bufp;
            ++argc;
         }

         // skip over word
         lastp = bufp;
         while(*bufp && (*bufp != '"' || *lastp == '\\'))
         {
            lastp = bufp;
            ++bufp;
         }
      }
      else
      {
         if(*bufp)
         {
            if(argv)
               argv[argc] = bufp;
            ++argc;
         }

         // skip over word
         while(*bufp && !isspace((int)*bufp))
            ++bufp;
      }

      if(*bufp)
      {
         if(argv)
            *bufp = '\0';
         ++bufp;
      }

      // strip out \ from \" sequences
      if(argv && last_argc != argc)
         Win32_unEscapeQuotes(argv[last_argc]);
      last_argc = argc;
   }

   if(argv)
      argv[argc] = NULL;

   return argc;
}

//
// Show an error dialog
//
static void ShowError(const char *title, const char *message)
{
   MessageBoxA(NULL, message, title, MB_ICONEXCLAMATION | MB_OK);
}

//
// Show an out-of-memory error dialog
//
static void OutOfMemory(void)
{
   ShowError("Fatal Error", "Out of memory");
}

//
// strdup polyfill
//
static char *mystrdup(const char *instr)
{
   size_t  len = strlen(instr) + 1;
   char   *buf = malloc(len);
   if(!buf)
   {
      OutOfMemory();
      return NULL;
   }
   return strncpy(buf, instr, len);
}

//
// WinMain function
//
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
   char **argv;
   int    argc;
   char  *cmdline;

   // grab the command lien
   char *text = GetCommandLineA();
   cmdline = mystrdup(text);

   if(!cmdline)
   {
      OutOfMemory();
      return 0;
   }

   // parse into argv, argc
   argc = Win32_parseCommandLine(cmdline, NULL);
   argv = (char **)(calloc(argc + 1, sizeof(char *)));
   if(!argv)
   {
      OutOfMemory();
      return 0;
   }
   Win32_parseCommandLine(cmdline, argv);

   // run application main program
   Jag68k_main(argc, argv);

   free(argv);
   free(cmdline);

   return 0;
}

#endif

// EOF

