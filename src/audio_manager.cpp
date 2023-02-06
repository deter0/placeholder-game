#include <stdio.h>
#include <stdlib.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "resource_manager.h"
#include "audio_manager.h"

p_private ALLEGRO_VOICE *voice;
p_private ALLEGRO_MIXER *mixer;
p_private bool disable_audio = false;

p_fn err_code audio_manager_init(void) {
  al_reserve_samples(4);

  voice = al_create_voice(48000, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
  mixer = al_create_mixer(48000, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
  
  if (!voice || !mixer) {
    fprintf(stderr, "[ERROR] Error initalizing ALLEGRO_VOICE or ALLEGRO_MIXER continuing with no audio.\n");
    disable_audio = true;
    return ERR_OKAY;
  }
  
  al_attach_mixer_to_voice(mixer, voice);
  al_set_default_mixer(mixer);
  al_set_default_voice(voice);
  
  for (u32 i = 0; i < p_sizeof_array(MusicFiles); i++) {
    err_code status = rm_create_audio_stream(MusicFiles[i], MusicFiles[i]);
    if (status != ERR_OKAY) {
      fprintf(stderr, "[ERROR] Error (0x%X) loading music file: `%s`\n", status, MusicFiles[i]);
    }
    
    ALLEGRO_AUDIO_STREAM *music;
    rm_get_audio_stream(MusicFiles[i], &music);
    al_attach_audio_stream_to_mixer(music, mixer);
    al_set_audio_stream_playing(music, false);
  }
  
  return ERR_OKAY;
}

p_fn err_code audio_manager_set_play_state(const char *rm_music_name, enum MusicPlayState state) {
  if (disable_audio)
    return ERR_OKAY;

  ALLEGRO_AUDIO_STREAM *stream;
  if (rm_get_audio_stream(rm_music_name, &stream) != ERR_OKAY || stream == NULL) {
    return ERR_AM_AUDIO_NOT_FOUND;
  }
  
  if (state == PlayStateToggle) {
    bool isPlaying = al_get_audio_stream_playing(stream);
    al_set_audio_stream_playing(stream, !isPlaying);
    
    return ERR_OKAY;
  }
  
  // Stopped = 0, Playing = 1
  al_set_audio_stream_playing(stream, state);
  return ERR_OKAY;
}

p_fn err_code audio_manager_set_music_params(const char *rm_music_name, MusicParams params) {
  if (disable_audio)
    return ERR_OKAY;

  ALLEGRO_AUDIO_STREAM *stream;
  if (rm_get_audio_stream(rm_music_name, &stream) != ERR_OKAY || stream == NULL) {
    return ERR_AM_AUDIO_NOT_FOUND;
  }
  
  al_set_audio_stream_pan(stream, params.pan);
  al_set_audio_stream_gain(stream, params.gain);
  al_set_audio_stream_speed(stream, params.speed);
  al_set_audio_stream_playmode(stream, params.play_mode);
  
  return ERR_OKAY;
}

p_fn err_code audio_manager_get_music_params(const char *rm_music_name, MusicParams *params_out) {
  if (disable_audio)
    return ERR_OKAY;

  ALLEGRO_AUDIO_STREAM *stream;
  if (rm_get_audio_stream(rm_music_name, &stream) != ERR_OKAY || stream == NULL) {
    return ERR_AM_AUDIO_NOT_FOUND;
  }
  
  params_out->pan = al_get_audio_stream_pan(stream);
  params_out->gain = al_get_audio_stream_gain(stream);
  params_out->speed = al_get_audio_stream_speed(stream);
  params_out->play_mode = al_get_audio_stream_playmode(stream);
  
  return ERR_OKAY;
}

p_fn err_code audio_manager_shutdown(void) {
  if (disable_audio)
    return ERR_OKAY;
    
  for (u32 i = 0; i < p_sizeof_array(MusicFiles); i++) {
    ALLEGRO_AUDIO_STREAM *music;
    err_code status = rm_get_audio_stream(MusicFiles[i], &music);
    if (status != ERR_OKAY) {
      fprintf(stderr, "[ERROR] Error (0x%X) getting music file: `%s`\n", status, MusicFiles[i]);
    }
    
    al_set_audio_stream_playing(music, false);
    al_detach_audio_stream(music);
  }

  al_destroy_mixer(mixer);
  al_destroy_voice(voice);
  return ERR_OKAY;
}
