#ifndef __USER_H__
#define __USER_H__

#include <time.h>
#include <stdbool.h>

#include "../core/base-def.h"

struct user_server_data_st {
	int socket_fd;
	unsigned ip;
	unsigned mac[4];
	int mode;
	int index;
	clock_t	last_recv_time;
	bool user_close;
	int timer_close;
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

#endif
