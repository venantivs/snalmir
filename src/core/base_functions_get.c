/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Agosto de 2025
 * Arquivo: core/base_functions_get.c
 * Descrição: Arquivo onde são implementadas algumas funções getters úteis.
 */

#include "base-def.h"
#include "../network/server.h"
#include "world.h"
#include "base_functions.h"
#include "../network/socket-utils.h"
#include "item_effect.h"
#include "game_items.h"
#include "game_skills.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

int BaseSIDCHM[4][6] = {
	{ 8, 4, 7, 6, 80, 45 }, // Transknight
	{ 5, 8, 5, 5, 60, 65 }, // Foema
	{ 6, 6, 9, 5, 70, 55 }, // BeastMaster
	{ 8, 9, 13, 6, 75, 60 } // Hunter
};

size_t
get_spawn_empty_index()
{
  for (size_t i = (BASE_MOB + 1); i < MAX_SPAWN_LIST; i++) {
    if (mobs[i].mode == MOB_EMPTY)
      return i;
  }

  return 0;
}

void
get_create_item(int item_index)
{
	struct packet_create_ground_item create_ground_item;
	struct ground_item_st *init = &ground_items[item_index];
	create_ground_item.header.index = 0x7530;
	create_ground_item.header.operation_code = 0x26E;
	create_ground_item.header.size = sizeof(struct packet_create_ground_item);
	create_ground_item.position.X = init->position.X;
	create_ground_item.position.Y = init->position.Y;
	create_ground_item.item_index = init->item_index + 10000; // ?????
	memcpy(&create_ground_item.item, &init->item_data, sizeof(struct item_st));
	create_ground_item.rotation = init->rotation;
	create_ground_item.status = init->status;
	send_grid_item(item_index, &create_ground_item);
}

int
get_item_id_and_effect(struct item_st item, int mount)
{
	int value;

	// montaria
	if (mount == 14) {
		if (item.item_id == 0)
			return 0;
		else if (item.EFV1 == 0)
			return 0;
		else if (item.EF2 <= 8)
			return item.item_id + (0 * 0x1000);
		else if (item.EF2 <= 16)
			return item.item_id + (1 * 0x1000);
		else if (item.EF2 <= 24)
			return item.item_id + (2 * 0x1000);
		else if (item.EF2 <= 32)
			return item.item_id + (3 * 0x1000);
		else if (item.EF2 <= 40)
			return item.item_id + (4 * 0x1000);
		else if (item.EF2 <= 48)
			return item.item_id + (5 * 0x1000);
		else if (item.EF2 <= 56)
			return item.item_id + (6 * 0x1000);
		else if (item.EF2 <= 64)
			return item.item_id + (7 * 0x1000);
		else if (item.EF2 <= 72)
			return item.item_id + (8 * 0x1000);
		else if (item.EF2 <= 80)
			return item.item_id + (9 * 0x1000);
		else if (item.EF2 <= 88)
			return item.item_id + (10 * 0x1000);
		else if (item.EF2 <= 96)
			return item.item_id + (11 * 0x1000);
		else if (item.EF2 <= 104)
			return item.item_id + (12 * 0x1000);
		else if (item.EF2 <= 112)
			return item.item_id + (13 * 0x1000);
		else if (item.EF2 <= 119)
			return item.item_id + (14 * 0x1000);
		else if (item.EF2 >= 120)
			return item.item_id + (15 * 0x1000);
	} else {
		if (item.item_id == 0)
			return 0;
		if (item.EF1 == 43 || (item.EF1 >= 116 && item.EF1 <= 125))
			value = item.EFV1;
		else if (item.EF2 == 43 || (item.EF2 >= 116 && item.EF2 <= 125))
			value = item.EFV2;
		else if (item.EF3 == 43 || (item.EF3 >= 116 && item.EF3 <= 125))
			value = item.EFV3;
		else
			return item.item_id;
	}

	if (value == 0)
		return item.item_id;
	else if (value <= 1)
		value = 1;
	else if (value <= 2)
		value = 2;
	else if (value <= 3)
		value = 3;
	else if (value <= 4)
		value = 4;
	else if (value <= 5)
		value = 5;
	else if (value <= 6)
		value = 6;
	else if (value <= 7)
		value = 7;
	else if (value <= 8)
		value = 8;
	else if (value <= 9)
		value = 9;
	else if (value >= 230 && value <= 233)
		value = 10;
	else if (value >= 234 && value <= 237)
		value = 11;
	else if (value >= 238 && value <= 241)
		value = 12;
	else if (value >= 242 && value <= 245)
		value = 13;
	else if (value >= 246 && value <= 249)
		value = 14;
	else if (value >= 250 && value <= 253)
		value = 15;
	else
		return item.item_id;

	return item.item_id + (value * 0x1000);
}

// Retorna 0 se o item for invalido ou sem refinação
// Retorna 10h se foi refinado com Diamante
// Retorna 20h se foi refinado com Esmeralda
// Retorna 30h se foi refinado com Coral
// Retorna 40h se foi refinado com Garnet
// TODO: No need to be a pointer
int
get_anct_code(struct item_st *item)
{
	int value = 0;

	if (item->EF1 == 43 || (item->EF1 >= 116 && item->EF1 <= 125))
		value = item->EFV1;
	else if (item->EF2 == 43 || (item->EF2 >= 116 && item->EF2 <= 125))
		value = item->EFV2;
	else if (item->EF3 == 43 || (item->EF3 >= 116 && item->EF3 <= 125))
		value = item->EFV3;

	if (value == 0)
		return 0;
	if (item->EF1 >= 116 && item->EF1 <= 125)
		return item->EF1;
	if (item->EF2 >= 116 && item->EF2 <= 125)
		return item->EF2;
	if (item->EF3 >= 116 && item->EF3 <= 125)
		return item->EF3;
	if (value < 230)
		return 43;

	switch (value % 4)
	{
	case 0:  return 0x30;
	case 1:  return 0x40;
	case 2:  return 0x10;
	default: return 0x20;
	}
}

void
get_action(int mob_index, short posX, short posY, int type, char *command)
{
	struct mob_st *mob = &mobs[mob_index].mob;

	struct packet_request_action request_action;
	request_action.header.size = sizeof(struct packet_request_action);
	request_action.header.operation_code = 0x366;
	request_action.header.index = mob_index;
	request_action.source_position = mob->current;
	request_action.speed = mob->status.speed;
	request_action.destiny_position.X = posX;
	request_action.destiny_position.Y = posY;
	request_action.type = type;

	memset(&request_action.command, 0, 24);
	
	if (command != NULL)
		strncpy(request_action.command, command, 24);

	send_grid_multicast_with_packet(mob_index, posX, posY, (unsigned char*) &request_action);

	if (command != NULL || type == MOVE_TELEPORT) {
		if (mob_index <= MAX_USERS_PER_CHANNEL)
			send_one_message((unsigned char*) &request_action, sizeof(struct packet_request_action), mob_index);
	}
}

void
get_affect(int offset, struct affect_st *skills, unsigned char *buffer)
{
	short *packet = (short*) &buffer[offset];

	for (size_t i = 0; i < MAX_AFFECT; i++) {
		int time = skills[i].time;
		if (time > 255)
			time = 255;

		packet[i] = ((skills[i].index << 8) + time);
	}
}

int
get_pk_points(int mob_index) {
	return mobs[mob_index].mob.inventory[63].effect[0].index;
}

int
get_current_kills(int mob_index) {
	return mobs[mob_index].mob.inventory[63].effect[0].value;
}

int
get_total_kills(int mob_index) {
	unsigned char f_tFrag = mobs[mob_index].mob.inventory[63].effect[1].value;
	unsigned char s_tFrag = mobs[mob_index].mob.inventory[63].effect[2].value;

	return f_tFrag + (s_tFrag << 8);
}

int
get_cape_id(int item_id) {
	switch (item_id) {
	case 543:
	case 545:
	case 549:
	case 734:
	case 736:
	case 1766:
	case 1767:
	case 1768:
	case 3191:
	case 3194:
	case 3197:
	case 3199:
		return 1; // Blue
	case 544:
	case 546:
	case 735:
	case 737:
	case 1720:
	case 1769:
	case 1770:
	case 1771:
	case 3192:
	case 3195:
	case 3198:
	case 4015:
		return 2; // Red
	default:
		return 3; // Outras
	}
}

void
set_total_kills(int mob_index, int frag)
{
	if (frag < 0)
		frag = 0;
	else if (frag > 9999)
		frag = 9999;

	mobs[mob_index].mob.inventory[63].effect[1].value = frag & 255;
	mobs[mob_index].mob.inventory[63].effect[2].value = frag >> 8;
}

int
get_guilty(int mob_index)
{
	unsigned char *guilty = &mobs[mob_index].mob.inventory[63].effect[1].index;
	if (*guilty > 50)
		*guilty = 0;

	return *guilty;
}

void
get_create_mob(int create_index, int send_index)
{
	struct mob_st *npc_mob = &mobs[create_index].mob;

	struct packet_spawn_info spawn_info;
	memset(&spawn_info, 0xCC, sizeof(struct packet_spawn_info));
	spawn_info.header.size = sizeof(struct packet_spawn_info);
	spawn_info.header.operation_code = 0x364;
	spawn_info.header.index = 0x7530;
	spawn_info.client_id = create_index;
	spawn_info.position.X = npc_mob->last_position.X;
	spawn_info.position.Y = npc_mob->last_position.Y;
	spawn_info.spawn.member_type = npc_mob->guild_member_type * 0x40;

	if (mobs[create_index].guild_disable == 0 && create_index <= MAX_USERS_PER_CHANNEL) {
		spawn_info.guild_index = npc_mob->guild_id;
		spawn_info.spawn.member_type = npc_mob->guild_member_type * 0x40;
	} else if (create_index <= MAX_USERS_PER_CHANNEL) {
		spawn_info.guild_index = 0;
		spawn_info.spawn.member_type = 0;
	}

	strncpy(spawn_info.name, npc_mob->name, 16);
	memcpy(&spawn_info.status, &npc_mob->status, sizeof(struct status_st));

	if (create_index < MAX_USERS_PER_CHANNEL) {
		spawn_info.chaos_points = get_pk_points(create_index);
		spawn_info.current_kill = get_current_kills(create_index);
		spawn_info.total_kill = get_total_kills(create_index);

		if (get_guilty(create_index) > 0)
			spawn_info.chaos_points = 0;
	}

	spawn_info.spawn.type = mobs[create_index].spawn_type;

	for (size_t i = 0; i < 16; i++) {
		struct item_st *equipped_item = &npc_mob->equip[i];

		// ????
		if (i == 14)
			if (equipped_item->item_id >= 2360 && equipped_item->item_id <= 2389)
				if (equipped_item->effect[0].index == 0)
					equipped_item->item_id = 0;

		spawn_info.item_refine[i] = get_item_id_and_effect(*equipped_item, i);
		spawn_info.anct_code[i] = get_anct_code(equipped_item);
	}

	// TODO: Vê isso aqui com calma???
	int offset = (int) ((int) spawn_info.affect) - ((int) &spawn_info);

	get_affect(offset, npc_mob->affect, (unsigned char *) &spawn_info);

	if (create_index <= MAX_USERS_PER_CHANNEL)
		strncpy(spawn_info.tab, npc_mob->tab, 26);
	else
		memset(spawn_info.tab, 0, 26);

	if (create_index >= BASE_MOB) {
		spawn_info.status.current_hp = npc_mob->status.current_hp;
		spawn_info.status.current_mp = npc_mob->status.current_mp;
	} else {
		if (is_dead(mobs[create_index])) {
			spawn_info.status.current_hp = 0;
			spawn_info.status.current_mp = 0;
		} else {
			spawn_info.status.current_hp = npc_mob->status.current_hp;
			spawn_info.status.current_mp = npc_mob->status.current_mp;
		}
	}

	send_one_message((unsigned char*) &spawn_info, xlen(&spawn_info), send_index);
}

void
get_guild_zone(struct mob_server_st mob, short *position_x, short *position_y)
{
	if (mob.mob.status.level <= 40 && mob.mob.class_master == CLASS_MORTAL) {
		*position_x = 2112;
		*position_y = 2042;
		return;
	}

	struct guildzone_st zone = guild_zone[mob.mob.info.city_id];

	*position_x = zone.city_x + (rand() % 8);
	*position_y = zone.city_y + (rand() % 8);
}

short
get_item_ability(struct item_st *item, int effect)
{
	int result = 0;

	//CheckItemIndex(item->item_id, return result);

	int unique = item_list[item->item_id].unique;
	int pos = item_list[item->item_id].pos;

	if (effect == EF_DAMAGEADD || effect == EF_MAGICADD)
	if (unique < 41 || unique > 50)
		return 0;

	if (effect == EF_CRITICAL)
	if (item->effect[1].index == EF_CRITICAL2 || item->effect[2].index == EF_CRITICAL2)
		effect = EF_CRITICAL2;

	if (effect == EF_DAMAGE && pos == 32)
	if (item->effect[1].index == EF_DAMAGE2 || item->effect[2].index == EF_DAMAGE2)
		effect = EF_DAMAGE2;

	if (effect == EF_MPADD)
	if (item->effect[1].index == EF_MPADD2 || item->effect[2].index == EF_MPADD2)
		effect = EF_MPADD2;

	if (effect == EF_ACADD)
	if (item->effect[1].index == EF_ACADD2 || item->effect[2].index == EF_ACADD2)
		effect = EF_ACADD2;
	/*//
	if(effect == EF_LEVEL && item->item_id >= 2330 && item->item_id < 2360)
	result = (item->effect[1].index - 1);
	else if(effect == EF_LEVEL)
	result += ItemList[item->item_id].Level;
	*/
	if (effect == EF_REQ_STR)
		result += item_list[item->item_id].strength;
	if (effect == EF_REQ_INT)
		result += item_list[item->item_id].intelligence;
	if (effect == EF_REQ_DEX)
		result += item_list[item->item_id].dexterity;
	if (effect == EF_REQ_CON)
		result += item_list[item->item_id].constitution;

	if (effect == EF_POS)
		result += item_list[item->item_id].pos;

	if (effect != EF_INCUBATE) {
		for (size_t i = 0; i < 12; i++) {
			if (item_list[item->item_id].effect[i].index != effect)
				continue;

			int val = item_list[item->item_id].effect[i].value;
			if (effect == EF_ATTSPEED && val == 1)
				val = 10;

			result += val;
			break;
		}
	}

	if (item->item_id >= 2330 && item->item_id < 2390) {
		if (effect == EF_MOUNTHP)
			return item->effect[0].index;

		if (effect == EF_MOUNTSANC)
			return item->effect[1].index;

		if (effect == EF_MOUNTLIFE)
			return item->effect[1].value;

		if (effect == EF_MOUNTFEED)
			return item->effect[2].index;

		if (effect == EF_MOUNTKILL)
			return item->effect[2].value;

		if (item->item_id >= 2362 && item->item_id < 2390 && item->effect[0].index > 0) {
			int ef2 = item->effect[1].index;

			if (effect == EF_DAMAGE)
				return ((get_effect_value(item->item_id, EF_DAMAGE) * (ef2 + 20)) / 100);

			if (effect == EF_MAGIC)
				return ((get_effect_value(item->item_id, EF_MAGIC) * (ef2 + 15)) / 100);

			if (effect == EF_PARRY)
				return get_effect_value(item->item_id, EF_PARRY);

			if (effect == EF_RUNSPEED)
				return get_effect_value(item->item_id, EF_RUNSPEED);

			if (effect == EF_RESIST1 || effect == EF_RESIST2 ||
				effect == EF_RESIST3 || effect == EF_RESIST4)
				return get_effect_value(item->item_id, EF_RESISTALL);
		}

		return result;
	}

	for (size_t i = 0; i < 3; i++) {
		if (item->effect[i].index != effect)
			continue;

		int val = item->effect[i].value;
		if (effect == EF_ATTSPEED && val == 1)
			val = 10;

		result += val;
		break;
	}

	if (effect == EF_RESIST1 || effect == EF_RESIST2 || effect == EF_RESIST3 || effect == EF_RESIST4) {
		for (size_t i = 0; i < 12; i++) {
			if (item_list[item->item_id].effect[i].index != EF_RESISTALL)
				continue;

			result += item_list[item->item_id].effect[i].value;
			break;
		}

		for (size_t i = 0; i < 3; i++) {
			if (item->effect[i].index != EF_RESISTALL)
				continue;

			result += item->effect[i].value;
			break;
		}
	}

	int sanc = get_item_sanc(item);

	if (sanc > 9) {
		sanc = 10 + ((sanc - 230) / 4);
		if (sanc > 15)
			sanc = 15;
	}

	if (item->item_id <= 40)
		sanc = 0;

	if (sanc >= 9 && (pos & 0xF00) != 0)
		sanc++;

	if (sanc != 0 && effect != EF_GRID && effect != EF_CLASS &&
		effect != EF_POS && effect != EF_WTYPE && effect != EF_RANGE &&
		effect != EF_LEVEL && effect != EF_REQ_STR && effect != EF_REQ_INT &&
		effect != EF_REQ_DEX && effect != EF_REQ_CON && effect != EF_VOLATILE &&
		effect != EF_INCUBATE && effect != EF_INCUDELAY) {
		
		if (sanc <= 10) {
			result = (((sanc + 10) * result) / 10);
		} else {
			static int Power[] = {
				220, 250, 280, 320, 370
			};

			int val = Power[sanc - 11];
			result = ((((result * 10) * val) / 100) / 10);
		}
	}

	if (effect == EF_RUNSPEED) {
		if (result >= 3)
			result = 2;

		if (result > 0 && sanc >= 9)
			result++;
	}

	if (effect == EF_HWORDGUILD || effect == EF_LWORDGUILD) {
		int x = result;
		result = x;
	}

	if (effect == EF_GRID) {
		if (result < 0 || result > 7)
			result = 0;
	}
	
	return result;
}

// Chances de isso aqui ser de servidor PvP ou whatever
void
get_affect_score(struct mob_server_st *user)
{
	for (size_t x = 0; x < MAX_AFFECT; x++) {
		if (user->mob.affect[x].index <= 0)
			continue;

		if (user->mob.affect[x].index == BM_MUTACAO) {
			int mul = 1;
			if (user->mob.class_master <= CLASS_ARCH) mul = 2;
			else if (user->mob.class_master >= CLASS_CELESTIAL) mul = 5;
			int base_m = (user->mob.affect[x].value / 10);
			if (base_m == 0) base_m = 1;
			int calc = (((user->mob.status.level * mul) / 10) * base_m);
			user->mob.status.attack += ((calc / 5) * 2);
			user->mob.status.defense += ((calc / 5) * 2);
			user->mob.critical += (((calc / 100) * 5) / 4);
			user->mob.atack_speed += ((calc / 100) / 0.375);
			user->mob.evasion += 5;
			user->mob.regen_hp += 1 * user->mob.affect[x].master;
			user->mob.regen_mp += 1 * user->mob.affect[x].master;
			user->mob.status.speed += 1;
			user->mob.status.max_mp += calc;
			user->mob.status.max_hp += calc;
			switch (user->mob.affect[x].master)
			{
			case LOBISOMEM:
				user->mob.status.attack *= 1.3;
				user->mob.critical += 22;
				user->mob.evasion += 5;
				break;
			case URSO:
				user->mob.status.defense *= 1.3;
				user->mob.status.max_hp *= 1.25;
				break;
			case ASTAROTH:
				user->mob.status.attack *= 1.15;
				user->mob.status.defense *= 1.1;
				user->mob.critical += 10;
				break;
			case TITA:
				user->mob.status.attack *= 1.1;
				user->mob.status.defense *= 1.2;
				user->mob.critical += 6;
				user->mob.status.max_hp *= 1.1;
				break;
			case EDEN:
				user->mob.status.attack *= 1.22;
				user->mob.status.defense *= 1.11;
				user->mob.critical += 15;
				user->mob.status.max_hp *= 1.15;
				break;
			}
		}
		else
		{
			int Calc = ((user->mob.affect[x].value / 10) + 100);
			if (Calc < 110) Calc = 110;
			int Calc2 = (user->mob.affect[x].value / 10);
			if (Calc2 < 10) Calc2 = 10;
			int fC = ((user->mob.status.f_master * Calc) / 100);
			int wC = ((user->mob.status.w_master * Calc) / 100);
			int sC = ((user->mob.status.s_master * Calc) / 100);
			int tC = ((user->mob.status.t_master * Calc) / 100);
			switch (user->mob.affect[x].index)
			{
			case TK_POSSUIDO:
				user->mob.status.constitution = ((user->mob.status.constitution * Calc) / 100);
				user->mob.status.max_hp += (Calc2 * 40);
				break;

			case TK_SAMARITANO:
				user->mob.status.defense = ((user->mob.status.defense * Calc) / 100);
				break;

			case TK_ASSALTO:
				user->mob.status.attack = ((user->mob.status.attack * Calc) / 100);
				break;

			case TK_AURAVIDA:
				user->mob.regen_hp = ((user->mob.regen_hp * Calc) / 100);
				user->mob.regen_mp = ((user->mob.regen_mp * Calc) / 100);
				break;

			case FM_ESCUDO_MAGICO:
				user->mob.status.defense = ((user->mob.status.defense * Calc) / 100);
				break;

			case FM_SKILLS:
				if (fC > 255) fC = 255;
				if (wC > 255) wC = 255;
				if (sC > 255) sC = 255;
				if (tC > 255) tC = 255;
				user->mob.status.f_master = fC;
				user->mob.status.w_master = wC;
				user->mob.status.s_master = sC;
				user->mob.status.t_master = tC;
				break;

			case FM_VELOCIDADE:
				user->mob.status.speed += 1;
				break;

			case FM_BUFFATK:
				user->mob.status.attack = ((user->mob.status.attack * Calc) / 100);
				break;

			case HT_EVASAO_APRIMORADA:
				user->mob.evasion += 12;
				break;

			case HT_MEDITACAO:
				user->mob.status.attack = ((user->mob.status.attack * (Calc + 6)) / 100);
				user->mob.status.defense *= 0.85;
				break;

			case HT_IMUNIDADE:
				int Resist[4];
				for (size_t x = 0; x < 4; x++)
				{
					Resist[x] = ((user->mob.resist[x] * Calc) / 100);
					if (Resist[x] > 150) Resist[x] = 150;
					user->mob.resist[x] = Resist[x];
				}
				break;

			case HT_LIGACAO_ESPCTRAL:
				user->mob.status.attack *= 1.1;
				break;

			case HT_ESCUDO_DOURADO:
				user->mob.status.defense *= 1.15;
				break;

			case HT_TROCAESP:
				user->mob.status.max_hp = ((user->mob.status.max_hp * Calc) / 100);
				user->mob.status.max_mp = ((user->mob.status.max_mp * (100 - (Calc - 100))) / 100);
				break;

			case LENTIDAO:
				user->mob.status.speed -= 1;
				break;

			case RESISTENCIA_N:
				for (size_t x = 0; x < 4; x++)
					user->mob.resist[x] *= 0.8;
				break;

			case VENENO:
				user->mob.drain_hp = (((user->mob.status.max_hp * 2) / 100));
				break;

			case ATKMENOS:
				user->mob.status.attack *= 0.75;
				break;

			case COMIDA:
				user->mob.status.attack *= 1.1;
				user->mob.magic_increment *= 1.1;
				if (user->mob.magic_increment > 500) user->mob.magic_increment = 500;
				user->mob.status.defense *= 1.1;
				break;

			case PvM:
				user->bonus_pvm = ((user->mob.status.attack * 10) / 100);
				user->mob.magic_increment *= 1.1;
				break;

			case POCAO_ATK:
				if (user->mob.affect[x].master == 1)
				{
					user->mob.status.attack *= 1.1;
					user->mob.status.speed += 2;
				}
				else if (user->mob.affect[x].master == 2)
				{
					user->mob.status.attack *= 1.15;
				}
				else if (user->mob.affect[x].master == 3)
				{
					user->mob.status.attack *= 1.25;
				}
				else if (user->mob.affect[x].master == 4)
				{
					user->mob.status.max_hp *= 1.1;
					user->mob.status.max_mp *= 1.1;
				}
				break;

			case BONUS_HP_MP:
				user->mob.status.max_hp *= 1.2;
				user->mob.status.max_mp *= 1.2;
				break;
			}
		}
	}

	if (user->mob.class_info == 0)
	{
		int Mestre_Armas = (9 % 24);
		if ((user->mob.learn & (1 << Mestre_Armas)) != 0)
		{
			if (user->mob.equip[6].item_id != 0 && user->mob.equip[7].item_id != 0)
				user->mob.status.attack += ((user->mob.status.s_master * 3) / 2);
		}
		int Nocao_Combate = (14 % 24);
		if ((user->mob.learn & (1 << Nocao_Combate)) != 0)
		{
			user->mob.status.attack += (user->mob.status.s_master / 2);
		}
		int Armadura_critica = (15 % 24);
		if ((user->mob.learn & (1 << Armadura_critica)) != 0)
		{
			user->mob.status.defense += (user->mob.status.s_master * 3);
			user->mob.critical += 30;
		}
	}
	else if (user->mob.class_info == 2)
	{
		int Escudo = get_item_ability(&user->mob.equip[6], 87);
		int Escudo2 = get_item_ability(&user->mob.equip[7], 87);
		int Armadura_Elemental = (65 % 24);
		if ((user->mob.learn & (1 << Armadura_Elemental)) != 0)
		{
			user->mob.absorption += 10;
			if (Escudo > 0 || Escudo2 > 0)
				user->mob.absorption += 7;
		}
		int Escudo_Tormento = (67 % 24);
		if ((user->mob.learn & (1 << Escudo_Tormento)) != 0)
		{
			user->mob.absorption += 5;
			if (Escudo > 0 || Escudo2 > 0)
				user->mob.status.defense += ((user->mob.status.t_master * 3) / 2);
		}
	}
	else if (user->mob.class_info == 3)
	{
		int Agressividade = (74 % 24);
		if ((user->mob.learn & (1 << Agressividade)) != 0)
		{
			user->mob.status.attack += ((user->mob.status.f_master * 3) / 2);
		}
		int Pericia_Cacador = (82 % 24);
		if ((user->mob.learn & (1 << Pericia_Cacador)) != 0)
		{
			if (user->mob.equip[6].item_id != 0 && user->mob.equip[7].item_id == 0)
				user->mob.status.attack += ((user->mob.status.s_master * 3) / 2);
		}
		int Visao_Cacadora = (90 % 24);
		if ((user->mob.learn & (1 << Visao_Cacadora)) != 0)
		{
			user->mob.critical += (user->mob.status.dexterity / 50);
		}
		int Lamina_Aerea = (93 % 24);
		if ((user->mob.learn & (1 << Lamina_Aerea)) != 0)
		{
			user->mob.status.attack += 150;
		}
		int ProtecaoSombras = (94 % 24);
		if ((user->mob.learn & (1 << ProtecaoSombras)) != 0)
		{
			user->mob.status.defense += ((user->mob.status.t_master * 3) / 2);
		}
	}
}

void
get_current_score(int user_index)
{
	struct mob_st *user = &mobs[user_index].mob;

	if (user_index >= BASE_MOB) {
		get_affect_score(&mobs[user_index]);
		return;
	}

	mobs[user_index].bonus_pvm = 0;
	int special[4] = { 0 };
	int special_all = 0;

	special[0] = user->b_status.w_master;
	special[1] = user->b_status.f_master;
	special[2] = user->b_status.s_master;
	special[3] = user->b_status.t_master;

	special_all = get_mob_ability(user, EF_SPECIALALL);
	special[0] += get_mob_ability(user, EF_SPECIAL1);
	special[1] += get_mob_ability(user, EF_SPECIAL2) + special_all;
	special[2] += get_mob_ability(user, EF_SPECIAL3) + special_all;
	special[3] += get_mob_ability(user, EF_SPECIAL4) + special_all;

	for (size_t i = 0; i < 4; i++) {
		if (special[i] > 255)
			special[i] = 255;
	}
	
	user->status.w_master = special[0];
	user->status.f_master = special[1];
	user->status.s_master = special[2];
	user->status.t_master = special[3];

	int resist = 0;
	if (get_item_sanc(&user->equip[HELM_SLOT]) >= 9)
		resist = 30;

	user->resist[0] = get_mob_ability(user, EF_RESIST1) + resist;
	user->resist[1] = get_mob_ability(user, EF_RESIST2) + resist;
	user->resist[2] = get_mob_ability(user, EF_RESIST3) + resist;
	user->resist[3] = get_mob_ability(user, EF_RESIST4) + resist;

	if (user->resist[0] > 150)
		user->resist[0] = 150;
	if (user->resist[1] > 150)
		user->resist[1] = 150;
	if (user->resist[2] > 150)
		user->resist[2] = 150;
	if (user->resist[3] > 150)
		user->resist[3] = 150;

	int magic = (((user->b_status.level + 1) / 11) * 2);
	magic += get_mob_ability(user, EF_MAGIC) >> 1;
	if (magic > 300)
		magic = 300;

	user->magic_increment = magic;

	int critical = ((get_mob_ability(user, EF_CRITICAL) / 10) * 5);
	if (critical > 255)
		critical = 255;

	user->critical = critical;

	if (user->regen_hp < 82)
		user->regen_hp = 82;

	if (user->regen_mp < 82)
		user->regen_mp = 82;

	user->regen_hp += get_mob_ability(user, EF_REGENHP);
	user->regen_mp += get_mob_ability(user, EF_REGENMP);
	user->save_mana = get_mob_ability(user, EF_SAVEMANA);

	if (user->regen_hp < 82)
		user->regen_hp = 82;

	if (user->regen_mp < 82)
		user->regen_mp = 82;

	user->status.strength = user->b_status.strength;
	user->status.intelligence = user->b_status.intelligence;
	user->status.dexterity = user->b_status.dexterity;
	user->status.constitution = user->b_status.constitution;
	user->status.strength += get_mob_ability(user, EF_STR);
	user->status.intelligence += get_mob_ability(user, EF_INT);
	user->status.dexterity += get_mob_ability(user, EF_DEX);
	user->status.constitution += get_mob_ability(user, EF_CON);

	int evasion = (user->status.dexterity / 62);
	if (evasion > 50)
		evasion = 50;

	user->evasion = evasion;

	user->status.level = user->b_status.level;
	user->status.merchant = user->b_status.merchant;

	int chaos = user->inventory[63].effect[0].index;
	user->b_status.ChaosRate = chaos / 10;
	user->status.ChaosRate = chaos / 10;

	int moves = 2;

	user->affect_info.speed_mov = 0;

	if (user->equip[5].item_id > 0)
		moves += 1;

	if (user->equip[14].item_id > 0)
		moves = 6;

	user->status.speed = moves;

	user->status.max_hp = user->b_status.max_hp;
	user->status.max_mp = user->b_status.max_mp;
	user->status.max_hp += get_current_hp(user_index);
	user->status.max_mp += get_current_mp(user_index);

	if (user->status.max_hp < user->status.current_hp)
		user->status.current_hp = user->status.max_hp;

	if (user->status.max_mp < user->status.current_mp)
		user->status.current_mp = user->status.max_mp;

	double AtkSpeed = 0;
	AtkSpeed += (user->status.dexterity / 8);
	AtkSpeed += get_mob_ability(user, EF_ATTSPEED) / 0.375;

	if (AtkSpeed > 300)
		AtkSpeed = 300;

	user->atack_speed = ((AtkSpeed * 2) / 3);

	int def_inc = user->b_status.defense;
	int atk_inc = user->b_status.attack;

	// TODO: Trocar para switch
	if (user->class_info == 0) {
		atk_inc += get_mob_ability(user, EF_DAMAGE);
		atk_inc += get_mob_ability(user, EF_DAMAGEADD);
		atk_inc += ((user->status.strength / 5) * 2);
		atk_inc += ((user->status.dexterity / 5) * 1);
		atk_inc += user->status.w_master;
		atk_inc += ((user->status.level * 3) / 2);
		def_inc += get_mob_ability(user, EF_AC);
		def_inc += get_mob_ability(user, EF_ACADD);
		def_inc += (user->status.level * 2);
		def_inc += ((user->status.max_hp / 100) * 5);
		def_inc = ((def_inc * 110) / 100);
	} else if (user->class_info == 1) {
		atk_inc += get_mob_ability(user, EF_DAMAGE);
		atk_inc += get_mob_ability(user, EF_DAMAGEADD);
		atk_inc += ((user->status.strength / 5) * 2);
		atk_inc += ((user->status.dexterity / 4) * 1);
		atk_inc += user->status.w_master;
		atk_inc += ((user->status.level * 3) / 2);
		atk_inc = ((atk_inc * 105) / 100);
		def_inc += get_mob_ability(user, EF_AC);
		def_inc += get_mob_ability(user, EF_ACADD);
		def_inc += (user->status.level * 2);
	} else if (user->class_info == 2) {
		atk_inc += get_mob_ability(user, EF_DAMAGE);
		atk_inc += get_mob_ability(user, EF_DAMAGEADD);
		atk_inc += ((user->status.strength / 4) * 2);
		atk_inc += ((user->status.dexterity / 5) * 2);
		atk_inc += user->status.w_master;
		atk_inc += ((user->status.level * 4) / 3);
		atk_inc = ((atk_inc * 110) / 100);
		def_inc += get_mob_ability(user, EF_AC);
		def_inc += get_mob_ability(user, EF_ACADD);
		def_inc += (user->status.level * 2);
		def_inc += ((user->status.max_hp / 100) * 2);
	} else if (user->class_info == 3) {
		atk_inc += get_mob_ability(user, EF_DAMAGE);
		atk_inc += get_mob_ability(user, EF_DAMAGEADD);
		atk_inc += ((user->status.strength / 4) * 1);
		atk_inc += ((user->status.dexterity / 6) * 3);
		atk_inc += user->status.w_master;
		atk_inc += ((user->status.level * 3) / 2);
		atk_inc = ((atk_inc * 120) / 100);
		def_inc += get_mob_ability(user, EF_AC);
		def_inc += get_mob_ability(user, EF_ACADD);
		def_inc += (user->status.level * 2);
		def_inc += ((user->status.max_hp / 100) * 1);
	}

	int w1 = get_item_ability(&user->equip[ARMA1_SLOT], EF_DAMAGE);
	int w2 = get_item_ability(&user->equip[ARMA2_SLOT], EF_DAMAGE);
	if (w1 > w2)
		user->weapon_damage = w1 + (w2 / 3);
	else
		user->weapon_damage = w2 + (w1 / 3);

	//00412688
	int right_weapon_id = user->equip[ARMA1_SLOT].item_id;
	if (right_weapon_id >= 0 || right_weapon_id < MAX_ITEM_LIST) {
		int sanc = get_item_sanc(&user->equip[ARMA1_SLOT]);

		if (sanc >= 230)
			sanc = (10 + ((sanc - 230) / 4));

		if (sanc >= 9) {
			user->weapon_damage += (40 * (sanc - 8));
			atk_inc += (40 * (sanc - 8));
		}
	}

	//004126F3
	int left_weapon_id = user->equip[ARMA2_SLOT].item_id;
	if (left_weapon_id >= 0 || left_weapon_id < MAX_ITEM_LIST) {
		int sanc = get_item_sanc(&user->equip[ARMA2_SLOT]);
		
		if (sanc >= 230) 
			sanc = (10 + ((sanc - 230) / 4));
		
		if (sanc >= 9) {
			user->weapon_damage += (40 * (sanc - 8));
			atk_inc += (40 * (sanc - 8));
		}		
	}

	for (size_t i = 1; i < 5; i++) {
		int sanc = get_item_sanc(&user->equip[i]);
		
		if (sanc >= 230)
			sanc = (10 + ((sanc - 230) / 4));
		
		if (sanc >= 9) {
			int di = (25 * (sanc - 8));
			def_inc += di;
		}
	}
	user->status.attack = atk_inc;
	user->status.defense = def_inc;
	int perfu = 0;
	int absorb = 0;
	for (size_t i = 1; i < 16; i++) {
		int item_id = user->equip[i].item_id;

		if (item_id <= 0)
			continue;

		int sanc = get_item_sanc(&user->equip[i]);
		if (sanc == 231 || sanc == 235 || sanc == 239 || sanc == 243 || sanc == 247 || sanc == 251) {
			// int v = sanc + 4;
			perfu += (sanc * 40);
		}

		if (sanc == 233 || sanc == 237 || sanc == 241 || sanc == 245 || sanc == 249 || sanc == 253) {
			// int v = sanc + 4;
			absorb += (sanc * 40);
		}

		if (i == 6 || i == 7) {
			if (item_id >= 2451 && item_id <= 2938) {
				int s = 0;
				for (size_t y = 801; y < 1001; y++) {
					// TODO: Trocar por switch
					if (item_list[y].extreme == item_id)
						s = 1;
					else if (item_list[y].extreme == (item_id - 1))
						s = 2;
					else if (item_list[y].extreme == (item_id - 2))
						s = 3;
					else if (item_list[y].extreme == (item_id - 3))
						s = 4;

					if (s == 2)
						perfu += 40;
					else if (s == 4)
						absorb += 40;

					if (s != 0)
						break;
				}
			} else if (item_id >= 3601 && item_id <= 3788) {
				int s = 0;
				for (size_t y = 1901; y < 1912; y++) {
					// TODO: Trocar por switch
					if (item_list[y].extreme == item_id)
						s = 1;
					else if (item_list[y].extreme == (item_id - 1))
						s = 2;
					else if (item_list[y].extreme == (item_id - 2))
						s = 3;
					else if (item_list[y].extreme == (item_id - 3))
						s = 4;

					if (s == 2)
						perfu += 40;
					else if (s == 4)
						absorb += 40;

					if (s != 0)
						break;
				}
			}
		}
	}

	if (user->class_master >= CLASS_ARCH) {
		perfu += 40;
		absorb += 40;
	}

	if (perfu > 0)
		user->perforation = perfu / 2;
	else
		user->perforation = 0;

	if (absorb > 0)
		user->absorption = absorb / 2;
	else
		user->absorption = 0;


	static const struct mount_info_st base_mount_info[30] = {
		{ 0, 0, 0, 0 },{ 0, 0, 0, 0 },{ 10, 1, 0, 0 },{ 16, 2, 0, 0 },{ 20, 3, 0, 0 },
		{ 30, 0, 0, 0 },{ 50, 7, 4, 0 },{ 60, 9, 5, 0 },{ 70, 9, 6, 0 },{ 80, 10, 7, 0 },
		{ 100, 12, 8, 0 },{ 50, 7, 0, 16 },{ 60, 9, 0, 20 },{ 70, 9, 0, 24 },{ 80, 10, 0, 28 },
		{ 100, 12, 0, 32 },{ 110, 13, 0, 0 },{ 120, 13, 0, 0 },{ 110, 13, 0, 20 },{ 130, 15, 6, 28 },
		{ 140, 16, 8, 32 },{ 114, 13, 2, 16 },{ 114, 13, 3, 8 },{ 114, 13, 4, 12 },{ 118, 14, 3, 20 },
		{ 120, 14, 4, 16 },{ 120, 14, 5, 16 },{ 120, 6, 6, 28 },{ 60, 14, 6, 28 },{ 30, 3, 0, 20 }
	};

	if (user->equip[MOUNT_SLOT].item_id >= 2360 && user->equip[MOUNT_SLOT].item_id <= 2389) {
		int mount_id = user->equip[MOUNT_SLOT].item_id - 2360;
		int mount_level = user->equip[MOUNT_SLOT].EF2;
		float bdano = (base_mount_info[mount_id].damage / 20);
		float rdano = (bdano * mount_level) + base_mount_info[mount_id].damage;
		float bmagic = (base_mount_info[mount_id].magic / 14.5);
		float rmagic = (bmagic * mount_level) + base_mount_info[mount_id].magic;
		user->status.attack += (int)rdano;
		user->magic_increment += (int)((int)rmagic >> 1);
		if (user->magic_increment >= 300)
			user->magic_increment = 300;

		for (size_t x = 0; x < 4; x++) {
			int resist = (user->resist[x] + (base_mount_info[mount_id].immunity / 4));
			if (resist > 150)
				resist = 150;

			user->resist[x] = resist;
		}

		user->evasion += base_mount_info[mount_id].evasion >> 1;
	}

	get_affect_score(&mobs[user_index]);
}

bool
get_bonus_skill_points(struct mob_st *user)
{
	int pnts = 0;
	if (user->class_master == CLASS_MORTAL) {
		pnts = (user->status.level * 3);
		int level = user->status.level - 199;
		if (level > 0)
			pnts += level;
	} else if (user->class_master == CLASS_ARCH) {
		pnts = (user->status.level * 4);
		int level = user->status.level - 354;

		if (level > 0)
			pnts += level;
	} else if (user->class_master == CLASS_CELESTIAL || user->class_master == CLASS_SUB_CELESTIAL || user->class_master == CLASS_HARDCORE) {
		pnts = 1500;
		pnts += (user->status.level * 4);
	}

	if ((user->quest_info & (1 << (QUEST_MAGIC_BEAN))) != 0)
		pnts += 9;

	short classid = user->class_info;
	if (classid < 0 || classid > 3)
		return false;

	int pts = 0, next = 1;
	for (size_t i = 0; i < 24; i++) {
		if ((user->learn & next) != 0)
			pts += skill_data[((classid * 24) + i)].points;

		next <<= 1;
	}

	int skill = pnts - pts;
	if (skill != 0) {
		user->p_skill = skill;
		return false;
	}

	return true;
}

// Simplesmente ininteligível
int 
get_mob_ability(struct mob_st *user, int effect)
{
	int LOCAL_1 = 0;

	if (effect == EF_RANGE)
	{
		LOCAL_1 = get_max_ability(user, effect);

		int LOCAL_2 = (user->equip[0].item_id / 10);
		if (LOCAL_1 < 2 && LOCAL_2 == 3)
		if ((user->learn & 0x100000) != 0)
			LOCAL_1 = 2;

		return LOCAL_1;
	}

	int LOCAL_18[16];
	for (size_t LOCAL_19 = 0; LOCAL_19 < 16; LOCAL_19++)
	{
		LOCAL_18[LOCAL_19] = 0;

		int LOCAL_20 = user->equip[LOCAL_19].item_id;
		if (LOCAL_20 == 0 && LOCAL_19 != 7)
			continue;

		if (LOCAL_19 >= 1 && LOCAL_19 <= 5)
			LOCAL_18[LOCAL_19] = item_list[LOCAL_20].unique;

		if (effect == EF_DAMAGE && LOCAL_19 == 6)
			continue;

		if (effect == EF_MAGIC && LOCAL_19 == 7)
			continue;

		if (LOCAL_19 == 7 && effect == EF_DAMAGE) {
			int dam1 = (get_item_ability(&user->equip[6], EF_DAMAGE) + get_item_ability(&user->equip[6], EF_DAMAGE2));
			int dam2 = (get_item_ability(&user->equip[7], EF_DAMAGE) + get_item_ability(&user->equip[7], EF_DAMAGE2));

			int arm1 = user->equip[6].item_id;
			int arm2 = user->equip[7].item_id;

			int unique1 = 0;
			if (arm1 > 0 && arm1 < MAX_ITEM_LIST)
				unique1 = item_list[arm1].unique;

			int unique2 = 0;
			if (arm2 > 0 && arm2 < MAX_ITEM_LIST)
				unique2 = item_list[arm2].unique;

			if (unique1 != 0 && unique2 != 0) {
				int porc = 0;
				if (unique1 == unique2)
					porc = 30;
				else
					porc = 20;

				if (dam1 > dam2)
					LOCAL_1 = ((LOCAL_1 + dam1) + ((dam2 * porc) / 100));
				else
					LOCAL_1 = ((LOCAL_1 + dam2) + ((dam1 * porc) / 100));

				continue;
			}

			if (dam1 > dam2)
				LOCAL_1 += dam1;
			else
				LOCAL_1 += dam2;

			continue;
		}

		int LOCAL_28 = get_item_ability(&user->equip[LOCAL_19], effect);
		if (effect == EF_ATTSPEED && LOCAL_28 == 1)
			LOCAL_28 = 10;

		LOCAL_1 += LOCAL_28;
	}

	if (effect == EF_AC && LOCAL_18[1] != 0) {
		if (LOCAL_18[1] == LOCAL_18[2] && LOCAL_18[2] == LOCAL_18[3] && LOCAL_18[3] == LOCAL_18[4] && LOCAL_18[4] == LOCAL_18[5])
			LOCAL_1 = ((LOCAL_1 * 105) / 100);
	}

	return LOCAL_1;
}

int
get_max_ability(struct mob_st *user, int effect)
{
	int max_ability = 0;

	for (size_t i = 0; i < 16; i++) {
		if (user->equip[i].item_id == 0)
			continue;

		short item_ability = get_item_ability(&user->equip[i], effect);

		if (max_ability < item_ability)
			max_ability = item_ability;
	}

	return max_ability;
}

short
get_item_sanc(struct item_st *item)
{
	int value = 0;

	if (item->item_id >= 2360 && item->item_id <= 2389) {
		//Montarias.
		value = (item->effect[2].index / 10);

		if (value > 9)
			value = 9;

		return value;
	}

	if (item->item_id >= 2330 && item->item_id <= 2359) {
		//Crias.
		return 0;
	}

	if (item->effect[0].index == 43 || (item->effect[0].index >= 116 && item->effect[0].index <= 125))
		value = item->effect[0].value;
	else if (item->effect[1].index == 43 || (item->effect[1].index >= 116 && item->effect[1].index <= 125))
		value = item->effect[1].value;
	else if (item->effect[2].index == 43 || (item->effect[2].index >= 116 && item->effect[2].index <= 125))
		value = item->effect[2].value;

	if (value >= 230)
	{

	}
	else
		value %= 10;

	return value;
}

int
get_bonus_score_points(struct mob_st *user)
{
	short classindex = user->class_info;
	if (classindex < 0 || classindex > 3)
		return false;

	if (user->class_master == CLASS_MORTAL) {
		int str = user->b_status.strength - BaseSIDCHM[classindex][0];
		int _int = user->b_status.intelligence - BaseSIDCHM[classindex][1];
		int dex = user->b_status.dexterity - BaseSIDCHM[classindex][2];
		int con = user->b_status.constitution - BaseSIDCHM[classindex][3];
		int total = str + _int + dex + con;
		int level = user->b_status.level;

		int pnts = level * 5;
		if (level >= 254)
			pnts += (level - 254) * 3;

		pnts -= total;
		if (pnts < -32000)
			pnts = -32000;
		else if (pnts > 32000)
			pnts = 32000;

		user->p_status = pnts;
	} else if (user->class_master == CLASS_ARCH) {
		int str = user->b_status.strength - 15;
		int _int = user->b_status.intelligence - 15;
		int dex = user->b_status.dexterity - 15;
		int con = user->b_status.constitution - 15;
		int total = str + _int + dex + con;
		int level = user->b_status.level;

		int pnts = 500;
		pnts += level * 6;
		if (level >= 299)
			pnts += (level - 254) * 4;

		pnts -= total;
		if (pnts < -32000)
			pnts = -32000;
		else if (pnts > 32000)
			pnts = 32000;

		user->p_status = pnts;
	} else if (user->class_master == CLASS_CELESTIAL) {
		int str = user->b_status.strength - 15;
		int _int = user->b_status.intelligence - 15;
		int dex = user->b_status.dexterity - 15;
		int con = user->b_status.constitution - 15;
		int total = str + _int + dex + con;
		int level = user->b_status.level;

		int pnts = 1001;

		if (user->arch_level >= 355 && user->arch_level <= 369)
			pnts += 100;
		else if (user->arch_level >= 370 && user->arch_level <= 379)
			pnts += 300;
		else if (user->arch_level >= 380 && user->arch_level <= 397)
			pnts += 600;
		else if (user->arch_level == 398)
			pnts += 900;
		else if (user->arch_level >= 399)
			pnts += 1200;

		pnts += level * 6;

		if (level >= 100)
			pnts += (level - 100) * 5;

		pnts -= total;
		if (pnts < -32000)
			pnts = -32000;
		else if (pnts > 32000)
			pnts = 32000;

		user->p_status = pnts;
	} else if (user->class_master == CLASS_SUB_CELESTIAL) {
		int str = user->b_status.strength - 15;
		int _int = user->b_status.intelligence - 15;
		int dex = user->b_status.dexterity - 15;
		int con = user->b_status.constitution - 15;
		int total = str + _int + dex + con;
		int level = user->b_status.level;

		int pnts = 1001;

		pnts += user->cele_level * 16;

		if (user->cele_level >= 120 && user->cele_level <= 139)
			pnts += 250;
		else if (user->cele_level >= 140 && user->cele_level <= 179)
			pnts += 450;
		else if (user->cele_level >= 180 && user->cele_level <= 197)
			pnts += 700;
		else if (user->cele_level == 198)
			pnts += 1000;
		else if (user->cele_level >= 199)
			pnts += 1400;

		pnts += level * 6;

		if (level >= 100)
			pnts += (level - 100) * 6;

		pnts -= total;
		if (pnts < -32000)
			pnts = -32000;
		else if (pnts > 32000)
			pnts = 32000;

		user->p_status = pnts;
	} else if (user->class_master == CLASS_HARDCORE) {
		int str = user->b_status.strength - 100;
		int _int = user->b_status.intelligence - 100;
		int dex = user->b_status.dexterity - 100;
		int con = user->b_status.constitution - 100;
		int total = str + _int + dex + con;
		int level = user->b_status.level;

		int pnts = 2002;

		pnts += user->arch_level * 14;

		if (user->arch_level >= 355 && user->arch_level <= 369)
			pnts += 300;
		else if (user->arch_level >= 370 && user->arch_level <= 379)
			pnts += 500;
		else if (user->arch_level >= 380 && user->arch_level <= 397)
			pnts += 800;
		else if (user->arch_level == 398)
			pnts += 1200;
		else if (user->arch_level >= 399)
			pnts += 1600;

		pnts += level * 7;

		if (level >= 100)
			pnts += (level - 100) * 7;

		pnts -= total;
		if (pnts < -32000)
			pnts = -32000;
		else if (pnts > 32000)
			pnts = 32000;

		user->p_status = pnts;
	}

	return true;
}

void
get_bonus_master_points(struct mob_st *user)
{
	int pnts = 0;

	// TODO: Trocar por switch
	if (user->class_master == CLASS_MORTAL)
		pnts = (user->status.level * 2);
	else if (user->class_master == CLASS_ARCH)
		pnts = (user->status.level * 3);
	else if (user->class_master == CLASS_CELESTIAL || user->class_master == CLASS_SUB_CELESTIAL || user->class_master == CLASS_HARDCORE) {
		pnts = 855;
		pnts += (user->status.level * 2);
	}

	pnts -= user->b_status.w_master;
	pnts -= user->b_status.f_master;
	pnts -= user->b_status.s_master;
	pnts -= user->b_status.t_master;
	user->p_master = pnts;
}

int
get_current_hp(int user_index)
{
	struct mob_st user = mobs[user_index].mob;

	static const int HPIncrementPerLevel[4] = {
		4, // Transknight
		1, // Foema
		2, // BeastMaster
		1  // Hunter
	};

	int hp_inc = get_mob_ability(&user, EF_HP) + 100;
	int hp_perc = get_mob_ability(&user, EF_HPADD);

	hp_inc += BaseSIDCHM[user.class_info][4];
	hp_inc += (HPIncrementPerLevel[user.class_info] * user.status.level);
	if (user.class_info == 0)
		hp_inc += (user.status.constitution * 2);
	else if (user.class_info == 2)
		hp_inc += user.status.constitution;
	else
		hp_inc += ((user.status.constitution * 2) / 3);

	hp_inc += ((hp_inc * hp_perc) / 100);

	if (user.class_master == CLASS_ARCH)
		hp_inc += hp_inc / 4;
	else if (user.class_master >= CLASS_CELESTIAL)
		hp_inc += hp_inc / 2;

	return hp_inc;
}

int
get_current_mp(int user_index)
{
	struct mob_st user = mobs[user_index].mob;

	static const int MPIncrementPerLevel[4] = {
		1, // Transknight
		4, // Foema
		2, // BeastMaster
		1  // Hunter
	};

	int mp_inc = get_mob_ability(&user, EF_MP) + 100;
	int mp_perc = get_mob_ability(&user, EF_MPADD);

	mp_inc += BaseSIDCHM[user.class_info][5];
	mp_inc += (MPIncrementPerLevel[user.class_info] * user.status.level);
	if (user.class_info == 1)
		mp_inc += (user.status.intelligence << 1);
	else if (user.class_info == 2)
		mp_inc += (user.status.intelligence);
	else
		mp_inc += ((user.status.intelligence * 2) / 3);
	mp_inc += ((mp_inc * mp_perc) / 100);

	if (user.class_master == CLASS_ARCH)
		mp_inc += mp_inc / 4;
	else if (user.class_master >= CLASS_CELESTIAL)
		mp_inc += mp_inc / 2;

	return mp_inc;
}

int
check_pvp_area(int mob_index)
{
	struct mob_st mob = mobs[mob_index].mob;

	////MENOS CP / COM FRAG
	if (mob.current.X >= 3330 && mob.current.Y >= 1026 && mob.current.X <= 3600 && mob.current.Y <= 1660) return 2;//Area das Pistas de Runas
	else if (mob.current.X >= 2176 && mob.current.Y >= 1150 && mob.current.X <= 2304 && mob.current.Y <= 1534) return 2;//Area Campo Azran Quest Imp
	else if (mob.current.X >= 2446 && mob.current.Y >= 1850 && mob.current.X <= 2546 && mob.current.Y <= 1920) return 2;//Area Torre Erion 02
	else if (mob.current.X >= 1678 && mob.current.Y >= 1550 && mob.current.X <= 1776 && mob.current.Y <= 1906) return 2;//Area de Reinos
	else if (mob.current.X >= 1150 && mob.current.Y >= 1676 && mob.current.X <= 1678 && mob.current.Y <= 1920) return 2;//Area ca�a Noatun
	else if (mob.current.X >= 3456 && mob.current.Y >= 2688 && mob.current.X <= 3966 && mob.current.Y <= 3083) return 2;//Area ca�a Gelo
	else if (mob.current.X >= 3582 && mob.current.Y >= 3456 && mob.current.X <= 3968 && mob.current.Y <= 3710) return 2;//Area Lan House

	////SEM CP / SEM FRAG
	else if (mob.current.X >= 2602 && mob.current.Y >= 1702 && mob.current.X <= 2652 && mob.current.Y <= 1750) return 1;//Area Coliseu Azran
	else if (mob.current.X >= 2560 && mob.current.Y >= 1682 && mob.current.X <= 2584 && mob.current.Y <= 1716) return 1;//Area PVP Azran
	else if (mob.current.X >= 2122 && mob.current.Y >= 2140 && mob.current.X <= 2148 && mob.current.Y <= 2156) return 1;//Area PVP Armia
	else if (mob.current.X >= 136 && mob.current.Y >= 4002 && mob.current.X <= 200 && mob.current.Y <= 4088) return 1;//Area Duelo

	////SEM CP / COM FRAG
	else if (mob.current.X >= 2174 && mob.current.Y >= 3838 && mob.current.X <= 2560 && mob.current.Y <= 4096) return 3;//Area Kefra
	else if (mob.current.X >= 1076 && mob.current.Y >= 1678 && mob.current.X <= 1150 && mob.current.Y <= 1778) return 3;//Area Castelo Noatun
	else if (mob.current.X >= 1038 && mob.current.Y >= 1678 && mob.current.X <= 1076 && mob.current.Y <= 1702) return 3;//Area Castelo Noatun Altar
	else if (mob.current.X >= 2498 && mob.current.Y >= 1868 && mob.current.X <= 2516 && mob.current.Y <= 1896) return 3;//Area Torre Erion 01
	else if (mob.current.X >= 130 && mob.current.Y >= 140 && mob.current.X <= 248 && mob.current.Y <= 240) return 3;//Area Guerra entre Guildas

	return 0;
}

double
get_distance(struct position_st position_a, struct position_st position_b)
{
	int pos_x = position_b.X - position_a.X;
	int pos_y = position_b.Y - position_a.Y;

	pos_x *= pos_x;
	pos_y *= pos_y;

	return sqrt((double)(pos_x + pos_y));
}

int
get_door_type(struct position_st position)
{
	if ((position.X > 100 && position.X < 300) && (position.Y > 100 && position.Y < 300))
		return DOOR_CITY_WAR;
	else if ((position.X > 2592 && position.X < 2635) && (position.Y > 1700 && position.Y < 1750))
		return DOOR_COLISEUM;
	else if ((position.X > 2064 && position.X < 2160) && (position.Y > 1950 && position.Y < 2026))
		return DOOR_CAMP;
	else if ((position.X > 2474 && position.X < 2539) && (position.Y > 2095 && position.Y < 2156))
		return DOOR_QUEST_FJ;
	else if ((position.X > 2205 && position.X < 2300) && (position.Y > 1164 && position.Y < 1280))
		return DOOR_IMP_CASTEL;
	else if ((position.X > 1065 && position.X < 1200) && (position.Y > 1680 && position.Y < 1727))
		return DOOR_NOATUN;
	else
		return 0;
}

void
set_pk_points(int mob_index, int pk_points)
{
	if (pk_points < 1)
		pk_points = 1;
	else if (pk_points > 150)
		pk_points = 150;

	mobs[mob_index].mob.inventory[63].effect[0].index = pk_points;
}

unsigned int
get_exp_by_kill(unsigned int exp, int attacker_index, int target_index)
{
	struct mob_st *attacker = &mobs[attacker_index].mob;
	struct mob_st *target = &mobs[target_index].mob;

	if ((attacker->class_master == CLASS_CELESTIAL || attacker->class_master == CLASS_SUB_CELESTIAL && attacker->equip[CAPE_SLOT].item_id >= 3197 && attacker->equip[CAPE_SLOT].item_id <= 3199)
		|| (target->class_master == CLASS_CELESTIAL || target->class_master == CLASS_SUB_CELESTIAL && target->equip[CAPE_SLOT].item_id >= 3197 && target->equip[CAPE_SLOT].item_id <= 3199)) { //Celestial e Sub
		if (attacker->status.level >= MAX_LEVELCSH - 1 || target->status.level >= MAX_LEVELCSH - 1 || attacker->status.level < 0 || target->status.level < 0) {
			if (attacker->experience >= 4050000000) {
				attacker->experience = 4050000000;
				exp = 0;
			}

			return exp;
		}

		//Block Level
		if (attacker->status.level >= 39 && attacker->quest_info <= 0) {
			attacker->quest_info = 0;
			attacker->status.level = 39;
			attacker->experience = 780000000;
			exp = 0;
			send_client_string_message("Desbloqueie o level 40 para continuar upando.", attacker_index);
			return exp;
		} else if (attacker->status.level >= 89 && attacker->quest_info <= 1) {
			attacker->quest_info = 1;
			attacker->status.level = 89;
			attacker->experience = 1780000000;
			exp = 0;
			send_client_string_message("Desbloqueie o level 90 para continuar upando.", attacker_index);
			return exp;
		}
	} else { //Mortal e Arch
		if (attacker->status.level >= MAX_LEVEL - 1 || target->status.level >= MAX_LEVEL - 1 || attacker->status.level < 0 || target->status.level < 0) {
			if (attacker->experience >= 4050000000) { //Bloqueia a xp lvl 400 2/4
				attacker->experience = 4050000000;
				exp = 0;
			}

			return exp;
		}

		if (attacker->class_master == CLASS_ARCH) {
			if (attacker->status.level >= 354 && attacker->quest_info <= 0) {
				attacker->quest_info = 0;
				attacker->status.level = 354;
				attacker->experience = 2039000000;
				exp = 0;
				send_client_string_message("Desbloquei o level 355 para continuar upando.", attacker_index);
				return exp;
			}
		}
	}

	int attackerLevel = attacker->status.level;
	int targetLevel = target->status.level;
	attackerLevel++;
	targetLevel++;

	int multiexp = (targetLevel * 100) / attackerLevel;
	if (multiexp < 80 && attackerLevel >= 50)
		multiexp = (multiexp * 2) - 100;
	else if (multiexp > 200)
		multiexp = 200;

	if (multiexp < 0)
		multiexp = 0;

	exp = exp * multiexp / 100;

	for (size_t i = 0; i < MAX_AFFECT; i++) {
		if (attacker->affect[i].index == BAU_EXP) { // Bau de XP
			exp = exp * 2;
			break;
		}
	}

	//Fadas de XP
	switch (attacker->equip[PET_SLOT].item_id) {
	case 3900: //
	case 3903: //
	case 3906: // Fadas Verdes
	case 3911: //
	case 3912: //
	case 3913: //

	case 3902: //
	case 3905: // Fadas Vermelhas
	case 3908: //

	case 3914: // Fada Prateada
		exp = (exp * 116) / 100;
		break;
	case 3915: // Fada Dourada
		exp = (exp * 118) / 100;
		break;
	}

	if (attacker->class_master == CLASS_SUB_CELESTIAL)
		exp /= 3;
	else if (attacker->class_master == CLASS_CELESTIAL)
		exp /= 2;

	return exp;
}

int
get_item_slot(int user_index, int item_id, int type)
{
	struct mob_st *mob = &mobs[user_index].mob;

	if (type == INV_TYPE) {
		bool bkp1 = false;
		bool bkp2 = false;

		if (mob->inventory[60].item_id == 3467)
			bkp1 = true;

		if (mob->inventory[61].item_id == 3467)
			bkp2 = true;

		for (size_t x = 0; x < 60; x++) {
			if ((x >= 30 && x <= 44) && !bkp1)
				continue;

			if ((x >= 45 && x <= 59) && !bkp2)
				continue;

			if (mob->inventory[x].item_id == item_id)
				return x;
		}
	}

	if (type == EQUIP_TYPE) {
		for (size_t x = 0; x < 16; x++) {
			if (mob->equip[x].item_id == item_id)
				return x;
		}
	}

	if (type == STORAGE_TYPE) {
		for (size_t x = 0; x < 128; x++) {
			if (users[user_index].storage[x].item_id == item_id)
				return x;
		}
	}

	return -1;
}
