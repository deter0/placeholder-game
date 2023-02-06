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
    map->camera_position -= glm::vec3(450.f * delta_time, 450.f * delta_time, 0.f);
  }
  if (al_key_down(state, ALLEGRO_KEY_C)) {
    map->camera_position.z -= 900.f * delta_time;
    map->camera_position -= glm::vec3(450.f * delta_time, 450.f * delta_time, 0.f);
  }
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

p_fn err_code tr_tile_map_render(TileMap *map, ALLEGRO_BITMAP *output_bitmap, ALLEGRO_DISPLAY *display, bool debug, float mouse_x, float mouse_y) {
  ALLEGRO_BITMAP *texture_atlas = NULL;
  err_code status = rm_get_image(map->identifier, &texture_atlas);
  if (status != ERR_OKAY) {
    return status;
  } else if (texture_atlas == NULL) {
    return ERR_TR;
  }
  
  u32 map_width = 5;
  u32 texture_size = map->texture_size;
  
  al_set_target_bitmap(map_renderer_bitmap);
  al_clear_to_color(al_map_rgba(0.f, 0.f, 0.f, 0.f));
  al_hold_bitmap_drawing(true);
  
  if (debug) {
    for (uint x = 0; x < map_renderer_res_x; x += texture_size) {
      al_draw_line(
        x, 0,
        x, map_renderer_res_y,
        al_map_rgba_f(1.0f, 0, 1.0f, 0.45f),
        1.5f
      );
    }
    for (uint y = 0; y < map_renderer_res_y; y += texture_size) {
      al_draw_line(
        0, y,
        map_renderer_res_x, y,
        al_map_rgba_f(1.f, 0.f, 1.f, 0.45f),
        1.5f
      );
    }
  }
  
  bool stop = false;
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

  float dx = -map->camera_position.x;
  float dy = -map->camera_position.y;
  float dsx = map_renderer_res_x + map->camera_position.z;
  float dsy = map_renderer_res_y + map->camera_position.z;
  
  float selection_x = round_to_m((((mouse_x - texture_size/2.f) - dx) / dsx) * map_renderer_res_x, texture_size);
  float selection_y = round_to_m((((mouse_y - texture_size/2.f) - dy) / dsy) * map_renderer_res_y, texture_size);

  al_draw_rectangle(selection_x, selection_y, selection_x+texture_size, selection_y+texture_size, al_map_rgba(0, 255, 0, 155), 2.5f);

  al_hold_bitmap_drawing(false);
  al_set_target_bitmap(output_bitmap);
  
  al_draw_scaled_bitmap(map_renderer_bitmap, 0, 0,
                        map_renderer_res_x,
                        map_renderer_res_y,
                        dx, dy,
                        dsx, dsy,
                        0);
                        
  al_draw_filled_circle(dx,         dy, 4, al_map_rgb(139, 0, 135));
  al_draw_filled_circle(dx+dsx,     dy, 4, al_map_rgb(139, 0, 135));
  al_draw_filled_circle(dx,     dy+dsy, 4, al_map_rgb(139, 0, 135));
  al_draw_filled_circle(dx+dsx, dy+dsy, 4, al_map_rgb(139, 0, 135));
  al_draw_rectangle(dx, dy, dx+dsx, dy+dsy, al_map_rgb(139, 0, 135), 1.f);
}
