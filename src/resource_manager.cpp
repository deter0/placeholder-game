#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>

#include "resource_manager.h"
#include "hashmap.h"

#include "p/p.h"

#define IMAGES_INITIAL_SIZE 256
struct hashmap_s loaded_images;

#define FONTS_INITIAL_SIZE 16
struct hashmap_s loaded_fonts;

#define AUDIO_STREAM_INITIAL_SIZE 16
struct hashmap_s loaded_audio_streams;

p_fn err_code rm_init(void) {
  if (hashmap_create(IMAGES_INITIAL_SIZE, &loaded_images) != 0) {
    return ERR_NOKAY;
  }
  if (hashmap_create(FONTS_INITIAL_SIZE, &loaded_fonts) != 0) {
    return ERR_NOKAY;
  }
  if (hashmap_create(AUDIO_STREAM_INITIAL_SIZE, &loaded_audio_streams) != 0) {
    return ERR_NOKAY;
  }
  return ERR_OKAY;
}

p_fn err_code rm_shutdown(void) {
  p_ASSERT_ERR(free_all_images());
  p_ASSERT_ERR(free_all_fonts());
  p_ASSERT_ERR(free_all_audio_streams());
  
  return ERR_OKAY;
}

// ++ Images ++

p_private p_fn  int free_all_images_itr(void* const context, struct hashmap_element_s *element) {
  printf("Freeing Image: %s\n", element->key);
  
  ALLEGRO_BITMAP *image = (ALLEGRO_BITMAP*)element->data;
  al_destroy_bitmap(image);

  return -1;
}

p_fn err_code free_all_images(void) {
  if (hashmap_iterate_pairs(&loaded_images, free_all_images_itr, NULL) != 0) {
    fprintf(stderr, "failed to deallocate hashmap entries?\n");
    // return ERR_RM_FAILED_DESTROY;
  }
  hashmap_destroy(&loaded_images);
  return ERR_OKAY;
}

p_fn err_code rm_insert_image(const char *image_name, ALLEGRO_BITMAP *precreated_bitmap) {
  ALLEGRO_BITMAP* image = (ALLEGRO_BITMAP*)hashmap_get(&loaded_images,
                                                       image_name,
                                                       strlen(image_name));
  if (image == NULL) {
    if (hashmap_put(&loaded_images, image_name,
                    strlen(image_name), precreated_bitmap) != 0) {
      return ERR_RM;
    }
    return ERR_OKAY;
  } else {
    return ERR_RM_RESOURCE_EXISTS;
  }
}

p_fn err_code rm_create_image(const char *image_name, const char *file_path) {
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

p_fn err_code rm_delete_image(const char *image_name) {
  ALLEGRO_BITMAP* image = (ALLEGRO_BITMAP*)hashmap_get(&loaded_images,
                                                      image_name,
                                                      strlen(image_name));

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

p_fn err_code rm_get_image(const char *image_name, ALLEGRO_BITMAP **bitmap_output) {
  ALLEGRO_BITMAP* image = (ALLEGRO_BITMAP*)hashmap_get(&loaded_images,
                                                      image_name,
                                                      strlen(image_name));
  
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

p_private p_fn int free_all_fonts_itr(void* const context, struct hashmap_element_s *element) {
  printf("Freeing Font: %s\n", element->key);
  
  ALLEGRO_FONT *font = (ALLEGRO_FONT*)element->data;
  al_destroy_font(font);

  return -1;
}

p_fn err_code free_all_fonts(void) {
  if (hashmap_iterate_pairs(&loaded_fonts, free_all_fonts_itr, NULL) != 0) {
    fprintf(stderr, "failed to deallocate hashmap entries (Fonts)\n");
    // return ERR_RM_FAILED_DESTROY;
  }
  hashmap_destroy(&loaded_fonts);
  return ERR_OKAY;
}

p_fn err_code rm_insert_font(const char *font_name, ALLEGRO_FONT *precreated_font) {
  ALLEGRO_FONT* font = (ALLEGRO_FONT*)hashmap_get(&loaded_fonts,
                                                  font_name,
                                                  strlen(font_name));
  if (font == NULL) {
    if (hashmap_put(&loaded_fonts, font_name, strlen(font_name), precreated_font) != 0) {
      return ERR_RM;
    }
    return ERR_OKAY;
  } else {
    return ERR_RM_RESOURCE_EXISTS;
  }
}

p_fn err_code rm_create_font(const char *font_name, const char *file_path, u32 font_size) {
  printf("Loading Font: %s\n", file_path);
  
  ALLEGRO_FONT *font = al_load_ttf_font(file_path, (int)font_size, ALLEGRO_TTF_NO_AUTOHINT);
  if (!font) {
    return ERR_RM_LOADING_RESOURCES;
  }

  err_code result = rm_insert_font(font_name, font);
  if (result != ERR_OKAY) {
    al_destroy_font(font);
  }
  
  return result;
}

p_fn err_code rm_delete_font(const char *font_name) {
  ALLEGRO_FONT* font = (ALLEGRO_FONT*)hashmap_get(&loaded_fonts,
                                                  font_name,
                                                  strlen(font_name));

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

p_fn err_code rm_get_font(const char *font_name, ALLEGRO_FONT **font_output) {
  ALLEGRO_FONT* font =
        (ALLEGRO_FONT*)hashmap_get(&loaded_fonts,
                                    font_name,
                                    strlen(font_name));
  
  if (font == NULL) {
    (*font_output) = NULL;
    return ERR_RM_NOT_FOUND;
  } else {
    (*font_output) = font;
    return ERR_OKAY;
  }
}

// -- Fonts --
// ++ Audio Streams ++

p_private p_fn int free_all_audio_streams_itr(void* const context, struct hashmap_element_s *element) {
  printf("Freeing Audio Stream: %s\n", element->key);
  
  ALLEGRO_AUDIO_STREAM *as = (ALLEGRO_AUDIO_STREAM*)element->data;
  al_destroy_audio_stream(as);

  return -1;
}

p_fn err_code free_all_audio_streams(void) {
  if (hashmap_iterate_pairs(&loaded_audio_streams, free_all_audio_streams_itr, NULL) != 0) {
    fprintf(stderr, "failed to deallocate hashmap entries (Audio Streams)\n");
    // return ERR_RM_FAILED_DESTROY;
  }
  hashmap_destroy(&loaded_audio_streams);
  
  return ERR_OKAY;
}

err_code rm_insert_audio_stream(const char *stream_name, ALLEGRO_AUDIO_STREAM *precreated_stream) {
  ALLEGRO_AUDIO_STREAM* stream =
      (ALLEGRO_AUDIO_STREAM*)hashmap_get(&loaded_audio_streams,
                                        stream_name,
                                        strlen(stream_name));
  if (stream == NULL) {
    if (hashmap_put(&loaded_audio_streams, stream_name,
                    strlen(stream_name), precreated_stream) != 0) {
      return ERR_RM;
    }
    return ERR_OKAY;
  } else {
    return ERR_RM_RESOURCE_EXISTS;
  }
}

p_fn err_code rm_create_audio_stream(const char *stream_name, const char *file_path) {
  // TODO(kay): The buffer_count and sample_count are set to these values,
  // make sure they work for every file
  printf("%s\n", file_path);
  ALLEGRO_AUDIO_STREAM *audio_stream
              = al_load_audio_stream(file_path, 8, 512);
  if (audio_stream == NULL) {
    return ERR_RM_LOADING_RESOURCES;
  }
  
  err_code status = rm_insert_audio_stream(stream_name, audio_stream);
  if (status != ERR_OKAY) {
    al_destroy_audio_stream(audio_stream);
  }
  
  return status;
}

p_fn err_code rm_delete_audio_stream(const char *stream_name) {
  ALLEGRO_AUDIO_STREAM* stream =
              (ALLEGRO_AUDIO_STREAM*)hashmap_get(&loaded_audio_streams,
                                                 stream_name,
                                                 strlen(stream_name));

  if (stream == NULL) {
    return ERR_RM_NOT_FOUND;
  } else {
    if (hashmap_remove(&loaded_audio_streams, stream_name, strlen(stream_name) != 0)) {
      return ERR_RM;
    }
    al_destroy_audio_stream(stream);
    return ERR_OKAY;
  }
}

p_fn err_code rm_get_audio_stream(const char *stream_name, ALLEGRO_AUDIO_STREAM **audio_stream_output) {
  ALLEGRO_AUDIO_STREAM *stream =
        (ALLEGRO_AUDIO_STREAM*)hashmap_get(&loaded_audio_streams,
                                           stream_name,
                                           strlen(stream_name));
  
  if (stream == NULL) {
    (*audio_stream_output) = NULL;
    return ERR_RM_NOT_FOUND;
  } else {
    (*audio_stream_output) = stream;
    return ERR_OKAY;
  }
}

// -- Audio Streams --
