#ifndef __PACKET_SECURITY_H__
#define __PACKET_SECURITY_H__

#include <stdbool.h>

void	encrypt(unsigned char *packet_buffer[]);
bool	decrypt(unsigned char *packet_buffer[]);

#endif
