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

#include "resource_manager.h"

#include "p/p.h"
#include "p/pv.h"
#include "p/parr.h"

private bool (*allegro_to_init[])(void) = {
  al_init_image_addon,
  al_init_font_addon,
  al_init_ttf_addon,
  al_init_acodec_addon,
  al_init_primitives_addon
};

private fn void init_allegro(void) {
  if (al_init() == false) {
    fprintf(stderr, "Error initalizing allegro-5 (Error in al_init).\n");
    exit(-1);
  }
  
  for (u32 i = 0; i < sizeof_array(allegro_to_init); i++) {
    if (allegro_to_init[i]() == false) {
      fprintf(stderr, "Error initalizing allegro-5 (Failed at index %d).\n", i);
      exit(-1);
    }
  }
}

int main(int argc, char **argv) {
  init_allegro();
  printf("[DEBUG] Allegro inited.\n");
  ASSERT_ERR(rm_init());
  printf("[DEBUG] Resource Manager inited.\n");
  
  ASSERT_ERR(rm_create_image("heart", "resources/heart.png"));
  ASSERT_ERR(rm_create_font("mw_16", "resources/Merriweather.ttf", 16));
  // ASSERT_ERR(rm_delete_font("mw_16"));
  
  al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
  al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
  
  al_set_new_display_flags(ALLEGRO_RESIZABLE | ALLEGRO_OPENGL_3_0);
  ALLEGRO_DISPLAY *display = al_create_display(1280, 720);
  
  al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
  
  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  assert(event_queue != NULL);
  
  if (al_install_keyboard() == false) {
    fprintf(stderr, "Failed to install allegro keyboard. Maybe keyboard is not connected?");
  }
  if (al_install_audio() == false) {
    fprintf(stderr, "Failed to install allegro audio.");
  }
  
  // al_reserve_samples(4);
  ALLEGRO_VOICE *voice = al_create_voice(48000, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
  ALLEGRO_MIXER *mixer = al_create_mixer(48000, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
  assert(mixer && voice);
  
  ASSERT_ERR(rm_create_audio_stream("fishes", "./resources/weird-fishes.flac"));
  // ALLEGRO_AUDIO_STREAM *song = al_load_audio_stream("./resources/weird-fishes.flac", 8, 512);
  // assert(song != NULL);
  
  ALLEGRO_AUDIO_STREAM *fishes;
  ASSERT_ERR(rm_get_audio_stream("fishes", &fishes));
  al_attach_audio_stream_to_mixer(fishes, mixer);

  al_attach_mixer_to_voice(mixer, voice);

  al_set_audio_stream_playing(fishes, true);
  
  al_register_event_source(event_queue, al_get_keyboard_event_source());
  al_register_event_source(event_queue, al_get_display_event_source(display));
  
  ALLEGRO_TIMER *frame_timer = al_create_timer(1.0 / 60.0);
  al_register_event_source(event_queue, al_get_timer_event_source(frame_timer));
  al_start_timer(frame_timer);
  
  bool running = true;
  while (running) {
    ALLEGRO_EVENT event;
    al_wait_for_event(event_queue, &event);

    switch (event.type) {
      case (ALLEGRO_EVENT_TIMER): {
        al_clear_to_color(al_map_rgba(24, 0, 16, 0));
        ALLEGRO_BITMAP *heart;
        ASSERT_ERR(rm_get_image("heart", &heart));

        ALLEGRO_FONT *mw_16;
        ASSERT_ERR(rm_get_font("mw_16", &mw_16));
        al_draw_text(mw_16, al_map_rgb(255, 255, 255), 50, 300, 0, "Hello, World!");

        al_draw_bitmap(heart, 24, 24, 0);
        al_draw_line(250, 250,  1125, 525, al_map_rgba(255, 15, 16, 255), 2.5f);
        
        al_draw_filled_circle(600, 600, 200, al_map_rgb(255, 0, 255));
        
        al_flip_display();
      } break;
      case (ALLEGRO_EVENT_DISPLAY_CLOSE): {
        running = false;
      } break;
      case (ALLEGRO_EVENT_DISPLAY_RESIZE): {
        al_acknowledge_resize(display);
      } break;
    }
  }
  
  al_destroy_display(display);
  al_destroy_event_queue(event_queue);
  al_detach_audio_stream(fishes);
  al_destroy_audio_stream(fishes);
  al_destroy_voice(voice);
  al_destroy_mixer(mixer);
  
  return 0;
}
