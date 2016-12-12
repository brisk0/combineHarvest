//Consider yourself warned. Haste makes code sad :c

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <stdlib.h>
#include <tgmath.h>

#include "types.h"
#include "data.h"

enum Elements get_tile(int x, int y) {
	int i = (int)(x/16 - 12);
	int j = (int)(y/16 - 9);
	if(i < 0) i = 0;
	if(j < 0) j = 0;
	if(i > 15) i = 15;
	if(j > 13) j = 13;
	return tiles[i][j];
}

enum Elements get_tile_adj(int x, int y, float angle) {
	x += 16*cos(angle);
	y += 16*sin(angle);
	return get_tile(x, y);
}


//returns the original contents of the tile
enum Elements set_tile(int x, int y, enum Elements elem) {
	int i = (int)(x/16 - 12);
	int j = (int)(y/16 - 9);
	if(i < 0) i = 0;
	if(j < 0) j = 0;
	if(i > 15) i = 15;
	if(j > 13) j = 13;
	enum Elements existing = tiles[i][j];
	tiles[i][j] = elem;
	return existing;
}

enum Elements set_tile_adj(int x, int y, enum Elements elem, float angle) {
	x += 16*cos(angle);
	y += 16*sin(angle);
	return set_tile(x, y, elem);
}

enum Direction direction_from_angle(float angle) {
	return ((int)(angle/(2*M_PI)*8+8.1)%8);

}

//returns the direction of the next closest tile to the target tile. x and y are tile coords this time
float find_tile(int x, int y, enum Elements tile) {
	float bestLength = 1000;
	int besti;
	int bestj;
	bool found = false;

	for(int j = 0; j < 14; j++) {
		for(int i = 0; i < 16; i++) {
			if(tiles[i][j] == tile && pow((i - x), 2) + pow((j - y),2) < bestLength) {
				besti = i;
				bestj = j;
				found = true;
			}
		}
	}
	if(!found) {
		return (float)rand() / RAND_MAX * 2*M_PI;
	}
	float angle = atan2(bestj - y, besti - x);
	return angle;
}

void tick_animal(struct Animal *me) {
	if(me->steps_remaining) {
		me->steps_remaining--;
	} else if(tiles[me->i][me->j] == VEG && !me->fed) {
		tiles[me->i][me->j] = EARTH;
		me->fed = true;
		printf("nom!\n");
	} else if(rand() < RAND_MAX/32) {
		//Moving
		 if(me->fed){
			//If I'm full I wander
			me->dir = (float)rand() / RAND_MAX * 2*M_PI;
		} else {
			//if I'm hungry, I seek!
			me->dir = 2.0*M_PI/8.0 * (float)find_tile(me->i, me->j, VEG);
		}
		 int oldi = me->i;
		 int oldj = me->j;
		 me->i += round(cos(me->dir));
		 me->j += round(sin(me->dir));
		 me->steps_remaining = 16;
		 //Can't walk off edge
		 if(me->i < 0 ){ me->i = 0;	me->steps_remaining = 0;}
		 if(me->j < 0 ){ me->j = 0;	me->steps_remaining = 0;}
		 if(me->i > 15 ){ me->i = 15;	me->steps_remaining = 0;}
		 if(me->j > 13 ){ me->j = 13;	me->steps_remaining = 0;}
		 //Can't pass through logs
		 if(tiles[me->i][me->j] == LOG){me->i = oldi; me->j = oldj; me->steps_remaining = 0;}
		 //Snap direction to actual movement for interpolation
		 me->dir = atan2(me->j - oldj, me->i - oldi);
	} else if(rand() < RAND_MAX/512) {
		if(me->fed) {
			printf("hungry!\n");
			me->fed = false;
		} else if(!tiles[me->i][me->j]) { //we only die on empty tiles because vanishing without a corpse is confusing
			tiles[me->i][me->j] = DEATH;
			*me = animals[animal_count - 1];
			animal_count--; //:c
		}
	}
}

int
main(int argc, char **argv) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_CreateWindowAndRenderer(SCREENW, SCREENH, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
	SDL_RenderSetLogicalSize(renderer, SCREENW, SCREENH);
	SDL_RenderSetIntegerScale(renderer, true);
	SDL_SetWindowTitle(window, "Combine Harvest");

	//Load Textures
	bg			= IMG_LoadTexture(renderer, "img/back.png");
	fg 			= IMG_LoadTexture(renderer, "img/fore.png");
	ss_player		= IMG_LoadTexture(renderer, "img/player.png");
	ss_items 		= IMG_LoadTexture(renderer, "img/items.png");
	ss_player_holding 	= IMG_LoadTexture(renderer, "img/playerHolding.png");
	ss_fire 		= IMG_LoadTexture(renderer, "img/fire.png");
	ss_water 		= IMG_LoadTexture(renderer, "img/water.png");
	ss_air			= IMG_LoadTexture(renderer, "img/air.png");
	ss_stone		= IMG_LoadTexture(renderer, "img/stone.png");
	ss_tick			= IMG_LoadTexture(renderer, "img/tick.png");
	ss_animal		= IMG_LoadTexture(renderer, "img/animal.png");

	//Load Sounds
	Mix_Init(0);
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 1, 4096)) {
		printf("Error opening audio channel: %s", Mix_GetError());
	}
	music = Mix_LoadMUS("snd/the-sweet-village.ogg");
	Mix_PlayMusic(music, -1);
	sound_drop = Mix_LoadWAV("snd/drop.wav");
	sound_pick = Mix_LoadWAV("snd/pickup.wav");

	SDL_SetRenderDrawColor(renderer, 190, 214, 253, 255);

	//Mirror Combination Matrix
	for(int j = ELEMENTS_COUNT - 1; j >= 0; j--) {
		for(int i = ELEMENTS_COUNT - 1; i > j; i--) {
			combine[i][j] = combine[j][i];
		}
	}


	SDL_Event event;
	bool game_on = true;
	int frame_count = 0;
	int cloudx[3] = {1*SCREENW/5, 2*SCREENW/3, 3*SCREENW/5};
	int cloudy[3] = {1*SCREENH/5, 3*SCREENH/3, 2*SCREENH/5};

	for(int j = 5; j < 9; j++) {
		for(int i = 6; i < 10; i++) {
			tiles[i][j] = EARTH;
		}
	}

	while(game_on)
	{
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_RIGHT:
						case SDLK_d:
							control_state.right = true;
							player.xv = +1;
							break;
						case SDLK_LEFT:
						case SDLK_a:
							control_state.left = true;
							player.xv = -1;
							break;
						case SDLK_DOWN:
						case SDLK_s:
							control_state.down = true;
							player.yv = +1;
							break;
						case SDLK_UP:
						case SDLK_w:
							control_state.up = true;
							player.yv = -1; 
							break;
						case SDLK_ESCAPE:
							exit(0);
						default:;
							//Pick up / put down
							int i = (int)(round(cos(player.dir) + player.x/16 - 12.5));
							int j = (int)(round(sin(player.dir) + player.y/16 - 9.5));
							if(i < 0) i = 0;
							if(j < 0) j = 0;
							if(i > 15) i = 15;
							if(j > 13) j = 13;
							if(player.holding) {
								//Cannot Place on Base Element Tiles
								if(!((i == 0 && j == 0) || (i == 15 && j == 0) || (i == 15 && j == 13) || (i == 0 && j == 13))){
									if(!(i == 5 && j == 0) && combine[player.holding][tiles[i][j]]) {
										//Combine
										Mix_PlayChannel(-1, sound_drop, 0);
										tiles[i][j] = combine[player.holding][tiles[i][j]];

										//If its an animal make an animal instead
										if(animal_count < MAX_ANIMALS && tiles[i][j] == ANIMAL) {
											animals[animal_count] = (struct Animal){
												.type = ANIMAL,
													.steps_remaining = 0,
													.i = i,
													.j = j,
													.dir = 2*M_PI*3/4,
													.fed = true
											};
											animal_count++;
											tiles[i][j] = NONE;
											discovered[ANIMAL - 1] = true;
											player.holding = NONE;

										} else {

											//if it's a log you just made keep an axe in your hand
											if(tiles[i][j] == LOG) {
												player.holding = AXE;
											} else {
												player.holding = NONE;
											}
											discovered[tiles[i][j] - 1] = true;
										}

									} else if(tiles[i][j] != TREE){
										//Swap
										Mix_PlayChannel(-1, sound_drop, 0);
										enum Elements temp = tiles[i][j];
										tiles[i][j] = player.holding;
										player.holding = temp;
									}
								}
							} else {
								if(tiles[i][j] && tiles[i][j] != TREE) {
									//We get to pick things up!
									Mix_PlayChannel(-1, sound_pick, 0);

									//Special cases for things that can't be put back
									if(tiles[i][j] == VEG) {
										player.holding = FOOD;
										tiles[i][j] = EARTH;
									} else {
										//Normal Pickup
										player.holding = tiles[i][j];
										//Cloning pad exempt from clearing (and certain objects exempt from cloning)
										if(!(i == 5 && j == 0) || tiles[i][j] > CLOUD) {
											tiles[i][j] = NONE;
										}
									}
								}
							}
							break;
					}
					break;
				case SDL_KEYUP:
					switch(event.key.keysym.sym) {
						case SDLK_RIGHT:
						case SDLK_d:
							player.xv = 0;
							control_state.right = false;
							break;
						case SDLK_LEFT:
						case SDLK_a:
							player.xv = 0;
							control_state.left = false;
							break;
						case SDLK_DOWN:
						case SDLK_s:
							player.yv = 0;
							control_state.down = false;
							break;
						case SDLK_UP:
						case SDLK_w:
							player.yv = 0;
							control_state.up = false;
							break;

					}
					break;
				case SDL_QUIT:
					return 0;
					break;
			}
		}
		//Update Controls
		//No movement if no keys are pressed
		player.moving = control_state.right || control_state.left || control_state.up || control_state.down;
		if(player.moving) {
			player.dir = atan2((float)player.yv, (float)player.xv);
			
			float speed = 1;
			player.x += speed*cos(player.dir);
			player.y += speed*sin(player.dir);
			if(player.x < 198) {
				player.x = 198;
			} else if(player.x > 442) {
				player.x = 442;
			}
			if(player.y < 145) {
				player.y = 145;
			} else if(player.y > 367) {
				player.y = 367;
			}
		}
		//Remember when we said we were just going to do momentum from now on? Yeah.

		if(player.holding){
			discovered[player.holding - 1] = true;
		}
		
		//Random Tick
		if(frame_count % 60 == 0){
			int i = (float)rand()/RAND_MAX * 16;
			int j = (float)rand()/RAND_MAX * 14;
			//Plant Growth
			if(tiles[i][j] == PLANT || tiles[i][j] == VEG) {
				tiles[i][j] = VEG;
				float angle = (float)rand() / RAND_MAX * 2*M_PI;
				int xoffset = round(cos(angle));
				int yoffset = round(sin(angle));
				if(tiles[i + xoffset][j + yoffset] == EARTH) {
					tiles[i + xoffset][j + yoffset] = PLANT;
				}
			} else if(tiles[i][j] == SAP) {
				tiles[i][j] = TREE;
				discovered[TREE - 1] = true;
			} else if(tiles[i][j] == TREE) {
				float angle = (float)rand() / RAND_MAX * 2*M_PI;
				int xoffset = round(cos(angle));
				int yoffset = round(sin(angle));
				if(tiles[i + xoffset][j + yoffset] == EARTH) {
					tiles[i + xoffset][j + yoffset] = SAP;
				}
			}
		}


		//Don't let our base elements change
		tiles[0][0] = AIR;
		tiles[0][13] = FIRE;
		tiles[15][0] = WATER;
		tiles[15][13] = STONE;
		//Rubbish slot
		tiles[10][0] = NONE;

		SDL_RenderClear(renderer);
		//Draw cloud!
		for(int i=0; i < 3; i++) {
			if(cloudx[i] < -16) {
				cloudx[i] = SCREENW + 16;
				cloudy[i] = (float)rand() / RAND_MAX * SCREENH - 8;
			} else {
				cloudx[i]--;
			}
			SDL_RenderCopy(renderer, ss_items, &(SDL_Rect){0, (CLOUD-1)*16, 16, 16}, &(SDL_Rect){cloudx[i], cloudy[i], 16, 16});
		}
		SDL_RenderCopy(renderer, bg, NULL, NULL);
		//Draw everything else
		/*Draw Tiles*/
		for(int j = 0; j < 14; j++) {
			for(int i = 0; i < 16; i++) {
				if(!((i == 0 && j == 0) || (i == 15 && j == 0) || (i == 15 && j == 13) || (i == 0 && j == 13))){
					SDL_RenderCopy(renderer, ss_items, &(SDL_Rect){16, 16*(tiles[i][j] - 1), 16, 16}, &(SDL_Rect){192+16*i, 144+16*j, 16, 16});
			}
			}
		}
		//Draw Base Elements
		SDL_RenderCopy(renderer, ss_fire, &(SDL_Rect){16*(frame_count/8%7), 0, 16, 32}, &(SDL_Rect){192, 336, 16, 32});
		SDL_RenderCopy(renderer, ss_water, &(SDL_Rect){16*(frame_count/8%11), 0, 16, 32}, &(SDL_Rect){432, 128, 16, 32});
		SDL_RenderCopy(renderer, ss_air, &(SDL_Rect){16*(frame_count/8%7), 0, 16, 32}, &(SDL_Rect){192, 128, 16, 32});
		SDL_RenderCopy(renderer, ss_stone, &(SDL_Rect){0, 0, 16, 32}, &(SDL_Rect){432, 336, 16, 32});
		
		//Animal Tick
		for(int i = 0; i < animal_count; i++) {
			tick_animal(&animals[i]);
			SDL_RenderCopy(renderer, ss_animal, &(SDL_Rect){16*direction_from_angle(animals[i].dir), 0, 16, 16}, \
					&(SDL_Rect){(animals[i].i+12)*16 - animals[i].steps_remaining*cos(animals[i].dir), (animals[i].j+9)*16 - animals[i].steps_remaining*sin(animals[i].dir), 16, 16});
		}

		if(player.holding) {
			SDL_RenderCopy(renderer, ss_player_holding, &(SDL_Rect){((int)(player.dir/(2*M_PI)*8+8.1)%8)*16, 0, 16, 16}, &(SDL_Rect){player.x - 8, player.y - 16, 16, 16});
			//draw the thing he's holding
			SDL_RenderCopy(renderer,ss_items, &(SDL_Rect){0, 16*(player.holding-1), 16, 16}, &(SDL_Rect){player.x - 8, player.y-24, 16, 16});
		} else {
			SDL_RenderCopy(renderer, ss_player, &(SDL_Rect){((int)(player.dir/(2*M_PI)*8+8.1)%8)*16, 0, 16, 16}, &(SDL_Rect){player.x - 8, player.y - 16, 16, 16});
		}
		SDL_RenderCopy(renderer, fg, NULL, NULL);

		for(int i = 0; i < ELEMENTS_COUNT - 1; i++) {
			SDL_RenderCopy(renderer, ss_items, &(SDL_Rect){0, 16*i, 16, 16}, &(SDL_Rect){464+48*(i/14), 144+16*(i%14), 16, 16});
			if(discovered[i]) {
				SDL_RenderCopy(renderer, ss_tick, &(SDL_Rect){0, 0, 16, 16}, &(SDL_Rect){464+48*(i/14)+16, 144+16*(i%14), 16, 16});
			}
		}

		SDL_RenderPresent(renderer);
		SDL_Delay(14);
		frame_count++;
	}
	return 0;
}
