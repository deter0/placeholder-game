#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "p.h"
#include "parr.h"

static i64 default_size = 8;

p_constructor p_fn struct p_array_ret p_new_array(u64 initial_size)
{
  p_array *new_array = (p_array*)malloc(sizeof(p_array));
  if (!new_array) {
    struct p_array_ret ret = {0};
    ret.error = ERR_P_ARRAY_MALLOC;
    return ret;
  }
  
  void *new_memory;
  new_memory = calloc(1, initial_size > 0 ? initial_size : default_size);

  new_array->m_mem = new_memory;
  new_array->mem_size = initial_size > 0 ? initial_size : default_size;
  new_array->mem_used = 0;
  
  struct p_array_ret ret = {0};
  ret.array = new_array;
  if (ret.array->m_mem == 0) {
    ret.error = ERR_P_ARRAY_MALLOC;
    ret.array->mem_size = 0;
  }
  
  return ret;
}

p_destructor p_fn void p_del_array(p_array *array)
{
  if (array->m_mem) {
    free(array->m_mem);
  }
  
  return;
}

p_private p_fn err_code p_arr_resize(p_array *array, u64 new_size)
{
  printf("Resized array\n");
  void *new_mem = realloc(array->m_mem, new_size);
  if (new_mem == 0) {
    return ERR_P_ARRAY_RELLOC;
  }
  
  array->m_mem = new_mem;
  array->mem_size = new_size;

  return ERR_OKAY;
}

p_fn void p_arr_push_(p_array *array, void *elm, u64 p_bytes elm_size)
{
  if (array->mem_used + elm_size > array->mem_size) {
    err_code resize_status = p_arr_resize(array, (array->mem_size+1)*2);
    if (resize_status != ERR_OKAY) {
      resize_status = p_arr_resize(array, (array->mem_size+1)*1.5);
    }
    
    if (resize_status != ERR_OKAY) {
      // TODO(kay): PLOG
      fprintf(stderr, "[FATAL]: Failed to resize array. Dropping new element: %p\n", elm);
    }
  }
  
  memcpy(((u8*)array->m_mem)+array->mem_used, &elm, elm_size);
  array->mem_used += elm_size;
}
p_fn void p_arr_at_(p_array *array, u64 index, u64 p_bytes elm_size, void *out)
{
  memcpy(out, ((u8*)array->m_mem)+(index*elm_size), elm_size);
}
p_fn void p_arr_del_(p_array *array, u64 index, u64 p_bytes elm_size)
{
  u8 *memory = (u8*)array->m_mem;
  u64 g_index = (index*elm_size);

  assert(index+elm_size <= array->mem_size); 
  memmove(memory+g_index, memory+g_index+elm_size, array->mem_size - index+elm_size);
}
p_fn void p_arr_atb(p_array *array, u64 p_bytes index, u64 p_bytes elm_size, void *out)
{
  memcpy(out, ((u8*)array->m_mem)+(index), elm_size);
}
p_fn void* p_arr_pop(p_array *array);

