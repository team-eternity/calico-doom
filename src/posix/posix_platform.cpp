/*
  CALICO
  
  POSIX Basic Platform Functions
  
  The MIT License (MIT)
  
  Copyright (c) 2017 James Haley
  
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

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)

#include <unistd.h>
#include <sys/stat.h>

#include "../elib/elib.h"
#include "../elib/misc.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "posix_platform.h"

using namespace std;

//
// Display a debug message.
//
static void POSIX_DebugMsg(const char *msg, ...)
{
#ifdef _DEBUG
   static FILE *error_log;

   if(!error_log)
   {
      qstring fn(POSIX_GetWriteDirectory());
      fn.pathConcatenate("output.txt");
      error_log = fopen(fn.constPtr(), "w");
   }

   if(error_log)
   {
      va_list args;
      va_start(args, msg);
      vfprintf(error_log, msg, args);
      va_end(args);      
   }
#endif
}

//
// Exit with a message that is not necessarily an error condition.
//
static void POSIX_ExitWithMsg(const char *msg, ...)
{
   va_list args;
   char buf[1024];

   va_start(args, msg);
   pvsnprintf(buf, sizeof(buf), msg, args);
   va_end(args);

   // try the media layer's method first; if it fails, use native
   if(hal_medialayer.msgbox("Calico", buf, HAL_FALSE))
   {
      fputs(buf, stderr); // try stderr (may have a console or be redirected)
      fputs("\n", stderr);
   }

   hal_medialayer.exit();
}

//
// Exit due to a fatal error
//
static void POSIX_FatalError(const char *msg, ...)
{
   va_list args;
   char buf[1024];

   va_start(args, msg);
   pvsnprintf(buf, sizeof(buf), msg, args);
   va_end(args);

   // try the media layer's method first; if it fails, use native
   if(hal_medialayer.msgbox("Calico", buf, HAL_TRUE))
   {
      fputs(buf, stderr); // try stderr (may have a console or be redirected)
      fputs("\n", stderr);
   }

   hal_medialayer.error();
}

//
// Create a directory
//
static bool POSIX_CreateDirectory(const char *name)
{
   return !mkdir(name, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
}

//
// Get the application write directory, creating it if it does not exist.
//
static const char *POSIX_GetWriteDirectory()
{
   static qstring writeDir;

   if(writeDir.empty())
   {
#if defined(__APPLE__)
      // MacOS X is a bit different, of course.
      writeDir = "~/Library/Application Support/net.mancubus.eternity.calico-doom";
      POSIX_CreateDirectory(writeDir.constPtr());      
#else
      // Try XDG standard location first; fallback to HOME otherwise.
      const char *s = nullptr;
      if((s = getenv("XDG_CONFIG_HOME")) && *s)
      {
         writeDir = s;
      }
      else if((s = getenv("HOME")) && *s)
      {
         writeDir = s;
         writeDir.pathConcatenate("/.config");
         POSIX_CreateDirectory(writeDir.constPtr());
      }
      if(s)
      {
         writeDir.pathConcatenate("/calico-doom");
         POSIX_CreateDirectory(writeDir.constPtr());
      }
#endif
   
      struct stat sbuf;
      if(stat(writeDir.constPtr(), &sbuf) || !S_ISDIR(sbuf.st_mode))
         POSIX_FatalError("Cannot locate a writable directory for save files; please reinstall the game.");
   }

   return writeDir.constPtr();
}

static void POSIX_SetIcon()
{
   // FIXME: Not implemented
}

//
// Populate the HAL platform interface with POSIX implementation function pointers
//
void POSIX_InitHAL(void)
{
   hal_platform.debugMsg          = POSIX_DebugMsg;
   hal_platform.exitWithMsg       = POSIX_ExitWithMsg;
   hal_platform.fatalError        = POSIX_FatalError;
   hal_platform.getWriteDirectory = POSIX_GetWriteDirectory;
   hal_platform.setIcon           = POSIX_SetIcon;
}

#endif

// EOF


