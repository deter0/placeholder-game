#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ui.h"

#define MAX_UI_BULK_COMPUTATION 128

typedef struct UILayoutData {
  int count;
  float val;
} UILayoutData;

typedef struct UILayoutComputation {
  float position;
  bool wrapped;
} UILayoutComputation;

typedef struct UILayoutBulkComputation {
  UILayoutComputation *computations;
  size_t num_computations;
} UILayoutBulkComputation;

UILayoutBulkComputation ui_layout_bulk_comp(float parent_size, float *child_sizes,
                                            size_t child_size_count, UILayout layout);
UILayoutComputation ui_layout_get_next(float parent_size, float child_size,
                                       UILayout layout, UILayoutData *pres_data);
