/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Arquivo: snalmir.c
 * Descrição: Arquivo principal do servidor, onde se encontra o entry point da aplicação.
 *  Aqui são inicializadas e tratadas as threads que compõem o servidor.
 */

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "core/utils.h"
#include "http/http-server.h"
#include "network/server.h"
#include "general-config.h"

int
main(void)
{
	pthread_t main_server_thread;	

	#if HTTP_SERVER_ENABLED
		pid_t http_pid = fork();

		if (http_pid < 0)
			fatal_error("fork http");
		
		if (http_pid == 0)
			init_http_server();
	#endif

	if ((errno = pthread_create(&main_server_thread, NULL, init_server, NULL)) < 0)
		fatal_error("pthread_create");

	pthread_join(main_server_thread, NULL);

	return EXIT_SUCCESS;
}
