/*
	This header file may be freely copied and distributed.
*/

/*
	Windows OS Macros for the ISO file format library
*/

#ifndef INCLUDED_MP4OSMACROS_H
#define INCLUDED_MP4OSMACROS_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TEST_RETURN
#define TEST_RETURN(err)
/* #define TEST_RETURN(err) assert((err)==0)*/
#endif

#ifdef __CYGWIN__
#define MP4_EXTERN(v) extern v
#else
#ifdef ISOMP4DLLAPI
#define MP4_EXTERN(v) extern __declspec( dllexport ) v __cdecl
#else
#define MP4_EXTERN(v) extern v __cdecl
#endif
#endif

#if defined(_MSC_VER) && !defined(__MWERKS__)
typedef _int64 u64;
typedef _int64 s64;
#else
typedef long long u64;
typedef long long s64;
#endif
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int s32;
typedef short s16;
typedef char s8;

#define MP4_FOUR_CHAR_CODE( a, b, c, d ) (((a)<<24)|((b)<<16)|((c)<<8)|(d))
#define strcasecmp stricmp

#endif
