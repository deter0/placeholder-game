#include <stdio.h>
#include <stdlib.h>

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>

#include <glm/glm.hpp>

#include "ui.h"

#include "resource_manager.h"
#include "p/p.h"
#include "p/parr.h"

static void ui_object_remove_child_indexed(UIObject *subject, u32 child_index) {
  for (u32 i = child_index; i < subject->children_count; i++) {
    subject->children[i] = subject->children[i + 1];
  }
  subject->children_count--;
}

void ui_set_object_parent(UIObject *subject, UIObject *new_parent) {
  assert(subject != NULL);
  
  if (subject->parent != NULL) {
    for (u32 i = 0; i < subject->parent->children_count; i++) {
      UIObject *child = subject->parent->children[i];
      if (child == subject && child->object_id == subject->object_id) {
        ui_object_remove_child_indexed(subject->parent, i);
        break;
      }
    }
  }

  if (new_parent) {   
    // TODO(kay): Dynamically allocate more children slots
    assert(new_parent->children_count <= new_parent->children_max);

    new_parent->children[new_parent->children_count++] = subject;

    subject->parent = new_parent;
  }  
}

UIObject *ui_new_object() {
  // TODO(kay): Use a memory pool for optimization & make code cache friendly when looping
  UIObject *created_object = (UIObject*)calloc(1, sizeof(UIObject));
  assert(created_object != NULL);

  created_object->children_max = 128;
  created_object->children = (UIObject**)calloc(created_object->children_max, sizeof(UIObject*));
  if (!created_object->children) {
    free(created_object);
    return NULL;
  }
  created_object->object_id = ++object_tick;
  created_object->pri_color = al_map_rgb(0, 0, 0);
  
  return created_object;
}

static void ui_compute_sizes(UIObject *subject, UIState *state) {
  for (u32 i = 0; i < subject->children_count; i++) {
    ui_compute_sizes(subject->children[i], state);
  }
  
  UIObject *parent = subject->parent;
  glm::vec2 parent_size = state->window_size;
  if (parent) {
    parent_size = parent->computed_size;
    if (parent->layout.direction != UI_LAYOUT_DISABLE) {
      parent_size -= glm::vec2(parent->layout.margin*2.f);
    }
  }
  
  glm::vec2 text_size;
  if (subject->text && subject->text_font) {
    int bbx, bby, bbw, bbh;
    al_get_text_dimensions(subject->text_font, subject->text,
                           &bbx, &bby, &bbw, &bbh);
    
    float ratio = (float)(bbh)/(float)(bbw);
    float new_w = subject->text_font_size/ratio;
    
    text_size.x = new_w;
    text_size.y = subject->text_font_size;
  } else if (subject->text && !subject->text_font) {
    fprintf(stderr, "[WARNING]: Text set on object but not font set. (Object ID: %d, Text: '%s')!\n", subject->object_id, subject->text);
  }

  float computed_size_x = 0.f;
  switch (subject->size_units[0]) {
    case (UI_UPERC): {
      computed_size_x = parent_size.x * subject->size.x;
    } break;

    case (UI_UAUTO): {
      if (text_size.x > 0 || text_size.y > 0) {
        computed_size_x = text_size.x;
      } else {
        // Post Layout Position Computations
        p_UNIMPLEMENTED();
      }
    } break;
    
    case (UI_UPIXS):
    default: {
      computed_size_x = subject->size.x;
    } break;
  }
  subject->computed_size.x = computed_size_x;

  float computed_size_y = 0.f;
  {
    switch (subject->size_units[1]) {
      case (UI_UPERC): {
        computed_size_y = parent_size.y * subject->size.y;
      } break;

      case (UI_UAUTO): {
        if (text_size.x > 0 || text_size.y > 0) {
          computed_size_y = text_size.y;
        } else {
          // Post Layout Position Computations
          p_UNIMPLEMENTED();
        }
      } break;
      
      case (UI_UPIXS):
      default: {
        computed_size_y = subject->size.y;
      } break;
    }
  }
  subject->computed_size.y = computed_size_y;
}

// TODO(kay): Layouts inside layouts with automatic sizes? Too complicated for now, do later.
//            -> Can compute child layouts first and then figure out their size and then computer our layout.
static void ui_compute_layouts(UIObject *subject, UIState *state) {
  for (u32 i = 0; i < subject->children_count; i++) {
    ui_compute_layouts(subject->children[i], state);
  }
  
  UILayout current_layout = subject->layout;
  if (current_layout.direction != UI_LAYOUT_DISABLE) {
    float fx = 0.f, fy = 0.f;
    float row_max_obj_height = 0;
    u32   objects_on_row = 0;
    float right = subject->computed_size.x;
    
    for (u32 i = 0; i < subject->children_count; i++) {
      UIObject *child = subject->children[i];

      switch (current_layout.direction) {
        case (UI_LAYOUT_DHORZVERT): {
          if (child->computed_size.y > row_max_obj_height) {
            row_max_obj_height = child->computed_size.y;
          }
          
          if ((fx + current_layout.padding + child->computed_size.x) > (right - current_layout.margin) && objects_on_row > 0) {
            fx = 0;
            fy += row_max_obj_height + current_layout.padding;
            row_max_obj_height = 0.f;
            objects_on_row = 0;
          }
          objects_on_row++;
          
          child->computed_position.x = subject->computed_position.x + fx + current_layout.margin;
          child->computed_position.y = subject->computed_position.y + fy + current_layout.margin;
          
          fx += child->computed_size.x + current_layout.padding;
        } break;

        case (UI_LAYOUT_DHORZ): {
          p_UNIMPLEMENTED();
        }
        case (UI_LAYOUT_DVERT): {
          p_UNIMPLEMENTED();
        } break;

        case (UI_LAYOUT_DISABLE):
        default: {
          subject->computed_position = glm::vec2(0.f);
        } break;
      }
    }
  }
  
  glm::vec2 parent_pos = glm::vec2();
  glm::vec2 parent_size = state->window_size;
  if (subject->parent) {
    parent_pos = subject->parent->computed_position;
    parent_size = subject->parent->computed_size;
  }

  if (subject->position_units[0] == UI_UPERC) {
    subject->computed_position.x = parent_pos.x + parent_size.x * subject->position.x;
  }
  if (subject->position_units[1] == UI_UPERC) {
    subject->computed_position.y = parent_pos.y + parent_size.y * subject->position.y;
  }
  
  if (subject->center.x != 0.f && subject->center.y != 0.f) {
    subject->computed_position -= subject->computed_size * subject->center;
  }
}

static inline void ui_draw_image(UIObject *subject, UIState *state) {
  al_draw_scaled_bitmap(subject->image,
                        0, 0,
                        al_get_bitmap_width(subject->image), al_get_bitmap_height(subject->image),
                        subject->computed_position.x,
                        subject->computed_position.y,
                        subject->computed_size.x,
                        subject->computed_size.y,
                        0);
}

static inline void ui_draw_border(UIObject *subject, UIState *state) {
  al_draw_rectangle(subject->computed_position.x, subject->computed_position.y,
                      subject->computed_position.x + subject->computed_size.x,
                      subject->computed_position.y + subject->computed_size.y,
                      subject->border.color,
                      subject->border.size);
}

#define TEXT_DRAW_BUFFER_W 1920
#define TEXT_DRAW_BUFFER_H 256
static ALLEGRO_BITMAP *text_draw_buffer = 0;

static inline void ui_draw_text(UIObject *subject, UIState *state) {
  if (!text_draw_buffer) {
    text_draw_buffer = al_create_bitmap(TEXT_DRAW_BUFFER_W, TEXT_DRAW_BUFFER_H);
    if (text_draw_buffer == 0) {
      fprintf(stderr, "[ERROR (UI, %s:%d)] Failed to create text_draw_buffer!\n", __FILE__, __LINE__);
      exit(1);
    }
  }
  
  // TODO(kay): Keep previously drawn text in a cache bitmap
  ALLEGRO_BITMAP *previous_target = al_get_target_bitmap();
  
  al_set_target_bitmap(text_draw_buffer);
  al_clear_to_color(al_map_rgba(0, 0, 0, 0));
  
  int bbx = 0, bby = 0, bbw = 0, bbh = 0;
  al_get_text_dimensions(subject->text_font, subject->text,
                         &bbx, &bby, &bbw, &bbh);
                         
  int iterations = (int)ceilf((float)bbw / (float)TEXT_DRAW_BUFFER_W);
  if (iterations <= 0) {
    iterations = 1;
  }
  
  float ratio = (float)bbh/(float)bbw;
  float new_w = subject->text_font_size/ratio;

  for (u32 iteration = 0; iteration < iterations; iteration++) {
    al_set_target_bitmap(text_draw_buffer);
    al_draw_text(subject->text_font, subject->pri_color,
                  TEXT_DRAW_BUFFER_W*iteration, 0, 0, subject->text);

    al_set_target_bitmap(previous_target);
    al_draw_scaled_bitmap(text_draw_buffer,
                          (float)bbx, (float)bby,
                          (float)bbw, (float)bbh,
                          subject->computed_position.x, subject->computed_position.y,
                          new_w, subject->text_font_size, 0);
  }
}

static inline void ui_draw_rect(UIObject *subject, UIState *state) {
  al_draw_filled_rectangle(subject->computed_position.x,
                           subject->computed_position.y,
                           subject->computed_position.x + subject->computed_size.x,
                           subject->computed_position.y + subject->computed_size.y,
                           subject->pri_color);  
}

void ui_draw(UIObject *subject, UIState *state) {
  if (subject->text == 0) {
    ui_draw_rect(subject, state);
  } else {
    ui_draw_text(subject, state);
  }
  
  if (subject->border.size > 0) {
    ui_draw_border(subject, state);
  }
  
  if (subject->image) {
    ui_draw_image(subject, state);
  }
  
  for (u32 i = 0; i < subject->children_count; i++) {
    ui_draw(subject->children[i], state);
  }
}

void ui_render(UIObject *subject, UIState *state) {   
  ui_compute_sizes(subject, state);
  ui_compute_layouts(subject, state);
  ui_draw(subject, state);
}
