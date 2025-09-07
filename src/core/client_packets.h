#ifndef __CLIENT_PACKETS_H__
#define __CLIENT_PACKETS_H__

#include <stdbool.h>

#include "../network/packet-def.h"

bool request_update_city(struct packet_request_change_city *request_change_city, int user_index);
bool request_logout_char(int user_index);
bool request_return_char_list(int user_index);
bool request_process_npc(struct packet_request_npc *request_npc, int user_index);
bool request_command(struct packet_request_command *request_command, int user_index);
bool request_movement(struct packet_request_action *request_action, int user_index);

#endif