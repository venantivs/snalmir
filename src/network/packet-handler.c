/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Atualização: Dezembro de 2020
 * Arquivo: network/packet-handler.c
 * Descrição: Módulo onde os packets "brutos" vêm para serem corretamente direcionados.
 */

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "server.h"
#include "packet-def.h"
#include "packet-handler.h"

bool
segregate_packet(unsigned char *packet, int user_index)
{
	struct packet_header *header = (struct packet_header *)packet;

	if (user_index > 0 && user_index < MAX_USERS_PER_CHANNEL)
		users[user_index].server_data.last_recv_time = sec_counter;

	printf("USER_INDEX: %d | OP_CODE: 0x%x.\n", user_index, header->operation_code);

	/* Packet enviado ao deslogar */
	if (header->operation_code == 0x3A0)
		return true;

	if (users[user_index].server_data.mode == USER_ACCEPT) {
		if (header->size != 116)
			return false;
		
		struct packet_request_login *login_request = (struct packet_request_login *)header;
		login_request->header.index = user_index; // DBSERV, remover
		login_request->header.operation_code = 0x100; // DBSERV, remover
		login_request->header.size = sizeof(struct packet_request_login); // DBSERV, remover

		login_user(login_request, user_index);

		users[user_index].server_data.mode = USER_LOGIN;
		users[user_index].server_data.last_recv_time = clock() + 110; /* ??? */
		strncpy(users[user_index].account_name, login_request->name, 16); /* PARECE BURRICE */
		return true;
	} else if (users[user_index].server_data.mode >= USER_LOGIN && header->operation_code != 0x20D) {
		
	}

	return false;
}

void
print_message(unsigned char *packet)
{
	for(size_t i = 0; i < xlen(packet); i++) {
		printf("%hhx ", packet[i]);
	}
	printf("\n");
}