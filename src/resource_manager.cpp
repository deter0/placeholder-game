#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>

#include "p/p.h"

#include <unordered_map>

private std::unordered_map<const char*, ALLEGRO_BITMAP*> loaded_images;
private std::unordered_map<const char*, ALLEGRO_FONT*>   loaded_fonts;

fn err_code rm_insert_image(const char *image_name, ALLEGRO_BITMAP *precreated_bitmap) {
  if (loaded_images.find(image_name) != loaded_images.end()) {
    loaded_images[image_name] = precreated_bitmap;
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
  if (loaded_images.find(image_name) != loaded_images.end()) {
    return ERR_RM_NOT_FOUND;
  } else {
    ALLEGRO_BITMAP *image = loaded_images.at(image_name);
    al_destroy_bitmap(image);
    loaded_images.erase(image_name);
    
    return ERR_OKAY;
  }
}

fn err_code rm_get_image(const char *image_name, ALLEGRO_BITMAP **bitmap_output) {
  
}


