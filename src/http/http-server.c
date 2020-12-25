/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Agosto de 2020
 * Arquivo: http/http-server.c
 * Descrição: Módulo servidor HTTP básico do servidor Snalmir.
 *  Responde apenas para GET /, qualquer outro método de requisição ou url resultará em códigos HTTP de status de erro (400, 404, 501, etc).
 *  Serve apenas para entregar dados sobre as quantidades de players dos canais de um servidor.
 * TODO: Atomizar init_http_server().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include "../core/utils.h"
#include "../general-config.h"
#include "http-server.h"

static bool get_request(int comm_fd, struct request* request_info);
static void send_response(int comm_fd, int status_code, const char* content);

void *
init_http_server()
{
	int master_socket_fd, comm_fd;

	struct sockaddr_in serv_addr;
	struct request request_info;

	master_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (master_socket_fd < 0)
		fatal_error("socket");

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
	serv_addr.sin_port = htons(HTTP_SERVER_PORT);

	if (bind(master_socket_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		fatal_error("bind");

	if (listen(master_socket_fd, HTTP_MAX_PENDING_CONNECTIONS) < 0)
		fatal_error("listen");

	while (true) {
		comm_fd = accept(master_socket_fd, (struct sockaddr*) NULL, NULL);
		
		if (comm_fd < 0)	
			fatal_error("accept");

		if (!get_request(comm_fd, &request_info))
			goto error_out;

		if (!strcasecmp(request_info.method, "POST") || !strcasecmp(request_info.method, "HEAD") ||
		    !strcasecmp(request_info.method, "PUT") || !strcasecmp(request_info.method, "DELETE") ||
		    !strcasecmp(request_info.method, "CONNECT") || !strcasecmp(request_info.method, "OPTIONS") ||
		    !strcasecmp(request_info.method, "TRACE") || !strcasecmp(request_info.method, "PATCH")) {
			send_response(comm_fd, 501, "Not Implemented");
			goto error_out;
		}

		if (strcasecmp(request_info.method, "GET")) {
			send_response(comm_fd, 400, "Bad Request");
			goto error_out;
		}

		if (strcasecmp(request_info.url, "/serv00")) {
			send_response(comm_fd, 404, "Not Found");
			goto error_out;
		}

		send_response(comm_fd, 200, "100\r\n-1\r\n-1\r\n-1\r\n-1\r\n-1\r\n-1\r\n-1\r\n-1\r\n");

error_out:		
		close(comm_fd);
	}

	return NULL;
}

/*
 * Lê a requisição vinda do socket representado pelo descritor de
 * arquivo 'comm_fd', e preenche 'request_info' com o método e a
 * url da requisição.
 * Parâmetros: Descritor de arquivo do socket de conexão com cliente
 *             Ponteiro para a struct de retorno 
 */
static bool
get_request(int comm_fd, struct request* request_info)
{
	char buffer[MAX_REQUEST_LENGTH] = {'\0'};

	/* Checa se há mais bytes no cabeçalho do que o máximo especificado em MAX_REQUEST_LENGTH. */
	/* if (available_bytes > MAX_REQUEST_LENGTH) {
		send_response(comm_fd, 413, "Entity Too Large");
		return false;
	} */
			
	memset(request_info, 0, sizeof(struct request));
		
	ssize_t bytes_read = recv(comm_fd, buffer, sizeof(buffer) - 1, 0);

	size_t i = 0, j = 0;
	/* Adquire o método (verbo) HTTP da request */
	while (!isspace(buffer[i]) && i < bytes_read && (i < sizeof(buffer) - 1) && i < sizeof(request_info->method)) {
		request_info->method[i] = buffer[i];
		i++;
	}

	request_info->method[i] = '\0';

	/* 'Caminha' até a próxima palavra */
	while (isspace(buffer[i]) && i < (sizeof(buffer) - 1))
		i++;

	/* Pega o URL da request */
	while (!isspace(buffer[i]) && i < bytes_read && (i < sizeof(buffer) - 1) && j < sizeof(request_info->url)) {
		request_info->url[j] = buffer[i];
		i++; j++;
	}

	request_info->url[j] = '\0';

	char *host_pos = strstr(buffer, "Host"); 
	/* Por padrão (RFC 2616 e 7230), o campo 'host' é obrigatório no cabeçalho da request HTTP 1.1. */
	if (host_pos == NULL) {
		send_response(comm_fd, 400, "Bad Request");
		return false;
	}

	i = 0; j = 0;
	while (!isspace(host_pos[i]) && i < strlen(host_pos))
		i++;

	while (isspace(host_pos[i]) && i < strlen(host_pos))
		i++;

	/* Adquire o campo host do cabeçalho da request. */
	while (host_pos[i] != '\r' && host_pos[i] != '\n' && j < sizeof(request_info->host)) {
		request_info->host[j] = host_pos[i];
		i++; j++;
	}

	return true;
}

/*
 * Envia uma response com o código de status 'status_code' e conteúdo
 * 'content' de volta para o cliente espeficicado pelo descritor de
 * arquivo 'comm_fd'. O limite da response é deifinida em MAX_RESPO
 * NSE_LENGTH. Para responses de tamanho variável (alocadas 
 * dinamicamente) ver a função open_memstream(3). 
 *
 * Parâmetros: Descritor de arquivo do socket de conexão com cliente
 *	       Código de status HTTP (200, 400, 404, 413, 501)
 *             Conteúdo da response
 */
static void
send_response(int comm_fd, int status_code, const char* content)
{
	char buffer[MAX_RESPONSE_LENGTH] = {'\0'};
	
	FILE *buffer_stream = fmemopen(buffer, sizeof(buffer), "w");

	if (buffer_stream == NULL)
		fatal_error("fmemopen");

	char time_buffer[128] = {0};
	time_t now = time(0);
	struct tm time_st = *gmtime(&now);

	strftime(time_buffer, sizeof(time_buffer), "%a, %d %b %Y %H:%M:%S %Z", &time_st);

	switch (status_code) {
	case 200: fprintf(buffer_stream, "HTTP/1.1 200 OK\r\n"); break;
	case 400: fprintf(buffer_stream, "HTTP/1.1 400 Bad Request\r\n"); break;
	case 404: fprintf(buffer_stream, "HTTP/1.1 404 Not Found\r\n"); break;
	case 413: fprintf(buffer_stream, "HTTP/1.1 413 Entity Too Large\r\n"); break;
	case 501: fprintf(buffer_stream, "HTTP/1.1 501 Not Implemented\r\n"); break;
	default:  fprintf(buffer_stream, "HTTP/1.1 500 Internal Server Error\r\n");
	}

	fprintf(buffer_stream, "Date: %s\r\n", time_buffer);
	fprintf(buffer_stream, "Server: %s\r\n", SERVER_STRING);
	fprintf(buffer_stream, "Content-Length: %zu\r\n", strlen(content));
	fprintf(buffer_stream, "Connection: close\r\n");
	fprintf(buffer_stream, "\r\n");
	fprintf(buffer_stream, "%s\r\n", content);
	fclose(buffer_stream);

	send(comm_fd, buffer, sizeof(buffer), 0);
}
