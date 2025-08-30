/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Atualização: Dezembro de 2023
 * Arquivo: core/user.c
 * Descrição: Arquivo onde são implementadas as funções que correspondem ao usuário em específico,
 *  como criar conta, login, logout, etc.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "../network/server.h"
#include "../network/packet-def.h"
#include "../network/socket-utils.h"
#include "utils.h"
#include "user.h"
#include "mob.h"
#include "world.h"
#include "base_functions.h"

bool
create_account(const char *user, const char *password)
{
	FILE *user_account = NULL;
	char file_path[1024] = { 0 };

	sprintf(file_path, "./bin/accounts/%s", user);

	if (access(file_path, F_OK) == 0) /* Conta já existente */
		return false;

	user_account = fopen(file_path, "w+b");

	if (user_account == NULL)
		return false;

	fclose(user_account);
	return true;
}

bool
search_logged_account(const char *user)
{
	for (size_t i = 0; i < MAX_USERS_PER_CHANNEL; i++) {
		if (strncmp(user, g_users[i].account_name, 16) == 0)
			return false;
	}

	return true;
}

bool
load_account(const char *user, const char *password, struct account_file_st *account, int user_index)
{
	FILE *user_account = NULL;
	char file_path[1024] = { 0 };

	sprintf(file_path, "./bin/accounts/%s", user); // TER EM MENTE QUE ./ SE REFERE AO DIRETÓRIO ATUAL DO TERMINAL, NÃO DO EXECUTÁVEL, PRECISA RESOLVER

	if (access(file_path, F_OK) < 0) /* Conta inexistente */
		return false;

	user_account = fopen(file_path, "rb");

	if (user_account == NULL)
		return false; // INSERT_ACCOUNT ????

	fread(account, sizeof(struct account_file_st), 1, user_account);

	fclose(user_account);
	return true;
}

void
save_account(int user_index)
{
	FILE *user_account = NULL;
	char file_path[1024] = { 0 };
	const char *account_name = g_users_db[user_index].profile.account_name;

	sprintf(file_path, "./bin/accounts/%s", account_name);

	user_account = fopen(file_path, "r+b");

	if (user_account == NULL)
		return;

	fwrite(&g_users_db[user_index].profile, sizeof(struct profile_file_st), 1, user_account);
	fclose(user_account);
}

bool
delete_account(const char *user, const char *password)
{
	char file_path[1024] = { 0 };

	sprintf(file_path, "./bin/accounts/%s", user);

	if (access(file_path, F_OK) < 0) /* Conta inexistente */
		return false;

	if (remove(file_path) == -1)
		perror("remove");
	else
		return true;

	return false;
}

bool
char_exists(const char *name)
{
	char file_path[1024] = { 0 };

	sprintf(file_path, "./bin/char/%s", name);

	return (access(file_path, F_OK) >= 0);
}

bool
accept_user(int user_index, int socket_fd, unsigned ip, char *ip_str)
{
	memset(&g_users[user_index], 0, sizeof(struct user_server_st));

	g_users[user_index].server_data.socket_fd = socket_fd;
	
	if (strlen(ip_str) <= 16)
		strncpy(g_users[user_index].server_data.ip_str, ip_str, strlen(ip_str));

	g_users[user_index].server_data.mode = USER_ACCEPT;
	g_users[user_index].server_data.index = user_index;

	return true;
}

bool
login_user(struct packet_request_login* request_login, int user_index)
{
	if (request_login->version != CLIENT_VERSION) {
		send_client_message("Atualize seu WYD.", user_index);
		return false;
	}

	if (strlen(request_login->name) == 16 || strlen(request_login->password) == 12) {
		send_client_message("Nome e/ou Senha muito grande.", user_index);
		return false;
	}

	request_login->name[11] = '\0'; /* Talvez dê pra fazer isso de outro modo */
	request_login->password[11] = '\0'; /* Talvez dê pra fazer isso de outro modo */

	struct account_file_st *account = &g_users_db[user_index];

	bool logged_account = search_logged_account(request_login->name);
	bool loaded_account = load_account(request_login->name, request_login->password, account, user_index);

	if (!loaded_account) {
		send_client_message("Conta nao encontrada.", user_index);
		return false;
	}

	if (strcmp(account->profile.account_password, request_login->password) != 0) {
		send_client_message("Senha incorreta.", user_index);
		return false;
	}

	if (!logged_account) {
		send_client_message("Conexao simultanea.", user_index);
		return false;
	}

	load_mob(0, user_index);
	load_mob(1, user_index);
	load_mob(2, user_index);
	load_mob(3, user_index);

	struct packet_char_list char_list = { 0 };

	char_list.header.size = sizeof(struct packet_char_list);
	char_list.header.operation_code = 0x10E;
	char_list.header.index = user_index;
	char_list.gold = account->profile.gold;
	char_list.cash = account->profile.cash;

	g_users[user_index].server_data.mode = USER_NUMERIC_PASSWORD;
	g_users[user_index].gold = char_list.gold;
	g_users[user_index].cash = char_list.cash;
	strncpy(g_users[user_index].account_name, request_login->name, 16); /* PARECE BURRICE */

	load_selchar(account->mob_account, &char_list.sel_list);
	memcpy(char_list.storage, account->profile.cargo, sizeof(char_list.storage));
	memcpy(&g_users[user_index].storage, char_list.storage, sizeof(struct item_st) * MAX_STORAGE);
	strncpy(char_list.name, request_login->name, sizeof(char_list.name));

	add_client_packet((unsigned char*)&char_list, char_list.header.size, user_index);
	send_all_packets(user_index);

	return true;
}

bool
login_user_numeric(struct packet_request_numeric_password *request_numeric_password, int user_index)
{
	struct account_file_st *account = &g_users_db[user_index];

	// REGEX PARA VALIDAR MENSAGEM

	if (*account->profile.numeric_password != '\0') {
		if (request_numeric_password->change_numeric != 1) {
			if (strncmp(request_numeric_password->numeric, account->profile.numeric_password, 6) == 0) {
				// Senha correta.
				g_users[user_index].server_data.mode = USER_SELCHAR;
				request_numeric_password->header.operation_code = 0xFDE;
				send_one_packet((unsigned char*) request_numeric_password, xlen(request_numeric_password), user_index);
				send_client_message("Seja bem-vindo ao servidor Snalmir!", user_index);
			} else {
				// Senha numérica não confere.
				send_client_message("Senha invalida.", user_index);
				send_signal(0xFDF, user_index); // Código de Operação de senha numérica incorreta.
			}
		} else {
			// Alteração de senha numérica.
			strncpy(account->profile.numeric_password, request_numeric_password->numeric, 6); // Tomar cuidado com o \0 que deve ficar no final.
			send_client_message("Senha alterada.", user_index);
			save_account(user_index);
		}
	} else {
		// Senha numérica ainda não atribuída (conta nova).
		strncpy(account->profile.numeric_password, request_numeric_password->numeric, 6); // Tomar cuidado com o \0 que deve ficar no final.
		send_client_message("Senha atribuida.", user_index);
		save_account(user_index);
	}

	return true;
}

bool
create_char(struct packet_request_create_char *request_create_char, int user_index)
{
	if (g_users[user_index].server_data.mode != USER_SELCHAR)
		return false;

	g_users[user_index].server_data.mode = USER_CREWAIT;

	struct account_file_st *account  = &g_users_db[user_index];

	bool error = false;

	if (account->profile.account_name[0] == '\0')
		error = true;

	if ((unsigned) request_create_char->slot_index > 3)
		error = true;

	if ((unsigned) request_create_char->class_index > 3)
		error = true;

	size_t name_length = strnlen(request_create_char->name, 16);

	if (name_length < 4 || name_length >= 12)
		error = true;

	for (size_t i = 0; i < 13; i++) {
		if (error) break;

		if (request_create_char->name[i] == '[' || request_create_char->name[i] == ']')
			error = true;
	}

	if (request_create_char->name[0] == '@' || request_create_char->name[0] == '.' || request_create_char->name[0] == ':')
		error = true;

	if (strncmp(request_create_char->name, "time", 16) == 0)
		error = true;

	struct mob_st *mob = &account->mob_account[request_create_char->slot_index];

	if (account->profile.char_info & (1 << request_create_char->slot_index))
		error = true;

	if (char_exists(request_create_char->name))
		error = true;

	if (error) {
		g_users[user_index].server_data.mode = USER_SELCHAR;
		send_signal(0x11A, user_index);

		return true;
	}

	if (g_users[user_index].server_data.mode != USER_CREWAIT)
		return false;

	account->profile.char_info |= 1 << request_create_char->slot_index;

	strncpy(account->profile.mob_name[request_create_char->slot_index], request_create_char->name, 16);
	memcpy(&account->mob_account[request_create_char->slot_index], &g_base_char_mobs[request_create_char->class_index], sizeof(struct mob_st));
	strncpy(mob->name, request_create_char->name, 16);
	mob->class_master = CLASS_MORTAL;

	save_mob(request_create_char->slot_index, false, user_index);
	save_account(user_index);

	struct packet_char_list char_list = { 0 };
	char_list.header.index = user_index;
	char_list.header.size = sizeof(struct packet_char_list);
	char_list.header.operation_code = 0x110;

	load_selchar(account->mob_account, &char_list.sel_list);
	send_one_packet((unsigned char*) &char_list, xlen(&char_list), user_index);

	g_users[user_index].server_data.mode = USER_SELCHAR;

	return true;
}

bool
delete_char(struct packet_request_delete_char *request_delete_char, int user_index)
{
	if (g_users[user_index].server_data.mode != USER_SELCHAR)
		return false;

	g_users[user_index].server_data.mode = USER_DELWAIT;

	struct account_file_st *account  = &g_users_db[user_index];

	bool error = false;

	if ((unsigned) request_delete_char->slot_index > 3)
		error = true;

	size_t name_length = strnlen(request_delete_char->name, 16);
	size_t password_length = strnlen(request_delete_char->password, 12);

	if (name_length < 4 || name_length >= 12 || password_length < 4 || password_length > 11)
		error = true;

	struct mob_st *mob = &account->mob_account[request_delete_char->slot_index];

	if (mob->name[0] == '\0' || strncmp(mob->name, request_delete_char->name, 16) != 0)
		error = true;

	if (!(account->profile.char_info & (1 << request_delete_char->slot_index)))
		error = true;

	if (strncmp(request_delete_char->password, account->profile.account_password, 12) != 0)
		error = true;

	if (error) {
		g_users[user_index].server_data.mode = USER_SELCHAR;
		send_signal(0x11B, user_index);

		return true;
	}

	if (g_users[user_index].server_data.mode != USER_DELWAIT)
		return false;

	memset(mob->name, 0, 16); // Limpa o nome do char da conta.
	save_mob(request_delete_char->slot_index, true, user_index);
	account->profile.char_info &= ~(1 << request_delete_char->slot_index);
	save_account(user_index);

	struct packet_delete_char delete_char = { 0 };
	delete_char.header.index = user_index;
	delete_char.header.size = sizeof(struct packet_delete_char);
	delete_char.header.operation_code = 0x112;

	load_selchar(account->mob_account, &delete_char.sel_list);
	send_one_packet((unsigned char*) &delete_char, xlen(&delete_char), user_index);

	g_users[user_index].server_data.mode = USER_SELCHAR;

	return true;
}

bool
enter_world(struct packet_request_enter_world *request_enter_world, int user_index)
{
	if (g_users[user_index].server_data.mode != USER_SELCHAR)
		return false;

	struct account_file_st *account = &g_users_db[user_index];

	bool error = false;

	if ((unsigned) request_enter_world->char_index > 3)
		error = true;

	struct mob_st *mob_account = &account->mob_account[request_enter_world->char_index];
	struct subcelestial_st *sub_cele = &account->subcelestials[request_enter_world->char_index];

	if (mob_account->name[0] == '\0')
		error = true;

	account->profile.sel_char = request_enter_world->char_index;
	mob_account->slot_index = request_enter_world->char_index;

	if (mob_account->sub == 2) {
		struct mob_st mob = { 0 };
		
		memcpy(&mob, mob_account, sizeof(struct packet_request_enter_world)); // COPIAR SÓ 36 BYTES????

		mob.b_status = sub_cele->b_status;
		mob.class_info = sub_cele->class_info;
		mob.class_master = sub_cele->class_master;
		mob.experience = sub_cele->experience;
		mob.equip[0].item_id = sub_cele->face;
		mob.hold = sub_cele->hold;
		mob.learn = sub_cele->learn;
		mob.p_skill = sub_cele->p_skill;
		mob.p_status = sub_cele->p_status;
		mob.p_master = sub_cele->p_master;

		memcpy(&mob.affect, sub_cele->affect, sizeof(struct affect_st) * 16);
		memcpy(&mob.skill_bar_1, sub_cele->skill_bar_1, 4);
		memcpy(&mob.skill_bar_2, sub_cele->skill_bar_2, 16);

		fprintf(stderr, "NÃO DEVERIA TER ENTRADO AQUI. TERMINAR DE IMPLEMENTAR\n");
		return true;
	}

	if (error) {
		send_client_message("Erro ao entrar no jogo.", user_index);
		g_users[user_index].server_data.mode = USER_SELCHAR;
		return true;
	}

	struct packet_enter_world enter_world = { 0 };
	enter_world.header.size = sizeof(struct packet_enter_world);
	enter_world.header.operation_code = 0x114;
	enter_world.header.index = 0x7531;
	enter_world.character = *mob_account;

	struct mob_server_st *character = &g_mobs[user_index];
	struct mob_st *character_mob = &character->mob;

	memcpy(character_mob, &enter_world.character, sizeof(struct mob_st));

	character->mob.client_index = (short) user_index;
	g_users[user_index].server_data.mode = USER_PLAY;
	g_mobs[user_index].mode = MOB_USER;

	get_bonus_score_points(character_mob);
	get_bonus_master_points(character_mob);
	get_bonus_skill_points(character_mob);
	get_current_score(user_index);

	character_mob->status.current_hp = character_mob->status.max_hp;
	character_mob->status.current_mp = character_mob->status.max_mp;

	struct position_st position = { -1, -1 };
	get_guild_zone(*character, &position);

	character->spawn_type = SPAWN_NORMAL;

	enter_world.position = position;
	character->mob.current = position;
	character->mob.last_position = position;

	g_users[user_index].sel_char = character->mob.slot_index;
	character->mob.guild_id = character->mob.equip[12].effect[1].value; // ????

	if (!update_world(user_index, &position, WORLD_MOB)) {
		clear_property(character);
		fprintf(stderr, "UPDATE WORLD FALHOU.\n");
		send_signal(0x119, user_index);
		g_users[user_index].server_data.mode = USER_SELCHAR;
		return true;
	}

	memcpy(&enter_world.character, character_mob, sizeof(struct mob_st));
	send_one_packet((unsigned char*) &enter_world, xlen(&enter_world) - 4, user_index);
	memset(&character->baby_mob[0], 0, sizeof(character->baby_mob));

	get_create_mob(user_index, user_index);
	send_grid_mob(user_index);

	// time_t current_time = time(NULL);
	// struct tm *no = localtime(&current_time);

	// int mount_id = character_mob->equip[MOUNT_SLOT].item_id;
	// if (mount_id >= 2330 && mount_id <= 2359)
	// 	int mount_index = mount_id - 2330 + 8;

	for (size_t i = 0; i < MAX_AFFECT; i++) {
		if (character_mob->affect[i].index > 0)
			character->buffer_time[i] = (character_mob->affect[i].time * 8);
		else
			character->buffer_time[i] = 0;
	}

	get_current_score(user_index);
	send_score(user_index);
	send_etc(user_index);
	send_affects(user_index);
	send_emotion(user_index, 14, 3);

	send_all_packets(user_index);

	return true;
}

void
save_client(int user_index)
{
	struct user_server_st user = g_users[user_index];
	struct account_file_st *account = &g_users_db[user_index];

	if (account != NULL && account->profile.mode >= ACCOUNT_SELCHAR) {
		account->profile.gold = user.gold;
		account->profile.cash = user.cash;

		memcpy(account->profile.cargo, user.storage, sizeof(account->profile.cargo));
		save_account(user_index);
	}
}

void
save_character(int user_index, int flag)
{
	get_current_score(user_index);

	struct mob_st *character = &g_mobs[user_index].mob;

	if ((unsigned) character->slot_index > 3)
		return;

	struct account_file_st *account = &g_users_db[user_index];
	
	if (account != NULL) {
		if (character->sub == 2) {
			// TODO: SUBCELE, IMPLEMENTAR
		}

		memcpy(&account->mob_account[character->slot_index], character, sizeof(struct mob_st));

		if (flag == 1) {
			save_mob(character->slot_index, false, user_index);
		}
	}
}

void
logout(int user_index)
{
	struct account_file_st *user = &g_users_db[user_index];

	if (user->profile.mode >= ACCOUNT_SELCHAR) {
		// TODO: DECREMENT PLAYER_COUNT

		save_account(user_index);

		for (size_t i = 0; i < MOB_PER_ACCOUNT; i++) {
			save_mob(i, false, user_index);
		}
	}

	memset(user, 0, sizeof(struct account_file_st));
}

bool
close_user(int user_index)
{
	struct user_server_st *user = &g_users[user_index];

	if (user->server_data.mode >= USER_SELCHAR) {
		save_client(user->server_data.index);
	}

	if (user->server_data.mode >= USER_PLAY) {
		struct mob_server_st *mob = &g_mobs[user->server_data.index];

		for (size_t i = 0; i < MAX_PARTY; i++) {
			int baby_index = mob->baby_mob[i];

			if (baby_index <= 0)
				continue;

			if (baby_index <= MAX_USERS_PER_CHANNEL || baby_index >= MAX_SPAWN_LIST) { 
				mob->baby_mob[i] = 0;
				continue;
			}

			struct mob_server_st *baby_mob = &g_mobs[baby_index];
			if (!is_summon(*baby_mob)) {
				mob->baby_mob[i] = 0;
				continue;
			}

			remove_object(baby_index, g_mobs[baby_index].mob.current, WORLD_MOB);
			send_remove_mob(baby_index, baby_index, DELETE_UNSPAWN);
			clear_property(baby_mob);

			mob->baby_mob[i] = 0;
		}

		save_character(user->server_data.index, 1);
		remove_object(user->server_data.index, g_mobs[user->server_data.index].mob.current, WORLD_MOB);
		send_remove_mob(user->server_data.index, user->server_data.index, DELETE_NORMAL);
		clear_property(mob);
	}

	if (user->server_data.mode == USER_LOGIN)
		logout(user_index);

	user->server_data.mode = USER_EMPTY;
	user->server_data.last_recv_time = 0;
	memset(&user->storage, 0, sizeof(g_users[user_index].storage));
	memset(&user->market_info, 0, sizeof(g_users[user_index].market_info));
	memset(user->account_name, 0, 16);
	user->server_data.index = 0;
	user->server_data.user_close = false;

	return true;
}