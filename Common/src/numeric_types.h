#pragma once

#include <inttypes.h>

typedef uint8_t byte;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float float32;
typedef double float64;

constexpr uint8 MAX_UINT8 = 0xFFui8;
constexpr uint16 MAX_UINT16 = 0xFFFFui16;
constexpr uint32 MAX_UINT32 = 0xFFFFFFFFui32;
constexpr uint64 MAX_UINT64 = 0xFFFFFFFFFFFFFFFFui64;

constexpr int8 MAX_INT8 = 0x7Fui8;
constexpr int16 MAX_INT16 = 0x7FFFui16;
constexpr int32 MAX_INT32 = 0x7FFFFFFFui32;
constexpr int64 MAX_INT64 = 0x7FFFFFFFFFFFFFFFui64;

constexpr uint8 HALF_UINT8 = ( MAX_UINT8 / 2 );
constexpr uint16 HALF_UINT16 = ( MAX_UINT16 / 2 );
constexpr uint32 HALF_UINT32 = ( MAX_UINT32 / 2 );
constexpr uint64 HALF_UINT64 = ( MAX_UINT64 / 2 );

constexpr int8 MIN_INT8 = ( -MAX_INT8 );
constexpr int16 MIN_INT16 = ( -MAX_INT16 );
constexpr int32 MIN_INT32 = ( -MAX_INT32 );
constexpr int64 MIN_INT64 = ( -MAX_INT64 );

constexpr float32 MAX_FLOAT32 = 0x1.fffffep127f;
constexpr float64 MAX_FLOAT64 = 0x1.fffffffffffffp1023;

constexpr float32 MIN_FLOAT32 = ( -MAX_FLOAT32 );
constexpr float64 MIN_FLOAT64 = ( -MAX_FLOAT64 );
