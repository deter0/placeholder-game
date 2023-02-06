#include <stdio.h>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <allegro5/allegro5.h>
#include <allegro5/bitmap.h>
#include <allegro5/allegro_primitives.h>

#include "p/p.h"
#include "audio_manager.h"
#include "resource_manager.h"

#include "tile_renderer.h"

p_private const u32 map_renderer_res_x = 640, map_renderer_res_y = 640;
p_private ALLEGRO_BITMAP *map_renderer_bitmap;

p_fn err_code tr_init() {
  map_renderer_bitmap = al_create_bitmap(map_renderer_res_x, map_renderer_res_y);
  if (map_renderer_bitmap == NULL)
    return ERR_TR;
  
  return ERR_OKAY;
}

p_fn err_code tr_new_tile_map(int *map_data, const char *map_identifier,
                           const char *texture_atlas_path, u32 texture_size,
                           u32 texture_atlas_row_count, TileMap **output_map)
{
  TileMap *tile_map = (TileMap*)malloc(sizeof(TileMap));
  if (tile_map == NULL) {
    (*output_map) = NULL;
    return ERR_TR_ALLOC_MAP;
  }
  memset(tile_map, 0, sizeof(TileMap));
    
  err_code status = rm_create_image(map_identifier, texture_atlas_path);
  if (status != ERR_OKAY) {
    (*output_map) = NULL;
    free(tile_map);
    return ERR_TR_ALLOC_MAP;
  }
  
  tile_map->identifier = map_identifier;
  tile_map->camera_position = glm::vec3(0.f, 0.f, 2.f);
  tile_map->map_data = map_data;
  tile_map->texture_size = texture_size;
  tile_map->texture_atlas_row_count = texture_atlas_row_count;
  
  (*output_map) = tile_map;
  
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
    map->camera_position.z += 900.f * delta_time;
  }
  if (al_key_down(state, ALLEGRO_KEY_C)) {
    map->camera_position.z -= 900.f * delta_time;
  }
}

p_fn err_code tr_tile_map_render(TileMap *map, ALLEGRO_BITMAP *output_bitmap, bool debug) {
  ALLEGRO_BITMAP *texture_atlas = NULL;
  err_code status = rm_get_image(map->identifier, &texture_atlas);
  if (status != ERR_OKAY) {
    return status;
  } else if (texture_atlas == NULL) {
    return ERR_TR;
  }
  
  al_set_target_bitmap(map_renderer_bitmap);
  al_hold_bitmap_drawing(true);
  
  u32 map_width = 5;
  bool stop = false;
  
  u32 texture_size = map->texture_size;
  if (debug) {
    for (uint x = 0; x < map_renderer_res_x; x += texture_size) {
      al_draw_line(
        x, 0,
        x, map_renderer_res_y,
        al_map_rgb(255, 0, 255),
        1.5f
      );
    }
    for (uint y = 0; y < map_renderer_res_y; y += texture_size) {
      al_draw_line(
        0, y,
        map_renderer_res_x, y,
        al_map_rgb(255, 0, 255),
        1.5f
      );
    }
  }
  
  for (u32 n = 0; stop == false; n++) {
    int index = n;
    int tile_data = map->map_data[n];
    
    if (n > 50) {
      break;
    }
    u32 x = n / map_width;
    u32 y = n % map_width;

    switch (tile_data) {
      case (-9999): {
        stop = true;
      } break;
      
      case (-1): break;

      default: {
        uint texture_index = tile_data;
        uint texture_x = texture_index % map->texture_atlas_row_count,
             texture_y = texture_index / map->texture_atlas_row_count;

        al_draw_bitmap_region(texture_atlas,
                              texture_x * texture_size,
                              texture_y * texture_size,
                              texture_size, texture_size,
                              x * texture_size, y * texture_size,
                              0);
        x++;
      } break;
    }
  }

  al_hold_bitmap_drawing(false);
  al_set_target_bitmap(output_bitmap);
  
  al_draw_scaled_bitmap(map_renderer_bitmap, 0, 0,
                        map_renderer_res_x,
                        map_renderer_res_y,
                        -map->camera_position.x,
                        -map->camera_position.y,
                        map_renderer_res_x * 3 + map->camera_position.z,
                        map_renderer_res_y * 3 + map->camera_position.z,
                        0);
}
