/*
 *
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Atualização: Agosto de 2025
 * Arquivo: general-config.h
 * Descrição: Arquivo de configuração geral do servidor.
 */

#ifndef __GENERAL_CONFIG_H__
#define __GENERAL_CONFIG_H__

/* Configuração do servidor */
#define SERVER_PORT 8281		/* Porta do servidor. 8281 para Malech (?). */
#define MAX_USERS_PER_CHANNEL 750	/* Máximo de usuários por canal. */
#define CHANNEL_COUNT 1			/* Número de canais abertos (MAX 8). */
#define CLIENT_VERSION 754		/* Não tenho certeza se da 754 pra 755 só muda a CLIVER. */
#define MAX_PENDING_CONNECTIONS 12	/* Máximo de conexões pendentes no servidor. */
#define EXP_RATE 1 /* Rate de EXP (MIN: 1, MAX: 32768). */
#define DROP_RATE 1 /* Rate de Drop (MIN: 1, MAX: 100). */

/* Configuração do servidor HTTP */
#define HTTP_SERVER_ENABLED 1		/* Caso 0, o servidor HTTP não será inicializado. */
#define HTTP_SERVER_PORT 80		/* Porta do servidor HTTP. */
#define HTTP_MAX_PENDING_CONNECTIONS 50	/* Máximo de conexões pendentes no servidor HTTP. */

/* Configurações de rede */
#define RECV_BUFFER_SIZE (128 * 1024)	/* Tamanho do buffer das mensagens recebidas do cliente. */
#define SEND_BUFFER_SIZE (128 * 1024)	/* Tamanho do buffer das mensagens enviadas ao cliente. */

#endif
