FILENAME=combineHarvest
SRCNAME=game.c
STD=gnu11

all: $(FILENAME)

release: win32 *.c
	clang $(SRCNAME) -o$(FILENAME) -std=$(STD) -lSDL2 -lSDL2_image -lSDL2_mixer -lm -lz -Wall -O2
	zip -r CHLinux.zip $(FILENAME) snd img

$(FILENAME): *.c
	clang $(SRCNAME) -o$(FILENAME) -std=$(STD) -lSDL2 -lSDL2_image -lSDL2_mixer -lm -lz -Wall -O0 -g

run: $(FILENAME)
	./$(FILENAME)

win32:
	i686-w64-mingw32-gcc $(SRCNAME) -owindows/$(FILENAME).exe -std=$(STD) `i686-w64-mingw32-sdl2-config --libs` `i686-w64-mingw32-pkg-config SDL2_image --libs` -lSDL2_mixer -lm -lz -Wall -O2
	zip -r CHWin.zip windows

.PHONY: all run
