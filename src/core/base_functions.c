/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/base_functions.c
 * Descrição: Arquivo onde são implementadas algumas funções getters úteis.
 */

#include "base-def.h"
#include "../network/server.h"

size_t
get_spawn_empty_index()
{
  for (size_t i = (BASE_MOB + 1); i < MAX_SPAWN_LIST; i++) {
    if (mobs[i].mode == MOB_EMPTY)
      return i;
  }

  return 0;
}
