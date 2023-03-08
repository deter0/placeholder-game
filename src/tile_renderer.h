#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <allegro5/allegro5.h>
#include <allegro5/bitmap.h>

#include "p/p.h"
#include "ui.h"
#include "audio_manager.h"
#include "resource_manager.h"

struct TextureAtlas {
  ALLEGRO_BITMAP *atlas_image;
  u32 texture_size;
  u32 row_count;
  u32 column_count;
  u32 total;
};

struct TileMapDataLayer {
  i32 layer_index;
  i32 *layer_tiles_data;
  u32 layer_tiles_count;
};

#define MAX_LAYER_POINTERS 32
struct TileMapData {
  TileMapDataLayer *layers[MAX_LAYER_POINTERS];
  u32 layers_count;
  u32 debug_layer_index;
};

struct TileMap {
  const char *identifier;
  TileMapData data;
  
  u32 tile_size;
  u32 tiles_count_x;
  u32 tiles_count_y;
  TextureAtlas atlas;
  
  glm::vec2 map_renderer_res;
  ALLEGRO_BITMAP *map_renderer_bitmap;
  
  glm::vec3 camera_position;
};

p_fn err_code tr_init();

p_fn err_code tr_create_tile_map(const char *map_name, uint tiles_count_x, uint tiles_count_y, uint tile_size, TileMap **out);
p_fn err_code tr_save_tile_map_data_to_file(TileMap *tile_map, const char *file_name);

p_fn err_code tr_map_provide_atlas(TileMap *tile_map, ALLEGRO_BITMAP *atlas, u32 texture_size, u32 texture_count);
p_fn err_code tr_tile_map_cam_input(TileMap *map, ALLEGRO_KEYBOARD_STATE *state, double delta_time);

p_fn err_code tr_tile_map_render(TileMap *map, UIState *state, bool debug);
