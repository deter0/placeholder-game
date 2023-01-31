#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>

#include "p/p.h"

fn err_code rm_init(void);

fn err_code rm_create_image(const char *image_name, const char *file_path);
fn err_code rm_insert_image(const char *image_name, ALLEGRO_BITMAP *precreated_bitmap);
fn err_code rm_delete_image(const char *image_name);
fn err_code rm_get_image(const char *image_name, ALLEGRO_BITMAP **bitmap_output);

fn err_code rm_create_font(const char *font_name, const char *file_path);
fn err_code rm_insert_font(const char *font_name, ALLEGRO_FONT *precreated_font);
fn err_code rm_delete_font(const char *font_name);
fn err_code rm_get_font(const char *font_name, ALLEGRO_FONT **font_output);

