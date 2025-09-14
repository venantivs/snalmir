/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/game_items.c
 * Descrição: Arquivo onde são implementadas as funções que carregam os items do jogo.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "game_items.h"

struct item_list_st g_item_list[MAX_ITEM_LIST];
struct ground_item_st g_init_list[MAX_INIT_ITEM_LIST];
int init_index, max_item_load, max_init_item;
char effect_name_list[MAX_EFFECT_INDEX][MAX_EFFECT_NAME];

void
load_item_effect()
{
  FILE *item_effect_fd = fopen("./bin/ItemEffect.h", "rt");

  if (item_effect_fd == NULL)
    fatal_error("Could not load ItemEffect.h");

  memset(effect_name_list, 0, MAX_EFFECT_INDEX * MAX_EFFECT_NAME);

  int fields_read, index;

  char tmp[1024] = { 0 };
  char value[MAX_EFFECT_NAME] = { 0 };

  while (fgets(tmp, sizeof(tmp), item_effect_fd)) {
    if (strncmp(tmp, "#define", 7) == 0) {
      index = -1;
      memset(value, 0, sizeof(value));

      fields_read = sscanf(tmp, "#define %15s %d", value, &index);

      if (fields_read != 2) continue;

      if (index >= 0 && index < MAX_EFFECT_INDEX)
        strncpy(effect_name_list[index], value, MAX_EFFECT_NAME);
    }
  }
  fclose(item_effect_fd);
}

void
load_init_items()
{
  FILE *init_item_fd = fopen("./bin/InitItem.csv", "rt");

  if (init_item_fd == NULL)
    fatal_error("Could not load InitItem.csv");

  memset(g_init_list, -1, sizeof(struct ground_item_st) * MAX_INIT_ITEM_LIST);
  init_index = 0;

  struct item_st item = { 0 };
  char tmp[1024] = { 0 };

  while(fgets(tmp, sizeof(tmp), init_item_fd)) {
    if (tmp[0] == '\n' || tmp[0] == '#') continue;

    // TODO: REVER SE TEM NECESSIDADE DISSO
    char *p = tmp;
    while (p[0] != '\0') {
      if (p[0] != ',')
        p[0] = ' ';
      p++;
    }

    memset(&item, 0, sizeof(struct item_st));
    memset(&g_init_list[init_index], 0, sizeof(struct ground_item_st));

    int read_fields = sscanf(tmp, "%hd %hd %hd %hd", &item.item_id, &g_init_list[init_index].position.X, &g_init_list[init_index].position.Y, &g_init_list[init_index].rotation);
    if (read_fields < 4) continue;

    g_init_list[init_index].item_index = init_index;
    g_init_list[init_index].drop_time = get_clock();
    g_init_list[init_index].owner_id = 0x7530; // ??????
    g_init_list[init_index].status = 3; // ????

    memcpy(&g_init_list[init_index].item_data, &item, sizeof(struct item_st));
    init_index++;

    if (init_index > max_init_item) max_init_item = init_index;
  }
  fclose(init_item_fd);
}

short
get_effect_index(const char *effect_name)
{
	for (size_t i = 0; i < MAX_EFFECT_INDEX; i++) {
    if (strncmp(effect_name_list[i], effect_name, MAX_EFFECT_NAME) == 0)
		  return i;
  }

	return atoi(effect_name);
}

short
get_effect_value(short item_id, char effect)
{
	if (item_id >= 0 && item_id <= MAX_ITEM_LIST) {
		for (size_t i = 0; i < MAX_EFFECT; i++) {
      if (g_item_list[item_id].effect[i].index == effect)
			  return g_item_list[item_id].effect[i].value;
    }
	}

	return 0;
}

void
load_item_list()
{
  FILE *item_list_fd = fopen("./bin/ItemList.csv", "rt");

  if (item_list_fd == NULL)
    fatal_error("Could not load ItemList.csv");

  memset(g_item_list, 0, sizeof(struct item_list_st) * MAX_INIT_ITEM_LIST);

  int item_id, fields_read;
  struct item_list_st item = { 0 };
  char tmp[1024] = { 0 };

  while(fgets(tmp, sizeof(tmp), item_list_fd)) {
    char mesh_buffer[MAX_MESH_BUFFER];
    char score_buffer[MAX_SCORE_BUFFER];
    char effect_buffer[MAX_EFFECT][MAX_EFFECT_NAME];

    if(tmp[0] == '\n' || tmp[0] == '#') continue;

    memset(mesh_buffer, 0, sizeof(mesh_buffer));    
    memset(score_buffer, 0, sizeof(score_buffer));
    memset(effect_buffer, 0, sizeof(effect_buffer));
    memset(&item, 0, sizeof(struct item_list_st));

    char *p = tmp;
    while (p[0] != '\0'){
      if (p[0] == ',')
        p[0] = ' ';
      p++;
    }

    fields_read = sscanf(tmp, "%d %63s %15s %31s %hd %d %hd %hd %hd %15s %hd %15s %hd %15s %hd %15s %hd %15s %hd %15s %hd %15s %hd %15s %hd %15s %hd %15s %hd %15s %hd %15s %hd",
			&item_id, item.name, mesh_buffer, score_buffer, &item.unique, &item.price, &item.pos, &item.extreme, &item.grade,
			effect_buffer[0], &item.effect[0].value, effect_buffer[1], &item.effect[1].value, effect_buffer[2], &item.effect[2].value,
			effect_buffer[3], &item.effect[3].value, effect_buffer[4], &item.effect[4].value, effect_buffer[5], &item.effect[5].value,
			effect_buffer[6], &item.effect[6].value, effect_buffer[7], &item.effect[7].value, effect_buffer[8], &item.effect[8].value,
			effect_buffer[9], &item.effect[9].value, effect_buffer[10], &item.effect[10].value, effect_buffer[11], &item.effect[11].value);

    if (fields_read < 9 || item_id <= 0 || item_id >= MAX_ITEM_LIST) continue;

    item.name[strlen(item.name) - 1] = '\0';
    sscanf(mesh_buffer, "%hd.%hd", &item.mesh_1, &item.texture);
    sscanf(score_buffer, "%hd.%hd.%hd.%hd.%hd", &item.level, &item.strength, &item.intelligence, &item.dexterity, &item.constitution);

    for(size_t i = 0; i < MAX_EFFECT; i++)
      item.effect[i].index = get_effect_index(effect_buffer[i]);

    memcpy(&g_item_list[item_id], &item, sizeof(struct item_list_st));
    if (item_id > max_item_load) max_item_load = item_id;
  }
  fclose(item_list_fd);
}

void
load_game_items()
{
  load_item_effect();
  load_item_list();
  load_init_items();
  // TODO: LOAD CUBO
}

bool
is_arch_item(int item_index)
{
	// SET'S
	if (item_index >= 1221 && item_index <= 1224)
    return true;
	else if (item_index >= 1356 && item_index <= 1359)
    return true;
	else if (item_index >= 1506 && item_index <= 1509)
    return true;
	else if (item_index >= 1656 && item_index <= 1659)
    return true;
	// ARMAS ARCH
	else if (item_index == 811 || item_index == 826 || item_index == 841)
    return true;
	else if (item_index == 856 || item_index == 871 || item_index == 886)
    return true;
	else if (item_index == 871 || item_index == 886 || item_index == 903 || item_index == 904)
    return true;
	else if (item_index == 912 || item_index == 937 || item_index == 2379 || item_index == 2380)
    return true;
	// ARMAS ARCH ANCT
	else if ((item_index >= 2491 && item_index <= 2494) || (item_index >= 2551 && item_index <= 2554))
    return true;
	else if ((item_index >= 2611 && item_index <= 2614) || (item_index >= 2671 && item_index <= 2674))
    return true;
	else if ((item_index >= 2791 && item_index <= 2794) || (item_index >= 2859 && item_index <= 2866))
    return true;
	else if ((item_index >= 2895 && item_index <= 2898) || (item_index >= 2935 && item_index <= 2938))
    return true;

	return false;
}

bool
is_hardcore_item(int item_index)
{ 
	// SETS
	if (item_index >= 1901 && item_index <= 1910){
		return true;
  } else if (item_index == 1230 || item_index == 1231 || item_index == 1232 || item_index == 1667) { // ARMAS ARCH
		return true;
	} else if (item_index == 1233 || item_index == 1365 || item_index == 1366 || item_index == 1668) {
		return true;
	} else if (item_index == 1367 || item_index == 1368 || item_index == 1515 || item_index == 1516) {
		return true;
	} else if (item_index == 1517 || item_index == 1518 || item_index == 1665 || item_index == 1666) {
		return true;
	}

	return false;
}
