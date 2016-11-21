/*
  CALICO
  
  Win32 Basic Platform Functions
  
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

#include "../elib/elib.h"
#include <direct.h>
#include <io.h>
#include <Windows.h>
#include "../../vc2015/resource.h"

#include "../elib/misc.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"

//
// Display a debug message. For Windows, this will open a debug console
// window the first time it is called.
//
static void Win32_DebugMsg(const char *msg, ...)
{
#ifdef _DEBUG
   va_list args;
   static BOOL debugInit = FALSE;
   size_t len = strlen(msg);

   if(!debugInit)
   {
      if(AllocConsole())
         freopen("CONOUT$", "w", stdout);
      debugInit = TRUE;
   }

   va_start(args, msg);
   vprintf(msg, args);
   va_end(args);

   if(len >= 1 && msg[len - 1] != '\n')
      printf("\n");
#endif
}

//
// Exit with a message that is not necessarily an error condition.
//
static void Win32_ExitWithMsg(const char *msg, ...)
{
   va_list args;
   char buf[1024];

   va_start(args, msg);
   pvsnprintf(buf, sizeof(buf), msg, args);
   va_end(args);

   // try the media layer's method first; if it fails, use native
   if(hal_medialayer.msgbox("Calico", buf, HAL_FALSE))
      MessageBoxA(NULL, buf, "Calico", MB_OK | MB_ICONINFORMATION);

   hal_medialayer.exit();
}

//
// Exit with a fatal error
//
static void Win32_FatalError(const char *msg, ...)
{
   va_list args;
   char buf[1024];

   va_start(args, msg);
   pvsnprintf(buf, sizeof(buf), msg, args);
   va_end(args);

   // try the media layer's method first; if it fails, use native
   if(hal_medialayer.msgbox("Calico", buf, HAL_TRUE))
      MessageBoxA(NULL, buf, "Calico", MB_OK | MB_ICONERROR);

   hal_medialayer.error();
}

//
// Obtain the application's base write directory.
// For Windows, this is the application directory.
//
static const char *Win32_GetWriteDirectory(void)
{
   return ".";
}

//
// Set the main application window icon. This implementation uses a native
// Win32 icon resource.
//
static void Win32_SetIcon(void)
{
   HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
   if(hIcon)
   {
      HWND hWnd = (HWND)(hal_video.getWindowHandle());
      if(hWnd)
      {
         SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
         SendMessage(hWnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
      }
   }
}

//
// Populate the HAL platform interface with Win32 implementation function pointers
//
void Win32_InitHAL(void)
{
   hal_platform.debugMsg          = Win32_DebugMsg;
   hal_platform.exitWithMsg       = Win32_ExitWithMsg;
   hal_platform.fatalError        = Win32_FatalError;
   hal_platform.getWriteDirectory = Win32_GetWriteDirectory;
   hal_platform.setIcon           = Win32_SetIcon;
}

#endif

// EOF

