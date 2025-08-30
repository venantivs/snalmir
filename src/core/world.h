#ifndef __WORLD_H__
#define __WORLD_H__

#include <stdbool.h>

#define WORLD_MOV 3
#define WORLD_ITEM 2
#define WORLD_MOB 1

extern short g_mob_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST];
extern short g_item_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST];
extern int g_height_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST];
extern struct guildzone_st g_guild_zone[MAX_GUILD_ZONE];

bool update_world(int, short *, short *, int);
bool get_empty_mob_grid(int, short *, short *);
bool get_empty_item_grid(int, short *, short *);

void read_height_map();
bool initialize_ground_items();
void read_teleport();
void read_guild_zone();
bool remove_object(int mob_index, struct position_st position, int flag);

#endif