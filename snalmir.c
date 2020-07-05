/*
 *
 * Licença: GPLv3
 * Autor: theskytalos
 * Data: Julho de 2020
 * Arquivo: snalmir.c
 * Descrição: Arquivo principal do servidor, onde se encontra a função main.
 *  Aqui são inicializadas e tratadas as threads que compõem o servidor.
 */

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "core/utils.h"
#include "http/http-server.h"
#include "general-config.h"

int main(void)
{
	pthread_t http_server_thread;
	
	if (HTTP_SERVER_ENABLED)
		if ((errno = pthread_create(&http_server_thread, NULL, init_http_server(), NULL)) != 0)
			fatal_error("pthread_create");

	pthread_join(http_server_thread, NULL);

	return EXIT_SUCCESS;
}
