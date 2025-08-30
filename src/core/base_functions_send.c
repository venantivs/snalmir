/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/base_functions_send.c
 * Descrição: Arquivo onde são implementadas algumas funções setters úteis.
 */

#include "base-def.h"
#include "../network/server.h"
#include "base_functions.h"
#include "world.h"
#include "../network/socket-utils.h"
#include "item_effect.h"
#include "experience_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
send_grid_mob(int index)
{
  if (index <= 0 || index >= MAX_SPAWN_LIST)
    return;

  struct mob_st *npc_mob = &g_mobs[index].mob;

  short posX = npc_mob->current.X, posY = npc_mob->current.Y;

  int vision_x = VIEW_GRIDX, vision_y = VIEW_GRIDY,
      min_position_x = (posX - HALF_GRIDX),
      min_position_y = (posY - HALF_GRIDY);

  if ((min_position_x + vision_x) >= MAX_GRIDX)
    vision_x = (vision_x - (vision_x + min_position_x - MAX_GRIDX));

  if ((min_position_y + vision_y) >= MAX_GRIDY)
    vision_y = (vision_y - (vision_y + min_position_y - MAX_GRIDY));

  if (min_position_x < 0) {
    min_position_x = 0;
    vision_x = (vision_x + min_position_x);
  }

  if (min_position_y < 0) {
    min_position_y = 0;
    vision_y = (vision_y + min_position_y);
  }

  int max_position_x = (min_position_x + vision_x),
    max_position_y = (min_position_y + vision_y);

  for (int y = min_position_y; y < max_position_y; y++) {
    for (int x = min_position_x; x < max_position_x; x++) {
      short mob_id = g_mob_grid[y][x];
      short item_id = g_item_grid[y][x];

      if (x > min_position_x || x <= max_position_x || y > min_position_y || y <= max_position_y)  {
        if (item_id > 0 && index <= MAX_USERS_PER_CHANNEL)
          get_create_item(item_id);
      }

      if (mob_id <= 0 || index == mob_id)
        continue;

      if (mob_id <= MAX_USERS_PER_CHANNEL)
        send_create_mob(mob_id, index);

      if (index <= MAX_USERS_PER_CHANNEL)
        send_create_mob(index, mob_id);
    }
  }
}

void
send_grid_item(int index, void *packet)
{
  if (index <= 0 || index >= MAX_INIT_ITEM_LIST)
    return;

  short posX = g_ground_items[index].position.X,
    posY = g_ground_items[index].position.Y;

  int VisX = VIEW_GRIDX, VisY = VIEW_GRIDY,
    minPosX = (posX - HALF_GRIDX),
    minPosY = (posY - HALF_GRIDY);

  if ((minPosX + VisX) >= MAX_GRIDX)
    VisX = (VisX - (VisX + minPosX - MAX_GRIDX));

  if ((minPosY + VisY) >= MAX_GRIDY)
    VisY = (VisY - (VisY + minPosY - MAX_GRIDY));

  if (minPosX < 0) {
    minPosX = 0;
    VisX = (VisX + minPosX);
  }

  if (minPosY < 0) {
    minPosY = 0;
    VisY = (VisY + minPosY);
  }

  int maxPosX = (minPosX + VisX),
    maxPosY = (minPosY + VisY);

  for (int nY = minPosY; nY < maxPosY; nY++) {
    for (int nX = minPosX; nX < maxPosX; nX++) {
      short mob_id = g_mob_grid[nY][nX];
      if (mob_id <= 0)
        continue;

      if (mob_id <= MAX_USERS_PER_CHANNEL)
        send_one_packet((unsigned char*) packet, xlen(packet), mob_id);
    }
  }
}

void
send_create_mob(int send_index, int create_index)
{
  if (create_index <= MAX_USERS_PER_CHANNEL && g_mobs[create_index].mode == MOB_MARKET) {
    // CREATES A MARKET MOB, TODO: Implement 
  } else {
		get_create_mob(create_index, send_index);
	}
}

void 
send_score(short index)
{
	if (index <= 0 || index >= MAX_SPAWN_LIST)
		return;

	struct packet_refresh_score refresh_score;
	refresh_score.header.size = sizeof(struct packet_refresh_score);
	refresh_score.header.operation_code = 0x336;
	refresh_score.header.index = index;

  struct mob_server_st user = g_mobs[index];

	if (index < MAX_USERS_PER_CHANNEL) {
		refresh_score.current_hp = user.mob.status.current_hp;
		refresh_score.current_mp = user.mob.status.current_mp;
	}

	refresh_score.critical = g_mobs[index].mob.critical;
	refresh_score.save_mana = g_mobs[index].mob.save_mana;

	if (user.guild_disable == 0) {
		refresh_score.guild_index = user.mob.guild_id;
		refresh_score.guild_member_type = user.mob.guild_member_type * 0x40;
	} else {
		refresh_score.guild_index = 0;
		refresh_score.guild_member_type = 0;
	}

	refresh_score.resist_1 = user.mob.resist[0];
	refresh_score.resist_2 = user.mob.resist[1];
	refresh_score.resist_3 = user.mob.resist[2];
	refresh_score.resist_4 = user.mob.resist[3];
	refresh_score.regen_hp = user.mob.regen_hp;
	refresh_score.regen_mp = user.mob.regen_mp;

	int m = user.mob.magic_increment;
	if (m >= 255)
		m = 254;

	refresh_score.magic_increment = m;
	refresh_score.status = user.mob.status;

  memset(&refresh_score.special, 0x0, 4);
	uintptr_t affect_index = (uintptr_t) ((uintptr_t) refresh_score.affect - (uintptr_t) &refresh_score); // Calcula a posição (em bytes) do affect no packet
  get_affect(affect_index, user.mob.affect, (unsigned char*) &refresh_score);
	send_grid_multicast(user.mob.current.X, user.mob.current.Y, (unsigned char*) &refresh_score, 0); // Envia para todos os Indexes
}

void 
send_etc(short index)
{
	if (index <= 0 || index >= MAX_USERS_PER_CHANNEL)
		return;

  struct mob_st mob = g_mobs[index].mob;

	struct packet_refresh_etc refresh_etc;
	refresh_etc.header.size = sizeof(struct packet_refresh_etc);
	refresh_etc.header.operation_code = 0x337;
	refresh_etc.header.index = index;
	refresh_etc.hold = mob.hold;
	refresh_etc.exp = mob.experience;
	refresh_etc.learn = mob.learn;
	refresh_etc.status = mob.p_status;
	refresh_etc.master = mob.p_master;
	refresh_etc.skills = mob.p_skill;
	refresh_etc.magic = mob.magic_increment;
	refresh_etc.gold = mob.gold;

	add_client_packet((unsigned char*) &refresh_etc, xlen(&refresh_etc), index);
}

void
send_affects(int index)
{
	if (index > MAX_USERS_PER_CHANNEL)
		return;

  struct mob_st user = g_mobs[index].mob;

  struct packet_affect send_affect;
	send_affect.header.operation_code = 0x3B9;
	send_affect.header.size = 140;
	send_affect.header.index = index;

	for (size_t i = 0; i < MAX_AFFECT; i++) {
		if (user.affect[i].time >= 0 && user.affect[i].index != 0) {
			send_affect.affect[i].index = user.affect[i].index;
			send_affect.affect[i].master = user.affect[i].master;
			send_affect.affect[i].value = user.affect[i].value;
			send_affect.affect[i].time = user.affect[i].time;
		} else {
			send_affect.affect[i].index = 0;
			send_affect.affect[i].master = 0;
			send_affect.affect[i].value = 0;
			send_affect.affect[i].time = 0;

			user.affect[i].index = 0;
			user.affect[i].master = 0;
			user.affect[i].value = 0;
			user.affect[i].time = 0;
		}
	}

	add_client_packet((unsigned char*) &send_affect, xlen(&send_affect), index);
}

void
send_emotion(int index, int effect_type, int effect_value)
{
  struct packet_emotion emotion;
	emotion.header.size = sizeof(struct packet_emotion);
	emotion.header.operation_code = 0x36A;
	emotion.header.index = index;
	emotion.effect_type = effect_type;
	emotion.effect_value = effect_value;
	emotion.unknown = 0;

  struct mob_st user = g_mobs[index].mob;

	send_grid_multicast(user.current.X, user.current.Y, (unsigned char *) &emotion, 0);
}

// TODO: REFATORAR
void 
send_grid_multicast(short position_x, short position_y, unsigned char *packet, int index)
{
	int VisX = VIEW_GRIDX, VisY = VIEW_GRIDY,
		minPosX = (position_x - HALF_GRIDX),
		minPosY = (position_y - HALF_GRIDY);

	if ((minPosX + VisX) >= MAX_GRIDX)
		VisX = (VisX - (VisX + minPosX - MAX_GRIDX));

	if ((minPosY + VisY) >= MAX_GRIDY)
		VisY = (VisY - (VisY + minPosY - MAX_GRIDY));

	if (minPosX < 0) {
		minPosX = 0;
		VisX = (VisX + minPosX);
	}

	if (minPosY < 0) {
		minPosY = 0;
		VisY = (VisY + minPosY);
	}

	int maxPosX = (minPosX + VisX),
		maxPosY = (minPosY + VisY);

	for (int nY = minPosY; nY < maxPosY; nY++) {
		for (int nX = minPosX; nX < maxPosX; nX++) {
			short mob_id = g_mob_grid[nY][nX];

			if (mob_id <= 0 || index == mob_id)
				continue;

			if (packet == NULL || mob_id >= MAX_USERS_PER_CHANNEL)
				continue;

      send_one_packet((unsigned char*) packet, xlen(packet), index);
		}
	}
}

void
send_grid_multicast_with_packet(int mob_index, short position_x, short position_y, unsigned char *packet)
{
	if (mob_index <= 0 || mob_index >= MAX_SPAWN_LIST)
		return;

	struct mob_st *mob = &g_mobs[mob_index].mob;

	int VisX = VIEW_GRIDX, VisY = VIEW_GRIDY,
		minPosX = (mob->current.X - HALF_GRIDX),
		minPosY = (mob->current.Y - HALF_GRIDY);

	if ((minPosX + VisX) >= MAX_GRIDX)
		VisX = (VisX - (VisX + minPosX - MAX_GRIDX));

	if ((minPosY + VisY) >= MAX_GRIDY)
		VisY = (VisY - (VisY + minPosY - MAX_GRIDY));

	if (minPosX < 0) {
		minPosX = 0;
		VisX = (VisX + minPosX);
	}

	if (minPosY < 0) {
		minPosY = 0;
		VisY = (VisY + minPosY);
	}

	int maxPosX = (minPosX + VisX),
		maxPosY = (minPosY + VisY);

	int dVisX = VIEW_GRIDX, dVisY = VIEW_GRIDY,
		dminPosX = (position_x - HALF_GRIDX),
		dminPosY = (position_y - HALF_GRIDY);

	if ((dminPosX + dVisX) >= MAX_GRIDX)
		dVisX = (dVisX - (dVisX + dminPosX - MAX_GRIDX));

	if ((dminPosY + dVisY) >= MAX_GRIDY)
		dVisY = (dVisY - (dVisY + dminPosY - MAX_GRIDY));

	if (dminPosX < 0) {
		dminPosX = 0;
		dVisX = (dVisX + dminPosX);
	}

	if (dminPosY < 0) {
		dminPosY = 0;
		dVisY = (dVisY + dminPosY);
	}

	int dmaxPosX = (dminPosX + dVisX),
		dmaxPosY = (dminPosY + dVisY);

	for (int nY = minPosY; nY < maxPosY; nY++) {
		for (int nX = minPosX; nX < maxPosX; nX++) {
			int mob_id = g_mob_grid[nY][nX];
			
			if (mob_id <= 0 || mob_index == mob_id)
				continue;
			
			if (packet != NULL && mob_id <= MAX_USERS_PER_CHANNEL)
				send_one_packet((unsigned char*) packet, xlen(packet), mob_id);

			if (nX < dminPosX || nX >= dmaxPosX || nY < dminPosY || nY >= dmaxPosY) {
				if (mob_id <= MAX_USERS_PER_CHANNEL) {
					struct packet_signal_parm signal_parm;
					signal_parm.header.size = sizeof(struct packet_signal_parm);
					signal_parm.header.index = mob_index;
					signal_parm.header.operation_code = 0x165;
					signal_parm.data = DELETE_NORMAL;

					send_one_packet((unsigned char*) &signal_parm, sizeof(struct packet_signal_parm), mob_id);
				}

				if (mob_index <= MAX_USERS_PER_CHANNEL) {
					struct packet_signal_parm signal_parm;
					signal_parm.header.size = sizeof(struct packet_signal_parm);
					signal_parm.header.index = mob_id;
					signal_parm.header.operation_code = 0x165;
					signal_parm.data = DELETE_NORMAL;

					send_one_packet((unsigned char*) &signal_parm, sizeof(struct packet_signal_parm), mob_index);
				}
			}
		}
	}

	mob->current.X = position_x;
	mob->current.Y = position_y;

	for (int nY = dminPosY; nY < dmaxPosY; nY++) {
		for (int nX = dminPosX; nX < dmaxPosX; nX++) {
			short init_id = g_item_grid[nY][nX];
			short mob_id = g_mob_grid[nY][nX];

			if (nX < minPosX || nX >= maxPosX || nY < minPosY || nY >= maxPosY) {
				if (mob_id > 0 && mob_index != mob_id) {
					if (mob_id <= MAX_USERS_PER_CHANNEL)
						send_create_mob(mob_id, mob_index);

					if (mob_index <= MAX_USERS_PER_CHANNEL)
						send_create_mob(mob_index, mob_id);

					if (packet != NULL && mob_id <= MAX_USERS_PER_CHANNEL)
						send_one_packet((unsigned char*) packet, xlen(packet), mob_id);
				}

				if (init_id > 0) {
					if (mob_index <= MAX_USERS_PER_CHANNEL) {
						get_create_item(init_id);

						struct ground_item_st init = g_ground_items[init_id];

						if (init.status == 1 || init.status == 4) {

							if (get_door_type(init.position) == 0)
								continue;

							struct packet_door door;
							door.header.operation_code = 0x374;
							door.header.index = 30000;
							door.header.size = sizeof(struct packet_door);
							door.status = init.status;
							door.init_id = init_id + 10000;

							if (mob_id <= MAX_USERS_PER_CHANNEL)
								send_one_packet((unsigned char*) &door, sizeof(struct packet_door), mob_id);

							if (mob_index <= MAX_USERS_PER_CHANNEL)
								send_one_packet((unsigned char*) &door, xlen(&door), mob_index);
						}
					}
				}
			}
		}
	}

	mob->current.X = position_x;
	mob->current.Y = position_y;
}

void
send_remove_mob(int mob_index, int to_remove_index, int delete_type)
{
	struct packet_signal_parm remove_mob_signal;
	remove_mob_signal.header.size = sizeof(struct packet_signal_parm);
	remove_mob_signal.header.index = to_remove_index;
	remove_mob_signal.header.operation_code = 0x165;
	remove_mob_signal.data = delete_type;

	send_grid_multicast_with_packet(0, g_mobs[mob_index].mob.current.X, g_mobs[mob_index].mob.current.Y, (unsigned char*) &remove_mob_signal);
}

void
send_teleport(int mob_index, struct position_st destination)
{
	struct mob_st *mob = &g_mobs[mob_index].mob;

	if (mob->current.X == destination.X && mob->current.Y == destination.Y)
		return;

	update_world(mob_index, &destination.X, &destination.Y, WORLD_MOB);

	mob->last_position.X = mob->current.X;
	mob->last_position.Y = mob->current.Y;

	get_action(mob_index, destination.X, destination.Y, MOVE_TELEPORT, NULL);
}

short
get_damage(short damage, short defense, unsigned char master)
{
	short result_damage = damage - (defense >> 1);
	if ((master >> 1) > 7)
		master = 7;

	int master_factory = 12 - master;
	if (master_factory <= 0)
		master_factory = 2;

	int rand_factory = (rand() % master_factory) + master + 99;

	result_damage = (result_damage * rand_factory) / 100;
	if (result_damage < -50)
		result_damage = 0;
	else if (result_damage >= -50 && result_damage < 0)
		result_damage = (result_damage + 50) / 7;
	else if (result_damage >= 0 && result_damage <= 50)
		result_damage = ((result_damage * 5) >> 2) + 7;

	if (result_damage <= 0)
		result_damage = 1;

	return result_damage;
}

void
send_party_experience(short mob_index, short killed_index)
{
	struct mob_server_st *mob = &g_mobs[mob_index];
	struct party_st *party = &g_parties[mob->party_index];

	for (size_t i = 0; i < MAX_PARTY; i++) {
		int party_player_index = party->players[i];
		
		if (party_player_index == -1)
			continue;
		
		if (party_player_index == mob_index)
			continue;
		
		struct mob_server_st *party_player = &g_mobs[party_player_index];

		int distance = get_distance(mob->mob.current, party_player->mob.current);
		if (distance < 8) {
			int class_experience = ((get_exp_by_kill(g_mobs[killed_index].mob.experience, party_player_index, killed_index) * 70) / 100);
			party_player->mob.experience += class_experience;

			struct packet_dead_mob dead_mob;
			dead_mob.header.size = 24;
			dead_mob.header.operation_code = 0x338;
			dead_mob.header.index = 0x7530;
			dead_mob.hold = party_player->mob.hold;
			dead_mob.killer_index = party_player_index;
			dead_mob.killed_index = killed_index;
			dead_mob.experience = party_player->mob.experience;

			add_client_packet((unsigned char*) &dead_mob, xlen(&dead_mob), party_player_index);
			level_up(party_player);
			get_current_score(party_player_index);
			send_score(party_player_index);
			send_etc(party_player_index);
			send_affects(party_player_index);

			send_all_packets(party_player_index);
		}
	}
}

bool
process_chaos_points(int killer_index, int killed_index)
{
	struct mob_st *killer = &g_mobs[killer_index].mob;
	struct mob_st *killed = &g_mobs[killer_index].mob;

	if (killer_index <= 0 || killer_index >= MAX_USERS_PER_CHANNEL)
		return false;
	else if (killed_index <= 0 || killed_index >= MAX_USERS_PER_CHANNEL)
		return false;

	short new_frag = get_total_kills(killer_index) + 1;
	int cape_killer = get_cape_id(killer->equip[CAPE_SLOT].item_id);
	int cape_killed = get_cape_id(killed->equip[CAPE_SLOT].item_id);
	
	if (cape_killer == 3 || cape_killed == 3)
		return false;
	
	if (cape_killer == cape_killed)
		return false;

	set_total_kills(killer_index, new_frag);
	killed->hold += ((experience_mortal_arch[killed->b_status.level] * 20) / 100);
	send_etc(killed_index);

	return true;
}

void
send_env_effect(struct position_st min, struct position_st max, short effect_id, short time)
{
	struct packet_send_effect send_effect;
	send_effect.header.index = 0x7530;
	send_effect.header.operation_code = 0x3A2;
	send_effect.header.size = sizeof(struct packet_send_effect);
	send_effect.min = min;
	send_effect.max = max;
	send_effect.effect_id = effect_id;
	send_effect.time = 0;

	for (size_t i = 1; i <= MAX_USERS_PER_CHANNEL; i++) {
		if (g_users[i].server_data.mode != USER_PLAY)
			continue;

		if ((g_mobs[i].mob.current.X >= min.X && g_mobs[i].mob.current.X <= max.X) && (g_mobs[i].mob.current.Y >= min.Y && g_mobs[i].mob.current.Y <= max.Y)) {
			send_one_packet((unsigned char*) &send_effect, xlen(&send_effect), i);
		}
	}
}

void
send_mob_dead(int killer_index, int killed_index)
{
	if (killed_index <= MAX_USERS_PER_CHANNEL && killer_index <= MAX_USERS_PER_CHANNEL) {
		if (check_pvp_area(killer_index) >= 2 && !g_mobs[killer_index].in_duel) {
			int minus_cp = 1;
			bool frag = process_chaos_points(killer_index, killed_index);
			
			if (frag)
				minus_cp = 4;
			
			short cpoint = (killer_index) - minus_cp;
			if (check_pvp_area(killer_index) == 2) {
				if (frag) {
					set_pk_points(killer_index, cpoint);
					send_client_message("Foi reduzido o CP em -1 e frag aumentado em +1.", killer_index);
				} else {
					set_pk_points(killer_index, cpoint);
					send_client_message("Seu CP foi reduzido em -4.", killer_index);
				}

				get_create_mob(killer_index, killer_index);
				send_grid_mob(killer_index);
			}
		} else if (g_mobs[killer_index].in_duel && g_mobs[killed_index].in_duel) {
			/*SendClientMessage(Killer, "Parabens, voce venceu o Duelo!!!");
			SendClientMessage(Killed, "Que pena, voce perdeu o Duelo!!!");
			DoTeleport(Killer, 2100, 2100);
			DoTeleport(Killed, 2100, 2100);
			MainServer.pMob[Killer].InDuelo = false;
			MainServer.pMob[Killed].InDuelo = false;
			Event.Duelo.DuelStart = false;
			Event.Duelo.Time = -1;*/
		}

		g_mobs[killer_index].mob.last_position = g_mobs[killer_index].mob.current;
		get_action(killer_index, g_mobs[killer_index].mob.current.X, g_mobs[killer_index].mob.current.Y, MOVE_NORMAL, NULL);

		return;
	}

	if (killed_index <= MAX_USERS_PER_CHANNEL)
		return;

	struct mob_server_st *killer = &g_mobs[killer_index];
	struct mob_server_st *killed = &g_mobs[killed_index];
	struct npcgener_st *n = &g_gener_list[killed->generate_index];

	if (killed->mob_type == MOB_TYPE_PESA_MOB || killed->mob_type == MOB_TYPE_PESA_BOSS) { //pesa mob dead
		fprintf(stderr, "MOB PESA MORREU, DEU RUIM\n");
		// n->MinuteGenerate = 1;
		// n->
		// MainServer.ModuleManager.PesaMod.MobDead(Killer, Killed);
		// TODO: IMPLEMENTAR
	} else {
		if (killer->in_party)
			send_party_experience(killer_index, killed_index);
	}

	struct packet_dead_mob dead_mob;
	dead_mob.header.size = sizeof(struct packet_dead_mob);
	dead_mob.header.operation_code = 0x338;
	dead_mob.header.index = 0x7530;
	dead_mob.hold = killer->mob.hold;
	dead_mob.killer_index = killer_index;
	dead_mob.killed_index = killed_index;
	dead_mob.experience = killer->mob.experience;
	send_one_packet((unsigned char *) &dead_mob, xlen(&dead_mob), killer_index);

	time_t now = time(NULL);
	n->death_time[killed->death_id] = now;
	n->current_num_mob--;

	if (killed->mob_type == MOB_TYPE_AGUA) {
		fprintf(stderr, "MOB AGUA MORREU, DEU RUIM\n");
		// TODO: AGUA, IMPLEMENTAR
		// MainServer.ModuleManager.AguaMod.MobDead(Killed, Killer);
	}

	send_remove_mob(killed_index, killed_index, DELETE_DEAD);
	remove_object(killed_index, killed->mob.current, WORLD_MOB);
	mob_drop(killer, killed_index);
	clear_property(killed);
}

void
send_attack(int attacker_index, int target_index)
{
	struct mob_server_st *attacker = &g_mobs[attacker_index];
	struct mob_server_st *target = &g_mobs[target_index];

	if (is_dead(*target))
		return;

	struct packet_attack_single attack_single = { 0 };
	attack_single.header.operation_code = 0x39D;
	attack_single.header.size = sizeof(struct packet_attack_single);
	attack_single.header.index = attacker_index;
	attack_single.attack_count = 1;
	attack_single.attacker_id = attacker_index;
	attack_single.attacker_pos = attacker->mob.current;

	if (attacker->mob.equip[6].item_id != 0) {
		int ef_range = get_item_ability(&attacker->mob.equip[6], EF_RANGE);
		if (ef_range == 2)
			attack_single.skill_index = 153;
		else if (ef_range > 2)
			attack_single.skill_index = 151;
	} else {
		attack_single.skill_index = -1;
	}

	attack_single.target.target_id = target_index;
	attack_single.target_pos = target->mob.current;
	attack_single.double_critical = p39x_NORMAL;

	int damage = get_damage(attacker->mob.status.attack, target->mob.status.defense, 1);
	if (damage <= 0)
		damage = -3;

	attack_single.target.damage = damage;

	if (damage == -3)
		damage = 0;

	if (target_index <= MAX_USERS_PER_CHANNEL) {
		for (size_t i = 0; i < MAX_PARTY; i++) {
			int index = target->baby_mob[i];
			
			if (index <= 0)
				continue;
			
			if (index <= MAX_USERS_PER_CHANNEL || index >= MAX_SPAWN_LIST) {
				target->baby_mob[i] = 0;
				continue;
			}

			struct mob_server_st *mob = &g_mobs[index];
			if (!is_summon(*mob)) {
				target->baby_mob[i] = 0;
				continue;
			}
			
			add_enemy_list(mob, attacker_index);
		}
	}

	bool CM = false;
	for (size_t i = 0; i < MAX_AFFECT; i++) {
		if (target->mob.affect[i].index == FM_CONTROLE_MANA) {
			CM = true;
			break;
		}
	}

	attack_single.current_mp = target->mob.status.current_mp;
	attack_single.current_exp = target->mob.experience;
	attack_single.motion = 5;

CM_CONT:
	if (CM) {
		int hp = (target->mob.status.current_mp - damage);
		short posX = target->mob.current.X;
		short posY = target->mob.current.Y;
		if (hp <= 0) {
			CM = false;
			for (size_t i = 0; i < MAX_AFFECT; i++) {
				if (target->mob.affect[i].index == FM_CONTROLE_MANA) {
					target->mob.affect[i].index = 0;
					target->mob.affect[i].time = 0;
					target->mob.affect[i].master = 0;
					target->mob.affect[i].value = 0;
					target->buffer_time[i] = 0;
				}
			}
			damage -= target->mob.status.current_mp;
			target->mob.status.current_mp = 0;
			goto CM_CONT;
		} else {
			target->mob.status.current_mp = target->mob.status.current_mp - damage;
			
			send_grid_multicast(posX, posY, (unsigned char*) &attack_single, 0);
			get_current_score(target_index);
			send_score(target_index);
			
			if (target_index < MAX_USERS_PER_CHANNEL) {
				send_etc(target_index);
				send_affects(target_index);
				send_all_packets(target_index);
			}
		}
	} else {
		int hp = (target->mob.status.current_hp - damage);
		short posX = target->mob.current.X;
		short posY = target->mob.current.Y;
		
		send_grid_multicast(posX, posY, (unsigned char*) &attack_single, 0);
		
		if (hp <= 0) {
			if (target_index <= MAX_USERS_PER_CHANNEL) {
				attacker->current_target = 0;
				target->mob.status.current_hp = 0;
				remove_enemy_list(attacker, target_index);
				get_create_mob(target_index, target_index);
				send_grid_mob(target_index);
				return;
			} else {
				if (is_summon(*target)) {
					if (target->summoner > 0 && target->summoner <= MAX_USERS_PER_CHANNEL) {
						struct mob_server_st *summoner = &g_mobs[target->summoner];
						for (int x = 0; x < MAX_PARTY; x++) {
							if (summoner->baby_mob[x] == target_index) {
								summoner->baby_mob[x] = 0;
								break;
							}
						}
					}
					remove_object(target_index, target->mob.current, WORLD_MOB);
					clear_property(target);
					target->mode = MOB_EMPTY;
				} else if (target->mob_type == MOB_TYPE_PESA_NPC) {
					printf("MOB PESA, NÃO IMPLEMENTADO.\n");
					// int PesaID = -1;
					// if (Def->GenerateIndex >= 270 && Def->GenerateIndex <= 274) PesaID = PesaA;
					// else if (Def->GenerateIndex >= 285 && Def->GenerateIndex <= 291) PesaID = PesaM;
					// else if (Def->GenerateIndex >= 303 && Def->GenerateIndex <= 309) PesaID = PesaA;
					// STRUCT_PESADELO *p = &MainServer.ModuleManager.PesaMod.Pesadelo;
					// STRUCT_NPCGENER *n = &MainServer.NPCGener.GenerList[Def->GenerateIndex];
					// n->CurrentNumMob--;
					// p->NpcsVivos[PesaID]--;
					// MainServer.World.RemoveObject(Defender, Def->MOB.Current.X, Def->MOB.Current.Y, WORLD_MOB);
					// SendRemoveMob(Defender, Defender, DELETE_UNSPAWN);
					// Def->ClearProperty();
					// SEND::SendNoticeArea("Npc Morreu!", p->AreaMin[PesaID].X, p->AreaMin[PesaID].Y, p->AreaMax[PesaID].X, p->AreaMax[PesaID].Y);
					// for (int y = 1; y <= MAX_USER; y++) {
					// 	if (MainServer.pUser[y].Mode != USER_PLAY) continue;
					// 	CMob *a = &MainServer.pMob[y];
					// 	if (a->IsDead()) continue;
					// 	if ((a->MOB.Current.X >= p->AreaMin[PesaID].X && a->MOB.Current.X <= p->AreaMax[PesaID].X) && (a->MOB.Current.Y >= p->AreaMin[PesaID].Y && a->MOB.Current.Y <= p->AreaMax[PesaID].Y)) {
					// 		MSG_3BBh packet;
					// 		memset(&packet, 0, sizeof(MSG_3BBh));
					// 		packet.header.Size = sizeof(MSG_3BBh);
					// 		packet.header.Code = 0x3BB;
					// 		packet.header.Index = 0x7530;
					// 		packet.cur_mob = p->NpcsVivos[PesaID];
					// 		if (PesaID == PesaN)
					// 			packet.max_mob = 5;
					// 		else
					// 			packet.max_mob = 7;
					// 		SEND_CLIENT(y, &packet);
					// 	}
					// }
				} else if (is_summon(*attacker) && !is_summon(*target)) {
					target->mob.status.current_hp = 0;
					if (attacker->summoner < 0 || attacker->summoner > MAX_USERS_PER_CHANNEL)
						return;

					if (g_users[attacker->summoner].server_data.mode != USER_PLAY)
						return;

					g_mobs[attacker->summoner].mob.experience += get_exp_by_kill(target->mob.experience, attacker->summoner, target_index);

					send_mob_dead(attacker->summoner, target_index);
					send_affects(attacker->summoner);
					level_up(&g_mobs[attacker->summoner]);
					send_all_packets(attacker->summoner);
				}
			}
		} else {
			target->mob.status.current_hp = (target->mob.status.current_hp - damage);
			get_current_score(target_index);
			send_score(target_index);
			if (target_index < MAX_USERS_PER_CHANNEL) {
				send_etc(target_index);
				send_affects(target_index);
				send_all_packets(target_index);
			}
		}
	}
	
	get_current_score(attacker_index);
	send_score(attacker_index);

	if (attacker_index <= MAX_USERS_PER_CHANNEL) {
		send_etc(attacker_index);
		send_affects(attacker_index);
		send_all_packets(attacker_index);
	}
}

void
send_create_item(int user_index, short inventory_type, short inventory_slot, struct item_st *item)
{
	struct packet_create_item create_item;
	create_item.header.size = sizeof(struct packet_create_item);
	create_item.header.operation_code = 0x182;
	create_item.header.index = user_index;

	create_item.inventory_type = inventory_type;
	create_item.inventory_slot= inventory_slot;

	if (item == NULL)
		memset(&create_item.item, 0, sizeof(struct item_st));
	else
		create_item.item = *item;

	send_one_packet((unsigned char *) &create_item, xlen(&create_item), user_index);
}