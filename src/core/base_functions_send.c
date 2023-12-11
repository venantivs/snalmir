/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/base_functions_send.c
 * Descrição: Arquivo onde são implementadas algumas funções setters úteis.
 */

#include "base-def.h"
#include "../network/server.h"
#include "base_functions.h"
#include "world.h"
#include "../network/socket-utils.h"

void
send_grid_mob(int index)
{
  if (index <= 0 || index >= MAX_SPAWN_LIST)
    return;

  struct mob_st *npc_mob = &mobs[index].mob;

  short posX = npc_mob->current.X, posY = npc_mob->current.Y;

  int vision_x = VIEW_GRIDX, vision_y = VIEW_GRIDY,
      min_position_x = (posX - HALF_GRIDX),
      min_position_y = (posY - HALF_GRIDY);

  if ((min_position_x + vision_x) >= MAX_GRIDX)
    vision_x = (vision_x - (vision_x + min_position_x - MAX_GRIDX));

  if ((min_position_y + vision_y) >= MAX_GRIDY)
    vision_y = (vision_y - (vision_y + min_position_y - MAX_GRIDY));

  if (min_position_x < 0) {
    min_position_x = 0;
    vision_x = (vision_x + min_position_x);
  }

  if (min_position_y < 0) {
    min_position_y = 0;
    vision_y = (vision_y + min_position_y);
  }

  int max_position_x = (min_position_x + vision_x),
    max_position_y = (min_position_y + vision_y);

  for (int y = min_position_y; y < max_position_y; y++) {
    for (int x = min_position_x; x < max_position_x; x++) {
      short mob_id = mob_grid[y][x];
      short initID = item_grid[y][x];

      if (x > min_position_x || x <= max_position_x || y > min_position_y || y <= max_position_y)  {
        if (initID > 0 && index <= MAX_USERS_PER_CHANNEL)
          get_create_item(initID); // Implementar
      }

      if (mob_id <= 0 || index == mob_id)
        continue;

      if (mob_id <= MAX_USERS_PER_CHANNEL)
        send_create_mob(mob_id, index);

      if (index <= MAX_USERS_PER_CHANNEL)
        send_create_mob(index, mob_id);
    }
  }
}

void
send_grid_item(int index, void *packet)
{
  if (index <= 0 || index >= MAX_INIT_ITEM_LIST)
    return;

  short posX = ground_items[index].position_x,
    posY = ground_items[index].position_y;

  int VisX = VIEW_GRIDX, VisY = VIEW_GRIDY,
    minPosX = (posX - HALF_GRIDX),
    minPosY = (posY - HALF_GRIDY);

  if ((minPosX + VisX) >= MAX_GRIDX)
    VisX = (VisX - (VisX + minPosX - MAX_GRIDX));

  if ((minPosY + VisY) >= MAX_GRIDY)
    VisY = (VisY - (VisY + minPosY - MAX_GRIDY));

  if (minPosX < 0) {
    minPosX = 0;
    VisX = (VisX + minPosX);
  }

  if (minPosY < 0) {
    minPosY = 0;
    VisY = (VisY + minPosY);
  }

  int maxPosX = (minPosX + VisX),
    maxPosY = (minPosY + VisY);

  for (int nY = minPosY; nY < maxPosY; nY++) {
    for (int nX = minPosX; nX < maxPosX; nX++) {
      short mob_id = mob_grid[nY][nX];
      if (mob_id <= 0)
        continue;

      if (mob_id <= MAX_USERS_PER_CHANNEL)
        send_one_message((unsigned char*)packet, xlen(packet), mob_id);
    }
  }
}

void
send_create_mob(int index, int mob_id)
{

}
