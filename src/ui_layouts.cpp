#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <glm/vec2.hpp>

#include "ui.h"
#include "ui_layouts.h"

static UILayoutComputation layout_computation_buffer[MAX_UI_BULK_COMPUTATION] = { 0 };

UILayoutBulkComputation ui_layout_bulk_comp(float parent_size, float *child_sizes,
                                            size_t child_size_count, UILayout layout) {
  UILayoutBulkComputation to_ret = { 0 };
  to_ret.computations = (UILayoutComputation*)&layout_computation_buffer;
  
  switch (layout.justify) {
    case (UI_LAYOUT_JSPACE_AROUND): {
      float leftover_space = parent_size;
      
      for (size_t i = 0; i < child_size_count; i++) {
        leftover_space -= child_sizes[i];
      }
      assert(leftover_space > 0.f); // TODO(kay): Handle negative
      float space_between = (leftover_space / (child_size_count*2));

      float pointer = 0.f;
      for (size_t i = 0; i < child_size_count; i++) {
        float child_size = child_sizes[i];

        pointer += space_between;
        float child_position = pointer;
        pointer += child_size + space_between;
        
        to_ret.computations[i].position = child_position;
        to_ret.num_computations++;
      }
    } break;
    
    case (UI_LAYOUT_JSPACE_BETWEEN): {
      p_UNIMPLEMENTED();
    } break;
    
    default: {
      p_UNIMPLEMENTED();
    } break;
  }
  
  return to_ret;
}

UILayoutComputation ui_layout_get_next(float parent_size, float child_size,
                                       UILayout layout, UILayoutData *pres_data) {
  UILayoutComputation to_ret = { 0 };
  
  switch (layout.justify) {
    case (UI_LAYOUT_JSTART): {
      if (pres_data->count == 0) {
        pres_data->val = 0;
      }
      
      to_ret.position = pres_data->val;
      pres_data->val += child_size + layout.padding;
      pres_data->count++;
      
      if (pres_data->val > parent_size - layout.margin) {
        to_ret.wrapped = true;
        pres_data->val = 0;
        to_ret.position = 0;
      }
    } break;
    
    case (UI_LAYOUT_JEND): {
      if (pres_data->count == 0) {
        pres_data->val = parent_size - layout.margin;
      }
      
      pres_data->val -= child_size + layout.padding;
      pres_data->count++;
      to_ret.position = pres_data->val;
      
      if (pres_data->val < 0) {
        to_ret.wrapped = true;
        pres_data->val = 0;
        to_ret.position = 0;
      }
      
      to_ret.position -= layout.margin; // To cancel out the addition we do later
    } break;
    case (UI_LAYOUT_JSPACE_AROUND): {
      p_UNIMPLEMENTED();
    } break;
    case (UI_LAYOUT_JSPACE_BETWEEN): {
      p_UNIMPLEMENTED();
    } break;
    case (UI_LAYOUT_JCENTER): {
      p_UNIMPLEMENTED();
    } break;
    
    default: {
      p_UNREACHABLE();
    } break;
  }
  
  to_ret.position += layout.margin;
  return to_ret;
}

