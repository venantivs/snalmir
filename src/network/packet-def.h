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

struct packet_signal_parm {
	struct packet_header header;
	int data;
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

struct packet_spawn_info {
	struct packet_header header;
	struct position_st position;
	unsigned short client_id;
	char name[16];
	unsigned char chaos_points;
	unsigned char current_kill;
	unsigned char total_kill;
	unsigned short item_refine[16];
	struct {
		unsigned char time;
		unsigned char index;
	} affect[32];
	unsigned short guild_index; // 100
	struct status_st status;
	struct {
		char type;
		char member_type;
	} spawn;
	unsigned char anct_code[16];
	char tab[26];
	unsigned char unknown[4];
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

// 336H
struct packet_refresh_score {
	struct packet_header header;
	struct status_st status;
	unsigned char critical;
	unsigned char save_mana;
	struct
	{
		unsigned char time;
		unsigned char index;
	} affect[MAX_AFFECT];
	unsigned char guild_member_type;
	unsigned char guild_index;
	signed char regen_hp;
	signed char regen_mp;
	unsigned char resist_1;
	unsigned char resist_2;
	unsigned char resist_3;
	unsigned char resist_4;
	signed short current_hp;
	signed short current_mp;
	unsigned short magic_increment;
	unsigned char special[4];
};

// 337H
struct packet_refresh_etc
{
	struct packet_header header;
	int hold;
	int exp;
	int learn;
	short status;
	short master;
	short skills;
	short magic;
	int gold;
};

// 3B9H
struct packet_affect
{
	struct packet_header header;
	struct affect_st affect[MAX_AFFECT];
};

// 36AH
struct packet_emotion
{
	struct packet_header header;
	short effect_type;
	short effect_value;
	int unknown;
};

// 366H
struct packet_request_action
{
	struct packet_header header;
	struct position_st source_position;
	int speed;
	int type;
	struct position_st destiny_position;
	char command[24];
};

// 374H
struct packet_door
{
	struct packet_header header;
	int init_id;
	int status;
};


#endif
