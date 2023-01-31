#!/bin/bash

ALLEGRO_LINKS=`pkg-config --libs --cflags allegro-5 allegro_acodec-5 allegro_primitives-5 allegro_audio-5 allegro_font-5 allegro_image-5 allegro_ttf-5`
g++ ./src/main.cpp ./src/p/parr.cpp -o ./game $ALLEGRO_LINKS
