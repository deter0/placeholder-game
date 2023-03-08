#pragma once

#include "./p.h"

#define RESOURCES_PATH "../resources/"
#define RESOURCE_PATH(subpath) "../resources/" #subpath

const char *get_resource_path(const char *subpath);
