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
#include "p/ppath.h"

// TODO(kay): Create `map_renderer_bitmap` with the TileMap struct and calculate the width and the height or get it as a parameter e.g. CreateMap(TilesX, TilesY, TilesSize, ...)
// TODO(kay): Restructure map data to be a continous buffer that is allocated when created, rewrite renderer
// TODO(kay): Reading and writing map data from a file

p_private u32 currently_drawing = 0;

p_fn err_code tr_init() {
  p_ASSERT_ERR(rm_create_font("mw_20", RESOURCE_PATH(Merriweather.ttf), 20));
  
  return ERR_OKAY;
}

p_fn err_code tr_save_tile_map_data_to_file(TileMap *tile_map, const char *file_name) {
  // FILE *tile_map_data_file = fopen(file_name, "wb");
  
  // // u32 file_size = tile_map->data.tiles_size;
  // // fprintf(tile_map_data_file, "%d", file_size);

  // fclose(tile_map_data_file);
  
  return ERR_OKAY;
}

p_fn err_code tr_create_map_layer(u32 index, u32 tiles_count_x, u32 tiles_count_y, TileMapDataLayer **out_layer) {
  TileMapDataLayer *layer = (TileMapDataLayer*)malloc(sizeof(TileMapDataLayer));
  if (!layer) {
    return ERR_TR_ALLOC_MAP;
  }
  memset(layer, 0, sizeof(*layer));
  
  i32 *tiles_data = (int*)malloc(sizeof(i32)*tiles_count_x*tiles_count_y);
  if (!tiles_data) {
    free(layer);
    return ERR_TR_ALLOC_MAP;
  }
  memset(tiles_data, -1, sizeof(i32)*tiles_count_x*tiles_count_y);
  layer->layer_index = index; 
  layer->layer_tiles_data = tiles_data;
  layer->layer_tiles_count = tiles_count_x*tiles_count_y;

  (*out_layer) = layer;
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
  
  TileMapDataLayer *default_layer = 0;
  tr_create_map_layer(0, tiles_count_x, tiles_count_y, &default_layer);
  
  tile_map->data.layers[tile_map->data.layers_count++] = default_layer;
  for (u32 i = 1; i < 3; i++) {
    TileMapDataLayer *layer = 0;
    tr_create_map_layer(i, tiles_count_x, tiles_count_y, &layer);
    tile_map->data.layers[tile_map->data.layers_count++] = layer;
  }
  
  (*out) = tile_map;
  return ERR_OKAY;
}

p_fn err_code tr_map_provide_atlas(TileMap *tile_map, ALLEGRO_BITMAP *atlas, u32 texture_size, u32 texture_count) {
  u32 texture_row_count = al_get_bitmap_width(atlas) / texture_size;
  
  if (texture_count == 0) {
    texture_count = (al_get_bitmap_width(atlas) / texture_size) * (al_get_bitmap_height(atlas) / texture_size);
  }
  
  tile_map->atlas.atlas_image = atlas;
  tile_map->atlas.texture_size = texture_size;
  tile_map->atlas.total = texture_count;
  tile_map->atlas.row_count = texture_row_count;
  tile_map->atlas.column_count = texture_count - texture_row_count;
  
  return ERR_OKAY;
}

p_private p_fn float round_to_m(float n, float mult) {
  return floorf((n + mult / 2.0f) / mult) * mult;
}

p_private p_fn float round_to_dp(float n, int num_decimal_places) {
  float mult = pow((float)10, (float)num_decimal_places);
  return floorf(n * mult + 0.5f) / mult;
}
p_private p_fn float map_range_f(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

p_private p_fn float lerp_f(float a, float b, float t) {
  return a + (b - a) * t;
}

p_private float GLOBAL_START_SELECTION = 0;
p_private float GLOBAL_START_SELECTION_TARGET = 0;
p_private i32 last_mouse_z = 0;

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
  ALLEGRO_MOUSE_STATE mouse_state;
  al_get_mouse_state(&mouse_state);
  i32 mouse_scroll_delta = last_mouse_z - mouse_state.z;
  last_mouse_z = mouse_state.z;
  
  GLOBAL_START_SELECTION_TARGET -= mouse_scroll_delta*45.f;
  GLOBAL_START_SELECTION = lerp_f(GLOBAL_START_SELECTION, GLOBAL_START_SELECTION_TARGET, 0.1f);
  
  return ERR_OKAY;
}

p_private p_fn void tr_draw_grid(TileMap *map) {
  for (uint x = 0; x < map->map_renderer_res.x; x += map->atlas.texture_size) {
    al_draw_line(
      x, 0,
      x, map->map_renderer_res.y,
      al_map_rgba_f(1.0f, 0, 1.0f, 0.45f),
      1.5f
    );
  }
  for (uint y = 0; y < map->map_renderer_res.y; y += map->atlas.texture_size) {
    al_draw_line(
      0, y,
      map->map_renderer_res.x, y,
      al_map_rgba_f(1.f, 0.f, 1.f, 0.45f),
      1.5f
    );
  }
}

#define SEL_PADDING (10.f)
p_private p_fn void tr_draw_selection_ui(TileMap *map, float screen_width, float screen_height, float mouse_x, float mouse_y) {
  float selection_window_size_x = 280, selection_window_size_y = screen_height - SEL_PADDING*2.f;
  
  ALLEGRO_BITMAP *screen_bitmap = al_get_target_bitmap();
  ALLEGRO_BITMAP *selection_menu_framebuffer;
  if (rm_get_image("selection_menu_render", &selection_menu_framebuffer) == ERR_RM_NOT_FOUND || selection_menu_framebuffer == NULL) {
    selection_menu_framebuffer = al_create_bitmap(selection_window_size_x, selection_window_size_y);
    rm_insert_image("selection_menu_render", selection_menu_framebuffer);
  }
  assert(selection_menu_framebuffer != NULL);
  
  ALLEGRO_BITMAP *items_framebuffer;
  if (rm_get_image("selection_menu_items_render", &items_framebuffer) == ERR_RM_NOT_FOUND || items_framebuffer == NULL) {
    items_framebuffer = al_create_bitmap(selection_window_size_x - SEL_PADDING*2, selection_window_size_y-SEL_PADDING*3-22);
    rm_insert_image("selection_menu_items_render", items_framebuffer);
  }
  assert(items_framebuffer != NULL);
  
  al_set_target_bitmap(selection_menu_framebuffer);
  glm::vec2 selection_menu_size = glm::vec2(al_get_bitmap_width(selection_menu_framebuffer),
                                            al_get_bitmap_height(selection_menu_framebuffer));
  glm::vec2 items_framebuffer_size = glm::vec2(al_get_bitmap_width(items_framebuffer),
                                                al_get_bitmap_height(items_framebuffer));
  
  al_draw_filled_rectangle(0, 0, selection_menu_size.x, selection_menu_size.y, al_map_rgb(60, 60, 60));
  al_draw_rectangle(0, 0, selection_menu_size.x, selection_menu_size.y, al_map_rgb(125, 125, 125), 2.f);

  ALLEGRO_FONT *mw_20;
  p_ASSERT_ERR(rm_get_font("mw_20", &mw_20));
  
  al_draw_text(mw_20, al_map_rgb(255, 255, 255),
              SEL_PADDING, SEL_PADDING,
              0, "Select Image");
  
  al_set_target_bitmap(items_framebuffer);
  al_clear_to_color(al_map_rgba(0, 0, 0, 0));
  float texture_scale = 3.5f;
  float texture_display_size = map->atlas.texture_size * texture_scale;
  float texture_y = 0, texture_x = 0;

  al_hold_bitmap_drawing(true);
  uint total = 0;
  for (uint n = 0; n < map->atlas.total; n++) {
    u32 tex_coord_x = (n % map->atlas.row_count) * map->atlas.texture_size;
    u32 tex_coord_y = (n / map->atlas.row_count) * map->atlas.texture_size;
    
    float texture_render_pos_x = texture_x;
    float texture_render_pos_y = texture_y + GLOBAL_START_SELECTION;
    
    if (texture_render_pos_y + texture_display_size < 0) {
      texture_x += texture_display_size + SEL_PADDING*2;
      if ((texture_x + texture_display_size) > items_framebuffer_size.x) {
        texture_x = 0;
        texture_y += texture_display_size + SEL_PADDING*2;
      }
      continue;
    }
    
    al_draw_filled_rectangle(texture_render_pos_x,
                              texture_render_pos_y,
                              texture_render_pos_x + texture_display_size,
                              texture_render_pos_y + texture_display_size,
                              al_map_rgba(90, 90, 90, 165));
    float border_offset = 2.f;
    
    ALLEGRO_COLOR outline_color = al_map_rgb(155, 155, 155);

    float relative_mouse_x = (mouse_x - SEL_PADDING*2);
    float relative_mouse_y = (mouse_y - (screen_height - selection_menu_size.y) - SEL_PADDING*2-22);
    al_draw_circle(relative_mouse_x, relative_mouse_y, 4, outline_color, 5.f);
    if (relative_mouse_x >= texture_render_pos_x && relative_mouse_x <= texture_render_pos_x+texture_display_size
          && relative_mouse_y >= texture_render_pos_y && relative_mouse_y <= texture_render_pos_y+texture_display_size) {
      outline_color = al_map_rgb(255, 0, 0);
      
      ALLEGRO_MOUSE_STATE mouse_state;
      al_get_mouse_state(&mouse_state);
      
      if (mouse_state.buttons & 1) {
        currently_drawing = n;
      }
    } else if (currently_drawing == n) {
      outline_color = al_map_rgb(155, 0, 0);
    }
    
    al_draw_rectangle(texture_render_pos_x - border_offset,
                      texture_render_pos_y - border_offset,
                      texture_render_pos_x + texture_display_size + border_offset,
                      texture_render_pos_y + texture_display_size + border_offset,
                      outline_color, 2.0f);

    al_draw_tinted_scaled_rotated_bitmap_region(map->atlas.atlas_image,
                                                tex_coord_x, tex_coord_y, // Source x,y
                                                map->atlas.texture_size, map->atlas.texture_size, // Source w,h
                                                al_map_rgb_f(1.f, 1.f, 1.f), // Tint
                                                0.f, 0.f, // Center x,y
                                                texture_render_pos_x, texture_render_pos_y, // Dest x,y
                                                texture_scale, texture_scale, // Dest w,h
                                                0.f, 0);
    texture_x += texture_display_size + SEL_PADDING*2;
    if ((texture_x + texture_display_size) > items_framebuffer_size.x) {
      texture_x = 0;
      texture_y += texture_display_size + SEL_PADDING*2;
    }
    
    total++;
    if (texture_y > items_framebuffer_size.y - GLOBAL_START_SELECTION) {
      break;
    }
  }

  al_hold_bitmap_drawing(false);
  al_set_target_bitmap(selection_menu_framebuffer);
  
  al_draw_bitmap_region(items_framebuffer, 0, 0, items_framebuffer_size.x, items_framebuffer_size.y,
                        SEL_PADDING, SEL_PADDING*3+22, 0);

  al_set_target_bitmap(screen_bitmap);
  float selection_top_left_x = SEL_PADDING,
        selection_top_left_y = screen_height - selection_menu_size.y - SEL_PADDING;
  al_draw_bitmap(selection_menu_framebuffer, selection_top_left_x, selection_top_left_y, 0);
}

p_fn err_code tr_tile_map_render(TileMap *map, float mouse_x, float mouse_y, ALLEGRO_KEYBOARD_STATE *state, bool debug) {
  u32 texture_size = map->atlas.texture_size;
  
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
  
  ALLEGRO_BITMAP *previous_target = al_get_target_bitmap();
  al_set_target_bitmap(map->map_renderer_bitmap);
  
  al_clear_to_color(al_map_rgb_f(0.f, 0.f, 0.f));
  al_hold_bitmap_drawing(true);
  
  if (debug) {
    tr_draw_grid(map);
  }

  if (debug) {
    if (al_key_down(state, ALLEGRO_KEY_0)) {
      map->data.debug_layer_index = 0;
    } else if (al_key_down(state, ALLEGRO_KEY_1)) {
      map->data.debug_layer_index = 1;
    } else if (al_key_down(state, ALLEGRO_KEY_2)) {
      map->data.debug_layer_index = 2;
    }
    
    assert(map->data.debug_layer_index < map->data.layers_count && map->data.debug_layer_index >= 0);
    TileMapDataLayer *editing_layer = map->data.layers[map->data.debug_layer_index];
    
    ALLEGRO_FONT *mw_20;
    rm_get_font("mw_20", &mw_20);
    al_draw_textf(mw_20, al_map_rgba(255, 255, 255, 155), 24, 24, 0, "Selected Layer: %d", editing_layer->layer_index);

    if (editing_layer != 0 && editing_layer->layer_tiles_data != 0) {
      u32 index = (selection_y/texture_size) * map->tiles_count_x + (selection_x/texture_size);
      if (al_key_down(state, ALLEGRO_KEY_F)) {
        editing_layer->layer_tiles_data[index] = currently_drawing;
      } else if (al_key_down(state, ALLEGRO_KEY_BACKSPACE)) {
        editing_layer->layer_tiles_data[index] = -1;
      }
    } else {
      fprintf(stderr, "[ERROR] %s:%d :: Map data is NULL!\n", __FILE__, __LINE__);
    }
  }
  
  for (u32 layer_index = 0; layer_index < map->data.layers_count; layer_index++) {
    printf("Rendering layer: %d\n", layer_index);
    TileMapDataLayer *current_layer = map->data.layers[layer_index];
    assert(current_layer->layer_tiles_data != NULL);

    for (u32 i = 0; i < current_layer->layer_tiles_count; i++) {
      i32 texture_index = current_layer->layer_tiles_data[i];
      
      switch (texture_index) {
        case -1: break; // Air block
        default: {
          u32 x = (i % map->tiles_count_x) * map->atlas.texture_size;
          u32 y = (i / map->tiles_count_x) * map->atlas.texture_size;
          
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
  }

  al_hold_bitmap_drawing(false);
  
  if (debug) {
    al_draw_rectangle(selection_x, selection_y,
                      selection_x+texture_size,
                      selection_y+texture_size,
                      al_map_rgba(0, 255, 0, 155), 2.5f);
  }
  
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
    
    al_set_target_bitmap(previous_target); 
    tr_draw_selection_ui(map, (float)al_get_bitmap_width(previous_target),
                        (float)al_get_bitmap_height(previous_target),
                        mouse_x, mouse_y);
  }
  
  return ERR_OKAY;
}
