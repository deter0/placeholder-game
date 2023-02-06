#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <allegro5/allegro5.h>
#include <allegro5/bitmap.h>

#include "p/p.h"
#include "audio_manager.h"
#include "resource_manager.h"

typedef struct TileMap {
  const char *identifier;
  int *map_data;
  u32 texture_size;
  u32 texture_atlas_row_count;
  glm::vec3 camera_position;
};

p_fn err_code tr_init();

p_fn err_code tr_new_tile_map(int *map_data, const char *map_identifier,
                           const char *texture_atlas_path, u32 texture_size,
                           u32 texture_atlas_row_count, TileMap **output_map);

p_fn err_code tr_tile_map_render(TileMap *map, ALLEGRO_BITMAP *output_bitmap, bool debug);

p_fn err_code tr_tile_map_cam_input(TileMap *map, ALLEGRO_KEYBOARD_STATE *state, double delta_time);
