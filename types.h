#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

enum Elements {
	NONE,
	WATER,
	FIRE,
	STONE,
	AIR,
	EARTH,
	METAL,
	CLOUD,
	LIFE,
	CLAY,
	ANIMAL,
	PLANT,
	FISH,
	ALGAE,
	MEAT,
	VEG,
	DEATH,
	FOOD,
	SAP,
	TREE,
	LOG,
	AXE,
	ELEMENTS_COUNT
};

enum Direction {
	E,
	SE,
	S,
	SW,
	W,
	NW,
	N,
	NE
};

struct Player {
	float x;
	float y;
	int xv;
	int yv;	
	float dir;
	bool moving;
	enum Elements holding;
};

struct ControlState {
	bool left;
	bool right;
	bool up;
	bool down;
};

struct Animal {
	enum Elements type;
	int steps_remaining;
	int i;
	int j;
	float dir;
	bool fed;
};

#endif
