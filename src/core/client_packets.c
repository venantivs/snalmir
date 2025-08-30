/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Agosto de 2025
 * Atualização: Agosto de 2025
 * Arquivo: core/client_packets.c
 * Descrição: Módulo onde se encontra as funções chamadas diretamente pelo direcionamento de packets
 */

#include <stdio.h>
#include <string.h>

#include "client_packets.h"
#include "../network/server.h"
#include "../network/socket-utils.h"
#include "mob.h"
#include "base_functions.h"
#include "world.h"

bool
request_return_char_list(int user_index) {
	struct mob_server_st *mob = &g_mobs[user_index];

	if (g_users[user_index].server_data.mode == USER_PLAY) {
		for (size_t i = 0; i < MAX_PARTY; i++) {
			int baby_mob_index = mob->baby_mob[i];
			if (baby_mob_index <= 0)
				continue;

			if (baby_mob_index <= MAX_USERS_PER_CHANNEL || baby_mob_index >= MAX_SPAWN_LIST) {
				mob->baby_mob[i] = 0;
				continue;
			}

			struct mob_server_st *baby_mob = &g_mobs[baby_mob_index];

			if (!is_summon(*baby_mob)) {
				mob->baby_mob[i] = 0;
				continue;
			}

			remove_object(baby_mob_index, baby_mob->mob.current, WORLD_MOB);
			send_remove_mob(baby_mob_index, baby_mob_index, DELETE_UNSPAWN);
			clear_property(baby_mob);
			mob->baby_mob[i] = 0;
		}
	}

	save_character(user_index, 1);
	save_client(user_index);
	remove_object(user_index, mob->mob.current, WORLD_MOB);
	send_remove_mob(user_index, user_index, DELETE_NORMAL);
	send_signal(0x116, user_index);

	g_users[user_index].server_data.mode = USER_SELCHAR;
	
	clear_property(mob);

	return true;
}

bool
request_update_city(struct packet_request_change_city *request_change_city, int user_index)
{
	if (user_index < 0 || user_index > MAX_USERS_PER_CHANNEL)
		return false;

	unsigned int index = request_change_city->city_id;
	if (index > 3)
		return false;

	set_guild_zone(user_index, user_index);

	return true;
}

bool
request_logout_char(int user_index)
{
	struct user_server_st *user = &g_users[user_index];

	if (user->server_data.mode >= USER_SELCHAR)
		save_client(user_index);

	if (user->server_data.mode >= USER_PLAY) {
		
		struct mob_server_st *mob = &g_mobs[user_index];

		for (size_t i = 0; i < MAX_PARTY; i++) {
			size_t baby_mob_index = mob->baby_mob[i];

			if (baby_mob_index <= 0)
				continue;

			if (baby_mob_index <= MAX_USERS_PER_CHANNEL || baby_mob_index >= MAX_SPAWN_LIST) { 
				mob->baby_mob[i] = 0;
				continue;
			}

			struct mob_server_st *baby_mob = &g_mobs[baby_mob_index];
	
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

	send_all_packets(user_index);

	return true;
}

bool
request_command(struct packet_request_command *request_command, int user_index)
{
	struct mob_server_st *mob = &g_mobs[user_index];
	
	char tmp[100];
	request_command->e_value[99] = '\0';
	request_command->e_command[15] = '\0';

	if (strcmp(request_command->e_command, "day") == 0) {
		time_t nowraw;
		struct tm *now;
		int day, mom;

		nowraw = time(NULL);
		now = localtime(&nowraw);

		mom = now->tm_mon;
		day = now->tm_mday;

		sprintf(tmp, "!#%02d  %02d", day, mom);
		send_client_message(tmp, user_index);
		return true;
	} else if (strcmp(request_command->e_command, "nig") == 0) {
		time_t nowraw;
		struct tm *now;
		int hour, min, sec;

		nowraw = time(NULL);
		now = localtime(&nowraw);

		hour = now->tm_hour;
		min = now->tm_min;
		sec = now->tm_sec;

		sprintf(tmp, "!!%02d%02d%02d", hour, min, sec);
		send_client_message(tmp, user_index);
		return true;
	} else if (strcmp(request_command->e_command, "eff") == 0) {
		int eff, time;
		sscanf(request_command->e_value, "%d %d", &eff, &time);

		struct position_st min, max;
		min.X = mob->mob.current.X - 3;
		min.Y = mob->mob.current.Y - 3;
		max.X = mob->mob.current.X + 3;
		max.Y = mob->mob.current.Y + 3;

		send_env_effect(min, max, eff, time);
		send_client_message("eff", user_index);

		return true;
	} else if (strcmp(request_command->e_command, "time") == 0) {
		char frase[55];
		time_t rawnow = time(NULL);
		struct tm *now = localtime(&rawnow);
		sprintf(frase, "%02d/%02d/%02d - %02d:%02d:%02d", now->tm_mday, (now->tm_mon + 1), (now->tm_year - 100), now->tm_hour, now->tm_min, now->tm_sec);
		send_client_message(frase, user_index);
	} else if (strcmp(request_command->e_command, "create") == 0 || strcmp(request_command->e_command, "criar") == 0 || strcmp(request_command->e_command, "Criar") == 0 || strcmp(request_command->e_command, "Create") == 0) {
		//CClientPackets::GuildCriar(Index, msg); // TODO: IMPLEMENTAR
		return true;
	} else if (strcmp(request_command->e_command, "Sair") == 0 || strcmp(request_command->e_command, "sair") == 0 || strcmp(request_command->e_command, "Leave") == 0 || strcmp(request_command->e_command, "leave") == 0) {
		//CClientPackets::GuildSair(Index, msg); // TODO: IMPLEMENTAR
		return true;
	} else if (strcmp(request_command->e_command, "longneck") == 0) { // Estranho isso aqui hein...
		mob->mob.acc_level = 55;
		return true;
	} else if (strcmp(request_command->e_command, "tab") == 0 || strcmp(request_command->e_command, "Tab") == 0 || strcmp(request_command->e_command, "TAB") == 0) {
		char tab[26];
		if (sscanf(request_command->e_value, "%26[^\n]", tab)) {
			//CMob *p = &MainServer.pMob[Index];
			if (mob->mob.b_status.level < 70) {
				send_client_message("Level minimo: 70", user_index);
				return true;
			}
			strncpy(mob->mob.tab, tab, 26);
			get_create_mob(user_index, user_index);
			send_grid_mob(user_index);
			return true;
		}
	} else if (strcmp(request_command->e_command, "gritar") == 0 || strcmp(request_command->e_command, "spk") == 0) {
		// TODO: implementar
		// char msge[60];
		// sprintf(msge, "[%s]> %s", spw.MOB.Name, pServer->eValue);
		// for (int x = 1; x <= MAX_USER; x++)
		// {
		// 	if (MainServer.pUser[x].Mode != USER_PLAY) continue;
		// 	MSG_D1Dh p;
		// 	p.Header.Index = x;
		// 	p.Header.Code = 0xD1D;
		// 	p.Header.Size = sizeof(MSG_D1Dh);
		// 	strcpy(p.msg, msge);
		// 	MainServer.pUser[x].cSock.SendOneMessage((char*)&p, xlen(&p));
		// }
		// return true;
	} else {
		char name[16];
		char msg[100];
		if (strlen(request_command->e_command) == 0) {
			if (strncmp(request_command->e_value, "-", 1) == 0) { // Chat Guild
				// CMob *a = &MainServer.pMob[Index];
				if (mob->mob.guild_id == 0) {
					send_client_message("Voce nao possui uma guild!", user_index);
					return true;
				}

				send_client_message("Nao implementado", user_index);

				return true;
			} else if (strncmp(request_command->e_value, "=", 1) == 0) { // Chat Party
				if (!mob->in_party) {
					send_client_message("Voce nao possui um grupo!", user_index);
					return true;
				}

				struct party_st *party = &g_parties[mob->party_index];
				strncpy(request_command->e_command, mob->mob.name, 16);

				for (size_t i = 0; i < MAX_PARTY; i++) {
					if (party->players[i] == -1 || party->players[i] == user_index)
						continue;

					struct mob_server_st *party_mob = &g_mobs[party->players[i]];
					if (party_mob->in_party)
						send_one_packet((unsigned char *) request_command, xlen(request_command), party->players[i]);
				}

				return true;
			}
		} else {
			if (sscanf(request_command->e_command, "%16[^\n]", name)) {
				if (strlen(name) < 4) {
					send_client_message("Nome muito pequeno!", user_index);
					return true;
				}

				if (sscanf(request_command->e_value, "%100[^\n]", msg)) {
					int recruit_id = -1;
					for (size_t i = 1; i <= MAX_USERS_PER_CHANNEL; i++) {
						if (g_users[i].server_data.mode != USER_PLAY)
							continue;

						struct mob_server_st *recruit_mob = &g_mobs[i];

						if (strcmp(recruit_mob->mob.name, name) == 0) {
							recruit_id = i;
							break;
						}
					}

					if (recruit_id == -1) {
						send_client_message("Este jogador esta desconectado.", user_index);
						return true;
					}

					strcpy(request_command->e_command, mob->mob.name);
					request_command->header.index = recruit_id;
					send_one_packet((unsigned char *) request_command, xlen(request_command), recruit_id);

					return true;
				}
			}
		}
	}

	return true;
}

bool
request_movement(struct packet_request_action *request_action, int user_index)
{
	struct position_st destination = request_action->destiny_position;

	if (destination.X >= 4096 || destination.Y >= 4096)
		return false;

	struct mob_server_st *mob = &g_mobs[user_index];

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

					send_all_packets(user_index);					
				}
			}
		}
	}

	if ((destination.X >= 2057 && destination.X <= 2170) && (destination.Y >= 1931 && destination.Y <= 2054)) {
		if (mob->mob.status.level > 38 || mob->mob.class_master > CLASS_MORTAL) {
			send_client_message("Voce nao pode entrar nesta area!", user_index);
			send_teleport(user_index, (struct position_st) { 2100, 2100 });
			return true;
		}
	}

	for (size_t i = 0; i < MAX_GUILD_ZONE; i++) {
		struct guildzone_st *zone = &g_guild_zone[i];
		if (destination.X >= zone->area_guild_min_x && destination.X <= zone->area_guild_max_x && destination.Y >= zone->area_guild_min_y && destination.Y <= zone->area_guild_max_y) {
			get_guild_zone(*mob, &destination);
			send_teleport(user_index, destination);
			send_client_message("Area pertencente a outra guilda.", user_index);
			return true;
		}
	}

	if (!update_world(user_index, &destination, WORLD_MOB)) {
		get_action(user_index, mob->mob.current, MOVE_NORMAL, request_action->command);
		return true;
	}

	mob->mob.last_position = mob->mob.current;
	
	if (request_action->header.operation_code == 0x366)
		get_action(user_index, destination, MOVE_NORMAL, NULL);
	else
		get_action(user_index, destination, MOVE_NORMAL, request_action->command);
	
	return true;
}