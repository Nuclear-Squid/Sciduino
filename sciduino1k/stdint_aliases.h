#pragma once

#include <stdint.h>

typedef   int8_t        i8;
typedef   int16_t       i16;
typedef   int32_t       i32;
typedef   int64_t       i64;

typedef   uint8_t       u8;
#if defined(ARDUINO_SAM_DUE) || defined(ARDUINO_GIGA)
typedef   uint16_t      u16;
#endif
typedef   uint32_t      u32;
typedef   uint64_t      u64;

typedef   float         f16;
typedef   float         f32;
typedef   double        f64;
typedef   long double   f128;


