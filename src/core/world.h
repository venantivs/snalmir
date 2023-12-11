#ifndef __WORLD_H__
#define __WORLD_H__

#include <stdbool.h>

#define WORLD_MOV 3
#define WORLD_ITEM 2
#define WORLD_MOB 1

extern short mob_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST];
extern short item_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST];
extern int height_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST];

bool update_world(int, short *, short *, int);
bool get_empty_mob_grid(int, short *, short *);
bool get_empty_item_grid(int, short *, short *);

#endif