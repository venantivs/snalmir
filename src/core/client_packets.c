/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Agosto de 2025
 * Atualização: Agosto de 2025
 * Arquivo: core/client_packets.c
 * Descrição: Módulo onde se encontra as funções chamadas diretamente pelo direcionamento de packets
 */

#include "client_packets.h"
#include "../network/server.h"
#include "../network/socket-utils.h"
#include "mob.h"
#include "base_functions.h"
#include "world.h"

bool
request_logout_char(int user_index)
{
	struct user_server_st *user = &users[user_index];

	if (user->server_data.mode >= USER_SELCHAR)
		save_client(user_index);

	if (user->server_data.mode >= USER_PLAY) {
		
		struct mob_server_st *mob = &mobs[user_index];

		for (size_t i = 0; i < MAX_PARTY; i++) {
			size_t baby_mob_index = mob->baby_mob[i];

			if (baby_mob_index <= 0)
				continue;

			if (baby_mob_index <= MAX_USERS_PER_CHANNEL || baby_mob_index >= MAX_SPAWN_LIST) { 
				mob->baby_mob[i] = 0;
				continue;
			}

			struct mob_server_st *baby_mob = &mobs[baby_mob_index];
	
			if (!is_summon(*baby_mob)){
				mob->baby_mob[i] = 0;
				continue;
			}

			struct position_st baby_mob_current_pos = baby_mob->mob.current;

			remove_object(baby_mob_index, baby_mob_current_pos, WORLD_MOB);
			send_remove_mob(baby_mob_index, baby_mob_index, DELETE_UNSPAWN);
			clear_property(baby_mob);

			mob->baby_mob[i] = 0;
		}

		save_character(user_index, 1);
		remove_object(user_index, mob->mob.current, WORLD_MOB);
		send_remove_mob(user_index, user_index, DELETE_NORMAL);
		clear_property(mob);
	}

	send_all_messages(user_index);

	return true;
}

bool
request_return_char_list(int user_index) {
	return true;
}

bool
request_movement(struct packet_request_action *request_action, int user_index)
{
	struct position_st destination = request_action->destiny_position;

	if (destination.X >= 4096 || destination.Y >= 4096)
		return false;

	struct mob_server_st *mob = &mobs[user_index];

	mob->mob.dest = destination;
	
	if (request_action->type != MOVE_NORMAL)
		return false;
	
	if (mob->mob.current.X == destination.X && mob->mob.current.Y == destination.Y)
		return true;
	
	if (get_distance(mob->mob.current, destination) > 30)
		return true;

	if (user_index < MAX_USERS_PER_CHANNEL) {
		if (mob->mob.class_info == 3) {
			for (size_t i = 0; i < MAX_AFFECT; i++) {
				if (mob->mob.affect[i].index == HT_INIVISIBILIDADE) {
					mob->mob.affect[i].index = 0;
					mob->mob.affect[i].time = 0;
					mob->mob.affect[i].master = 0;
					mob->mob.affect[i].value = 0;
					mob->buffer_time[i] = 0;

					get_current_score(user_index);
					send_etc(user_index);					
					send_score(user_index);
					send_affects(user_index);

					send_all_messages(user_index);					
				}
			}
		}
	}

	if ((destination.X >= 2057 && destination.X <= 2170) && (destination.Y >= 1931 && destination.Y <= 2054)) {
		if (mob->mob.status.level > 38 || mob->mob.class_master > CLASS_MORTAL) {
			send_client_string_message("Voce nao pode entrar nesta area!", user_index);
			send_teleport(user_index, (struct position_st) { 2100, 2100 });
			return true;
		}
	}

	for (size_t i = 0; i < MAX_GUILD_ZONE; i++) {
		struct guildzone_st *zone = &guild_zone[i];
		if (destination.X >= zone->area_guild_min_x && destination.X <= zone->area_guild_max_x && destination.Y >= zone->area_guild_min_y && destination.Y <= zone->area_guild_max_y) {
			get_guild_zone(*mob, &destination.X, &destination.Y);
			send_teleport(user_index, destination);
			send_client_string_message("Area pertencente a outra guilda.", user_index);
			return true;
		}
	}

	if (!update_world(user_index, &destination.X, &destination.Y, WORLD_MOB)) {
		get_action(user_index, mob->mob.current.X, mob->mob.current.Y, MOVE_NORMAL, request_action->command);
		return true;
	}

	mob->mob.last_position = mob->mob.current;
	
	if (request_action->header.operation_code == 0x366)
		get_action(user_index, destination.X, destination.Y, MOVE_NORMAL, NULL);
	else
		get_action(user_index, destination.X, destination.Y, MOVE_NORMAL, request_action->command);
	
	return true;
}