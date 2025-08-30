#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__

#define INITCODE			    0x1F11F311 /* Hello Packet */
#define MAX_MESSAGE_SIZE	20000

#include <stdbool.h>

void	refresh_recv_buffer(int user_index);
void	refresh_send_buffer(int user_index);

unsigned char	*read_client_packet(int user_index);
bool			add_client_packet(unsigned char *message, size_t size, int user_index);
bool			receive(int user_index);
bool 			send_one_packet(unsigned char* message, size_t size, int user_index);
bool			send_all_packets(int user_index);
void      send_client_message(const char* message, int user_index);
void      send_signal(short operation_code, int user_index);

#endif