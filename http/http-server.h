#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#define SERVER_STRING "Snalmir Basic HTTP/1.0 Server"

struct request 
{
	char method[256];
	char url[256];
};

void* init_http_server();

void get_request(int, struct request*);
void send_response(int, int, const char*);

#endif
