/*
 *
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Arquivo: http/http-server.c
 * Descrição: Módulo servidor HTTP básico do servidor Snalmir.
 *  Responde apenas para GET /, qualquer outro método de requisição ou url resultará em códigos HTTP de status de erro (400, 404, 501, etc).
 *  Serve apenas para entregar dados sobre as quantidades de players dos canais de um servidor.
 * TODO: Atomizar init_http_server(). Padronizar a response de acordo com HTTP/1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "../core/utils.h"
#include "../general-config.h"
#include "http-server.h"

void* init_http_server()
{
	int master_socket_fd, comm_fd;

	struct sockaddr_in serv_addr;
	struct request request_info;

	master_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (master_socket_fd == -1)
		fatal_error("socket");

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
	serv_addr.sin_port = htons(HTTP_SERVER_PORT);

	if (bind(master_socket_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
		fatal_error("bind");

	if (listen(master_socket_fd, 10) == -1)
		fatal_error("listen");

	while(true)
	{
		comm_fd = accept(master_socket_fd, (struct sockaddr*) NULL, NULL);

		if (comm_fd == -1)
			fatal_error("accept");

		get_request(comm_fd, &request_info);

		if (!strcasecmp(request_info.method, "POST") || !strcasecmp(request_info.method, "HEAD"))
		{
			send_response(comm_fd, 501, "Not Implemented");
			goto error_out;
		}

		if (strcasecmp(request_info.method, "GET"))
		{
			send_response(comm_fd, 400, "Bad Request");
			goto error_out;
		}

		if (strcasecmp(request_info.url, "/serv00.htm"))
		{
			send_response(comm_fd, 404, "Not Found");
			goto error_out;
		}

		send_response(comm_fd, 200, "0\r\n-1\r\n-1\r\n-1\r\n-1\r\n-1\r\n-1\r\n-1\r\n-1\r\n");

error_out:		
		close(comm_fd);
	}

	exit(EXIT_SUCCESS);
}

/**********************************************************************/
/* Lê a requisição vinda do socket representado pelo descritor de
 * arquivo 'comm_fd', e preenche 'request_info' com o método e a
 * url da requisição.
 * Parâmetros: Descritor de arquivo do socket de conexão com cliente
 *             Ponteiro para a struct de retorno */
/**********************************************************************/
void get_request(int comm_fd, struct request* request_info)
{
	char buffer[8192] = {'\0'};

	memset(request_info, 0, sizeof(struct request));
	ssize_t bytes_read = recv(comm_fd, buffer, sizeof(buffer), 0);

	size_t i = 0, j = 0;
	// Get HTTP Request Method
	while (!isspace(buffer[i]) && i < bytes_read && i < (sizeof(buffer) - 1))
	{
		request_info->method[i] = buffer[i];
		i++;
	}

	request_info->method[i] = '\0';

	// Walk to the next word
	while (isspace(buffer[i]) && i < (sizeof(buffer) - 1))
		i++;

	// Get HTTP Request URL
	while (!isspace(buffer[i]) && i < bytes_read && i < (sizeof(buffer) - 1))
	{
		request_info->url[j] = buffer[i];
		i++; j++;
	}

	request_info->url[j] = '\0';
}

/**********************************************************************/
/* Envia uma response com o código de status 'status_code' e conteúdo
 * 'content' de volta para o cliente espeficicado pelo descritor de
 * arquivo 'comm_fd'. A response como um todo é limitada em 1024
 * caracteres. Para responses com tamanho variável (alocadas 
 * dinamicamente) ver a função open_memstream(3). 
 *
 * Parâmetros: Descritor de arquivo do socket de conexão com cliente
 *	       Código de status HTTP (200, 400, 404, 501)
 *             Conteúdo da response */
/**********************************************************************/
void send_response(int comm_fd, int status_code, const char* content)
{
	char buffer[1024] = { 0 };
	FILE *buffer_stream = fmemopen(buffer, sizeof(buffer), "w");

	if (buffer_stream == NULL)
		fatal_error("fmemopen");

	switch (status_code)
	{
		case 200: fprintf(buffer_stream, "HTTP/1.0 200 OK\r\n"); break;
		case 400: fprintf(buffer_stream, "HTTP/1.0 400 Bad Request\r\n"); break;
		case 404: fprintf(buffer_stream, "HTTP/1.0 404 Not Found\r\n"); break;
		case 501: fprintf(buffer_stream, "HTTP/1.0 501 Not Implemented\r\n"); break;
		default:  fprintf(buffer_stream, "HTTP/1.0 500 Internal Server Error\r\n");
	}

	fprintf(buffer_stream, "Server: %s\r\n", SERVER_STRING);
	fprintf(buffer_stream, "Content-Type: text/html\r\n");
	fprintf(buffer_stream, "\r\n");
	fprintf(buffer_stream, "%s\r\n", content);
	fclose(buffer_stream);

	send(comm_fd, buffer, sizeof(buffer), 0);
}
