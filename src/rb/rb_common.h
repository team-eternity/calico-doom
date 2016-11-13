/*
   Copyright(C) 2007-2014 Samuel Villarreal
   Copyright(C) 2014 Night Dive Studios, Inc.

   OpenGL Renderer Common Defines
*/

#ifndef RB_COMMON_H__
#define RB_COMMON_H__

#define D_RGBA(r,g,b,a) \
   ((unsigned int)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

#define D_BGRA(r,g,b,a) \
   ((unsigned int)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

#endif

// EOF

