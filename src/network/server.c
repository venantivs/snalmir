/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Atualização: Dezembro de 2020
 * Arquivo: network/server.c
 * Descrição: Arquivo onde o servidor principal é propriamente inicializado, recebe e envia pacotes.
 * TODO: Implementar error handling muito rigoroso.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include "../core/utils.h"
#include "../core/user.h"
#include "../core/game_items.h"
#include "socket-utils.h"
#include "packet-handler.h"
#include "../general-config.h"
#include "server.h"

struct user_server_st users[MAX_USERS_PER_CHANNEL];
struct user_server_st temp_user;
struct mob_server_st mobs[30000];
struct ground_item_st ground_items[4096];
struct party_st parties[500];

/* DB */
struct account_file_st users_db[MAX_USERS_PER_CHANNEL];
struct mob_st base_char_mobs[MOB_PER_ACCOUNT];

unsigned long current_time;
int current_weather;

int sec_counter = 0;
int min_counter = 0;

static void start_server();

static void 
min_timer()
{
	min_counter++;
}

static void
sec_timer()
{
	sec_counter++;
	current_time = (unsigned long) time(NULL);

	if (sec_counter % 60 == 0)
		min_timer();
}

void
*init_server()
{
	memset(users, 0, sizeof(struct user_server_st) * MAX_USERS_PER_CHANNEL);
	memset(&temp_user, 0, sizeof(struct user_server_st));
	memset(mobs, 0, sizeof(struct mob_server_st) * MAX_SPAWN_LIST);
	memset(ground_items, 0, sizeof(struct ground_item_st) * 4096);
	memset(parties, -1, sizeof(struct party_st) * 500);

	/* DB */
	memset(users_db, 0, sizeof(struct account_file_st) * MAX_USERS_PER_CHANNEL);
	memset(base_char_mobs, 0, sizeof(struct mob_st) * MOB_PER_ACCOUNT);

	current_time = 0;
	current_weather = 0;

	sec_counter = 0;
	min_counter = 0;

	struct itimerval it_val;

	memset(&it_val, 0, sizeof(struct itimerval));

	if (signal(SIGALRM, (void (*)(int)) sec_timer) == SIG_ERR)
		fatal_error("signal");

	it_val.it_value.tv_sec = 1;
	it_val.it_value.tv_usec = 0;
	it_val.it_interval = it_val.it_value;

  if (setitimer(ITIMER_REAL, &it_val, NULL) == -1)
		fatal_error("setitimer");

	load_game_items();
	load_base_char_mobs();
	load_npcs();
	start_server();

	return NULL;
}

static void
epoll_ctl_add(int epfd, int fd, uint32_t events)
{
	struct epoll_event ev;

	ev.events = events;
	ev.data.fd = fd;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
		fatal_error("epoll_ctl add");
}

static void
set_non_blocking(int socket_fd)
{
	int opts;

	if ((opts = fcntl(socket_fd, F_GETFL, 0)) < 0)
		fatal_error("fcntl get");

	if (fcntl(socket_fd, F_SETFL, opts | O_NONBLOCK) < 0)
		fatal_error("fcntl set");
}

static int
get_user_index_from_socket(int socket_fd)
{
	/* No Linux, creio que em BSDs também, descritores de arquivo 0, 1 e 2 são reservados pelo sistema. */
	if (socket_fd < 3) {
		fprintf(stderr, "Descritor de arquivo inválido. (< 3)\n");
		return -1;
	}

	/* Implementação burra, adiciona complexidade O(n) (((aparentemente))) desnecessária a cada evento do epoll. */
	for (size_t i = 0; i < MAX_USERS_PER_CHANNEL; i++)
		if (users[i].server_data.socket_fd == socket_fd)
			return i;

	return -1;
}

static int
get_empty_user(void)
{
	for (size_t i = 0; i < MAX_USERS_PER_CHANNEL; i++)
		if (users[i].server_data.mode == USER_EMPTY)
			return i;

	return -1;
}

static void
print_buffer(int user_index)
{
	for (size_t i = 0; i < 1024; i++)
		printf("%hhx ", users[user_index].server_data.buffer.recv_buffer[i]);

	printf("\n");
}

static void
start_server()
{
	int listen_socket_fd, connection_socket_fd, epfd, nfds;
	struct sockaddr_in server_address, client_address;
	struct epoll_event events[MAX_USERS_PER_CHANNEL];
	socklen_t client_length;

	if ((epfd = epoll_create(256)) < 0)
		fatal_error("epoll_create");

	if ((listen_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fatal_error("socket");

	set_non_blocking(listen_socket_fd);

	epoll_ctl_add(epfd, listen_socket_fd, EPOLLIN | EPOLLET);	

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(SERVER_PORT);

	if (bind(listen_socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		fatal_error("bind");
	
	if (listen(listen_socket_fd, 16) < 0)
		fatal_error("listen");

	while (true) {
		if ((nfds = epoll_wait(epfd, events, 20, 500)) < 0)
			fatal_error("epoll_wait");
			
		for (size_t i = 0; i < nfds; ++i) {
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
				close(events[i].data.fd);
				continue;
			} else if (events[i].data.fd == listen_socket_fd) {
				while (true) {
					int user_index = get_empty_user();

					/* Checar se recv_buffer e send_buffer são NULL pode ser inútil uma vez que ambos são estáticos. */
					if (user_index < 0 || user_index >= MAX_USERS_PER_CHANNEL || users[user_index].server_data.buffer.recv_buffer == NULL || users[user_index].server_data.buffer.send_buffer == NULL) {
						/* DEU RUIM FILHO */
						fprintf(stderr, "Não foi possível aceitar o usuário.\n");
						continue;
					}

					if ((connection_socket_fd = accept(listen_socket_fd, (struct sockaddr *) &client_address, &client_length)) < 0) {
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
							break;
						else {
							perror("accept");
							break;	
						}
					}
					
					char *ip_str = inet_ntoa(client_address.sin_addr);

					set_non_blocking(connection_socket_fd);

					epoll_ctl_add(epfd, connection_socket_fd, EPOLLIN | EPOLLET);

					if (!accept_user(user_index, connection_socket_fd, client_address.sin_addr.s_addr, ip_str)) {
						close(connection_socket_fd);
						continue;
					}

					printf("Accepted connection on fd %d, host %s\n", connection_socket_fd, ip_str);
				}

				continue;
			} else if (events[i].events & EPOLLIN) {
				bool done = false;

				int user_index = get_user_index_from_socket(events[i].data.fd);

				if (user_index < 0)
					done = true;

				if (users[user_index].server_data.mode == USER_EMPTY)
					done = true;

				if (user_index > MAX_USERS_PER_CHANNEL) {
					close_user(user_index);
					done = true;
				}

				if (!receive(user_index)) {
					users[user_index].server_data.user_close = true;
					done = true;
				}

				unsigned char *message = NULL;

				while (!done) {
					/* AQUI É ONDE O PAU QUEBRA */

					message = read_client_message(user_index);

					if (message == NULL)
						break;

					if (!segregate_packet(message, user_index)) {
						users[user_index].server_data.user_close = true;
						done = true;
						break;
					}
				}

				if (done) {
					if (user_index >= 0 && user_index < MAX_USERS_PER_CHANNEL)
						users[user_index].server_data.mode = USER_EMPTY;

					printf("Closed connection on fd %d.\n", events[i].data.fd);

					close(events[i].data.fd);
				}
			}
		}
	}

	close(listen_socket_fd);
}