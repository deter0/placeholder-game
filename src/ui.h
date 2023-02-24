#pragma once

#include "p/p.h"
#include "p/parr.h"


enum UILayoutDir {
  UI_LAYOUT_DISABLE = 0,
  UI_LAYOUT_DHORZ,
  UI_LAYOUT_DVERT,
  UI_LAYOUT_DHORZVERT,
};

enum UIUnit {
  UI_UPIXS = 0,
  UI_UPERC,
  UI_UAUTO
};

typedef struct UILayout {
  enum UILayoutDir direction;
  // Padding -> space between elements (e.g. [ ]xxxx[ ])
  float padding;
  // Margin -> space around edges of container
  float margin;
} UILayout;

typedef struct UIBorder {
  float size;
  ALLEGRO_COLOR color;
};

static u32 object_tick = 0;
typedef struct UIObject {
  u32 object_id;

  glm::vec2 size;
  enum UIUnit size_units[2];
  glm::vec2 position;
  enum UIUnit position_units[2];
  
  glm::vec2 computed_size;
  glm::vec2 computed_position;

  ALLEGRO_COLOR pri_color;
  ALLEGRO_COLOR alt_color;
  UIBorder border;

  char *text;

  UILayout layout;

  u32 children_count;
  u32 children_max;
  struct UIObject **children;

  struct UIObject *parent;
} UIObject;

void ui_set_object_parent(UIObject *subject, UIObject *new_parent);
UIObject *ui_new_object();
void ui_render(UIObject *subject);