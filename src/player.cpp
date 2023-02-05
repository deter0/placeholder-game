#include <stdio.h>
#include <stdlib.h>
#include <glm/glm.hpp>

#include "audio_manager.h"
#include "resource_manager.h"

#include "player.h"

Player player_new(void) {
  Player player = {
    .position = glm::vec2(0)
  };
  rm_create_image("player", "./resources/babelfish.png");
  
  return player;
}

void player_render(Player *player) {
  ALLEGRO_BITMAP *player_sprite;
  
  // TODO(kay): Use a missing image
  p_ASSERT_ERR(rm_get_image("player", &player_sprite));
  
  al_draw_bitmap(player_sprite, player->position.x, player->position.y, 0);
}

void player_handle_input(Player *player, double dt, ALLEGRO_KEYBOARD_STATE *state) {
  if (al_key_down(state, ALLEGRO_KEY_D)) {
    player->position.x += 400.f * dt;
  }
  if (al_key_down(state, ALLEGRO_KEY_A)) {
    player->position.x -= 400.f * dt;
  }
  if (al_key_down(state, ALLEGRO_KEY_W)) {
    player->position.y -= 400.f * dt;
  }
  if (al_key_down(state, ALLEGRO_KEY_S)) {
    player->position.y += 400.f * dt;
  }
}
