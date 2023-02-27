#include <allegro5/allegro_font.h>
#include "p/p.h"

#define MAX_LOADED_SIZES 16

p_private u32 FONT_SIZES[] = {
  10, 12, 18, 24, 30, 36, 48, 60, 72
};
static_assert(p_sizeof_array(FONT_SIZES) < MAX_LOADED_SIZES);

typedef struct p_font {
  ALLEGRO_FONT *loaded_sizes[MAX_LOADED_SIZES];
} p_font;

p_constructor p_fn err_code pfont_load_from_file(const char *font_file_path, p_font **output);
p_fn ALLEGRO_FONT *pfont_get_size(p_font *font, u32 size);
