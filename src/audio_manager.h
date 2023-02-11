#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "p/p.h"

enum SoundEffects {
  SAD = 0  
};
p_private const char *SoundEffectsFiles[] = {
  [SAD] = "./resources/sad.wav"
};

enum Music {
  WEIRD_FISHES = 0,
  BODYSNATCHERS
};
p_private const char *MusicFiles[] = {
  [WEIRD_FISHES] = "weird-fishes.flac",
  [BODYSNATCHERS] = "bodysnatchers-8bit.ogg"
};

enum MusicPlayState {
  PlayStateStopped = 0,
  PlayStatePlaying = 1,
  PlayStateToggle = -1
};

struct MusicParams {
  float pan;
  float gain;
  float speed;
  ALLEGRO_PLAYMODE play_mode;
};

err_code audio_manager_init(void);
err_code audio_manager_set_play_state(const char *rm_music_name, enum MusicPlayState state);
err_code audio_manager_set_music_params(const char *rm_music_name, MusicParams params);
err_code audio_manager_get_music_params(const char *rm_music_name, MusicParams *params_out);
err_code audio_manager_shutdown(void);
