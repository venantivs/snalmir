#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <stdbool.h>

#define SERVER_STRING "Snalmir Basic HTTP/1.1 Server"
#define MAX_REQUEST_LENGTH 1024
#define MAX_RESPONSE_LENGTH 1024

struct request 
{
	char method[256];
	char url[256];
	char host[256];
};

void	*init_http_server();

#endif
