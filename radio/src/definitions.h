/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <inttypes.h>

#if defined(SIMU) &&  __GNUC__
  #define __ALIGNED(x)        __attribute__((aligned(x)))
  #define __SECTION_USED(s)   __attribute__((used))
#elif defined(SIMU)
  #define __ALIGNED(x)        __declspec(align(x))
  #define __SECTION_USED(s)
#else
  #define __ALIGNED(x)        __attribute__((aligned(x)))
  #define __SECTION_USED(s)   __attribute__((section(s), used))
#endif

#if defined(SIMU)
  #define __DMA
#elif (defined(STM32F4) && !defined(BOOT)) || defined(PCBHORUS) || defined(PCBNV14)
  #define __DMA __attribute__((section(".ram"), aligned(32)))
#else
  #define __DMA __ALIGNED(4)
#endif

#if (defined(PCBHORUS) || defined(PCBNV14)) && !defined(SIMU)
  #define __SDRAM   __attribute__((section(".sdram"), aligned(4)))
  #define __NOINIT  __attribute__((section(".noinit")))
#else
  #define __SDRAM   __DMA
  #define __NOINIT
#endif

#if defined(SIMU) || defined(CPUARM) || GCC_VERSION < 472
typedef int32_t int24_t;
#else
typedef __int24 int24_t;
#endif

#if __GNUC__
  #define PACK( __Declaration__ )      __Declaration__ __attribute__((__packed__))
#else
  #define PACK( __Declaration__ )      __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

#if defined(SIMU)
  #if !defined(FORCEINLINE)
    #define FORCEINLINE inline
  #endif
  #if !defined(NOINLINE)
    #define NOINLINE
  #endif
  #define CONVERT_PTR_UINT(x) ((uint32_t)(uint64_t)(x))
  #define CONVERT_UINT_PTR(x) ((uint32_t*)(uint64_t)(x))
#else
  #define FORCEINLINE inline __attribute__ ((always_inline))
  #define NOINLINE    __attribute__ ((noinline))
  #define SIMU_SLEEP(x)
  #define SIMU_SLEEP_NORET(x)
  #define CONVERT_PTR_UINT(x) ((uint32_t)(x))
  #define CONVERT_UINT_PTR(x) ((uint32_t *)(x))
#endif

// RESX range is used for internal calculation; The menu says -100.0 to 100.0; internally it is -1024 to 1024 to allow some optimizations
#define RESX_SHIFT 10
#define RESX       1024
#define RESXu      1024u
#define RESXul     1024ul
#define RESXl      1024l

#undef DIM
#define DIM(arr) (sizeof((arr))/sizeof((arr)[0]))

#if defined(__cplusplus)
template<class t> FORCEINLINE t min(t a, t b) { return a<b?a:b; }
template<class t> FORCEINLINE t max(t a, t b) { return a>b?a:b; }
template<class t> FORCEINLINE t sgn(t a) { return a>0 ? 1 : (a < 0 ? -1 : 0); }
template<class t> FORCEINLINE t limit(t mi, t x, t ma) { return min(max(mi,x),ma); }
template<class t> void SWAP(t & a, t & b) { t tmp = b; b = a; a = tmp; }
#endif

#if defined(CPUARM)
typedef const unsigned char pm_uchar;
typedef const char pm_char;
typedef const uint16_t pm_uint16_t;
typedef const uint8_t pm_uint8_t;
typedef const int16_t pm_int16_t;
typedef const int8_t pm_int8_t;
#endif

#if !defined(CPUAVR) || defined(SIMU)
#define PROGMEM
#endif

#endif // _DEFINITIONS_H_
