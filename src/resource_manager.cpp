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


