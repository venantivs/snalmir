/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Agosto de 2025
 * Atualização: Agosto de 2025
 * Arquivo: core/client_packets.c
 * Descrição: Módulo onde se encontra as funções chamadas diretamente pelo direcionamento de packets
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_packets.h"
#include "../network/server.h"
#include "../network/socket-utils.h"
#include "mob.h"
#include "base_functions.h"
#include "world.h"
#include "item_effect.h"
#include "game_skills.h"
#include "game_items.h"
#include "utils.h"
#include "npc.h"

bool
request_add_points(struct packet_request_add_points *request_add_points, int user_index)
{
	int max;
	short *info;
	unsigned char *master;
	struct mob_st *user = &g_mobs[user_index].mob;

	switch (request_add_points->mode) {
	case 0:
		if (user->p_status <= 0)
			break;

		info = (short*) ((char*) &user->b_status.strength + (sizeof(short) * request_add_points->info));
		if (*info >= 32000) {
			send_client_message("Maximo de pontos e 32.000", user_index);
			break;
		}

		if (user->p_status >= 200) {
			(*info) += 100;
			user->p_status -= 100;
		} else {
			(*info)++;
			user->p_status--;
		}

		get_current_score(user_index);
		send_etc(user_index);
		send_score(user_index);
		send_all_packets(user_index);

		break;
	case 1:
		if (user->p_master <= 0)
			break;

		if (user->class_master <= CLASS_ARCH) {
			if ((user->learn & (128 << (request_add_points->info * 8))) == 0) {
				max = (((user->b_status.level + 1) * 3) >> 1);
				if (max > 200)
					max = 200;
			} else {
				max = 255;
			}
		} else if (user->class_master >= CLASS_CELESTIAL) {
			max = 255;
		}

		master = (unsigned char*) ((char*) &user->b_status.w_master + request_add_points->info);
		if (*master >= max) {
			send_client_message("Maximo de pontos neste atributo.", user_index);
			break;
		}

		(*master)++;
		user->p_master--;
		
		get_current_score(user_index);
		send_etc(user_index);
		send_score(user_index);
		send_all_packets(user_index);

		break;
	case 2:
		if (request_add_points->info < 5000 || request_add_points->info > 5095)
			break;

		struct item_list_st item = g_item_list[request_add_points->info];
		struct skill_data_st spell = g_skill_data[request_add_points->info - 5000];

		int skill_index = (request_add_points->info - 5000) % 24;
		int skill_divisor = skill_index / 8;
		short reqmaster = spell.points;
		unsigned char master = *(unsigned char*)(&user->status.f_master + skill_divisor);

		bool success = false;
		int required_class = get_effect_value(request_add_points->info, EF_CLASS);
		int class_info = user->class_info;
		
		if (((required_class >> class_info) & 1) == 0) {
			send_client_message("Nao pode aprender skills desta classe!", user_index);
			return true;
		}

		if (user->class_master >= CLASS_ARCH){
			if (master < reqmaster)
				master = reqmaster;
		}

		if ((user->learn & (1 << skill_index)) == 0) {
			if (master >= reqmaster) {
				if (user->p_skill >= spell.points || user->class_master >= CLASS_CELESTIAL) {
					if (user->gold >= item.price) {
						if (user->status.level >= item.level || user->class_master >= CLASS_CELESTIAL) {
							success = true;
						} else
							send_client_message("Nao ha Level suficiente para adquirir a Skill.", user_index);
					} else
						send_client_message("Nao ha dinheiro suficiente para adquirir a Skill.", user_index);
				} else
					send_client_message("Nao ha Pontos de Skill suficientes.", user_index);
			} else
				send_client_message("Nao ha Pontos Masters suficientes.", user_index);
		} else
			send_client_message("Voce ja aprendeu essa skill.", user_index);

		if (success) {
			user->learn |= 1 << skill_index;
			user->p_skill -= spell.points;

			get_current_score(user_index);
			send_etc(user_index);
			send_score(user_index);
			send_all_packets(user_index);
		}

		break;
	}

	return true;
}

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
request_process_npc(struct packet_request_npc *request_npc, int user_index)
{
	struct mob_server_st *user = &g_mobs[user_index];
	struct mob_server_st *npc_mob = &g_mobs[request_npc->npc_index];

	if (request_npc->npc_index < BASE_MOB || request_npc->npc_index >= MAX_SPAWN_LIST)
		return true;

	double range = get_distance(user->mob.current, npc_mob->mob.current);

	if (range > 25)
		return true;

	int grade = get_item_ability(&npc_mob->mob.equip[0], EF_GRADE0);

	if (request_npc->click_ok == 1) {
		fprintf(stderr, "T=1 NPC NÃO IMPLEMENTADO: %s | merch: %d | grade: %d.\n", npc_mob->mob.name, npc_mob->mob.info.merchant, grade);
		return true;
	} else {
		switch(npc_mob->mob.info.merchant) {
		case 74:
			return true;
		case 12:
			return true;
		case 14:
			return true;
		case 36:
			return true;
		case 40:
			return true;
		case 41:
			return true;
		case 58:
			return true;
		case 4:
			return true;
		default:
			fprintf(stderr, "T=2 NPC NÃO IMPLEMENTADO: %s | merch: %d | grade: %d.\n", npc_mob->mob.name, npc_mob->mob.info.merchant, grade);
			return true;
		}
	}
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
		// sprintf(msge, "[%s]> %s", spw.MOB.Name, request_move_item->eValue);
		// for (int x = 1; x <= MAX_USERS_PER_CHANNEL; x++)
		// {
		// 	if (MainServer.pUser[x].Mode != USER_PLAY) continue;
		// 	MSG_D1Dh p;
		// 	p.header.Index = x;
		// 	p.header.Code = 0xD1D;
		// 	p.header.Size = sizeof(MSG_D1Dh);
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
request_move_item(struct packet_request_move_item *request_move_item, int user_index)
{
	struct mob_st *user = &g_mobs[user_index].mob;

	if (request_move_item->source_type == EQUIP_TYPE && request_move_item->source_slot == 12)
		return true;

	if ((request_move_item->source_slot > 62 && request_move_item->source_type == INV_TYPE))
		return true;

	if (request_move_item->destination_slot > 62 && request_move_item->destination_type == INV_TYPE)
		return true;

	if ((request_move_item->source_slot >= 127 && request_move_item->source_type == STORAGE_TYPE))
		return true;

	if ((request_move_item->destination_slot >= 127 && request_move_item->destination_type == STORAGE_TYPE))
		return true;

	if ((request_move_item->source_slot > 15 && request_move_item->source_type == EQUIP_TYPE))
		return true;

	if ((request_move_item->destination_slot > 15 && request_move_item->destination_type == EQUIP_TYPE))
		return true;

	if (request_move_item->source_slot < 0 || request_move_item->destination_slot < 0)
		return true;

	if (user->class_master > CLASS_MORTAL && request_move_item->destination_slot == HELM_SLOT && request_move_item->destination_type == EQUIP_TYPE) {
		send_client_message("Voce nao pode equipar isso !", user_index);
		return true;
	}

	if (user->class_master > CLASS_ARCH && request_move_item->source_slot == HELM_SLOT && request_move_item->source_type == EQUIP_TYPE) {
		if (user->status.level < 199 && user->class_master >= CLASS_ARCH)
			send_client_message("Voce nao pode retirar!", user_index);

		return true;
	}

	struct item_st *source_item_ptr = get_item_pointer(user_index, request_move_item->source_type, request_move_item->source_slot);
	struct item_st *destination_item_ptr = get_item_pointer(user_index, request_move_item->destination_type, request_move_item->destination_slot);

	if (source_item_ptr == NULL || destination_item_ptr == NULL)
		return false;

	if (source_item_ptr == destination_item_ptr)
		return false;

	bool destination_success = (destination_item_ptr->item_id == 0);
	bool source_success = false;

	struct item_st source_item, destination_item;
	memcpy(&source_item, source_item_ptr, sizeof(struct item_st));
	memcpy(&destination_item, destination_item_ptr, sizeof(struct item_st));

	if (destination_item.item_id == 413 && source_item.item_id == 413 || destination_item.item_id == 412 && source_item.item_id == 412) {
		int outs = 0;
		if (destination_item_ptr->effect[1].value + source_item_ptr->effect[1].value >= 100) {
			send_create_item(user_index, request_move_item->source_type, request_move_item->source_slot, source_item_ptr);
			return true;
		} else if (destination_item_ptr->effect[1].index != 61) {
			destination_item_ptr->effect[1].index = 61;
			if (source_item_ptr->effect[1].index == 61) {
				destination_item_ptr->effect[1].value += source_item_ptr->effect[1].value;
				source_item_ptr->item_id = 0;
				destination_item_ptr->effect[1].value += 1;
			} else {
				source_item_ptr->item_id = 0;
				destination_item_ptr->effect[1].value += 2;
			}
		} else if (destination_item_ptr->effect[1].index == 61){
			if (source_item_ptr->effect[1].index == 61) {
				source_item_ptr->item_id = 0;
				destination_item_ptr->effect[1].value += source_item_ptr->effect[1].value;
			} else {
				source_item_ptr->item_id = 0;
				destination_item_ptr->effect[1].value += 1;
			}
		} else {
			source_item_ptr->item_id = 0;
			destination_item_ptr->effect[1].value += 2;
		}

		send_create_item(user_index, request_move_item->destination_type, request_move_item->destination_slot, destination_item_ptr);
		send_create_item(user_index, request_move_item->source_type, request_move_item->source_slot, source_item_ptr);

		return true;
	}

	source_item_ptr->item_id = 0;
	destination_item_ptr->item_id = 0;

	if (destination_item.item_id != 0) {
		if (request_move_item->source_type == EQUIP_TYPE) {
			destination_success = can_equip(user->class_master,
				&destination_item,
				&user->status,
				request_move_item->source_slot,
				user->class_info,
				user->equip);

		} else if (request_move_item->source_type == INV_TYPE) {
			int slot_error = -1;

			destination_success = can_carry(
				&destination_item,
				user->inventory,
				(request_move_item->source_slot),
				&slot_error);

			if (slot_error != -1) {
				slot_error--;
				send_create_item(user_index, INV_TYPE, slot_error, &user->inventory[slot_error]);
			}
		} else if (request_move_item->source_type == STORAGE_TYPE) {
			int slot_error = -1;
			struct item_st *storage = g_users[user_index].storage;

			destination_success = can_cargo(
				&destination_item,
				storage,
				(request_move_item->source_slot),
				&slot_error);

			if (slot_error != -1) {
				slot_error--;
				send_create_item(user_index, STORAGE_TYPE, slot_error, &storage[slot_error]);
			}
		}
	}

	if (destination_success && source_item.item_id != 0) {
		if (request_move_item->destination_type == EQUIP_TYPE) {
			source_success = can_equip(user->class_master,
				&source_item,
				&user->status,
				request_move_item->destination_slot,
				user->class_info,
				user->equip);

		} else if (request_move_item->destination_type == INV_TYPE) {
			int slot_error = -1;

			source_success = can_carry(
				&source_item,
				user->inventory,
				(request_move_item->destination_slot),
				&slot_error);

			if (slot_error != -1) {
				slot_error--;
				send_create_item(user_index, INV_TYPE, slot_error, &user->inventory[slot_error]);
			}
		} else if (request_move_item->destination_type == STORAGE_TYPE) {
			int slot_error = -1;
			struct item_st *storage = g_users[user_index].storage;

			source_success = can_cargo(
				&source_item,
				storage,
				(request_move_item->destination_slot),
				&slot_error);

			if (slot_error != -1) {
				slot_error--;
				send_create_item(user_index, STORAGE_TYPE, slot_error, &storage[slot_error]);
			}
		}
	}

	if (destination_success && source_success) {
		memcpy(source_item_ptr, &destination_item, sizeof(struct item_st));
		memcpy(destination_item_ptr, &source_item, sizeof(struct item_st));

		struct packet_request_move_item move_item;
		move_item.header.size = sizeof(struct packet_request_move_item);
		move_item.header.operation_code = 0x376;
		move_item.header.index = user_index;
		move_item.destination_slot = request_move_item->destination_slot;
		move_item.destination_type = request_move_item->destination_type;
		move_item.source_slot = request_move_item->source_slot;
		move_item.source_type = request_move_item->source_type;

		add_client_packet((unsigned char *) &move_item, xlen(&move_item), user_index);

		if (request_move_item->destination_type == EQUIP_TYPE || request_move_item->source_type == EQUIP_TYPE) {
			// Movimenta a arma do slot 2 para o slot da arma 1
			if ((request_move_item->destination_slot == 6 || request_move_item->source_slot == 6) && user->equip[6].item_id == 0 && user->equip[7].item_id != 0) {
				int unique_item = get_item_ability(&user->equip[7], EF_POS);
				if (((unique_item >> 6) & 1) != 0) {
					memcpy(&user->equip[6], &user->equip[7], sizeof(struct item_st));
					memset(&user->equip[7], 0, sizeof(struct item_st));
					move_item.destination_slot = 6;
					move_item.destination_type = EQUIP_TYPE;
					move_item.source_slot = 7;
					move_item.source_type = EQUIP_TYPE;

					add_client_packet((unsigned char *) &move_item, xlen(&move_item), user_index);
				}
			}
			/*else if(request_move_item->destination_slot == MOUNT_SLOT || request_move_item->source_slot == MOUNT_SLOT)
			{
			int itemID = user->Equip[MOUNT_SLOT].item_id;

			if(itemID >= 2330 && itemID <= 2359)
			{
			int mountIndex = itemID - 2330 + 8;
			//Spawn.UngenerateBabyMob(Index, DELETE_UNSPAWN);
			//Spawn.GenerateBabyMob(Index, mountIndex);
			}
			}*/
			send_refresh_equip_items(user_index, user_index);
		}

		get_current_score(user_index);
		send_score(user_index);
		send_etc(user_index);
		send_all_packets(user_index);
	} else {
		memcpy(destination_item_ptr, &destination_item, sizeof(struct item_st));
		memcpy(source_item_ptr, &source_item, sizeof(struct item_st));
	}

	return true;
}

bool
request_attack(struct packet_attack_area *attack_area, int user_index)
{
	if (attack_area->header.operation_code == 0x39D && attack_area->header.size == 96)
		attack_area->header.size = 48;

	attack_area->header.index = 0x7530;

	struct mob_server_st *mob = &g_mobs[user_index];

	if (g_users[user_index].server_data.mode != USER_PLAY) {
		send_hp_mode(user_index);
		return false;
	}

	if (g_mobs[user_index].mob.status.current_hp <= 0 || g_users[user_index].server_data.mode != USER_PLAY) {
		send_hp_mode(user_index);
		return false;
	}

	short unknow[13];
	memset(unknow, 0x0, 13 * 2);

	unsigned int timestamp = attack_area->header.time;
	if (timestamp != 0x0E0A1ACA && g_users[user_index].last_attack_tick != 0x0E0A1ACA) {
		if (timestamp < g_users[user_index].last_attack_tick + 900) {
			g_users[user_index].last_attack_tick = timestamp;
			g_users[user_index].last_attack = attack_area->skill_index;
		}
	}

	if (timestamp != 0x0E0A1ACA)
		g_users[user_index].last_attack_tick = timestamp;

	int skill_id = attack_area->skill_index;

	struct position_st target_position = { attack_area->target_pos.X, attack_area->target_pos.Y };

	if (skill_id != 42) {
		get_hit_position(attack_area->attacker_pos, &target_position);
		if (target_position.X != attack_area->target_pos.X || target_position.Y != attack_area->target_pos.Y)
			return true;
	}

	if (skill_id < -1)
		return false;

	if (skill_id >= 0 && skill_id < MAX_SKILL_DATA) {
		if (g_skill_data[skill_id].passive_check > 0)
			return true;
			
		int skill_tick = g_users[user_index].last_skill_tick[skill_id];
		if (skill_tick != -1 && timestamp != 0x0E0A1ACA) {
			unsigned int result_timestamp = timestamp - skill_tick;
			int skill_delay = g_skill_data[skill_id].delay;

			skill_delay = skill_delay * 1000;
			if (skill_delay <= 0)
				skill_delay = 1100;

			if (result_timestamp < skill_delay - 100)
				return true;
		}

		g_users[user_index].last_skill_tick[skill_tick] = attack_area->header.time;
	}

	unsigned char special = false;
	int skill_delay = 100;
	unsigned char save_special = false;
	if (skill_id >= 0 && skill_id < 96) {

		int skill_master = ((skill_id % 24) / 8);
		if (timestamp != 0x0E0A1ACA) {

			int learn = skill_id % 24;
			int learned_skill = 1 << learn;
			if (attack_area->header.time != 0x0E0A1ACA && (g_mobs[user_index].mob.learn & learned_skill) == false)
				return false;

			if (skill_master < 0 || skill_master > 2)
				return false;
		}

		special = *(unsigned char*)(&g_mobs[user_index].mob.status.f_master + skill_master);
		skill_delay = skill_delay + special;
		save_special = special;
	} else {
		if (skill_id >= 96 && skill_id <= 100) {
			if (skill_id == 97) {
				if (attack_area->attacker_pos.X > 0 && attack_area->attacker_pos.X < 4096 && attack_area->attacker_pos.Y > 0 && attack_area->attacker_pos.Y < 4096) {
					int item_id = g_item_grid[attack_area->attacker_pos.X][attack_area->attacker_pos.Y];
					if (item_id > 0 && item_id < MAX_ITEM_LIST && g_ground_items[item_id].item_data.item_id == 746)
						attack_area->motion = MOVE_NORMAL;
					else
						return true;
				} else {
					return true;
				}
			} else {
				int learn = skill_id - 72;
				int learned_skill = 1 << learn;

				if (attack_area->header.time != 0x0E0A1ACA && (g_mobs[user_index].mob.learn & learned_skill) == false)
					return false;
			}

			special = g_mobs[user_index].mob.status.level;
			skill_delay = skill_delay + special;
			save_special = special;
		}
	}

	if (skill_id == 85) {
		int skill_gold_absorption = special * 100;
		if (g_mobs[user_index].mob.gold < skill_gold_absorption)
			return true;

		g_mobs[user_index].mob.gold = g_mobs[user_index].mob.gold - skill_gold_absorption;
		send_etc(user_index);
		send_all_packets(user_index);
	}

	int current_mp = g_mobs[user_index].mob.status.current_mp;
	if (skill_id >= 0 && skill_id < 98) {
		attack_area->required_mp = (short) (g_skill_data[skill_id].mana + ((special / 18) * 4)); //2a8
		if ((g_mobs[user_index].mob.status.current_mp - attack_area->required_mp) < 0)
			return true;

		g_mobs[user_index].mob.status.current_mp = g_mobs[user_index].mob.status.current_mp - attack_area->required_mp;
	}

	attack_area->current_mp = g_mobs[user_index].mob.status.current_mp;

	int calc_skill_master_3 = 0;
	if (g_mobs[user_index].mob.class_info == 0 && (g_mobs[user_index].mob.learn & (1 << 14) != 0)) {
		calc_skill_master_3 = g_mobs[user_index].mob.status.s_master / 20;
		if (calc_skill_master_3 < 0)
			calc_skill_master_3 = 0;
		else if (calc_skill_master_3 > 15)
			calc_skill_master_3 = 15;
	}

	if (mob->mob.class_info == 3) {
		for (size_t i = 0; i < MAX_AFFECT; i++) {
			if (mob->mob.affect[i].index == HT_INIVISIBILIDADE) {
				mob->mob.affect[i].index = 0;
				mob->mob.affect[i].time = 0;
				mob->mob.affect[i].master = 0;
				mob->mob.affect[i].value = 0;
				mob->buffer_time[i] = 0;
			}
		}
	}

	int result_exp = 0;
	int level = mob->mob.status.level;
	int hp = mob->mob.status.current_hp;
	int r_exp = 0;
	unsigned char double_critical;

	for (size_t i = 0; i < 13; i++) {
		if (i >= 1 && attack_area->header.size <= sizeof(struct packet_attack_single))
			break;

		if (i >= 2 && attack_area->header.size <= sizeof(struct packet_attack_straight))
			break;

		int target_index = attack_area->target[i].target_id; //2cc
		if (target_index <= 0 || target_index >= MAX_SPAWN_LIST)
			continue;

		struct mob_server_st *target_mob = &g_mobs[target_index];
		if (target_mob->mode == MOB_EMPTY) {
			remove_object(target_index, target_mob->mob.current, WORLD_MOB);
			send_remove_mob(user_index, target_index, 1);
			clear_property(target_mob);
			continue;
		}

		if (mob->mob.status.current_hp <= 0) {
			attack_area->target[i].target_id = MOB_EMPTY;
			attack_area->target[i].damage = 0;
			remove_object(target_index, target_mob->mob.current, WORLD_MOB);
			send_remove_mob(user_index, target_index, 1); // TODO: COLOCAR CONSTANTE
			clear_property(target_mob);
			continue;
		}

		if (get_distance(mob->mob.current, target_mob->mob.current) > 23)
			continue;

		int damage = attack_area->target[i].damage;
		if (damage != -2 && damage != -1 && damage != 0) {
			attack_area->target[i].damage = 0;
			continue;
		}

		//00431C9B
		int cape_info = mob->mob.cape; //2ec
		int target_cape_info = target_mob->mob.cape; //2f0
		bool flag_cape_info = false; //2f4
		if (cape_info == 7 && target_cape_info == 7 || cape_info == 8 && target_cape_info == 8)
			flag_cape_info = true;

		//00431D02
		if (damage == -2) {
			int distance = get_distance(attack_area->attacker_pos, attack_area->target_pos); //2f8
			if (distance > 7)
				return true;

			//00431D76
			damage = 0;
			if (i > 0 && attack_area->header.size < sizeof(struct packet_attack_straight) && mob->mob.class_info != 3 && (mob->mob.learn & (1 << 6)) == false)
				continue;
			
			//00431DD3
			int flag_double_critical = false; //2fc
			if (i == 0) {
				get_double_critical(&mob->mob, &double_critical);
				flag_double_critical = true;
			}

			//00431E24
			damage = mob->mob.status.attack;
			if ((double_critical & 2) != false) {
				if (target_index < MAX_USERS_PER_CHANNEL)
					damage = (((rand() % 3) + 15) * damage) / 10;
				else
					damage = (((rand() % 4) + 20) * damage) / 10;
			}

			//00431EA9
			int defense = target_mob->mob.status.defense; //300
			if (target_index < MAX_USERS_PER_CHANNEL)
				defense = defense << 1;

			//00431ED9
			damage = get_damage(damage, defense, calc_skill_master_3);

			//00431EFC
			if (i == 0 && attack_area->header.size >= sizeof(struct packet_attack_straight) && mob->mob.class_info == 3 && (mob->mob.learn & (1 << 21)) != false && (rand() % 4) == 0) {
				//00431F66
				int skill_damage = (mob->mob.status.strength + mob->mob.status.t_master) >> 1; //304
				unsigned int learn = 0; //308
				if (target_index >= MAX_USERS_PER_CHANNEL && target_mob->mob.b_status.level >= 300) {
					learn = target_mob->mob.learn;
					skill_damage = ((100 - learn) * skill_damage) / 100;
				}

				//00431FFC
				attack_area->target[1].target_id = MOB_EMPTY;
				attack_area->target[1].damage = skill_damage;
				double_critical = double_critical | 4;
				damage = damage + skill_damage;
			}

			//00432040
			if (double_critical & 1 != false)
				damage = damage << 1;

			attack_area->double_critical = double_critical;
		} else if (damage == -1 && skill_id >= 0 && skill_id <= 100) { //00432075
			damage = 0;
			int skill_max_target = g_skill_data[skill_id].max_target; //30c
			if (timestamp != 0x0E0A1ACA && i >= skill_max_target)
				continue;

			//004321CA
			int unk2 = 0; //310
			int skill_instance_type = g_skill_data[skill_id].instance_type; //314
			if (skill_instance_type >= 0 && skill_instance_type <= 5) {
				//00432203
				int current_weather = g_current_weather; //318
				int range_weather; //2088
				if (mob->mob.current.X >> 7 < 12 && mob->mob.current.Y >> 7 > 25)
					range_weather = 1;
				else
					range_weather = 0;

				if (range_weather != 0)
					current_weather = 0;

				//00432266
				if (timestamp == 0x0E0A1ACA && attack_area->motion == 254 && (attack_area->skill_index == 32 || attack_area->skill_index == 34 || attack_area->skill_index == 36)) {
					//004322BE
					int level = mob->mob.status.level; //31c
					int pet_sanc = get_item_sanc(&mob->mob.equip[MOUNT_SLOT]); //320
					switch (attack_area->skill_index) {
					case 32:
						damage = (pet_sanc * 200) + (level * 8);
						break;
					case 34:
						damage = (pet_sanc * 300) + (level * 8);
						break;
					case 36:
						damage = (pet_sanc * 350) + (level * 8);
						break;
					}
				} else { //00432376
					damage = get_skill_damage(skill_id, &mob->mob, current_weather, (mob->mob.weapon_damage));
				}

				//004323B2
				int defense = target_mob->mob.status.defense; //324
				if (target_index < MAX_USERS_PER_CHANNEL)
					defense = defense << 1;

				//004323E2
				if (target_mob->mob.class_info == 1 || target_mob->mob.class_info == 2)
					defense = (defense << 1) / 3;

				//00432424
				damage = get_skill_damage_by_master(damage, defense, calc_skill_master_3);
				
				//Fanatismo, Carga, Golpe_Mortal, Espada_da_F�nix, Golpe_Duplo, Contra_Ataque, Ataque_da_Alma, Punhalada_Venenosa, Nevoa_Venenosa, Som_das_Fadas,
				//F�ria_de_Gaia, Ataque_Fatal, Tempestade_De_Raios, Golpe_Felino, Explos�o_Et�rea, L�mina_das_Sombras, Prote��o_Das_Sombras
				if (skill_instance_type == 1) {
					int calc_damage = 0; //328
					if (target_index >= MAX_USERS_PER_CHANNEL)
						calc_damage = target_mob->mob.learn;
					else
						calc_damage = 10;

					damage = ((100 - calc_damage) * damage) / 100;
				} else { //004324AB
					//2-Destino, 2-Espada_Flamejante, 3-Lamina_Congelada, 2-Exterminar, 2-Tempestade_De_Gelo, 4-Choque_Divino, 4-Julgamento_Divino, 2-Ataque_de_Fogo, 5-Rel�mpago,
					//3-Lan�a_de_Gelo, 2-Tempestade_de_Meteoro, 3-Nevasca, 2-F�nix_de_Fogo, 2-Inferno, 2-Fera_Flamejante, 2-Espirito_Vingador, 2-Canh�o_Guardi�o
					if (skill_instance_type >= 2 && skill_instance_type <= 5) {
						int get_skill_instance_type = skill_instance_type - 2; //32c
						int resist = target_mob->mob.resist[0] + get_skill_instance_type; //330
						if (target_index < MAX_USERS_PER_CHANNEL)
							resist = resist / 2;

						damage = ((100 - resist) * damage) / 100;
					}
				}

				//00432528
				//Destino, Espada_Flamejante, Exterminar, Tempestade_De_Gelo, Ataque_de_Fogo, Fera_Flamejante
				if (skill_instance_type == 2) {
					int calc = abs(rand() % 4) + 10; //334
					if (calc < 0) {
						calc = 0;
						attack_area->target[i].target_id = MOB_EMPTY;
					}

					damage = (damage * calc) / 10;
				} else { //00432585
					//Giro_da_F�ria, Flecha_M�gica, Choque_Divino, Julgamento_Divino
					if (skill_instance_type == 4) { 
						int calc = abs(rand() % 4) + 10;
						if (calc < 0) {
							calc = 0;
							attack_area->target[i].target_id = MOB_EMPTY;
						}

						damage = (damage * calc) / 10;
					}
				}
			} //004325E9 - Cura, Recuperar, Renascimento
			else if (skill_instance_type == 6) { 
			
				if (target_mob->mob.cape == 4)
					continue;

				//004326A0
				if (skill_id == 27) //Cura
					damage = (save_special * 2) + g_skill_data[skill_id].instance_value;
				else
					damage = ((save_special * 3) / 2) + g_skill_data[skill_id].instance_value;

				damage = -damage;
				if (damage < 0 && damage > -6)
					damage = -6;

				int pTargetHP = target_mob->mob.status.current_hp; //340

				target_mob->mob.status.current_hp = target_mob->mob.status.current_hp - damage;

				//004327E3
				if (target_mob->mob.status.current_hp > target_mob->mob.status.max_hp)
					target_mob->mob.status.current_hp = target_mob->mob.status.max_hp;

				if (target_index > 0 && target_index < MAX_USERS_PER_CHANNEL)
					send_score(target_index);

				int pTargetHpFinal = target_mob->mob.status.current_hp; //348
				int calc = (pTargetHpFinal - pTargetHP) >> 3; //34c
				if (calc > 120)
					calc = 120;
				if (calc < 0)
					calc = 0;
			} else if (skill_instance_type == 7) { //00432933 - Flash
				//0043293C
				target_mob->mode = MOB_PEACE;
				target_mob->current_target = MOB_EMPTY;
				for (size_t j = MOB_EMPTY; j < MAX_ENEMY; j++) //354
					target_mob->enemy_list[j] = MOB_EMPTY;
			} else if (skill_instance_type == 8) { //004329AE - Desintoxicar,
				//004329BB
				int flagSkillOff = false; //358
				for (int k = 0; k < MAX_AFFECT; k++) { //35c
					int skillAffectType = target_mob->mob.affect[k].index;
					if (skillAffectType == 1 || skillAffectType == 3 || skillAffectType == 5 || skillAffectType == 7 || skillAffectType == 10 || skillAffectType == 12 || skillAffectType == 20) {
						target_mob->mob.affect[k].index = 0;
						flagSkillOff = true;
					}
				}

				if (flagSkillOff == true) {
					get_current_score(target_index);
					send_score(target_index);
					if (target_index <= MAX_USERS_PER_CHANNEL) {
						send_affects(target_index);
						send_all_packets(target_index);
					}
				}
			} else if (skill_instance_type == 9) { //00432AB2 - Teleporte,
				//00432ABF
				if (target_mob->mob.status.current_hp <= 0) {
					send_client_message("Nao e possivel sumonar personagem morto", user_index); //N�o � poss�vel summonar personagem morto.
					return true;
				}

				//00432AEC
				unsigned char getAtt = check_pvp_area(user_index);
				if (getAtt != false && mob->mob.status.level < 1000) {
					send_client_message("Area Indisponivel para sumonar", user_index); //�rea restrita para uso.
					continue;
				}

				//00432B5A
				if (target_mob->mob.status.current_hp > mob->mob.status.current_hp + special + 30) {
					send_client_message("Level muito alto para sumonar", user_index); //Level muito alto para summonar.
					continue;
				}

				//00432BA1
				if ((target_mob->mob.current.X & 0xFF00) == 0 && (target_mob->mob.current.Y & 0xFF00) == 0) {
					send_client_message("Nao pode usar Aqui", user_index); //N�o pode usar aqui.
					return true;
				}

				//00432BEF
				if (target_index < MAX_USERS_PER_CHANNEL && g_users[target_index].server_data.mode == USER_PLAY) {
					char temp[99];
					send_teleport(target_index, mob->mob.current);
					sprintf(temp, "Voce foi summonado por %s.", mob->mob.name); //Voc� foi summonado por %s.
					send_client_message(temp, target_index);
				}
			} else if (skill_instance_type == 10 && target_index < MAX_USERS_PER_CHANNEL) { //00432C76
				for (int l = MAX_USERS_PER_CHANNEL; l < MAX_SPAWN_LIST; l++) { //368
					if (g_mobs[l].mode == MOB_COMBAT && g_mobs[l].current_target == target_index) { //36c
						for (int m = MOB_EMPTY; m < MAX_ENEMY; m++) {
							if (g_mobs[l].enemy_list[m] == target_index)
								g_mobs[l].enemy_list[m] = user_index;
						}

						g_mobs[l].current_target = user_index;
					}
				}
			}
			//00432D76 - Evocar_Condor, Evocar_Javali_Selvagem, Evocar_Lobo_Selvagem,
			//Evocar_Urso_Selvagem, Evocar_Grande_Tigre, Evocar_Gorila_Gigante, Evocar_Drag�o_Negro, Evocar_Succubus
			else if (skill_instance_type == 11) {
				//00432D83
				int master = special;
				int max_mob_gen = (1 + (master / 33));
				int max = 0;
				int mob_gen_index = attack_area->skill_index - 56;
				switch (mob_gen_index)
				{
				case 7: //Succubus
					max = 2;
					break;
				case 6: //Dragao
					max = 2;
					break;
				case 5: //Gorila
					max = 4;
					break;
				default:
					max = 6;
					break;
				}

				if (max_mob_gen > max)
					max_mob_gen = max;

				struct mob_st *gen = &g_baby_list[mob_gen_index];
				
				if (mob->evoc_time > 150)
					return true;

				for (int bb = 0; bb < MAX_PARTY; bb++) {
					int index = mob->baby_mob[bb];
					if (index <= 0)
						continue;
					
					if (index <= MAX_USERS_PER_CHANNEL || index >= MAX_SPAWN_LIST) {
						mob->baby_mob[bb] = 0;
						continue; 
					}

					struct mob_server_st *baby_mob = &g_mobs[index];

					if (!is_summon(*baby_mob)) {
						mob->baby_mob[bb] = 0;
						continue;
					}

					struct position_st baby_position = g_mobs[index].mob.current;
					remove_object(index, baby_position, WORLD_MOB);
					send_remove_mob(index, index, DELETE_UNSPAWN);
					clear_property(baby_mob);
					mob->baby_mob[bb] = 0;
				}

				mob->evoc_time = 0;
				int i = 1;
				for (int j = 0; i < MAX_PARTY && j <= max_mob_gen; i++) {
					int index = mob->baby_mob[i];
					if (index != 0)
						continue;

					index = get_spawn_empty_index();
					if (index == 0)
						break;

					struct position_st mob_position = mob->mob.current;

					int LeaderPT = 0;

					if (!update_world(index, &mob_position, WORLD_MOB))
						continue;

					struct mob_server_st *baby_mob = &g_mobs[index];
					memcpy(&baby_mob->mob, gen, sizeof(struct mob_st));
					baby_mob->mode = MOB_IDLE;
					baby_mob->mob.client_index = index;
					baby_mob->next = (get_clock() + 800);
					baby_mob->spawn_type = SPAWN_BABYGEN;
					baby_mob->mob_type = MOB_TYPE_SUMMON;
					baby_mob->summoner = user_index;
					baby_mob->baby_mob[1] = user_index;
					baby_mob->mob.current.X = mob_position.X;
					baby_mob->mob.current.Y = mob_position.Y;
					baby_mob->mob.dest.X = mob_position.X;
					baby_mob->mob.dest.Y = mob_position.Y;
					memcpy(&baby_mob->mob.status, &baby_mob->mob.b_status, sizeof(struct status_st));
					get_current_score(index);
					int hp = ((400 + (50 * mob_gen_index)) + (37 * master));
					hp += (mob_gen_index * (master * 4));
					int atk = ((200 + (50 * mob_gen_index)) + (4 * master));
					atk += ((mob_gen_index + 1) * (mob->mob.status.intelligence / 7));
					int def = ((200 + (50 * mob_gen_index)) + (5 * master));
					def += ((mob_gen_index + 1) * (mob->mob.status.intelligence / 9));
					baby_mob->mob.guild_id = mob->mob.guild_id;
					baby_mob->mob.status.max_hp = hp;
					baby_mob->mob.status.max_mp = hp;
					baby_mob->mob.status.attack = atk;
					baby_mob->mob.status.defense = def;
					baby_mob->mob.status.current_hp = baby_mob->mob.status.max_hp;
					baby_mob->mob.status.current_mp = baby_mob->mob.status.max_mp;
					baby_mob->mob.cape = 4;
					baby_mob->mob.status.level = mob->mob.status.level;
					mob->baby_mob[i] = index;
					send_grid_mob(index);
					baby_mob->spawn_type = SPAWN_NORMAL;
					if (index > g_spawn_count)
						g_spawn_count = index;
					j++;
				}
				mob->evoc_time = 180;
				damage = 0;
			} else if (skill_instance_type == 12) { //00432E0C - Chamas_Et�reas
				// TODO
			}

			//004332FC
			if (skill_id == 6) { //F�ria_Divina
				//00433309
				if (target_index >= MAX_USERS_PER_CHANNEL && target_mob->mob.status.merchant != 0)
					break;

				if (target_mob->mob.cape == 6)
					break;

				struct position_st mob_position = mob->mob.current;
				if (mob_position.X < target_mob->mob.current.X)
					mob_position.X++;

				if (mob_position.X > target_mob->mob.current.X)
					mob_position.X--;

				if (mob_position.Y < target_mob->mob.current.Y)
					mob_position.Y++;

				if (mob_position.Y > target_mob->mob.current.Y)
					mob_position.Y--;

				if (update_world(target_index, &mob_position, WORLD_MOB) == false)
					break;

				int skillMaster2 = mob->mob.status.f_master; //3c4
				int calcskillmaster2 = (skillMaster2 / 10) + 40; //3c8
				if (target_index > MAX_USERS_PER_CHANNEL)
					calcskillmaster2 = (skillMaster2 / 5) + 60;

				//004334A0
				int restLevel = mob->mob.status.level - target_mob->mob.status.level; //3cc
				restLevel = restLevel >> 1;

				if (rand() % 100 >= calcskillmaster2 + restLevel)
					break;

				int saveSpeed = target_mob->mob.status.speed;
				target_mob->mob.status.speed = 5;
				get_action(target_index, mob_position, 7, NULL);
				target_mob->mob.status.speed = saveSpeed;
				break;
			}

			if (skill_id == 22) { //00433587 //Exterminar
				//00433594
				int curMp = mob->mob.status.current_mp; //404
				mob->mob.status.current_mp = 0;
				attack_area->current_mp = 0;
				int skillMaster4 = mob->mob.status.t_master; //408
				int curSTR = mob->mob.status.strength; //40c
				damage = (damage + curMp) + (curSTR / 2);
			} else if (skill_id == 30) { //004337CB //Julgamento_Divino
				//004337D4
				damage = damage + hp;
				mob->mob.status.current_hp = (mob->mob.status.current_hp * 2) / 3;
			}

			//00433A7A
			int skillAggressive = g_skill_data[skill_id].aggressive; //0
			int flagRegenMp = true; //470
			if (skillAggressive != 0) {
				int skillAffectResist = g_skill_data[skill_id].affect_resist; //474
				int difLevel = target_mob->mob.status.level - mob->mob.status.level; //478
				difLevel = difLevel / 2;
				if (skillAffectResist >= 1 && skillAffectResist <= 4) {
					int random = rand() % 100; //47c
					if (random < target_mob->mob.regen_mp + skillAffectResist + difLevel)
						flagRegenMp = false;
				}

				if (user_index < MAX_USERS_PER_CHANNEL && target_mob->mob.cape == 6)
					flagRegenMp = false;
			}

			//00433BC3
			if (flagRegenMp != false) {
				get_set_affect(target_index, (struct affect_st) { g_skill_data[skill_id].affect_type, g_skill_data[skill_id].time, g_skill_data[skill_id].affect_value, special });
				unk2 = true;

				if (unk2 != false) {
					get_current_score(target_index);
					send_score(target_index);
					if (target_index <= MAX_USERS_PER_CHANNEL) {
						send_affects(target_index);
						send_all_packets(target_index);
					}
				}
			}

			struct skill_data_st *skill = &g_skill_data[skill_id];
			//00433C6A
			if (g_skill_data[skill_id].affect_type == BM_MUTACAO) {
				int faceID = 0;
				if (skill->affect_value >= 1 && skill->affect_value <= 4)
					faceID = ((skill->affect_type + 5) + skill->affect_value);
				else if (skill->affect_value == 5)
					faceID = EDEN_FACE;

				mob->mob.equip[0].EFV2 = mob->mob.equip[0].item_id;
				mob->mob.equip[0].item_id = faceID;
				int sanc = special / 25;
				if (sanc >= 10) sanc = 9;
				else if (sanc < 0) sanc = 0;
				mob->mob.equip[0].EFV1 = sanc;
				send_refresh_equip_items(user_index, 0);
				send_score(user_index);
				send_grid_mob(user_index);
			}
		} else { //00433C9E
			return true;
		}

		//00433CD0
		attack_area->target[i].damage = damage;
		if (damage <= 0) {
			attack_area->target[i].damage = -3;
			continue;
		}
			

		if (target_index < MAX_USERS_PER_CHANNEL || target_mob->mob.cape == 4) { // calculo do dano adicional do bonus de perfura��o
			//00433D17
			if (attack_area->double_critical & 4 != false) {
				int calcSecondDamage = damage - attack_area->target[1].damage; //480
				calcSecondDamage = calcSecondDamage >> 2;
				damage = attack_area->target[1].damage + calcSecondDamage;
			} else {
				damage = damage >> 2;
			}
		}

		//00433D85
		if (mob->force_damage != 0) {
			//00433D97
			if (damage == 1)
				damage = mob->force_damage;
			else {
				if (damage > 0)
					damage = damage + mob->force_damage;
			}

			attack_area->target[i].damage = damage;
		}

		if (target_index >= MAX_USERS_PER_CHANNEL && flag_cape_info != false)
			damage = 0;

		int pTargetSummoner = target_index; //484
		if (target_index >= MAX_USERS_PER_CHANNEL && target_mob->mob.cape == 4 && target_mob->summoner > MOB_EMPTY && target_mob->summoner < MAX_USERS_PER_CHANNEL)
			pTargetSummoner = target_mob->summoner;

		//00433ECF
		if (pTargetSummoner < MAX_USERS_PER_CHANNEL) {
			//004341B1
			int pTargetParry = target_mob->mob.evasion; //528
			int pRand = rand() % 100; //52c
			if (pRand < pTargetParry) {
				damage = -3;
			}
		}

		attack_area->target[i].damage = damage;
		if (damage <= 0)
			continue;

		bool Congelar = false;
		bool Veneno = false;
		bool CM = false;
		for (int x = 0; x < MAX_AFFECT; x++) {
			if (mob->mob.affect[x].index == HT_GELO)
				Congelar = true;
			else if (mob->mob.affect[x].index == HT_VENENO)
				Veneno = true;
		}

		//004343A2
		if (Congelar != false) {
			int _pRand = rand() % 2; //53c
			if (_pRand == 0) {
				int skill_master_1 = mob->mob.status.f_master;
				get_set_affect(target_index, (struct affect_st) { LENTIDAO, skill_master_1, 4, special });
				int unk2 = true;

				if (unk2 != false) {
					get_current_score(target_index);
					send_score(target_index);
					if (target_index <= MAX_USERS_PER_CHANNEL) {
						send_affects(target_index);
						send_all_packets(target_index);
					}
				}
			}
		}
		if (Veneno != false) {
			int _pRand = rand() % 2; //53c
			if (_pRand == 0) {
				int skill_master_1 = mob->mob.status.f_master;
				get_set_affect(target_index, (struct affect_st) { VENENO, skill_master_1, 4, special });
				int unk2 = true;

				if (unk2 != false) {
					get_current_score(target_index);
					send_score(target_index);
					if (target_index <= MAX_USERS_PER_CHANNEL) {
						send_affects(target_index);
						send_all_packets(target_index);
					}
				}
			}
		}

		//00434426
		int pHpOurDamage; //208c
		if (target_mob->mob.status.current_hp < damage)
			pHpOurDamage = target_mob->mob.status.current_hp;
		else
			pHpOurDamage = damage;

		//00434468
		int _pHpOurDamage = pHpOurDamage; //540
		int calcExp = (target_mob->mob.experience * _pHpOurDamage) / target_mob->mob.status.max_hp; //544
		calcExp = get_exp_by_kill(calcExp, user_index, target_index);
		if (target_mob->mob.cape == 4)
			calcExp = 0;

		if (target_index >= MAX_USERS_PER_CHANNEL)
			r_exp = r_exp + calcExp;

		if (target_index > 0 && target_index < MAX_USERS_PER_CHANNEL && damage > 0) {
			if (target_mob->reflect_damage > 0)
				damage = damage - target_mob->reflect_damage;

			if (damage <= 0) {
				damage = 0;
				attack_area->target[i].damage = damage;
				continue;
			}

			attack_area->target[i].damage = damage;
		}

		//004345BE
		int _pDamage = damage; //548
		int _calcDamage = 0; //54c
		int pTargetMountId = target_mob->mob.equip[MOUNT_SLOT].item_id; //550
		if (target_index < MAX_USERS_PER_CHANNEL && pTargetMountId >= 2360 && pTargetMountId < 2390 && target_mob->mob.equip[MOUNT_SLOT].EF1 > 0) {
			_pDamage = (damage * 3) >> 2;
			_calcDamage = damage - _pDamage;
			if (_pDamage <= 0)
				_pDamage = 1;

			attack_area->target[i].damage = _pDamage;
		}

		//0043467E
		int tDamage = _pDamage; // 554
		if (target_mob->mob.equip[PET_SLOT].item_id == 786) {
			//004346A8
			int itemSanc = get_item_sanc(&target_mob->mob.equip[PET_SLOT]); //558
			if (itemSanc < 2)
				itemSanc = 2;

			tDamage = _pDamage / itemSanc;
			if (tDamage > target_mob->mob.status.current_hp)
				tDamage = target_mob->mob.status.current_hp;

			target_mob->mob.status.current_hp = target_mob->mob.status.current_hp - tDamage;
		} else {
			//00434751
			if (tDamage > target_mob->mob.status.current_hp)
				tDamage = target_mob->mob.status.current_hp;

			target_mob->mob.status.current_hp = target_mob->mob.status.current_hp - tDamage;
		}

		if (target_mob->mob.status.current_hp <= 0) {
			target_mob->mob.status.current_hp = 0;
			unknow[i] = target_index;
			continue;
		}

		//004348F5
		if (target_mob->mode != MOB_EMPTY && target_mob->mob.status.current_hp > 0) {
			//00434929
			add_enemy_list(target_mob, user_index);
			mob->current_target = target_index;
			int party_leader = mob->party_leader; //55c
			int party_index = mob->party_index;
			if (party_leader <= MOB_EMPTY)
				party_leader = user_index;

			int r; //560
			if (party_index > 0) {
				for (r = MOB_EMPTY; r < MAX_PARTY; r++) {
					int party_member_index = g_parties[party_index].players[r]; //564
					if (party_member_index <= MAX_USERS_PER_CHANNEL)
						continue;

					struct mob_server_st *party_member = &g_mobs[party_member_index];

					// if (party_member->mode == MOB_EMPTY || party_member->mob.status.current_hp <= 0)
					// 	send_remove_party(party_member_index, false); // IMPLEMENTAR
					// else
					// 	add_enemy_list(party_member, target_index);
				}
			}
			
			//00434A5B
			party_leader = target_mob->party_leader;
			party_index = target_mob->party_index;
			if (party_leader <= MOB_EMPTY)
				party_leader = user_index;

			if (party_index > 0) {
				for (r = MOB_EMPTY; r < MAX_PARTY; r++) {
					int party_member_index = g_parties[party_index].players[r]; //564
					if (party_member_index <= MAX_USERS_PER_CHANNEL)
						continue;

					struct mob_server_st *party_member = &g_mobs[party_member_index];

					// if (party_member->mode == MOB_EMPTY || party_member->mob.status.current_hp <= 0)
					// 	// send_remove_party(party_member_index, false); IMPLEMENTAR
					// else
					// 	add_enemy_list(party_member, user_index);
				}
			}
		}
	}

	if (r_exp > 0)
		result_exp = r_exp;

	if (skill_id == 30) //Julgamento_Divino
		send_score(user_index);

	if (result_exp <= 0)
		result_exp = 0;

	if (mob->mob.hold > 0) {
		if (mob->mob.hold < result_exp) {
			int diff = (result_exp  - mob->mob.hold);
			mob->mob.hold = 0;
			mob->mob.experience += diff;
		}
		else mob->mob.hold -= result_exp;
	}
	else mob->mob.experience += result_exp;

	attack_area->current_exp = mob->mob.experience;
	if (timestamp == 0x0E0A1ACA)
		attack_area->header.time = g_current_time;

	send_grid_multicast(mob->mob.current, (unsigned char *) attack_area, false);

	for (size_t s = MOB_EMPTY; s < 13; s++) { //56c
		if (unknow[s] > MOB_EMPTY && unknow[s] < MAX_SPAWN_LIST && g_mobs[unknow[s]].mode != MOB_EMPTY)
			send_mob_dead(user_index, unknow[s]);
	}

	//00434CF2
	int retSegment = check_get_level(mob); //570
	if (retSegment >= 1 && retSegment <= 4) {
		if (retSegment == 4)
			send_client_message("++++Parabens, Subiu de Level++++", user_index);
		if (retSegment == 3)
			send_client_message("Adquiriu 3 / 4 de bonus.", user_index);
		if (retSegment == 2)
			send_client_message("Adquiriu 2 / 4 de bonus.", user_index);
		if (retSegment == 1)
			send_client_message("Adquiriu 1 / 4 de bonus.", user_index);

		send_score(user_index);
		send_emotion(user_index, 0xE, 3);

		if (retSegment == 4) {
			//00434DB7
			send_etc(user_index);
			get_current_score(user_index);
			send_etc(user_index);
			send_score(user_index);
			send_affects(user_index);
			send_all_packets(user_index);
			int pk_points = get_pk_points(user_index) + 5; //574
			get_create_mob(user_index, user_index);
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