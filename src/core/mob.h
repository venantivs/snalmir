#ifndef __MOB_H__
#define __MOB_H__

#include <stdbool.h>

#include "base-def.h"

#define MOB_RESPAWN_DELAY 37

#define MAX_ENEMY       8
#define MOB_EMPTY       0
#define MOB_USERDOCK    1
#define MOB_USER        2
#define MOB_IDLE        3
#define MOB_PEACE       4
#define MOB_COMBAT      5
#define MOB_RETURN			6
#define MOB_FLEE				7
#define MOB_ROAM				8
#define MOB_DEAD        9
#define MOB_MARKET			10

#define MOB_TYPE_SUMMON			1
#define MOB_TYPE_GUARD			2
#define MOB_TYPE_MONSTER 		3
#define MOB_TYPE_USER				4
#define MOB_TYPE_NPC				5
#define MOB_GUARD_PESA			6
#define MOB_TYPE_AGUA				7
#define MOB_TYPE_PESA_NPC		8
#define MOB_TYPE_PESA_MOB		9
#define MOB_TYPE_PESA_BOSS	10
#define MOB_TYPE_CUBO				11

struct mob_server_st {
	struct mob_st mob;
	int  mode;
	clock_t next_action;
	clock_t	potion_hp;
	clock_t	potion_mp;
	int generate_index;
	int current_target;
	int summoner;
	int mob_type;
	int guild_disable;
	int drop_bonus;
	int exp_bonus;
	int max_damage;
	int max_damage_value;
	int low_hp;
	int low_hp_value;
	int low_defense;
	int low_defense_value;
	int low_level;
	int low_level_value;
	int force_damage;
	int reflect_damage;
	int skill_delay[33];
	int enemy_list[MAX_ENEMY];
	int enemy_count;
	int baby_mob[13];
	int spawn_type;
	int buffer_time[16];
	int evoc_time;
	struct trade_st trade_info;
	bool is_trading;
	clock_t trade_time;
	int segment;
	int death_id;
	int next;
	int party_index;
	int party_leader;
	int bonus_pvm;
	bool in_cancel;
	bool in_pvp;
	bool in_arena;
	bool in_duel;
	bool in_party;
};

void load_base_char_mobs();
void load_mob(int, int);
void save_mob(int, bool, int);
void load_selchar(struct mob_st _char[4], struct char_list_st *);
void clear_property(struct mob_server_st *mob);
int standby_processor(struct mob_server_st *mob);
void movement(struct mob_server_st *mob, int x, int y, int type);
void processor_sec_timer_mob(struct mob_server_st *mob, int sec_counter);
void add_enemy_list(struct mob_server_st *mob, short target_id);
void remove_enemy_list(struct mob_server_st *mob, short target_id);
void level_up(struct mob_server_st *mob);
void mob_drop(struct mob_server_st *user, int mob_index);
int battle_processor(struct mob_server_st *mob);

bool is_dead(struct mob_server_st mob);
bool is_summon(struct mob_server_st mob);

#endif
