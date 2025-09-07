/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Arquivo: core/utils.c
 * Descrição: Arquivo onde funções auxiliares como de error handling, por exemplo, são implementadas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

#include "utils.h"

struct timespec start_time;
bool started_clock = false;

/*
 * Printa um erro em stderr e sai da thread atual com código de erro.
 * Utilizar apenas em funções em que errno é apropriadamente utilizado.
 * Parâmetros: Nome da função cujo erro ocorreu
 */
void
fatal_error(const char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

void
start_clock()
{
	if (started_clock) {
		fprintf(stderr, "#start_clock chamado mais de uma vez.\n");
		return;
	}

	clock_gettime(CLOCK_MONOTONIC, &start_time);
	started_clock = true;
}

/*
 * A função clock() no Windows possui um bug conhecido, onde o mesmo retorna o "wall clock time"
 * da thread, já em sistemas POSIX, clock() retorna o tempo de CPU de um processo, onde o mesmo
 * para de contar quando o processo não está na CPU (e.g. sleep), e conta mais rápido se houver
 * mais de uma thread sob o processo, que pode ser o caso do snalmir caso o servidor HTTP esteja
 * habilitado. Então para replicar as funcionalidades que usam clock() nos emuladores/servidores
 * Windows, foi preciso reimplementar algo parecido para linux.
 * Retorna: Tempo, em milisegundos, desde que o servidor foi inicializado.
 */
clock_t
get_clock()
{
	struct timespec spec;
	clock_gettime(CLOCK_MONOTONIC, &spec);

	struct timespec result;
	result.tv_sec = spec.tv_sec - start_time.tv_sec;
	result.tv_nsec = spec.tv_nsec - start_time.tv_nsec;

	return (result.tv_sec * 1000) + lround(result.tv_nsec / 1.0e6);
}