#ifndef __SERVER_H__
#define __SERVER_H__

#include "../general-config.h"
#include "../core/user.h"
#include "../core/mob.h"
#include "../core/npc.h"

extern struct user_server_st g_users[MAX_USERS_PER_CHANNEL];
extern struct mob_server_st g_mobs[MAX_SPAWN_LIST];
extern struct ground_item_st g_ground_items[4096];
extern struct party_st g_parties[500];

/* DB */
extern struct account_file_st g_users_db[MAX_USERS_PER_CHANNEL];
extern struct mob_st g_base_char_mobs[MOB_PER_ACCOUNT];

extern int g_sec_counter;
extern int g_min_counter;

extern unsigned long g_current_time;
extern int g_current_weather;

void	*init_server();

#endif
