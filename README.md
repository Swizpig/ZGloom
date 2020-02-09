# ZGloom

A re-implementation of [Amiga FPS Gloom](https://en.wikipedia.org/wiki/Gloom_(video_game)).

# Screenshots

![font.png](screenshots/font.png)
![deluxe2.png](screenshots/deluxe2.png)
![deluxe3.png](screenshots/deluxe3.png)
![organs.png](screenshots/organs.png)

# Instructions

## Windows

Drop everything in the bin folder in the root of Gloom (ie, above the misc/, pics/ etc dirs). The game will auto-detect Zombie Massacre depending on the presence of the "stuf/stages" file.
Will need the VS2013 runtime.

## Linux

1. Install libSDL2, libSDL2_mixer and [libXMP](https://github.com/cmatsuoka/libxmp).
2. Compile with `make`.

# Controls

Cursors, left alt to strafe, left ctrl to fire. F1 skips level. F12 toggles fullscreen. PrintScreen dumps a bitmap.

# TODO

* Backend of the renderer is mostly mine, and is flaky. There's Z-fighting that doesn't exist in the original, for instance
* Sound is deafening on occasion, need variable volume playback and priority system
* Some monsters are missing their logic still
* Incomplete HUD and infinite lives currently
* Very basic in-game menu
* No defender sub-game
* LibXMP's MED playback isn't great on some tunes

# License

Dunno. The Gloom source release says only the .s and .bb2 files are open source, but the Gloom executable bakes in some maths lookup tables (but then, they are generated by the .bb2 files), bullet and sparks graphics, and 
the title screen for Classic Gloom. I probably won't add the latter and just display the Black Magic image, or something.

Uses LibXMP for MED playback
http://xmp.sourceforge.net/

Uses SDL2 and SDL2 mixer
https://www.libsdl.org/

DeCrunchmania C code by Robert Leffman, licence unknown
http://aminet.net/package/util/pack/decrunchmania_os4
