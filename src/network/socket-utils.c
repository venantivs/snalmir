/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2020
 * Arquivo: network/socket-utils.c
 * Descrição: Módulo onde são implementadas as funções auxiliares para tratar da comunicação em si do servidor com o cliente, tais como manipulação de buffer.
 * TODO: Checar com calma o que é INITCODE, e tratar erros provenientes das funções aqui implementadas.
 */

#include <stdlib.h>
#include <string.h>

#include "packet-security.h"
#include "packet-def.h"
#include "server.h"
#include "../general-config.h"
#include "socket-utils.h"

extern struct user_server_st users[MAX_USERS_PER_CHANNEL];

void
refresh_recv_buffer(int user_index)
{
	struct user_server_buffering_st buffer = users[user_index].server_data.buffer;
	int left = buffer.recv_position - buffer.proc_position;

	if (left <= 0)
		return;

	memcpy(buffer.recv_buffer, buffer.recv_buffer + buffer.proc_position, left);

	buffer.proc_position = 0;
	buffer.recv_position = left;
}

void
refresh_send_buffer(int user_index)
{
	struct user_server_buffering_st buffer = users[user_index].server_data.buffer;
	int left = buffer.send_position - buffer.sent_position;

	if (left <= 0)
		return;

	memcpy(buffer.send_buffer, buffer.send_buffer + buffer.sent_position, left);

	buffer.sent_position = 0;
	buffer.send_position = left;
}

unsigned char *
read_client_message(int user_index)
{
	struct user_server_buffering_st buffer = users[user_index].server_data.buffer;

	if (buffer.proc_position >= buffer.recv_position) {
		buffer.recv_position = 0;
		buffer.proc_position = 0;
		return NULL;
	}

	if (!buffer.initialized) {
		if (buffer.recv_position - buffer.proc_position < 4)
			return NULL;
		
		int init_code = *((unsigned int *)(buffer.recv_buffer + buffer.proc_position));

		if (init_code != INITCODE) {
			/* ERRO FUDIDO */
			/* FECHA SOCKET */
			return NULL;
		}

		buffer.initialized = true;
		buffer.proc_position += 4;
		return NULL;
	}

	unsigned short size = *((unsigned short *)(buffer.recv_buffer + buffer.proc_position));

	if (size > MAX_MESSAGE_SIZE || size < 12) {
		/* ERRO DE TAMANHO DE MENSAGEM */
		buffer.recv_position = 0;
		buffer.proc_position = 0;
		return NULL;
	}

	unsigned short rest = buffer.recv_position - buffer.proc_position;

	if (size > rest)
		return NULL;

	unsigned char *message = &(buffer.recv_buffer[buffer.proc_position]);

	buffer.proc_position += size;

	if (buffer.recv_position <= buffer.proc_position) {
		buffer.recv_position = 0;
		buffer.proc_position = 0;	
	}

	if (!decrypt(&message)) {
		/* ERRO DE DECRIPTAÇÃO */
		return NULL;
	}

	return message;
}

bool
add_client_message(unsigned char *message, size_t size, int user_index)
{
	if (users[user_index].server_data.socket_fd < 3)
		return false;

	if (message == NULL)
		return false;

	struct user_server_buffering_st buffer = users[user_index].server_data.buffer;

	struct packet_header *header = (struct packet_header *)message;

	if (buffer.send_position + size >= SEND_BUFFER_SIZE) {
		refresh_send_buffer(user_index);
		if (buffer.send_position + size >= SEND_BUFFER_SIZE)
			return false;
	}

	if (size < 0 || size > MAX_MESSAGE_SIZE)
		return false;

	int rand_key = (rand() % 255);
	header->size = size;
	header->key = rand_key;
	header->checksum = 0;

	encrypt(&message);

	memcpy(buffer.send_buffer + buffer.send_position, message, 4);
	buffer.queued_messages_count++;
	buffer.send_position += size;

	if (buffer.queued_messages_count > 25) {
		send_all_messages(user_index);
		buffer.queued_messages_count = 0;
	}

	return true;
}

bool
receive(int user_index)
{
	return false;
}

bool
send_one_message(int user_index)
{
	return false;
}

bool
send_all_messages(int user_index)
{
	return false;
}