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

short g_mob_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST] = { 0 };
short g_item_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST] = { 0 };
int height_grid[MAX_INIT_ITEM_LIST][MAX_INIT_ITEM_LIST] = { 0 };
struct guildzone_st g_guild_zone[MAX_GUILD_ZONE] = { 0 };
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
update_world(int index, struct position_st *position, int flag)
{
  bool empty_grid = false;
  switch(flag) {
  case WORLD_MOB:
    if (index >= MAX_SPAWN_LIST || index < 0) return false;

    empty_grid = get_empty_mob_grid(index, position);

    if (empty_grid) {
      g_mob_grid[g_mobs[index].mob.current.Y][g_mobs[index].mob.current.X] = 0;
      g_mob_grid[position->Y][position->X] = index;
      return true;
    } else
      return false;
  case WORLD_ITEM:
    if (index >= MAX_INIT_ITEM_LIST || index < 0)
      return false;

    empty_grid = get_empty_item_grid(index, position);

    if (empty_grid)
    {
      g_item_grid[position->Y][position->X] = index;
      return true;
    } else
      return false;
  default:
    return false;
  }
}

/* REFATORAR ISSO */
bool
get_empty_mob_grid(int index, struct position_st *position)
{
  if (position->X < 0 || position->X >= MAX_GRIDX || position->Y < 0 || position->Y >= MAX_GRIDY) return false;

  if (g_mob_grid[position->Y][position->X] == index) return true;

  if (g_mob_grid[position->Y][position->X] == 0)
    if (height_grid[position->Y][position->X] != 127)
      return true;

  for (int y = (position->Y - 1); y <= (position->Y + 1); y++)
  {
    for (int x = (position->X - 1); x <= (position->X + 1); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (g_mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          position->X = x;
          position->Y = y;
          return true;
        }
      }
    }
  }

  for (int y = (position->Y - 2); y <= (position->Y + 2); y++)
  {
    for (int x = (position->X - 2); x <= (position->X + 2); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (g_mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          position->X = x;
          position->Y = y;
          return true;
        }
      }
    }
  }

  for (int y = (position->Y - 3); y <= (position->Y + 3); y++)
  {
    for (int x = (position->X - 3); x <= (position->X + 3); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (g_mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          position->X = x;
          position->Y = y;
          return true;
        }
      }
    }
  }

  for (int y = (position->Y - 4); y <= (position->Y + 4); y++)
  {
    for (int x = (position->X - 4); x <= (position->X + 4); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (g_mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          position->X = x;
          position->Y = y;
          return true;
        }
      }
    }
  }

  for (int y = (position->Y - 5); y <= (position->Y + 5); y++)
  {
    for (int x = (position->X - 5); x <= (position->X + 5); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (g_mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          position->X = x;
          position->Y = y;
          return true;
        }
      }
    }
  }

  for (int y = (position->Y - 6); y <= (position->Y + 6); y++)
  {
    for (int x = (position->X - 6); x <= (position->X + 6); x++)
    {
      if (x < 0 || y < 0 || x >= MAX_GRIDX || y >= MAX_GRIDY)
        continue;

      if (g_mob_grid[y][x] == 0)
      {
        if (height_grid[y][x] != 127)
        {
          position->X = x;
          position->Y = y;
          return true;
        }
      }
    }
  }

  return false;
}

bool
get_empty_item_grid(int index, struct position_st *position)
{
  if (position->X < 0 || position->X >= 4096 || position->Y < 0 || position->Y >= 4096)
    return false;

  if (g_item_grid[position->Y][position->X] == index)
    return true;

  if (g_item_grid[position->Y][position->X] == 0) // ??????
    if (height_grid[position->Y][position->X] != 127)
      return true;

  for (int y = (position->Y - 1); y <= (position->Y + 1); y++) {
    for (int x = (position->X - 1); x <= (position->X + 1); x++) {
      if (x < 0 || y < 0 || x >= 4096 || y >= 4096)
        continue;

      if (g_item_grid[y][x] == 0) {
        if (height_grid[y][x] != 127) {
          position->X = x;
          position->Y = y;
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
		if (g_init_list[i].item_index == -1)
			continue;

		size_t j = 1;
		for (; j < MAX_INIT_ITEM_LIST; j++) {
      if (g_ground_items[j].item_index== 0)
			  break;
    }

		if (j == (MAX_INIT_ITEM_LIST - 1))
			return false;

		memcpy(&g_ground_items[j], &g_init_list[i], sizeof(struct ground_item_st));
		g_ground_items[j].item_index = j;

		struct ground_item_st *init_item = &g_ground_items[j];

		if (!update_world(j, &init_item->position, WORLD_ITEM)) {
			memset(&g_ground_items[j], 0, sizeof(struct init_item_st));
			continue;
		}

		if (j > max_item_init)
			max_item_init = j;

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
			memcpy(&g_guild_zone[zone_index], &zone, sizeof(struct guildzone_st));
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

		g_mob_grid[position.Y][position.X] = 0;

		return true;
	} else if (flag == WORLD_ITEM) {
		if (mob_index >= MAX_INIT_ITEM_LIST || mob_index < 0)
			return false;

		g_item_grid[position.Y][position.X] = 0;
		return true;
	} else
		return false;

	return false;
}

