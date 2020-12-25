/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Arquivo: network/server.c
 * Descrição: Arquivo onde o servidor principal é propriamente inicializado, recebe e envia pacotes.
 * TODO: Implement rigorous error handling.
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
#include "../general-config.h"
#include "server.h"

struct user_server_st users[MAX_USERS_PER_CHANNEL];
struct mob_server_st mobs[30000];
struct ground_item_st ground_items[4096];
struct party_st parties[500];

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
	memset(mobs, 0, sizeof(struct mob_server_st) * 30000);
	memset(ground_items, 0, sizeof(struct ground_item_st) * 4096);
	memset(parties, -1, sizeof(struct party_st) * 500);

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
	/* Implementação burra, adiciona complexidade O(n) desnecessária a cada iteração do servidor. */
	for (size_t i = 0; i < MAX_USERS_PER_CHANNEL; i++)
		if (users[i].server_data.socket_fd == socket_fd)
			return i;
}

static void
start_server()
{
        int listen_socket_fd, connection_socket_fd, epfd, nfds, bytes_read;
        char buffer[1024];
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
					if ((connection_socket_fd = accept(listen_socket_fd, (struct sockaddr *) &client_address, &client_length)) < 0) {
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
							break;
						else {
							perror("accept");
							break;	
						}
					}
					
					char *str = inet_ntoa(client_address.sin_addr);
                                	printf("Accepted connection on fd %d, host %s\n", connection_socket_fd, str);

					set_non_blocking(connection_socket_fd);

					epoll_ctl_add(epfd, connection_socket_fd, EPOLLIN | EPOLLET);
				}

				continue;
			} else if (events[i].events & EPOLLIN) {
				bool done = false;

				while (true) {
					/* AQUI É QUE ONDE O PAU QUEBRA */

					int user_index = get_user_index_from_socket(events[i].data.fd);

					/* No Linux, creio que em BSDs também, descritores de arquivo 0, 1 e 2 são reservados pelo sistema. */
					if (user_index < 3) {
						fprintf(stderr, "Descritor de arquivo inválido. (<3)\n");
						done = true;
						break;
					}



					/*memset(buffer, 0, sizeof(buffer));
					
					if ((bytes_read = read(events[i].data.fd, buffer, sizeof(buffer))) < 0) {
						if (errno != EAGAIN) {
							perror("read");
							done = true;
						}

						break;
					}  else if (bytes_read == 0) {
						done = true;
						break;
					}

					write(1, buffer, bytes_read);*/
				}

				if (done) {
					printf("Closed connection on fd %d.\n", events[i].data.fd);

					close(events[i].data.fd);
				}
                        }
                }
        }

	close(listen_socket_fd);
}