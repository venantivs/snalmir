/*
 *
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2020
 * Arquivo: core/utils.c
 * Descrição: Arquivo onde funções auxiliares, como de error por exemplo, são implementadas.
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * Printa um erro em stderr e sai da thread atual com código de erro.
 * Utilizar apenas em funções em que errno é apropriadamente utilizado.
 * Parâmetros: Nome da função cujo erro ocorreu
 */
void fatal_error(const char* s)
{
	perror(s);
	exit(EXIT_FAILURE);
}
