#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__

#define INITCODE			    0x1F11F311 /* Hello Packet */
#define MAX_MESSAGE_SIZE	20000

#include <stdbool.h>

void	refresh_recv_buffer(int);
void	refresh_send_buffer(int);

unsigned char	*read_client_packet(int);
bool			add_client_packet(unsigned char *, size_t, int);
bool			receive(int);
bool 			send_one_packet(unsigned char*, size_t, int);
bool			send_all_packets(int);
void      send_client_message(const char*, int);
void      send_signal(short, int);

#endif