#pragma once

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>

#include "p/p.h"

p_fn err_code rm_init(void);
p_fn err_code rm_shutdown(void);

p_fn err_code free_all_images(void);
p_fn err_code free_all_fonts(void);
p_fn err_code free_all_audio_streams(void);

p_fn err_code rm_insert_image(const char *image_name,
                            ALLEGRO_BITMAP *precreated_bitmap);
p_fn err_code rm_create_image(const char *image_name, const char *file_path);
p_fn err_code rm_delete_image(const char *image_name);
p_fn err_code rm_get_image(const char *image_name, ALLEGRO_BITMAP **bitmap_output);


p_fn err_code rm_insert_font(const char *font_name, ALLEGRO_FONT *precreated_font);
p_fn err_code rm_create_font(const char *font_name,
                           const char *file_path, u32 font_size);
p_fn err_code rm_delete_font(const char *font_name);
p_fn err_code rm_get_font(const char *font_name, ALLEGRO_FONT **font_output);


p_fn err_code rm_insert_audio_stream(const char *stream_name,
                                   ALLEGRO_AUDIO_STREAM *precreated_stream);
p_fn err_code rm_create_audio_stream(const char *stream_name,
                                   const char *file_path);
p_fn err_code rm_delete_audio_stream(const char *stream_name);
p_fn err_code rm_get_audio_stream(const char *stream_name,
                                ALLEGRO_AUDIO_STREAM **audio_stream_output);

