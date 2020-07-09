#ifndef __SERVER_H__
#define __SERVER_H__

#include "../general-config.h"
#include "../core/user.h"
#include "../core/mob.h"

extern struct user_server_st users[MAX_USERS_PER_CHANNEL];
extern struct mob_server_st mobs[30000];
extern struct ground_item_st ground_items[4096];
extern struct party_st parties[500];
extern struct settings_st settings;

extern int current_time;
extern int current_weather;

void* init_server();
void start_server();

#endif
