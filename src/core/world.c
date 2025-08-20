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
#include "game_items.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

short mob_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST] = { 0 };
short item_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST] = { 0 };
int height_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST] = { 0 };
struct guildzone_st guild_zone[MAX_GUILD_ZONE] = { 0 };
struct teleport_st teleport_list[MAX_TELEPORT] = { 0 };
int teleport_count = 0;
bool at_war = false;
int max_item_init = 0;

void
load_world()
{
  read_height_map();
  if (!initialize_ground_items())
    fatal_error("NÃO FOI POSSÍVEL INICIALIZAR GROUND ITEMS");
  read_teleport();
  read_guild_zone();
}

bool
update_world(int index, short *position_x, short *position_y, int flag)
{
  bool empty_grid = false;
  switch(flag) {
  case WORLD_MOB:
    if (index >= MAX_SPAWN_LIST || index < 0) return false;

    empty_grid = get_empty_mob_grid(index, position_x, position_y);

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

void
read_height_map()
{
  FILE *height_map_fd = NULL;
  
  height_map_fd = fopen("HeightMap.dat", "rb");
	if (height_map_fd == NULL)
		fatal_error("Could not read HeightMap.dat");

	fread(height_grid, 4096, 4096, height_map_fd);
	fclose(height_map_fd);
}

bool
initialize_ground_items()
{
//  int y;
	max_item_init = 0;
	for (size_t i = 0; i < MAX_INIT_ITEM_LIST; i++) {
		if (init_list[i].item_index == -1)
			continue;

		size_t j = 1;
		for (; j < MAX_INIT_ITEM_LIST; j++) {
      if (ground_items[j].item_index== 0)
			  break;
    }

		if (j == (MAX_INIT_ITEM_LIST - 1))
			return false;

		memcpy(&ground_items[j], &init_list[i], sizeof(struct ground_item_st));
		ground_items[j].item_index = j;
		struct ground_item_st *init_item = &ground_items[j];
		short position_x = init_item->position.X, position_y = init_item->position.Y;

		if (!update_world(j, &position_x, &position_y, WORLD_ITEM)) {
			memset(&ground_items[j], 0, sizeof(struct init_item_st));
			continue;
		}

		if (j > max_item_init)
			max_item_init = j;

		init_item->position.X = position_x;
		init_item->position.Y = position_y;
	}

	return true;  
}

void
read_teleport()
{
  FILE *teleport_fd = NULL;
  
  teleport_fd = fopen("teleport.txt", "rt");
	if (teleport_fd == NULL)
    fatal_error("Could not read teleport.txt");

	int ret;
	struct teleport_st teleport;

	memset(teleport_list, 0, sizeof(struct teleport_st));
	char tmp[255];
	while (fgets(tmp, sizeof(tmp), teleport_fd) && teleport_count < MAX_TELEPORT) {
		if (*tmp == '\n' || *tmp == '#')
			continue;

		ret = sscanf(tmp, "%04hu, %04hu, %04hu, %04hu, %u", &teleport.from.X,
			&teleport.from.Y, &teleport.to.X, &teleport.to.Y, &teleport.price);

		teleport.func = 0;

		if (ret == 5) {
			memcpy(&teleport_list[teleport_count], &teleport, sizeof(struct teleport_st));
			teleport_count++;
		}
	}

	fclose(teleport_fd);
}

void
read_guild_zone()
{
  FILE *guild_zone_fd = NULL;

  guild_zone_fd = fopen("guild_zone.txt", "rt");

  if (guild_zone_fd == NULL)
    fatal_error("Could not load guild_zone.txt");

  int zone_index;
	struct guildzone_st zone;
	char tmp[1024];

	while (fgets(tmp, sizeof(tmp), guild_zone_fd)) {
		if (tmp[0] == '\n' || tmp[0] == '#')
			continue;

		zone_index = -1;
		memset(&zone, 0, sizeof(zone));

		sscanf(tmp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			&zone_index, &zone.area_guild_x,
			&zone.area_guild_y, &zone.city_x, &zone.city_y, &zone.city_min_x,
			&zone.city_min_y, &zone.city_max_x, &zone.city_max_y, &zone.area_guild_min_x,
			&zone.area_guild_min_y, &zone.area_guild_max_x, &zone.area_guild_max_y, &zone.war_min_x,
			&zone.war_min_y, &zone.war_max_x, &zone.war_max_y, &zone.guilda_war_x,
			&zone.guilda_war_y, &zone.guildb_war_x, &zone.guildb_war_y);

		if (zone_index >= 0 && zone_index < MAX_GUILD_ZONE)
			memcpy(&guild_zone[zone_index], &zone, sizeof(struct guildzone_st));
	}

  fclose(guild_zone_fd);
}

bool
remove_object(int mob_index, struct position_st position, int flag)
{
	if (position.X < 0 || position.X >= MAX_GRIDX || position.Y < 0 || position.Y >= MAX_GRIDY)
		return false;

	if (height_grid[position.Y][position.X] == 127)
		return false;

	if (flag == WORLD_MOB) {
		if (mob_index >= MAX_SPAWN_LIST || mob_index < 0)
			return false;

		mob_grid[position.Y][position.X] = 0;

		return true;
	} else if (flag == WORLD_ITEM) {
		if (mob_index >= MAX_INIT_ITEM_LIST || mob_index < 0)
			return false;

		item_grid[position.Y][position.X] = 0;
		return true;
	} else
		return false;

	return false;
}

