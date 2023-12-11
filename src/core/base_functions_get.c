/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/base_functions_get.c
 * Descrição: Arquivo onde são implementadas algumas funções getters úteis.
 */

#include "base-def.h"
#include "../network/server.h"
#include "base_functions.h"

size_t
get_spawn_empty_index()
{
  for (size_t i = (BASE_MOB + 1); i < MAX_SPAWN_LIST; i++) {
    if (mobs[i].mode == MOB_EMPTY)
      return i;
  }

  return 0;
}

void
get_create_item(int item_index)
{
	struct packet_create_ground_item create_ground_item;
	struct ground_item_st *init = &ground_items[item_index];
	create_ground_item.header.index = 0x7530; // ??????
	create_ground_item.header.operation_code = 0x26E;
	create_ground_item.header.size = sizeof(struct packet_create_ground_item);
	create_ground_item.position.X = init->position_x;
	create_ground_item.position.Y = init->position_y;
	create_ground_item.item_index = init->item_index + 10000; // ?????
	memcpy(&create_ground_item.item, &init->item_data, sizeof(struct item_st));
	create_ground_item.rotation = init->rotation;
	create_ground_item.status = init->status;
	send_grid_item(item_index, &create_ground_item);
}

void
get_current_score()
{

}
