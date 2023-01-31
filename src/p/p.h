#pragma once

#include <stdint.h>

#define i64 int64_t
#define i32 int32_t
#define i16 int16_t
#define i8 int8_t

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t

#define f64 double_t
#define f32 float_t

typedef enum err_code {
  ERR_OKAY = 0,
  
  // p_array, series (0xAA0)
  ERR_P_ARRAY        = 0xAA0,
  ERR_P_ARRAY_MALLOC = 0xAA1, // - Error creating p_array (malloc fail, too big?)
  ERR_P_ARRAY_RELLOC = 0xAA2, // - Error resizing p_array (realloc fail, too big?)
  
  // Resource Manager, series (0xAB0)
  ERR_RM = 0xAB0,
  ERR_RM_LOADING_RESOURCES = 0xAB1,
  ERR_RM_NOT_FOUND         = 0xAB2,
  ERR_RM_RESOURCE_EXISTS   = 0xAB3
} err_code;

#define byte u8

#define bytes
#define function
#define fn
#define constructor
#define destructor
#define private static

#define sizeof_array(array) sizeof(array)/sizeof(array[0])
