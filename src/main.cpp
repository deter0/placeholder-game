#include <stdio.h>
#include <stdlib.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_native_dialog.h>

#include <pthread.h>

#include "resource_manager.h"
#include "audio_manager.h"
#include "tile_renderer.h"
#include "ui.h"

#include "player.h"

#include "p/p.h"
#include "p/pv.h"
#include "p/parr.h"
#include "p/ppath.h"

p_private bool (*allegro_to_init[])(void) = {
  al_init_image_addon,
  al_init_font_addon,
  al_init_ttf_addon,
  al_init_acodec_addon,
  al_init_native_dialog_addon,
  al_init_primitives_addon
};

p_private p_fn void init_allegro(void) {
  if (al_init() == false) {
    fprintf(stderr, "Error initalizing allegro-5 (Error in al_init).\n");
    exit(-1);
  }
  
  for (u32 i = 0; i < p_sizeof_array(allegro_to_init); i++) {
    if (allegro_to_init[i]() == false) {
      fprintf(stderr, "Error initalizing allegro-5 (Failed at index %d).\n", i);
      exit(-1);
    }
  }
}

int main(int argc, char **argv) {
  init_allegro();
  printf("[DEBUG] Allegro inited.\n");
  p_ASSERT_ERR(rm_init());
  printf("[DEBUG] Resource Manager inited.\n");
  // p_ASSERT_ERR(ui_init());
  printf("[DEBUG] UI inited.\n");
  p_ASSERT_ERR(tr_init());
  printf("[DEBUG] Tile Renderer inited.\n");
  
  al_set_app_name("Gayme");
  if (al_install_audio() == false) {
    fprintf(stderr, "Failed to install allegro audio.");
  }
  
  p_ASSERT_ERR(audio_manager_init());
  printf("[DEBUG] Audio Manager inited.\n");
  
  p_ASSERT_ERR(rm_create_image("heart", RESOURCE_PATH(heart.png)));
  // ASSERT_ERR(rm_delete_font("mw_16"));
  
  al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
  al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
  
  al_set_new_display_flags(ALLEGRO_RESIZABLE | ALLEGRO_OPENGL_3_0);
  ALLEGRO_DISPLAY *display = al_create_display(1280, 720);
  
  // al_show_native_message_box(display, "ERROR", "hi", "Experienced an error!", 0, 0);
  
  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
  
  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  assert(event_queue != NULL);
  
  Player player = player_new();
  
  if (al_install_keyboard() == false) {
    fprintf(stderr, "Failed to install allegro keyboard. Maybe keyboard is not connected?");
  }
  if (al_install_mouse() == false) {
    fprintf(stderr, "Failed to install allegro mouse. Maybe mouse is not connected?");
  }
  
  MusicParams params;
  audio_manager_get_music_params(MusicFiles[BODYSNATCHERS], &params);
  params.pan = 0.f;
  params.speed = 0.95f;
  params.gain = 0.5f;
  params.play_mode = ALLEGRO_PLAYMODE_LOOP;
  audio_manager_set_music_params(MusicFiles[BODYSNATCHERS], params);
  audio_manager_set_play_state(MusicFiles[BODYSNATCHERS], PlayStateStopped);
  
  al_register_event_source(event_queue, al_get_keyboard_event_source());
  al_register_event_source(event_queue, al_get_display_event_source(display));
  
  ALLEGRO_TIMER *frame_timer = al_create_timer(1.0 / 60.0);
  al_register_event_source(event_queue, al_get_timer_event_source(frame_timer));
  al_start_timer(frame_timer);

  TileMap *tile_map_a;
  // p_ASSERT_ERR(tr_new_tile_map((int*)TemplateMapData, "map_a", "./resources/texture_atlas_sample.png", 16, 20, &tile_map_a));
  p_ASSERT_ERR(tr_create_tile_map("map_a", 200, 200, 16, &tile_map_a));
  tr_save_tile_map_data_to_file(tile_map_a, "./map-data.bin");
  ALLEGRO_BITMAP *texture_atlas = al_load_bitmap(RESOURCE_PATH(basictiles.png));
  assert(texture_atlas != NULL);
  
  p_ASSERT_ERR(tr_map_provide_atlas(tile_map_a, texture_atlas, 12, 0));
  
  ALLEGRO_BITMAP *screen_buffer = al_create_bitmap(200, 200);
  bool running = true;
  double last_frame = al_get_time();

  UIObject *root = ui_new_object();
  UIObject *c1 = ui_new_object();
  root->size = glm::vec2(150.f);
  root->layout.direction = UI_LAYOUT_DHORZVERT;
  root->layout.margin = 8.f;
  root->layout.padding = 8.f;
  root->pri_color = al_map_rgba(255, 255, 255, 100);

  c1->size_units[0] = UI_UAUTO;
  c1->size_units[1] = UI_UAUTO;
  c1->position_units[0] = UI_UAUTO;
  c1->position_units[1] = UI_UAUTO;
  c1->border.size = 1.f;
  c1->border.color = al_map_rgb(255, 0, 255);
  c1->text = "hi!";

  UIObject *c2 = ui_new_object();
  c2->position_units[0] = UI_UAUTO;
  c2->position_units[1] = UI_UAUTO;
  c2->size_units[0] = UI_UAUTO;
  c2->size_units[1] = UI_UAUTO;
  c2->border.size = 1.f;
  c2->border.color = al_map_rgb(255, 0, 0);
  c2->text = "Hello";
  
  ui_set_object_parent(c2, root);
  ui_set_object_parent(c1, root);
  for (u32 i = 0; i < 15; i++) {
    UIObject *c = ui_new_object();
    c->position_units[0] = UI_UAUTO;
    c->position_units[1] = UI_UAUTO;
    c->size.x = 25.f;
    c->size.y = 25.f + (float)i;
    c->border.size = 1.f;
    c->border.color = al_map_rgb(255, 0, 0);
    ui_set_object_parent(c, root);
  }


  while (running) {
    ALLEGRO_EVENT event;
    al_wait_for_event(event_queue, &event);

    switch (event.type) {
      case (ALLEGRO_EVENT_TIMER): {
        double frame_now = al_get_time();
        double delta_time = frame_now - last_frame;
        // printf("%f\n", 1.f/delta_time);
        
        ALLEGRO_KEYBOARD_STATE state;
        al_get_keyboard_state(&state);
        player_handle_input(&player, delta_time, &state);

        al_clear_to_color(al_map_rgba(24, 0, 16, 0));
        
        float gmouse_x = 0, gmouse_y = 0;
        float win_x = 0, win_y = 0;
        {
          int wxi = 0, wyi = 0;
          al_get_window_position(display, &wxi, &wyi);
          int mxi = 0, myi = 0;
          al_get_mouse_cursor_position(&mxi, &myi);
          
          gmouse_x = mxi;
          gmouse_y = myi;
          win_x = wxi;
          win_y = wyi;
        }
        
        float mouse_x = gmouse_x - win_x;
        float mouse_y = gmouse_y - win_y;
        
        // tr_tile_map_cam_input(tile_map_a, &state, delta_time);
        // tr_tile_map_render(tile_map_a, mouse_x, mouse_y, &state, true);
        
        root->children[5]->size = glm::vec2(30.f + cos(al_get_time()*3.f) * 30.f, 20.f);
        root->size = glm::vec2(150.f + sin(al_get_time()) * 100);
        for (u32 i = 0; i < 1000; i++) {
          ui_render(root);
        }
        
        al_flip_display();

        last_frame = frame_now;
      } break;
      case (ALLEGRO_EVENT_DISPLAY_CLOSE): {
        running = false;
      } break;
      case (ALLEGRO_EVENT_KEY_UP): {
        if (event.keyboard.keycode == ALLEGRO_KEY_P) {
          audio_manager_set_play_state(MusicFiles[WEIRD_FISHES], PlayStateToggle);
        } else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
          running = false;
        }
      } break;
      case (ALLEGRO_EVENT_DISPLAY_RESIZE): {
        al_acknowledge_resize(display);
      } break;
    }
  }
  
  printf("Exiting.\n");
  
  al_destroy_display(display);
  al_destroy_event_queue(event_queue);
  
  audio_manager_shutdown();
  rm_shutdown();
  
  al_uninstall_system();
  printf("[GOODBYE] Okay!\n");

  do {
    exit(0);
  } while(1);
}
