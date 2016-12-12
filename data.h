#ifndef DATA_H
#define DATA_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_mixer.h>
#include "types.h"

#define SCREENW 640
#define SCREENH 480

//Technical
//=========
SDL_Window* window;
SDL_Renderer* renderer;

struct ControlState control_state = {
	false,
	false,
	false,
	false
};

typedef struct Point {
	float x;
	float y;
} Point;


//Assets
//======

//Textures
SDL_Texture *ss_player, *ss_player_holding, *ss_items, *ss_water, *ss_fire, *ss_stone, *ss_air, *bg, *fg, *ss_tick, *ss_animal;

//Sounds
Mix_Music *music;
Mix_Chunk *sound_drop;
Mix_Chunk *sound_pick;

//Game
//====

//Elements discovered index (-1) as true;
bool discovered[ELEMENTS_COUNT-1] = {};

enum Elements combine[ELEMENTS_COUNT][ELEMENTS_COUNT] = {
#include "element_matrix.h"
};

enum Elements tiles[16][14] = {};

struct Player player = (struct Player){320, 160, 0, 0, S, false, NONE};

#define MAX_ANIMALS 256
struct Animal animals[MAX_ANIMALS] = {};
unsigned int animal_count = 0;




#endif
