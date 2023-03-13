project('placeholder', 'cpp')

sources = [
  './src/main.cpp',
  './src/tile_renderer.cpp',
  './src/player.cpp',
  './src/p/parr.cpp',
  './src/p/ppath.cpp',
  './src/resource_manager.cpp',
  './src/audio_manager.cpp',
  './src/ui.cpp',
  './src/ui_layouts.cpp',
  './src/pfont.cpp'
]

include_dirs = [
  'include'
]

dependencies = [
  dependency('allegro-5'),
  dependency('allegro_dialog-5'),
  dependency('allegro_acodec-5'),
  dependency('allegro_primitives-5'),
  dependency('allegro_audio-5'),
  dependency('allegro_font-5'),
  dependency('allegro_image-5'),
  dependency('allegro_ttf-5')
]

executable('game', sources : sources, dependencies : dependencies, include_directories : include_dirs)
