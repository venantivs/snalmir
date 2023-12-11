#ifndef __PACKET_DEF_H__
#define __PACKET_DEF_H__

#include "../core/base-def.h"

#define LOGIN_OPERATION_CODE 0x20D
#define xlen(buffer_) *(unsigned short*)buffer_

// 12 bytes
struct packet_header {
	short size;
	unsigned char key;
	unsigned char checksum;
	short operation_code;
	short index;
	unsigned time;
};

struct packet_signal {
	struct packet_header header;
};

struct packet_string_message {
	struct packet_header header;
	char string_message[96];
};

struct packet_request_login {
	struct packet_header header;
	char name[16];
	char password[12];
	int version;
	int unknown;
	char keys[16];
	char ip_address[16];
};

struct packet_request_numeric_password {
	struct packet_header header;
	char numeric[6];
	char unknown[10];
	int change_numeric;
};

struct packet_request_logout {
	struct packet_header header;
	int session;
};

struct packet_char_list {
	struct packet_header header;
	struct char_list_st sel_list;
	struct item_st storage[MAX_STORAGE];
	int gold;
	char name[16];
	char keys[16];
	int cash;
	int unknown;
};

struct packet_request_create_char {
	struct packet_header header;
	int slot_index;
	char name[16];
	int class_index;
};

struct packet_create_char {
	struct packet_header header;
	struct char_list_st sel_list;
};

struct packet_create_sub {
	struct packet_header header;
	char name[16];
	int sub_class;
	unsigned char face;
	int cythera;
};

struct packet_request_delete_char {
	struct packet_header header;
	int slot_index;
	char name[16];
	char password[12];
};

struct packet_delete_char {
	struct packet_header header;
	struct char_list_st sel_list;
};

struct packet_request_enter_world {
	struct packet_header header;
	int char_index;
	char zero[18];
};

struct packet_enter_world {
	struct packet_header header;
	struct position_st position;
	struct mob_st character;
};

// MSGS ?

struct packet_create_ground_item {
	struct packet_header header;
	struct position_st position;
	unsigned short item_index;
	struct item_st item;
	char rotation;
	char status;
};

#endif
