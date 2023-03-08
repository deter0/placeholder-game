#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "audio_manager.h"
#include "resource_manager.h"

#include <glm/glm.hpp>
// #include "p/p.h"

struct Player {
  glm::vec2 position;
};

Player player_new(void);
void player_render(Player *player);
void player_handle_input(Player *player, double dt, ALLEGRO_KEYBOARD_STATE *state);
