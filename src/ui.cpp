#include <stdio.h>
#include <stdlib.h>

#include <allegro5/allegro_font.h>
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

void ui_render(UIObject *subject) {   
  if (subject->layout.direction != UI_LAYOUT_DISABLE) {
    UILayout current_layout = subject->layout;

    float fx = 0.f, fy = 0.f;
    float row_max_obj_height = 0;
    float right = subject->size.x;
    
    for (u32 i = 0; i < subject->children_count; i++) {
      UIObject *child = subject->children[i];

      switch (current_layout.direction) {
        case (UI_LAYOUT_DHORZVERT): {
          if (child->computed_size.y > row_max_obj_height) {
            row_max_obj_height = child->computed_size.y;
          }
          
          if ((fx + current_layout.padding + child->computed_size.x)
              > (right - current_layout.margin)) {
            fx = 0;
            fy += row_max_obj_height + current_layout.padding;
            row_max_obj_height = 0.f;
          }
          
          child->position.x = fx + current_layout.margin;
          child->position.y = fy + current_layout.margin;
          
          fx += child->computed_size.x + current_layout.padding;
        } break;

        case (UI_LAYOUT_DVERT): {
          
        } break;

        case (UI_LAYOUT_DISABLE):
        default: {
          subject->computed_position = glm::vec2(0.f);
        } break;
      }
    }
  }
  
  if (subject->text == 0) {
    al_draw_filled_rectangle(subject->position.x,
                             subject->position.y,
                             subject->position.x + subject->size.x,
                             subject->position.y + subject->size.y,
                             subject->pri_color);
  } else {
    ALLEGRO_FONT *default_font;
    rm_get_font("mw_20", &default_font);

    int bbx, bby, bbw, bbh;
    al_get_text_dimensions(default_font, subject->text,
                           &bbx, &bby, &bbw, &bbh);
    subject->computed_size.x = bbw + bbx;
    subject->computed_size.y = bbh + bby;
    
    al_draw_text(default_font, subject->pri_color,
                               subject->position.x,
                               subject->position.y,
                               0, subject->text);
  }
  
  if (subject->size_units[0] == UI_UAUTO) {
    subject->size.x = subject->computed_size.x;
  } else {
    subject->computed_size.x = subject->size.x;
  }
  if (subject->size_units[1] == UI_UAUTO) {
    subject->size.y = subject->computed_size.y;
  } else {
    subject->computed_size.y = subject->size.y;
  }
  
  if (subject->border.size > 0.f) {
    al_draw_rectangle(subject->position.x,
                      subject->position.y,
                      subject->position.x + subject->size.x,
                      subject->position.y + subject->size.y,
                      subject->border.color, subject->border.size);
  }
  for (u32 i = 0; i < subject->children_count; i++) {
    ui_render(subject->children[i]);
  }
}
