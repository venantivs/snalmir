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
#include "utils.h"
#include "user.h"

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