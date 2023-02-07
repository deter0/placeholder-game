#!/bin/bash

PLACEHOLDER_GAME=`pwd`
cd /tmp
git clone https://github.com/g-truc/glm

cp glm/glm -r "$PLACEHOLDER_GAME/include"
 
