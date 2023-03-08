#include <allegro5/allegro_ttf.h>
#include "p/p.h"
#include "pfont.h"

// Crash when a font size fails to load
#define PFONT_CRASH_ON_NLOAD true
#define PFONT_AL_TTF_FONT_FLAGS 0

p_fn err_code pfont_load_from_file(const char *font_file_path, p_font **output) {
  p_font *loaded_font = (p_font*)calloc(1, sizeof(p_font));
  if (!loaded_font) {
    return ERR_PF_ALLOC;
  }
  
  for (u32 i = 0; i < p_sizeof_array(FONT_SIZES); i++) {
    u32 size_to_load = FONT_SIZES[i];
    
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
    ALLEGRO_FONT *loaded_size = al_load_ttf_font(font_file_path, size_to_load, PFONT_AL_TTF_FONT_FLAGS);
    al_set_new_bitmap_flags(0);
    if (!loaded_size) {
      fprintf(stderr, "[WARNING]: Failed to load font: `%s` at size: %d\n", font_file_path, size_to_load);
      if (PFONT_CRASH_ON_NLOAD) {
        fprintf(stderr, "\t^[ERROR]: PFONT_CRASH_ON_NLOAD set to true. Crashing.\n");
        exit(1);
      }
    }
    
    loaded_font->loaded_sizes[i] = loaded_size;
  }
  
  (*output) = loaded_font;
  
  return ERR_OKAY;
}

p_fn err_code pfont_destroy(p_font *font) {
  if (!font) {
    return ERR_PF_BAD_FONT;
  }
  
  for (u32 i = 0; i < MAX_LOADED_SIZES; i++) {
    if (font->loaded_sizes[i] != 0) {
      al_destroy_font(font->loaded_sizes[i]);
    }
  }
  free(font);
  
  return ERR_OKAY;
}

p_private p_fn i32 pfont_round_size_to_sizes_index(u32 size) {
  for (i32 i = 0; i < p_sizeof_array(FONT_SIZES); i++) {
    if (i > 0) {
      if (size <= FONT_SIZES[i] && size >= FONT_SIZES[i - 1]) {
        return i;
      }
    } else {
      if (size <= FONT_SIZES[i]) {
        return 0;
      }
    }
  }

  return -1;
}

p_fn ALLEGRO_FONT *pfont_get_size(p_font *font, u32 size) {
  u32 selected_size_index = pfont_round_size_to_sizes_index(size);  
  
  assert(selected_size_index != -1);
  assert(font->loaded_sizes[selected_size_index]);

  return font->loaded_sizes[selected_size_index];
}
