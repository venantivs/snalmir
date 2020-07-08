/*
 * Licença: GPLv3
 * Autor: theskytalos
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

bool create_account(char user[], char password[])
{
	FILE *user_account = NULL;
	char file_path[1024] = { 0 };

	sprintf(file_path, "./accounts/%s", user);

	if (access(file_path, F_OK) != -1) // Conta já existente
		return false;

	user_account = fopen(file_path, "w+b");

	if (user_account == NULL)
		return false;

	fclose(user_account);
}

bool delete_account(char user[], char password[])
{
	char file_path[1024] = { 0 };

	sprintf(file_path, "./accounts/%s", user);

	if (access(file_path, F_OK) == -1) // Conta inexistente
		return false;

	return remove(file_path) == 0 ? true : false;
}

bool login(unsigned char channel, unsigned short index, unsigned char packet_buffer[])
{
	
}
