#ifndef __GAME_ITEMS_H__
#define __GAME_ITEMS_H__

#include <stdbool.h>

#include "base-def.h"

extern struct item_list_st item_list[MAX_ITEM_LIST];
extern struct ground_item_st init_list[MAX_INIT_ITEM_LIST];

void load_game_items();

short get_effect_value(short item_id, char effect);

#endif