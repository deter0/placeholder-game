#pragma once

#include <stdint.h>

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef double f64;
typedef float f32;

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
  
  // Audio Manager, series (0xAC0)
  ERR_AM                 = 0xAC0, // - Generic Error
  ERR_AM_AUDIO_NOT_FOUND = 0xAC1, // - Audio not found in `MusicFiles` or `SoundEffectFiles` arrays
} err_code;

#define p_ASSERT_ERR(err) if (err != ERR_OKAY) {\
                          fprintf(stderr, "[ERROR] %s:%d: EXPECTED 0x0 (ERR_OKAY) GOT ERROR: 0x%X.\n", __FILE__, __LINE__, err); \
                          exit(1); \
                        }

#define p_byte u8

#define p_bytes
#define p_function
#define p_fn
#define p_constructor
#define p_destructor
#define p_private static

#define p_sizeof_array(array) sizeof(array)/sizeof(array[0])
