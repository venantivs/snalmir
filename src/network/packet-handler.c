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
#include "../core/utils.h"
#include "../core/client_packets.h"

bool
segregate_packet(unsigned char *packet, int user_index)
{
	struct packet_header *header = (struct packet_header *)packet;

	if (user_index > 0 && user_index < MAX_USERS_PER_CHANNEL)
		g_users[user_index].server_data.last_recv_time = g_sec_counter;

	printf("=======================================\n");
	printf("USER_INDEX: %d | OP_CODE: 0x%x.\n", user_index, header->operation_code);

	/* Packet keep-alive? */
	if (header->operation_code == 0x3A0)
		return true;

	if (g_users[user_index].server_data.mode == USER_ACCEPT) {
		if (header->size != 116)
			return false;
		
		g_users[user_index].server_data.mode = USER_LOGIN;

		struct packet_request_login *login_request = (struct packet_request_login *) header;

		if (!login_user(login_request, user_index))
			return false;

		g_users[user_index].server_data.last_recv_time = get_clock() + 110; /* ??? */

		return true;
	}

	if (!(g_users[user_index].server_data.mode >= USER_LOGIN && header->operation_code != 0x20D))
		return false;

	switch(header->operation_code) {
	case 0xFDE:
		if (g_users[user_index].server_data.mode >= USER_NUMERIC_PASSWORD && g_users[user_index].server_data.mode <= USER_NUMERIC_PASSWORD_RSUCCESS) {
			struct packet_request_numeric_password *numeric_password_request = (struct packet_request_numeric_password *)header;

			if (numeric_password_request->change_numeric == 1)
				g_users[user_index].server_data.mode = USER_NUMERIC_PASSWORD_CHANGE;
			else
				g_users[user_index].server_data.mode = USER_NUMERIC_PASSWORD_RSUCCESS;

			return login_user_numeric(numeric_password_request, user_index);
		}
	case 0x20F:
		return create_char((struct packet_request_create_char *) header, user_index);
	case 0x211:
		return delete_char((struct packet_request_delete_char *) header, user_index);
	case 0x213:
		return enter_world((struct packet_request_enter_world *) header, user_index);
	case 0x215:
		return request_return_char_list(user_index);
	case 0x28B:
		return request_process_npc((struct packet_request_npc *) header, user_index);
	case 0x291:
		return request_update_city((struct packet_request_change_city *) header, user_index);
	case 0x334:
		return request_command((struct packet_request_command *) header, user_index);
	case 0x3AE:
		return request_logout_char(user_index);
	case 0x366:
	case 0x367:
		return request_movement((struct packet_request_action *) header, user_index);
	case 0x369:
	case 0x2CB:
		return true;
	default:
		printf("PACKET NÃO IMPLEMENTADO: %x\n", header->operation_code);
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