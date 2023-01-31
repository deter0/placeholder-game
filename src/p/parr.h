#pragma once

#include "p.h"
#include "assert.h"

typedef struct p_array {
  void *m_mem;
  u64   mem_size;
  u64   mem_used;
} p_array;

struct p_array_ret {
  p_array array;
  err_code error;
};

typedef struct p_arr_ref {
  u64 a;
  u64 b;
} p_arr_ref;

constructor fn struct p_array_ret p_new_array(u64 initial_size);
destructor  fn void   p_del_array(p_array   *array);

fn void  p_arr_push_(p_array *array, void *elm, u64 bytes elm_size);
fn void* p_arr_pop(p_array *array);

fn void p_arr_at_(p_array *array, u64 index, u64 bytes elm_size, void *out);
fn void p_arr_atb(p_array *array, u64 bytes index, u64 bytes elm_size, void *out);

#define p_arr_get(type, name, array, index) type name; \
                                            p_arr_at_(array, index, sizeof(type), &name)
#define p_arr_push(array, var) p_arr_push_(array, &var, sizeof(var))
