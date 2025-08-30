#ifndef __NPC_H__
#define __NPC_H__

#define MAX_NPCGENER 8192
#define NPCG_EMPTY 0
#define NPCG_CHARGED 1

#include "base-def.h"

extern struct npcgener_st g_gener_list[MAX_NPCGENERATOR];
extern struct mob_st g_baby_list[MAX_MOB_BABY];

void load_npcs();
void read_npc_generator();
void load_npc(const char *name, int index);
void load_mob_baby();
void spawn_mobs();
void action_mob(int sec_counter);

#endif
