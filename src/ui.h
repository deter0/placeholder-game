#pragma once

#include <allegro5/keyboard.h>

#include "p/p.h"
#include "p/parr.h"

#include "pfont.h"

typedef struct UIState {
  glm::vec2 mouse_position;
  glm::vec2 window_size;
  ALLEGRO_KEYBOARD_STATE *keyboard_state;
} UIState;

enum UILayoutDir : u8 {
  UI_LAYOUT_DISABLE = 0,
  UI_LAYOUT_DHORZ,
  UI_LAYOUT_DVERT,
  UI_LAYOUT_DHORZVERT,
};

enum UIUnit : u8 {
  UI_UPIXS = 0,
  UI_UPERC,
  UI_UAUTO
};

enum UILayoutJustify : u8 {
  UI_LAYOUT_JSTART = 0,
  UI_LAYOUT_JCENTER,
  UI_LAYOUT_JSPACE_BETWEEN,
  UI_LAYOUT_JSPACE_AROUND,
  UI_LAYOUT_JEND
};

typedef struct UIValue {
  float val;   // 4 bytes
  UIUnit unit; // 1 byte
  char pad[3]; // 3 bytes (4 + 1 + 3 = 8 for alignment) 
} __attribute__((packed)) UIValue;

typedef struct UIVec {
  glm::vec2 val;   // 8 bytes
  UIUnit unit_x;   // 1 byte
  UIUnit unit_y;   // 1 byte
  char padding[2]; // 2 bytes (8 + 1 + 1 + 2 = 12 byte padded)
} __attribute__((packed)) UIVec;

typedef struct UILayout {
  u8 enable;
  enum UILayoutJustify justify;

  // Padding -> space between elements (e.g. [ ]xxxx[ ])
  float padding;
  // Margin -> space around edges of container
  float margin;
} UILayout;

typedef struct UIBorder {
  float size;
  ALLEGRO_COLOR color;
} UIBorder;

static u32 object_tick = 0;
typedef struct UIObject {
  char name[32];
  u32 object_id;

  UIVec size;
  UIVec position;
  
  // Center Unit is percent (%)
  glm::vec2 center;
  
  glm::vec2 computed_size;
  glm::vec2 computed_position;

  ALLEGRO_COLOR pri_color;
  ALLEGRO_COLOR alt_color;
  UIBorder border;

  char   *text;
  p_font *text_font;
  float   text_font_size;
  
  ALLEGRO_BITMAP *image;

  UILayout layout_x;
  UILayout layout_y;

  u32 children_count;
  u32 children_max;
  struct UIObject **children;

  struct UIObject *parent;
} UIObject;


UIObject *ui_new_object();
UIObject *ui_object_dup(UIObject *subject);

void ui_set_object_parent(UIObject *subject, UIObject *new_parent);
void ui_render(UIObject *subject, UIState *ui_state);