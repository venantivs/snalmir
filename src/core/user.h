#ifndef __USER_H__
#define __USER_H__

#include <time.h>
#include <stdbool.h>

#include "../core/base-def.h"
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

bool	create_account(const char *, const char *);
bool	delete_account(const char *, const char *);
bool	accept_user(int, int, unsigned, char *);
bool 	close_user(int);

#endif
