/*

  CALICO

  C/C++ compiler-specific keywords and support macros

*/

#ifndef KEYWORDS_H__
#define KEYWORDS_H__

//
// Types
//
typedef enum { false, true } boolean;
typedef unsigned char byte;

//
// Structure packing via #pragma pack
//
#if defined(_MSC_VER) || defined(__GNUC__)
#define CALICO_HAS_PACKING
#endif

#endif

// EOF
