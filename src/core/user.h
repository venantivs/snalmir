#ifndef __USER_H__
#define __USER_H__

#include <time.h>
#include <stdbool.h>

#include "../core/base-def.h"
#include "../network/packet-def.h"
#include "../general-config.h"

enum user_mode { USER_EMPTY, USER_ACCEPT, USER_LOGIN, USER_NUMERIC_PASSWORD,
		 USER_NUMERIC_PASSWORD_CHANGE, USER_NUMERIC_PASSWORD_RSUCCESS, 
		 USER_SELCHAR, USER_CHARWAIT, USER_CREWAIT, USER_DELWAIT, 
		 USER_PLAY, USER_SAVING_TO_QUIT };

struct user_server_buffering_st {
	unsigned char recv_buffer[RECV_BUFFER_SIZE];
	unsigned char send_buffer[SEND_BUFFER_SIZE];
	int recv_position;
	int send_position;
	int proc_position;
	int sent_position;
	bool initialized;
	unsigned char queued_messages_count;
};

struct user_server_data_st {
	int socket_fd;
	unsigned ip;
	char ip_str[18];
	unsigned mac[4];
	enum user_mode mode;
	int index;
	clock_t	last_recv_time;
	bool user_close;
	int timer_close;
	struct user_server_buffering_st buffer;
};

struct user_server_st {
	char account_name[16];
	struct item_st storage[128];
	struct market_st market_info;
	time_t save_time;
	int gold;
	int cash;
	int sel_char;
	bool trading;
	int last_skill_tick[104];
	int last_attack_tick;
	int last_attack;
	struct user_server_data_st server_data;
};

/* ACCOUNT FILE RELATED */

struct profile_file_st {
	unsigned int account_id;
	char account_name[ACCOUNTNAME_LENGTH];
	char account_password[ACCOUNTPASS_LENGTH];
	char numeric_password[6];
	char real_name[REALNAME_LENGTH];
	char email[EMAIL_LENGTH];
	char telephone[TELEPHONE_LENGTH];
	char address[ADDRESS_LENGTH];
	char mob_name[MOB_PER_ACCOUNT][16];
	unsigned short game_server;
	char keys[16];
	char ipaddr[16];
	char last_ip2[16];
	char last_ip3[16];
	int gold;
	unsigned int char_info;
	int cash;
	unsigned int sel_char;
	struct item_st cargo[128];
	int Mode;
};

struct account_file_st {
	struct profile_file_st profile;
	struct mob_st mob_account[MOB_PER_ACCOUNT];
	struct subcelestial_st subcelestials[MOB_PER_ACCOUNT];
};

bool	create_account(const char *, const char *);
bool	delete_account(const char *, const char *);
bool	accept_user(int, int, unsigned, char *);
bool 	login_user(struct packet_request_login*, int);
bool 	close_user(int);

#endif
