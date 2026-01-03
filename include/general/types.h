#ifndef SNLS_TYPE_HEADER
#define SNLS_TYPE_HEADER

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int64_t s64;

#ifndef _EE
typedef uint32_t u32;
typedef int32_t s32;
#else
typedef unsigned int u32;
typedef int s32;
#endif

#endif
