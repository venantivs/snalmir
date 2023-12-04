/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Atualização: Dezembro de 2020
 * Arquivo: core/user.c
 * Descrição: Arquivo onde são implementadas as funções que correspondem ao usuário em específico,
 *  como criar conta, login, logout, etc.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../network/server.h"
#include "../network/packet-def.h"
#include "../network/socket-utils.h"
#include "utils.h"
#include "user.h"
#include "mob.h"

bool
create_account(const char *user, const char *password)
{
	FILE *user_account = NULL;
	char file_path[1024] = { 0 };

	sprintf(file_path, "./accounts/%s", user);

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
		if (strncmp(user, users[i].account_name, 16) == 0)
			return false;
	}

	return true;
}

bool
load_account(const char *user, const char *password, struct account_file_st *account, int user_index)
{
	FILE *user_account = NULL;
	char file_path[1024] = { 0 };

	sprintf(file_path, "./accounts/%s", user); // TER EM MENTE QUE ./ SE REFERE AO DIRETÓRIO ATUAL DO TERMINAL, NÃO DO EXECUTÁVEL, PRECISA RESOLVER

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
	const char *account_name = users_db[user_index].profile.account_name;

	sprintf(file_path, "./accounts/%s", account_name);

	user_account = fopen(file_path, "r+b");

	if (user_account == NULL)
		return;

	fwrite(&users_db[user_index].profile, sizeof(struct profile_file_st), 1, user_account);
	fclose(user_account);
	return;	
}

bool
delete_account(const char *user, const char *password)
{
	char file_path[1024] = { 0 };

	sprintf(file_path, "./accounts/%s", user);

	if (access(file_path, F_OK) < 0) /* Conta inexistente */
		return false;

	if (remove(file_path) == -1)
		perror("remove");
	else
		return true;

	return false;
}

bool
accept_user(int user_index, int socket_fd, unsigned ip, char *ip_str)
{
	memset(&users[user_index], 0, sizeof(struct user_server_st));

	users[user_index].server_data.socket_fd = socket_fd;
	
	if (strlen(ip_str) <= 16)
		strncpy(users[user_index].server_data.ip_str, ip_str, strlen(ip_str));

	users[user_index].server_data.mode = USER_ACCEPT;
	users[user_index].server_data.index = user_index;

	return true;
}

bool
login_user(struct packet_request_login* request_login, int user_index)
{
	if (request_login->version != CLIENT_VERSION) {
		send_client_string_message("Atualize seu WYD.", user_index);
		return false;
	}

	if (strlen(request_login->name) == 16 || strlen(request_login->password) == 12) {
		send_client_string_message("Nome e/ou Senha muito grande.", user_index);
		return false;
	}

	request_login->name[11] = '\0'; /* Talvez dê pra fazer isso de outro modo */
	request_login->password[11] = '\0'; /* Talvez dê pra fazer isso de outro modo */

	struct account_file_st *account = &users_db[user_index];

	bool logged_account = search_logged_account(request_login->name);
	bool loaded_account = load_account(request_login->name, request_login->password, account, user_index);

	if (!loaded_account) {
		send_client_string_message("Conta nao encontrada.", user_index);
		return false;
	}

	if (strcmp(account->profile.account_password, request_login->password) != 0) {
		send_client_string_message("Senha incorreta.", user_index);
		return false;
	}

	if (!logged_account) {
		send_client_string_message("Conexao simultanea.", user_index);
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

	users[user_index].server_data.mode = USER_NUMERIC_PASSWORD;
	users[user_index].gold = char_list.gold;
	users[user_index].cash = char_list.cash;
	strncpy(users[user_index].account_name, request_login->name, 16); /* PARECE BURRICE */

	load_selchar(account->mob_account, &char_list.sel_list);
	memcpy(char_list.storage, account->profile.cargo, sizeof(char_list.storage));
	memcpy(&users[user_index].storage, char_list.storage, sizeof(struct item_st) * MAX_STORAGE);
	strncpy(char_list.name, request_login->name, sizeof(char_list.name));

	add_client_message((unsigned char*)&char_list, char_list.header.size, user_index);
	send_all_messages(user_index);

	return true;
}

bool
login_user_numeric(struct packet_request_numeric_password *request_numeric_password, int user_index)
{
	struct account_file_st *account = &users_db[user_index];

	// REGEX PARA VALIDAR MENSAGEM

	if (*account->profile.numeric_password != '\0') {
		if (request_numeric_password->change_numeric != 1) {
			if (strncmp(request_numeric_password->numeric, account->profile.numeric_password, 6) == 0) {
				// Senha correta.
				users[user_index].server_data.mode = USER_SELCHAR;
				request_numeric_password->header.operation_code = 0xFDE;
				send_one_message((unsigned char*) request_numeric_password, xlen(request_numeric_password), user_index);
				send_client_string_message("Seja bem-vindo ao servidor Snalmir!", user_index);
			} else {
				// Senha numérica não confere.
				send_client_string_message("Senha invalida.", user_index);

				struct packet_signal signal = { 0 };

				signal.header.index = user_index;
				signal.header.size = sizeof(struct packet_signal);
				signal.header.operation_code = 0xFDF; // Código de Operação de senha numérica incorreta. 
				add_client_message((unsigned char*) &signal, xlen(&signal), user_index);
				send_all_messages(user_index);
			}
		} else {
			// Alteração de senha numérica.
			strncpy(account->profile.numeric_password, request_numeric_password->numeric, 6); // Tomar cuidado com o \0 que deve ficar no final.
			send_client_string_message("Senha alterada.", user_index);
			save_account(user_index);
		}
	} else {
		// Senha numérica ainda não atribuída (conta nova).
		strncpy(account->profile.numeric_password, request_numeric_password->numeric, 6); // Tomar cuidado com o \0 que deve ficar no final.
		send_client_string_message("Senha atribuida.", user_index);
		save_account(user_index);
	}

	return true;
}

bool
close_user(int user_index)
{
	if (users[user_index].server_data.mode == USER_SELCHAR) {
		/* SAVE CLIENT */
	}

	if (users[user_index].server_data.mode == USER_PLAY) {

	}

	if (users[user_index].server_data.mode == USER_LOGIN) {

	}

	return true;
}