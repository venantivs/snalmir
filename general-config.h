/*
 *
 * Licença: GPLv3
 * Autor: theskytalos
 * Data: Julho de 2020
 * Arquivo: general-config.h
 * Descrição: Arquivo de configuração geral do servidor.
 */

#ifndef __GENERAL_CONFIG_H__
#define __GENERAL_CONFIG_H__

// Server Config
#define MAX_USERS_PER_CHANNEL 500	/* Máximo de usuários por canal */
#define CHANNEL_COUNT 1			/* Número de canais abertos (MAX 8) */

// HTTP Config
#define HTTP_SERVER_ENABLED 1		/* Caso 0, o servidor HTTP não será inicializado */
#define HTTP_SERVER_PORT 22000		/* Porta do servidor HTTP */
#define MAX_PENDING_CONNECTIONS 50	/* Máximo de conexões pendentes no servidor HTTP */

#endif
