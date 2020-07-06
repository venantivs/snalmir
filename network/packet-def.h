#ifndef __PACKET_DEF_H__
#define __PACKET_DEF_H__

#include "../core/base-def.h"

#define LOGIN_OPCODE 0x20D

struct packet_header
{
	short size;
	char key;
	char checksum;
	short opcode;
	short index;
	unsigned time;
};

struct packet_charlist
{
	
};

#endif
