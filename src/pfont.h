#pragma once

#include <allegro5/allegro_font.h>
#include "p/p.h"

#define MAX_LOADED_SIZES 16

p_private u32 FONT_SIZES[] = {
  10, 12, 14, 16, 18, 22, 28, 30, 35, 40, 50, 60, 72, 128
};
static_assert(p_sizeof_array(FONT_SIZES) < MAX_LOADED_SIZES);

typedef struct p_font {
  ALLEGRO_FONT *loaded_sizes[MAX_LOADED_SIZES];
} p_font;

p_constructor p_fn err_code pfont_load_from_file(const char *font_file_path, p_font **output);
p_destructor  p_fn err_code pfont_destroy(p_font *font);
p_fn ALLEGRO_FONT *pfont_get_size(p_font *font, u32 size);
