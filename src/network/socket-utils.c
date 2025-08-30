/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2020
 * Atualização: Dezembro de 2020
 * Arquivo: network/socket-utils.c
 * Descrição: Módulo onde são implementadas as funções auxiliares para tratar da comunicação em si do servidor com o cliente, tais como manipulação de buffer.
 * TODO: Checar com calma o que é INITCODE, e tratar erros provenientes das funções aqui implementadas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include "packet-security.h"
#include "packet-def.h"
#include "server.h"
#include "../general-config.h"
#include "socket-utils.h"

unsigned char key[] = {
	0x84, 0x87, 0x37, 0xD7, 0xEA, 0x79, 0x91, 0x7D, 0x4B, 0x4B, 0x85, 0x7D, 0x87, 0x81, 0x91, 0x7C,
	0x0F, 0x73, 0x91, 0x91, 0x87, 0x7D, 0x0D, 0x7D, 0x86, 0x8F, 0x73, 0x0F, 0xE1, 0xDD, 0x85, 0x7D,
	0x05, 0x7D, 0x85, 0x83, 0x87, 0x9C, 0x85, 0x33, 0x0D, 0xE2, 0x87, 0x19, 0x0F, 0x79, 0x85, 0x86,
	0x37, 0x7D, 0xD7, 0xDD, 0xE9, 0x7D, 0xD7, 0x7D, 0x85, 0x79, 0x05, 0x7D, 0x0F, 0xE1, 0x87, 0x7E,
	0x23, 0x87, 0xF5, 0x79, 0x5F, 0xE3, 0x4B, 0x83, 0xA3, 0xA2, 0xAE, 0x0E, 0x14, 0x7D, 0xDE, 0x7E,
	0x85, 0x7A, 0x85, 0xAF, 0xCD, 0x7D, 0x87, 0xA5, 0x87, 0x7D, 0xE1, 0x7D, 0x88, 0x7D, 0x15, 0x91,
	0x23, 0x7D, 0x87, 0x7C, 0x0D, 0x7A, 0x85, 0x87, 0x17, 0x7C, 0x85, 0x7D, 0xAC, 0x80, 0xBB, 0x79,
	0x84, 0x9B, 0x5B, 0xA5, 0xD7, 0x8F, 0x05, 0x0F, 0x85, 0x7E, 0x85, 0x80, 0x85, 0x98, 0xF5, 0x9D,
	0xA3, 0x1A, 0x0D, 0x19, 0x87, 0x7C, 0x85, 0x7D, 0x84, 0x7D, 0x85, 0x7E, 0xE7, 0x97, 0x0D, 0x0F,
	0x85, 0x7B, 0xEA, 0x7D, 0xAD, 0x80, 0xAD, 0x7D, 0xB7, 0xAF, 0x0D, 0x7D, 0xE9, 0x3D, 0x85, 0x7D,
	0x87, 0xB7, 0x23, 0x7D, 0xE7, 0xB7, 0xA3, 0x0C, 0x87, 0x7E, 0x85, 0xA5, 0x7D, 0x76, 0x35, 0xB9,
	0x0D, 0x6F, 0x23, 0x7D, 0x87, 0x9B, 0x85, 0x0C, 0xE1, 0xA1, 0x0D, 0x7F, 0x87, 0x7D, 0x84, 0x7A,
	0x84, 0x7B, 0xE1, 0x86, 0xE8, 0x6F, 0xD1, 0x79, 0x85, 0x19, 0x53, 0x95, 0xC3, 0x47, 0x19, 0x7D,
	0xE7, 0x0C, 0x37, 0x7C, 0x23, 0x7D, 0x85, 0x7D, 0x4B, 0x79, 0x21, 0xA5, 0x87, 0x7D, 0x19, 0x7D,
	0x0D, 0x7D, 0x15, 0x91, 0x23, 0x7D, 0x87, 0x7C, 0x85, 0x7A, 0x85, 0xAF, 0xCD, 0x7D, 0x87, 0x7D,
	0xE9, 0x3D, 0x85, 0x7D, 0x15, 0x79, 0x85, 0x7D, 0xC1, 0x7B, 0xEA, 0x7D, 0xB7, 0x7D, 0x85, 0x7D,
	0x85, 0x7D, 0x0D, 0x7D, 0xE9, 0x73, 0x85, 0x79, 0x05, 0x7D, 0xD7, 0x7D, 0x85, 0xE1, 0xB9, 0xE1,
	0x0F, 0x65, 0x85, 0x86, 0x2D, 0x7D, 0xD7, 0xDD, 0xA3, 0x8E, 0xE6, 0x7D, 0xDE, 0x7E, 0xAE, 0x0E,
	0x0F, 0xE1, 0x89, 0x7E, 0x23, 0x7D, 0xF5, 0x79, 0x23, 0xE1, 0x4B, 0x83, 0x0C, 0x0F, 0x85, 0x7B,
	0x85, 0x7E, 0x8F, 0x80, 0x85, 0x98, 0xF5, 0x7A, 0x85, 0x1A, 0x0D, 0xE1, 0x0F, 0x7C, 0x89, 0x0C,
	0x85, 0x0B, 0x23, 0x69, 0x87, 0x7B, 0x23, 0x0C, 0x1F, 0xB7, 0x21, 0x7A, 0x88, 0x7E, 0x8F, 0xA5,
	0x7D, 0x80, 0xB7, 0xB9, 0x18, 0xBF, 0x4B, 0x19, 0x85, 0xA5, 0x91, 0x80, 0x87, 0x81, 0x87, 0x7C,
	0x0F, 0x73, 0x91, 0x91, 0x84, 0x87, 0x37, 0xD7, 0x86, 0x79, 0xE1, 0xDD, 0x85, 0x7A, 0x73, 0x9B,
	0x05, 0x7D, 0x0D, 0x83, 0x87, 0x9C, 0x85, 0x33, 0x87, 0x7D, 0x85, 0x0F, 0x87, 0x7D, 0x0D, 0x7D,
	0xF6, 0x7E, 0x87, 0x7D, 0x88, 0x19, 0x89, 0xF5, 0xD1, 0xDD, 0x85, 0x7D, 0x8B, 0xC3, 0xEA, 0x7A,
	0xD7, 0xB0, 0x0D, 0x7D, 0x87, 0xA5, 0x87, 0x7C, 0x73, 0x7E, 0x7D, 0x86, 0x87, 0x23, 0x85, 0x10,
	0xD7, 0xDF, 0xED, 0xA5, 0xE1, 0x7A, 0x85, 0x23, 0xEA, 0x7E, 0x85, 0x98, 0xAD, 0x79, 0x86, 0x7D,
	0x85, 0x7D, 0xD7, 0x7D, 0xE1, 0x7A, 0xF5, 0x7D, 0x85, 0xB0, 0x2B, 0x37, 0xE1, 0x7A, 0x87, 0x79,
	0x84, 0x7D, 0x73, 0x73, 0x87, 0x7D, 0x23, 0x7D, 0xE9, 0x7D, 0x85, 0x7E, 0x02, 0x7D, 0xDD, 0x2D,
	0x87, 0x79, 0xE7, 0x79, 0xAD, 0x7C, 0x23, 0xDA, 0x87, 0x0D, 0x0D, 0x7B, 0xE7, 0x79, 0x9B, 0x7D,
	0xD7, 0x8F, 0x05, 0x7D, 0x0D, 0x34, 0x8F, 0x7D, 0xAD, 0x87, 0xE9, 0x7C, 0x85, 0x80, 0x85, 0x79,
	0x8A, 0xC3, 0xE7, 0xA5, 0xE8, 0x6B, 0x0D, 0x74, 0x10, 0x73, 0x33, 0x17, 0x0D, 0x37, 0x21, 0x19
};

void
refresh_recv_buffer(int user_index)
{
	struct user_server_buffering_st *buffer = &g_users[user_index].server_data.buffer;
	int left = buffer->recv_position - buffer->proc_position;

	if (left <= 0)
		return;

	memcpy(buffer->recv_buffer, buffer->recv_buffer + buffer->proc_position, left);

	buffer->proc_position = 0;
	buffer->recv_position = left;
}

void
refresh_send_buffer(int user_index)
{
	struct user_server_buffering_st *buffer = &g_users[user_index].server_data.buffer;
	int left = buffer->send_position - buffer->sent_position;

	if (left <= 0)
		return;

	memcpy(buffer->send_buffer, buffer->send_buffer + buffer->sent_position, left);

	buffer->sent_position = 0;
	buffer->send_position = left;
}

/*
 * Essa função precisa retornar erros pra fechar o socket lá fora.
 */
unsigned char *
read_client_message(int user_index)
{
	struct user_server_buffering_st *buffer = &g_users[user_index].server_data.buffer;

	if (buffer->proc_position >= buffer->recv_position) {
		buffer->recv_position = 0;
		buffer->proc_position = 0;
		return NULL;
	}

	if (!buffer->initialized) {
		if (buffer->recv_position - buffer->proc_position < 4)
			return NULL;

		int init_code = *((unsigned int *)(buffer->recv_buffer + buffer->proc_position));

		if (init_code != INITCODE) {
			/* ERRO FUDIDO */
			/* FECHA SOCKET */
			return NULL;
		}

		buffer->initialized = true;
		buffer->proc_position += 4;
		return NULL;
	}

	unsigned short size = *((unsigned short *)(buffer->recv_buffer + buffer->proc_position));

	if (size > MAX_MESSAGE_SIZE || size < 12) {
		/* ERRO DE TAMANHO DE MENSAGEM */
		buffer->recv_position = 0;
		buffer->proc_position = 0;
		return NULL;
	}

	unsigned short rest = buffer->recv_position - buffer->proc_position;

	if (size > rest)
    	return NULL;

	unsigned char *message = &(buffer->recv_buffer[buffer->proc_position]);

	buffer->proc_position += size;

	if (buffer->recv_position <= buffer->proc_position) {
		buffer->recv_position = 0;
		buffer->proc_position = 0;	
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
	if (g_users[user_index].server_data.socket_fd < 3)
		return false;

	if (message == NULL)
		return false;

	struct user_server_buffering_st *buffer = &g_users[user_index].server_data.buffer;

	struct packet_header *header = (struct packet_header *)message;

	if (buffer->send_position + size >= SEND_BUFFER_SIZE) {
		refresh_send_buffer(user_index);
		if (buffer->send_position + size >= SEND_BUFFER_SIZE)
			return false;
	}

	if (size < 0 || size > MAX_MESSAGE_SIZE)
		return false;

	int rand_key = (rand() % 255);
	header->size = size;
	header->key = rand_key;
	header->checksum = 0;

	/* CUSTOM ENCRYPT FOR TESTING - START */
	// int i;
	// int pos, Key, rand_key;
	// int sum1 = 0, sum2 = 0;
	// rand_key = (rand() % 255);
	// header->size = size;
	// header->key = rand_key;
	// header->checksum = 0;
	// pos = key[rand_key * 2];
	// for (i = 4; i < size; i++, pos++)
	// {
	// 	Key = key[((pos & 0xFF) * 2) + 1];
	// 	switch (i & 3)
	// 	{
	// 	case 0:
	// 		Key *= 2;
	// 		Key &= 255;
	// 		buffer->send_buffer[buffer->send_position + i] = message[i] + Key;
	// 		break;
	// 	case 1:
	// 		Key >>= 3;
	// 		Key &= 255;
	// 		buffer->send_buffer[buffer->send_position + i] = message[i] - Key;
	// 		break;
	// 	case 2:
	// 		Key *= 4;
	// 		Key &= 255;
	// 		buffer->send_buffer[buffer->send_position + i] = message[i] + Key;
	// 		break;
	// 	default: case 3:
	// 		Key >>= 5;
	// 		Key &= 255;
	// 		buffer->send_buffer[buffer->send_position + i] = message[i] - Key;
	// 		break;
	// 	}
	// 	printf("%hhx ", buffer->send_buffer[buffer->send_position + i]);
		
	// 	sum1 += message[i];
	// 	sum2 += buffer->send_buffer[buffer->send_position + i];
	// }
	// printf("\n");
	// sum2 &= 0xFF;
	// sum1 &= 0xFF;
	// header->checksum = ((sum2 - sum1) & 255);

	/* CUSTOM ENCRYPT FOR TESTING - END */

	encrypt(&message);

	memcpy(buffer->send_buffer + buffer->send_position, message, xlen(message));
	buffer->queued_messages_count++;
	buffer->send_position += size;

	if (buffer->queued_messages_count > 25) {
		send_all_messages(user_index);
		buffer->queued_messages_count = 0;
	}

	return true;
}

bool
receive(int user_index)
{
	struct user_server_buffering_st *buffer = &g_users[user_index].server_data.buffer;

	int rest = RECV_BUFFER_SIZE - buffer->recv_position;
	int received_bytes = recv(g_users[user_index].server_data.socket_fd, (unsigned char*)(buffer->recv_buffer + buffer->recv_position), rest, 0);

	if (received_bytes < 0) {
		perror("recv");
		return false;
	}

	if (received_bytes == 0)
		return false;
	
	buffer->recv_position += received_bytes;

	/* NOTE: Acho que dá pra melhorar isso aqui */
	if (received_bytes == rest) {
		refresh_recv_buffer(user_index);
		rest = RECV_BUFFER_SIZE - buffer->recv_position;
		received_bytes = recv(g_users[user_index].server_data.socket_fd, (unsigned char*)(buffer->recv_buffer + buffer->recv_position), rest, 0);
		buffer->recv_position += received_bytes;
	}

	return true;
}

bool
send_one_message(unsigned char* message, size_t size, int user_index)
{
	if (g_users[user_index].server_data.socket_fd < 3)
		return false;

	bool message_added = add_client_message(message, size, user_index);
	message_added = send_all_messages(user_index);

	return message_added;
}

bool
send_all_messages(int user_index)
{
	if (g_users[user_index].server_data.socket_fd < 3)
		return false;

	struct user_server_buffering_st *buffer = &g_users[user_index].server_data.buffer;

	if (buffer->sent_position > 0)
		refresh_send_buffer(user_index);

	if (buffer->send_position > SEND_BUFFER_SIZE || buffer->send_position < 0 || g_users[user_index].server_data.socket_fd < 3) {
		buffer->send_position = 0;
		buffer->sent_position = 0;
		return false;
	}

	if (buffer->sent_position > buffer->send_position || buffer->sent_position >= SEND_BUFFER_SIZE || buffer->sent_position < 0)
	{
		buffer->send_position = 0;
		buffer->sent_position = 0;
	}

	int left = buffer->send_position - buffer->sent_position;

	// FLUSHING
	int flag = 1;
	setsockopt(g_users[user_index].server_data.socket_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
	int sent_bytes = send(g_users[user_index].server_data.socket_fd, buffer->send_buffer + buffer->sent_position, left, 0);
	flag = 0;
	setsockopt(g_users[user_index].server_data.socket_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

	printf("sent_bytes: %d\n", sent_bytes);

	if (sent_bytes >= 0)
		buffer->sent_position += sent_bytes;
	else
		perror("send");

	if (buffer->sent_position < buffer->send_position || sent_bytes < 0) {
		/* ? */
	} else {
		buffer->send_position = 0;
		buffer->sent_position = 0;
		return true;
	}

	return true;
}

void
send_client_string_message(const char* message, int user_index)
{
	struct packet_string_message string_message;

	string_message.header.size = sizeof(struct packet_string_message);
	string_message.header.operation_code = 0x101;
	strncpy(string_message.string_message, message, 96);

	send_one_message((unsigned char*) &string_message, xlen(&string_message), user_index);
}

void
send_signal(short operation_code, int user_index)
{
	struct packet_signal signal = { 0 };

	signal.header.index = user_index;
	signal.header.size = sizeof(struct packet_signal);
	signal.header.operation_code = operation_code;

	send_one_message((unsigned char*) &signal, xlen(&signal), user_index);
}
