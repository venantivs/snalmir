#ifndef __BASE_FUNCTIONS_H__
#define __BASE_FUNCTIONS_H__

#include <stddef.h>

// GET
size_t get_spawn_empty_index();
void get_create_mob(int mob_index, int client_index);
void get_guild_zone(struct mob_server_st mob, short *position_x, short *position_y);
void get_current_score(int user_index);
int get_bonus_score_points(struct mob_st *user);
void get_bonus_master_points(struct mob_st *user);
bool get_bonus_skill_points(struct mob_st *user);
short get_item_sanc(struct item_st *item);
int get_mob_ability(struct mob_st *user, int effect);
int get_current_hp(int user_index);
int get_current_mp(int user_index);
int get_total_kills(int mob_index);
void set_total_kills(int mob_index, int frag);
void get_create_item(int item_index);
int get_max_ability(struct mob_st *user, int effect);
void get_affect(int offset, struct affect_st *skills, unsigned char *buffer);
int check_pvp_area(int mob_index);
double get_distance(struct position_st position_a, struct position_st position_b);
int get_door_type(struct position_st position);
void get_action(int mob_index, short posX, short posY, int type, char *command);
short get_item_ability(struct item_st *item, int effect);
void set_pk_points(int mob_index, int pk_points);
int get_cape_id(int item_id);
unsigned int get_exp_by_kill(unsigned int exp, int attacker_index, int target_index);
int get_item_slot(int user_index, int item_id, int type);

// SEND
void send_grid_mob(int);
void send_grid_item(int index, void *packet);
void send_score(short index);
void send_etc(short index);
void send_affects(int index);
void send_emotion(int index, int effect_type, int effect_value);
void send_create_mob(int user_index, int mob_index);
void send_remove_mob(int mob_index, int to_remove_index, int delete_type);
void send_grid_multicast(short position_x, short position_y, void *packet, int index);
void send_grid_multicast_with_packet(int mob_index, short position_x, short position_y, unsigned char *packet);
void send_teleport(int mob_index, struct position_st destination);
void send_create_item(int user_index, short inventory_type, short inventory_slot, struct item_st *item);
void send_attack(int attacker_index, int target_index);
void send_env_effect(struct position_st min, struct position_st max, short effect_id, short time);

#endif