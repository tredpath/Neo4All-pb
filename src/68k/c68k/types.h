
#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef _arch_dreamcast
#include <arch/types.h>
#else
/*typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;*/

typedef signed char int8;
typedef signed short int int16;
typedef signed long int int32;
#endif


// target specific
///////////////////

#if defined(_arch_dreamcast) || defined(__QNXNTO__)
#ifndef __sh__
#define __sh__
#endif
#else
#ifndef __x86__
#define __x86__
#endif
#endif

// compiler specific
/////////////////////

#ifndef INLINE
#ifdef __x86__
#define INLINE      __inline__
#else
#define INLINE
#endif
#endif

#ifndef FASTCALL
#ifdef __win32__
#define FASTCALL    __fastcall
#else
#define FASTCALL
#endif
#endif

// types definitions
/////////////////////

#ifndef NULL
#define NULL 0
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef signed char S8;
typedef signed short S16;
typedef signed int S32;


#endif /* _TYPES_H_ */

