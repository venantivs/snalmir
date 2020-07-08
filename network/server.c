/*
 * Licença: GPLv3
 * Autor: theskytalos
 * Data: Julho de 2020
 * Arquivo: network/server.c
 * Descrição: Arquivo onde o servidor principal é propriamente inicializado, recebe e envia pacotes.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "../core/utils.h"
#include "../general-config.h"
#include "server.h"

void* start_server()
{
    	int master_socket_fd, new_socket, client_socket[MAX_USERS_PER_CHANNEL], bytes_read;
	int max_sd;
    	struct sockaddr_in server_address;
     
        unsigned char buffer[1025];
     
    	// Set of socket descriptors.
    	fd_set readfds;
     
    	for (size_t i = 0; i < MAX_USERS_PER_CHANNEL; i++) 
        	client_socket[i] = 0;
     
    	if((master_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        	fatal_error("socket");
 
    	// Set master socket to allow multiple connections.
    	if(setsockopt(master_socket_fd, SOL_SOCKET, SO_REUSEADDR, NULL, 0) < 0)
		fatal_error("setsocketopt");
 
    	server_address.sin_family = AF_INET;
    	server_address.sin_addr.s_addr = INADDR_ANY;
    	server_address.sin_port = htons(SERVER_PORT);
     
    	if (bind(master_socket_fd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) 
		fatal_error("bind");
	
    	if (listen(master_socket_fd, 3) < 0)
    		fatal_error("listen");
    
	while(true)
    	{
        	FD_ZERO(&readfds);
        	FD_SET(master_socket_fd, &readfds);

		max_sd = master_socket_fd;
		
        	// Add child sockets to set
        	for (size_t i = 0; i < MAX_USERS_PER_CHANNEL; i++) 
        	{
			// If valid socket descriptor then add to read list
			if(client_socket[i] > 0)
				FD_SET(client_socket[i], &readfds);
            
            		// Highest file descriptor number, need it for the select function
            		if(client_socket[i] > max_sd)
				max_sd = client_socket[i];
        	}
 
        	// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
        	if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0)
			perror("select");
   
        	if (errno != EINTR) 
            		perror("select");
         
        	// If something happened on the master socket, then its an incoming connection
        	if (FD_ISSET(master_socket_fd, &readfds)) 
        	{
            		if ((new_socket = accept(master_socket_fd, (struct sockaddr*) &server_address, (socklen_t*) sizeof(server_address))) < 0)
            			fatal_error("accept");
         
            		printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
       
            		// Add new socket to array of sockets
            		for (size_t i = 0; i < MAX_USERS_PER_CHANNEL; i++) 
            		{
				if(client_socket[i] == 0)
                		{
                    			client_socket[i] = new_socket;
                    			printf("Adding to list of sockets as %ld\n", i);
					break;
                		}
            		}
        	}
         
        	// Else its some IO operation on some other socket
        	for (size_t i = 0; i < MAX_USERS_PER_CHANNEL; i++) 
        	{
            		if (FD_ISSET(client_socket[i], &readfds)) 
            		{
                		// Check if it was for closing, and also read the incoming message
                		if ((bytes_read = recv(client_socket[i], buffer, 1024, 0)) == 0)
                		{
                    			// Somebody disconnected, get his details and print
                    			getpeername(client_socket[i], (struct sockaddr*) &server_address, (socklen_t*) sizeof(server_address));
                    			printf("Host disconnected, ip %s, port %d\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
                    			close(client_socket[i]);
                    			client_socket[i] = 0;
                		}
                		else
                		{
					for (size_t j = 0; j < bytes_read; j++)
						printf("%hhx ", buffer[j]);
					printf("\n");
                		}
            		}
        	}
    	}
}
