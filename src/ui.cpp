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

static float ui_compute_ui_val(UIObject *object, UIValue val) {
  switch (val.unit) {
    case (UI_UPIXS): {
      return val.val;
    } break;
    case (UI_UPERC): {
      p_UNIMPLEMENTED();
    } break;
    case (UI_UAUTO): {
      p_UNIMPLEMENTED();
    }
    default: {
      p_UNREACHABLE();
    } break;
  }
}

static glm::vec2 ui_compute_ui_vec2(UIObject *object, UIVec vec2) {
  return glm::vec2(
    ui_compute_ui_val(object, (UIValue){
      .val = vec2.val.x,
      .unit = vec2.unit_x
    }),
    ui_compute_ui_val(object, (UIValue){
      .val = vec2.val.y,
      .unit = vec2.unit_y
    })
  );
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

UIObject *ui_object_dup(UIObject *subject) {
  UIObject *new_object = (UIObject*)malloc(sizeof(*subject));
  memcpy(new_object, subject, sizeof(*subject));
  new_object->object_id = ++object_tick;
  
  return new_object;
}

static void ui_compute_sizes(UIObject *subject, UIState *state) {
  for (u32 i = 0; i < subject->children_count; i++) {
    ui_compute_sizes(subject->children[i], state);
  }
  
  UIObject *parent = subject->parent;
  glm::vec2 parent_size = state->window_size;
  if (parent) {
    parent_size = parent->computed_size;
    if (false && parent->layout.enable) {
      parent_size -= glm::vec2(ui_compute_ui_val(parent, parent->layout.margin) * 2.f);
    }
  }
  
  glm::vec2 text_size;
  if (subject->text && subject->text_font) {
    int bbx, bby, bbw, bbh;
    ALLEGRO_FONT *font = pfont_get_size(subject->text_font, (u32)subject->text_font_size);
    al_get_text_dimensions(font, subject->text,
                           &bbx, &bby, &bbw, &bbh);
    
    text_size.x = bbw;
    text_size.y = bbh;
  } else if (subject->text && !subject->text_font) {
    fprintf(stderr, "[WARNING]: Text set on object but not font set. (Object ID: %d, Text: '%s')!\n", subject->object_id, subject->text);
  }

  float computed_size_x = 0.f;
  switch (subject->size.unit_x) {
    case (UI_UPERC): {
      computed_size_x = parent_size.x * subject->size.val.x;
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
      computed_size_x = subject->size.val.x;
    } break;
  }
  subject->computed_size.x = computed_size_x;

  float computed_size_y = 0.f;
  {
    switch (subject->size.unit_y) {
      case (UI_UPERC): {
        computed_size_y = parent_size.y * subject->size.val.y;
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
        computed_size_y = subject->size.val.y;
      } break;
    }
  }
  subject->computed_size.y = computed_size_y;
}

static void ui_compute_just_start(UIObject *parent, UIObject *child, float *remember_x) {
  child->computed_position.x = *remember_x;
  (*remember_x) += child->computed_size.x + ui_compute_ui_val(parent, parent->layout.padding) + ui_compute_ui_val(parent, parent->layout.margin);
}
// TODO(kay): Layouts inside layouts with automatic sizes? Too complicated for now, do later.
//            -> Can compute child layouts first and then figure out their size and then computer our layout.
static void ui_compute_layouts(UIObject *subject, UIState *state) {
  for (u32 i = 0; i < subject->children_count; i++) {
    ui_compute_layouts(subject->children[i], state);
  }
  
  UILayout current_layout = subject->layout;
  
  float remember_x = 0;
  for (size_t i = 0; i < subject->children_count; i++) {
    switch (current_layout.justify) {
      case (UI_LAYOUT_JUST_START): {
        ui_compute_just_start(subject, subject->children[i], &remember_x);
      } break;
      
      case (UI_LAYOUT_JUST_CENTER): {
        p_UNIMPLEMENTED();
      } break;
      
      case (UI_LAYOUT_JUST_SPACE_AROUND): {
        p_UNIMPLEMENTED();
      } break;
      
      case (UI_LAYOUT_JUST_SPACE_BETWEEN): {
        p_UNIMPLEMENTED();
      } break;
    }
  }
  
  // Percent Positions
  glm::vec2 parent_pos = glm::vec2();
  glm::vec2 parent_size = state->window_size;
  if (subject->parent) {
    parent_pos = subject->parent->computed_position;
    parent_size = subject->parent->computed_size;
  }

  glm::vec2 subject_pos = ui_compute_ui_vec2(subject, subject->position);
  if (subject->position.unit_x == UI_UPERC) {
    subject->computed_position.x = parent_pos.x + parent_size.x * subject_pos.x;
  }
  if (subject->position.unit_y == UI_UPERC) {
    subject->computed_position.y = parent_pos.y + parent_size.y * subject_pos.y;
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

static inline void ui_draw_text(UIObject *subject, UIState *state) {
  // TODO(kay): Keep previously drawn text in a cache bitmap
  ALLEGRO_FONT *font = pfont_get_size(subject->text_font, (u32)subject->text_font_size);
  
  al_draw_text(font, subject->pri_color,
               subject->computed_position.x, subject->computed_position.y, 
               0, subject->text);
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
