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
  ERR_NOKAY = -1,
  
  // p_array, series (0xAA0)
  ERR_P_ARRAY        = 0xAA0, // - Generic Error
  ERR_P_ARRAY_MALLOC = 0xAA1, // - Error creating p_array (malloc fail, too big?)
  ERR_P_ARRAY_RELLOC = 0xAA2, // - Error resizing p_array (realloc fail, too big?)
  
  // Resource Manager, series (0xAB0)
  ERR_RM                   = 0xAB0, // - Generic Error
  ERR_RM_INIT_FAIL         = 0xAB1, // - Failed to init the resource manager
  ERR_RM_LOADING_RESOURCES = 0xAB2, // - Error loading in resource
  ERR_RM_NOT_FOUND         = 0xAB3, // - Getting or deleting resource that doesn't exist
  ERR_RM_RESOURCE_EXISTS   = 0xAB4, // - Creating a resource that already exists
} err_code;
#define ASSERT_ERR(err) if (err != ERR_OKAY) {\
                          fprintf(stderr, "[ERROR] %s:%d: EXPECTED 0x0 (ERR_OKAY) GOT ERROR: 0x%X.\n", __FILE__, __LINE__, err); \
                          exit(1); \
                        }

#define byte u8

#define bytes
#define function
#define fn
#define constructor
#define destructor
#define private static

#define sizeof_array(array) sizeof(array)/sizeof(array[0])
