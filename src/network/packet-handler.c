/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Atualização: Dezembro de 2020
 * Arquivo: network/packet-handler.c
 * Descrição: Módulo onde os packets "brutos" vêm para serem corretamente direcionados.
 */

#include <stdio.h>

#include "server.h"
#include "packet-def.h"
#include "packet-handler.h"

bool
segregate_packet(unsigned char *message, int user_index)
{
	struct packet_header *header = (struct packet_header *)message;

	if (user_index > 0 && user_index < MAX_USERS_PER_CHANNEL)
		users[user_index].server_data.last_recv_time = sec_counter;

	/* ????? */
	if (header->operation_code == 0x3A0)
		return true;

	if (users[user_index].server_data.mode == USER_ACCEPT) {
		if (header->size != 116)
			return false;
		

	}

	return false;
} 
