#include <stdio.h>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <allegro5/allegro5.h>
#include <allegro5/bitmap.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>

#include "p/p.h"
#include "audio_manager.h"
#include "resource_manager.h"

#include "tile_renderer.h"

// TODO(kay): Create `map_renderer_bitmap` with the TileMap struct and calculate the width and the height or get it as a parameter e.g. CreateMap(TilesX, TilesY, TilesSize, ...)
// TODO(kay): Restructure map data to be a continous buffer that is allocated when created, rewrite renderer
// TODO(kay): Reading and writing map data from a file

p_fn err_code tr_init() {
  p_ASSERT_ERR(rm_create_font("mw_20", "resources/Merriweather.ttf", 20));
  
  return ERR_OKAY;
}

p_fn err_code tr_save_tile_map_data_to_file(TileMap *tile_map, const char *file_name) {
  FILE *tile_map_data_file = fopen(file_name, "wb");
  
  u32 file_size = tile_map->data.tiles_size;
  fprintf(tile_map_data_file, "%d", file_size);

  fclose(tile_map_data_file);
  
  return ERR_OKAY;
}

p_fn err_code tr_create_tile_map(const char *map_name, uint tiles_count_x, uint tiles_count_y, uint tile_size, TileMap **out) {
  TileMap *tile_map = (TileMap*)malloc(sizeof(TileMap));
  if (tile_map == NULL) {
    return ERR_TR_ALLOC_MAP;
  }

  memset(tile_map, 0, sizeof(TileMap));
  
  uint renderer_w = tiles_count_x * tile_size;
  uint renderer_h = tiles_count_y * tile_size;
  
  tile_map->tiles_count_x = tiles_count_x;
  tile_map->tiles_count_y = tiles_count_y;
  
  tile_map->map_renderer_bitmap = al_create_bitmap(renderer_w, renderer_h);
  if (!tile_map->map_renderer_bitmap) {
    free(tile_map);
    return ERR_TR_ALLOC_MAP;
  }
  
  ALLEGRO_BITMAP *previous_target = al_get_target_bitmap();
  al_set_target_bitmap(tile_map->map_renderer_bitmap);
  al_clear_to_color(al_map_rgb(255, 0, 255));
  al_set_target_bitmap(previous_target);

  tile_map->identifier = map_name;
  tile_map->tile_size = tile_size;
  tile_map->map_renderer_res.x = renderer_w;
  tile_map->map_renderer_res.y = renderer_h;
  
  tile_map->data.tiles = (int*)malloc(sizeof(i32)*tiles_count_x*tiles_count_y);
  if (tile_map->data.tiles) {
    memset(tile_map->data.tiles, -1, sizeof(i32)*tiles_count_x*tiles_count_y);
    tile_map->data.tiles_size = tiles_count_x*tiles_count_y;
  }
  
  (*out) = tile_map;
  return ERR_OKAY;
}

p_fn err_code tr_map_provide_atlas(TileMap *tile_map, ALLEGRO_BITMAP *atlas, u32 texture_size, u32 texture_count) {
  u32 texture_row_count = al_get_bitmap_width(atlas) / texture_size;
  
  if (texture_count == 0) {
    texture_count = (al_get_bitmap_width(atlas) / texture_size) + (al_get_bitmap_height(atlas) / texture_size);
  }
  
  tile_map->atlas.atlas_image = atlas;
  tile_map->atlas.texture_size = texture_size;
  tile_map->atlas.total = texture_count;
  tile_map->atlas.row_count = texture_row_count;
  tile_map->atlas.column_count = texture_count - texture_row_count;
  
  return ERR_OKAY;
}

p_fn err_code tr_tile_map_cam_input(TileMap *map, ALLEGRO_KEYBOARD_STATE *state, double delta_time) {
  if (al_key_down(state, ALLEGRO_KEY_D)) {
    map->camera_position.x += 400.f * delta_time;
  }
  if (al_key_down(state, ALLEGRO_KEY_A)) {
    map->camera_position.x -= 400.f * delta_time;
  }
  if (al_key_down(state, ALLEGRO_KEY_W)) {
    map->camera_position.y -= 400.f * delta_time;
  }
  if (al_key_down(state, ALLEGRO_KEY_S)) {
    map->camera_position.y += 400.f * delta_time;
  }
  if (al_key_down(state, ALLEGRO_KEY_SPACE)) {
    map->camera_position.z += 1200.f * delta_time;
  }
  if (al_key_down(state, ALLEGRO_KEY_C)) {
    map->camera_position.z -= 1200.f * delta_time;
  }
  
  return ERR_OKAY;
}

p_private inline p_fn float round_to_m(float n, float mult) {
  return floorf((n + mult / 2.0f) / mult) * mult;
}

p_private inline p_fn float round_to_dp(float n, int num_decimal_places) {
  float mult = pow((float)10, (float)num_decimal_places);
  return floorf(n * mult + 0.5f) / mult;
}
p_private inline p_fn float map_range_f(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

p_private inline p_fn float lerp_f(float a, float b, float t) {
  return a + (b - a) * t;
}

p_fn err_code tr_tile_map_render(TileMap *map, float mouse_x, float mouse_y, ALLEGRO_KEYBOARD_STATE *state, bool debug) {
  u32 texture_size = map->atlas.texture_size;
  
  ALLEGRO_BITMAP *previous_target = al_get_target_bitmap();
  al_set_target_bitmap(map->map_renderer_bitmap);
  
  al_clear_to_color(al_map_rgb_f(0.f, 0.f, 0.f));
  al_hold_bitmap_drawing(true);
  
  if (debug) {
    for (uint x = 0; x < map->map_renderer_res.x; x += texture_size) {
      al_draw_line(
        x, 0,
        x, map->map_renderer_res.y,
        al_map_rgba_f(1.0f, 0, 1.0f, 0.45f),
        1.5f
      );
    }
    for (uint y = 0; y < map->map_renderer_res.y; y += texture_size) {
      al_draw_line(
        0, y,
        map->map_renderer_res.x, y,
        al_map_rgba_f(1.f, 0.f, 1.f, 0.45f),
        1.5f
      );
    }
  }

  float dx = -map->camera_position.x;
  float dy = -map->camera_position.y;
  float dsx = map->map_renderer_res.x + map->camera_position.z;
  float dsy = map->map_renderer_res.y + map->camera_position.z;

  float selection_x = round_to_m((mouse_x - texture_size/2.f - dx) / dsx * map->map_renderer_res.x, texture_size);
  float selection_y = round_to_m((mouse_y - texture_size/2.f - dy) / dsy * map->map_renderer_res.y, texture_size);
  if (selection_x > map->map_renderer_res.x - texture_size) {
    selection_x = map->map_renderer_res.x - texture_size;
  }
  if (selection_x < 0) {
    selection_x = 0;
  }
  if (selection_y > map->map_renderer_res.y - texture_size) {
    selection_y = map->map_renderer_res.y - texture_size;
  }
  if (selection_y < 0) {
    selection_y = 0;
  }

  if (debug) {
    al_draw_rectangle(selection_x, selection_y, selection_x+texture_size, selection_y+texture_size, al_map_rgba(0, 255, 0, 155), 2.5f);
    
    if (map->data.tiles != NULL) {
      u32 index = (selection_y/texture_size) * map->tiles_count_x + (selection_x/texture_size);
      if (al_key_down(state, ALLEGRO_KEY_F)) {
        map->data.tiles[index] = 2;
      } else if (al_key_down(state, ALLEGRO_KEY_BACKSPACE)) {
        map->data.tiles[index] = -1;
      }
    } else {
      fprintf(stderr, "[ERROR] %s:%d :: Map data is NULL!\n", __FILE__, __LINE__);
    }
  }
  
  if (map->data.tiles != NULL) {
    for (u32 i = 0; i < map->data.tiles_size; i++) {
      i32 texture_index = map->data.tiles[i];
      
      switch (texture_index) {
        case -1: break; // Air block
        default: {
          u32 x = (i % map->tiles_count_x) * map->tile_size;
          u32 y = (i / map->tiles_count_x) * map->tile_size;
          
          uint texture_x = texture_index % map->atlas.row_count,
               texture_y = texture_index / map->atlas.row_count;
          
          al_draw_bitmap_region(map->atlas.atlas_image,
                                texture_x * map->atlas.texture_size,
                                texture_y * map->atlas.texture_size,
                                map->atlas.texture_size, map->atlas.texture_size,
                                x, y, 0);
        } break;
      }
    }
  } else {
    fprintf(stderr, "[ERROR] %s:%d :: Map data is NULL!\n", __FILE__, __LINE__);
  }

  al_hold_bitmap_drawing(false);
  al_set_target_bitmap(previous_target);
  
  al_draw_scaled_bitmap(map->map_renderer_bitmap, 0, 0,
                        map->map_renderer_res.x,
                        map->map_renderer_res.y,
                        dx, dy,
                        dsx, dsy,
                        0);
                        
  if (debug) {
    al_draw_filled_circle(dx,         dy, 4, al_map_rgb(139, 0, 135));
    al_draw_filled_circle(dx+dsx,     dy, 4, al_map_rgb(139, 0, 135));
    al_draw_filled_circle(dx,     dy+dsy, 4, al_map_rgb(139, 0, 135));
    al_draw_filled_circle(dx+dsx, dy+dsy, 4, al_map_rgb(139, 0, 135));
    al_draw_rectangle(dx, dy, dx+dsx, dy+dsy, al_map_rgb(139, 0, 135), 1.f);
    
    ALLEGRO_FONT *mw_20;
    rm_get_font("mw_20", &mw_20);
    al_draw_textf(mw_20, al_map_rgba(255, 0, 255, 200), 12, 12, 0, "Selection: %d, %d", (int)selection_x/texture_size, (int)selection_y/texture_size);
  }
  
  return ERR_OKAY;
}
