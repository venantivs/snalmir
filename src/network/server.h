#ifndef __SERVER_H__
#define __SERVER_H__

#include "../general-config.h"
#include "../core/user.h"
#include "../core/mob.h"
#include "../core/npc.h"

extern struct user_server_st users[MAX_USERS_PER_CHANNEL];
extern struct mob_server_st mobs[MAX_SPAWN_LIST];
extern struct ground_item_st ground_items[4096];
extern struct party_st parties[500];
extern struct settings_st server_settings;

/* DB */
extern struct account_file_st users_db[MAX_USERS_PER_CHANNEL];
extern struct mob_st base_char_mobs[MOB_PER_ACCOUNT];

extern int sec_counter;
extern int min_counter;

extern unsigned long current_time;
extern int current_weather;

void	*init_server();

#endif
