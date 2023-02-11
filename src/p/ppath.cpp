#include <string.h>
#include <stdlib.h>

#include "p.h"
#include "ppath.h"

const char *get_resource_path(const char *subpath) {
  char *final_string;
  u32 final_string_len = strlen(RESOURCES_PATH) + strlen(subpath);
  
  final_string = (char*)malloc(final_string_len + 1);
  if (final_string == NULL) {
    return NULL;
  }
  
  strcpy(final_string, RESOURCES_PATH);
  strcat(final_string, subpath);
  
  return final_string;
}
