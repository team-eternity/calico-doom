/*
  CALICO

  Win32 Main Routine
*/

#ifdef _WIN32

#include <Windows.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

extern void Jag68k_main(void);

static void UnEscapeQuotes(char *arg)
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
static int ParseCommandLine(char *cmdline, char **argv)
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
         UnEscapeQuotes(argv[last_argc]);
      last_argc = argc;
   }

   if(argv)
      argv[argc] = NULL;

   return argc;
}

static void ShowError(const char *title, const char *message)
{
   MessageBoxA(NULL, message, title, MB_ICONEXCLAMATION | MB_OK);
}

static void OutOfMemory(void)
{
   ShowError("Fatal Error", "Out of memory");
}

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
   argc = ParseCommandLine(cmdline, NULL);
   argv = (char **)(calloc(argc + 1, sizeof(char *)));
   if(!argv)
   {
      OutOfMemory();
      return 0;
   }
   ParseCommandLine(cmdline, argv);

   // run application main program
   Jag68k_main();

   free(argv);
   free(cmdline);

   return 0;
}

#endif

// EOF

