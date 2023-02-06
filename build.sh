#!/bin/bash

ALLEGRO_LINKS=`pkg-config --libs --cflags allegro-5 allegro_dialog-5 allegro_acodec-5 allegro_primitives-5 allegro_audio-5 allegro_font-5 allegro_image-5 allegro_ttf-5`
gcc -Wall -lstdc++ -g3 --std=c++11 -Iinclude ./src/main.cpp ./src/tile_renderer.cpp ./src/player.cpp ./src/p/parr.cpp ./src/resource_manager.cpp ./src/audio_manager.cpp -o ./game $ALLEGRO_LINKS -lm
