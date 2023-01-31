#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>

#include "hashmap.h"

#include "p/p.h"

#define IMAGES_INITIAL_SIZE 256
struct hashmap_s loaded_images;

#define FONTS_INITIAL_SIZE 8
struct hashmap_s loaded_fonts;

fn err_code rm_init(void) {
  if (hashmap_create(IMAGES_INITIAL_SIZE, &loaded_images) != 0) {
    return ERR_NOKAY;
  }
  if (hashmap_create(FONTS_INITIAL_SIZE, &loaded_fonts) != 0) {
    return ERR_NOKAY;
  }
  return ERR_OKAY;
}

// ++ Images ++

fn err_code rm_insert_image(const char *image_name, ALLEGRO_BITMAP *precreated_bitmap) {
  ALLEGRO_BITMAP* image = (ALLEGRO_BITMAP*)hashmap_get(&loaded_images, image_name, strlen(image_name));
  if (image == NULL) {
    if (hashmap_put(&loaded_images, image_name, strlen(image_name), precreated_bitmap) != 0) {
      return ERR_RM;
    }
    return ERR_OKAY;
  } else {
    return ERR_RM_RESOURCE_EXISTS;
  }
}

fn err_code rm_create_image(const char *image_name, const char *file_path) {
  ALLEGRO_BITMAP *bitmap = al_load_bitmap(file_path);
  if (!bitmap) {
    return ERR_RM_LOADING_RESOURCES;
  }

  err_code result = rm_insert_image(image_name, bitmap);
  if (result != ERR_OKAY) {
    al_destroy_bitmap(bitmap);
  }
  
  return result;
}

fn err_code rm_delete_image(const char *image_name) {
  ALLEGRO_BITMAP* image = (ALLEGRO_BITMAP*)hashmap_get(&loaded_images, image_name, strlen(image_name));

  if (image == NULL) {
    return ERR_RM_NOT_FOUND;
  } else {
    if (hashmap_remove(&loaded_images, image_name, strlen(image_name) != 0)) {
      return ERR_RM;
    }
    al_destroy_bitmap(image);
    return ERR_OKAY;
  }
}

fn err_code rm_get_image(const char *image_name, ALLEGRO_BITMAP **bitmap_output) {
  ALLEGRO_BITMAP* image = (ALLEGRO_BITMAP*)hashmap_get(&loaded_images, image_name, strlen(image_name));
  
  if (image == NULL) {
    (*bitmap_output) = NULL;
    return ERR_RM_NOT_FOUND;
  } else {
    (*bitmap_output) = image;
    return ERR_OKAY;
  }
}

// -- Images --
// ++ Fonts ++

fn err_code rm_insert_font(const char *font_name, ALLEGRO_FONT *precreated_font) {
  ALLEGRO_FONT* font = (ALLEGRO_FONT*)hashmap_get(&loaded_fonts, font_name, strlen(font_name));
  if (font == NULL) {
    if (hashmap_put(&loaded_fonts, font_name, strlen(font_name), precreated_font) != 0) {
      return ERR_RM;
    }
    return ERR_OKAY;
  } else {
    return ERR_RM_RESOURCE_EXISTS;
  }
}

fn err_code rm_create_font(const char *font_name, const char *file_path, u32 font_size) {
  ALLEGRO_FONT *font = al_load_ttf_font(file_path, (int)font_size, 0);
  if (!font) {
    return ERR_RM_LOADING_RESOURCES;
  }

  err_code result = rm_insert_font(font_name, font);
  if (result != ERR_OKAY) {
    al_destroy_font(font);
  }
  
  return result;
}

fn err_code rm_delete_font(const char *font_name) {
  ALLEGRO_FONT* font = (ALLEGRO_FONT*)hashmap_get(&loaded_fonts, font_name, strlen(font_name));

  if (font == NULL) {
    return ERR_RM_NOT_FOUND;
  } else {
    if (hashmap_remove(&loaded_fonts, font_name, strlen(font_name)) != 0) {
      return ERR_RM;
    }
    al_destroy_font(font);
    return ERR_OKAY;
  }
}

fn err_code rm_get_font(const char *font_name, ALLEGRO_FONT **font_output) {
  ALLEGRO_FONT* font = (ALLEGRO_FONT*)hashmap_get(&loaded_fonts, font_name, strlen(font_name));
  
  if (font == NULL) {
    (*font_output) = NULL;
    return ERR_RM_NOT_FOUND;
  } else {
    (*font_output) = font;
    return ERR_OKAY;
  }
}

// -- Fonts --
