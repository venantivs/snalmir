/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/world.c
 * Descrição: Arquivo onde são implementadas as funções relacionadas ao mundo em si.
 */

#include "../network/server.h"
#include "world.h"

#include <stdio.h>

short mob_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST] = { 0 };
short item_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST] = { 0 };
int height_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST] = { 0 };

bool
update_world(int index, short *position_x, short *position_y, int flag)
{
  bool empty_grid = false;
  switch(flag) {
  case WORLD_MOB:
    if (index >= MAX_SPAWN_LIST || index < 0) return false;

    empty_grid = get_empty_mob_grid(index, position_x, position_y);

    printf("empty_grid: %d\n", empty_grid);

    if (empty_grid) {
      mob_grid[mobs[index].mob.current.Y][mobs[index].mob.current.X] = 0;
      mob_grid[*position_y][*position_x] = index;
      return true;
    } else
      return false;
  case WORLD_ITEM:
    if (index >= MAX_INIT_ITEM_LIST || index < 0)
      return false;

    empty_grid = get_empty_item_grid(index, position_x, position_y);

    if (empty_grid)
    {
      item_grid[*position_y][*position_x] = index;
      return true;
    } else
      return false;
  default:
    return false;
  }
}

/* REFATORAR ISSO */
bool
get_empty_mob_grid(int index, short *position_x, short *position_y)
{
  if (*position_x < 0 || *position_x >= MAX_GRIDX || *position_y < 0 || *position_y >= MAX_GRIDY) return false;

  if (mob_grid[*position_y][*position_x] == index) return true;

  if (mob_grid[*position_y][*position_x] == 0)
  if (height_grid[*position_y][*position_x] != 127)
    return true;

  for (int y = (*position_y - 1); y <= (*position_y + 1); y++)
  {
    for (int x = (*position_x - 1); x <= (*position_x + 1); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          *position_x = x;
          *position_y = y;
          return true;
        }
      }
    }
  }

  for (int y = (*position_y - 2); y <= (*position_y + 2); y++)
  {
    for (int x = (*position_x - 2); x <= (*position_x + 2); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          *position_x = x;
          *position_y = y;
          return true;
        }
      }
    }
  }

  for (int y = (*position_y - 3); y <= (*position_y + 3); y++)
  {
    for (int x = (*position_x - 3); x <= (*position_x + 3); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          *position_x = x;
          *position_y = y;
          return true;
        }
      }
    }
  }

  for (int y = (*position_y - 4); y <= (*position_y + 4); y++)
  {
    for (int x = (*position_x - 4); x <= (*position_x + 4); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          *position_x = x;
          *position_y = y;
          return true;
        }
      }
    }
  }

  for (int y = (*position_y - 5); y <= (*position_y + 5); y++)
  {
    for (int x = (*position_x - 5); x <= (*position_x + 5); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          *position_x = x;
          *position_y = y;
          return true;
        }
      }
    }
  }

  for (int y = (*position_y - 6); y <= (*position_y + 6); y++)
  {
    for (int x = (*position_x - 6); x <= (*position_x + 6); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          *position_x = x;
          *position_y = y;
          return true;
        }
      }
    }
  }

  return false;
}

bool
get_empty_item_grid(int index, short *position_x, short *position_y)
{
  if (*position_x < 0 || *position_x >= 4096 || *position_y < 0 || *position_y >= 4096)
    return false;

  if (item_grid[*position_y][*position_x] == index)
    return true;

  if (item_grid[*position_y][*position_x] == 0) // ??????
  if (height_grid[*position_y][*position_x] != 127)
    return true;

  for (int y = (*position_y - 1); y <= (*position_y + 1); y++) {
    for (int x = (*position_x - 1); x <= (*position_x + 1); x++) {
      if (x < 0 || y < 0 || x >= 4096 || y >= 4096)
        continue;

      if (item_grid[y][x] == 0) {
        if (height_grid[y][x] != 127) {
          *position_x = x;
          *position_y = y;
          return true;
        }
      }
    }
  }

  return false;
}
