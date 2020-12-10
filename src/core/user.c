/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Arquivo: core/user.c
 * Descrição: Arquivo onde são implementadas as funções que correspondem ao usuário em específico,
 *  como criar conta, login, logout, etc.
 */

#include <stdio.h>
#include <unistd.h>

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
