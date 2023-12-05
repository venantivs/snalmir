/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Atualização: Dezembro de 2023
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

	printf("=======================================\n");
	printf("USER_INDEX: %d | OP_CODE: 0x%x.\n", user_index, header->operation_code);

	/* Packet keep-alive? */
	if (header->operation_code == 0x3A0)
		return true;

	if (users[user_index].server_data.mode == USER_ACCEPT) {
		if (header->size != 116)
			return false;
		
		users[user_index].server_data.mode = USER_LOGIN;

		struct packet_request_login *login_request = (struct packet_request_login *)header;

		if (!login_user(login_request, user_index))
			return false;

		users[user_index].server_data.last_recv_time = clock() + 110; /* ??? */
		return true;
	}

	if (!(users[user_index].server_data.mode >= USER_LOGIN && header->operation_code != 0x20D))
		return false;

	switch(header->operation_code) {
	case 0xFDE:
		if (users[user_index].server_data.mode >= USER_NUMERIC_PASSWORD && users[user_index].server_data.mode <= USER_NUMERIC_PASSWORD_RSUCCESS) {
			struct packet_request_numeric_password *numeric_password_request = (struct packet_request_numeric_password *)header;

			if (numeric_password_request->change_numeric == 1)
				users[user_index].server_data.mode = USER_NUMERIC_PASSWORD_CHANGE;
			else
				users[user_index].server_data.mode = USER_NUMERIC_PASSWORD_RSUCCESS;

			return login_user_numeric(numeric_password_request, user_index);
		}
	case 0x20F:
		return create_char((struct packet_request_create_char *) header, user_index);
	case 0x211:
		return delete_char((struct packet_request_delete_char *) header, user_index);
	case 0x213:
		return enter_world((struct packet_request_enter_world *) header, user_index);
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