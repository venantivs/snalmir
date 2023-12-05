/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/npc.c
 * Descrição: Arquivo onde são implementadas as funções que carregam os NPCs no mundo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "npc.h"
#include "utils.h"

struct npcgener_st gener_list[MAX_NPCGENERATOR];
struct mob_st baby_list[MAX_MOB_BABY];
unsigned load_npc_count = 0;

void
load_npcs()
{
  memset(gener_list, 0, sizeof(struct npcgener_st) * MAX_NPCGENERATOR);
  memset(baby_list, 0, sizeof(struct mob_st) * MAX_MOB_BABY);
  load_npc_count = 0;

  read_npc_generator();
  load_mob_baby();
}

/* TODO: ENTENDER NECESSIDADE DO NPCGener.new.txt.
 * VERIFICAR POSSIBILIDADE DE REFATORAMENTO. */
void
read_npc_generator()
{
  FILE *npc_gener_fd = NULL;

  npc_gener_fd = fopen("NPCGener.txt", "rt");

  if (npc_gener_fd == NULL)
    fatal_error("Could not load NPCGener.txt");

  int index = -1;
  char tmp[1024], cmd[128], val[128];

  while(fgets(tmp, sizeof(tmp), npc_gener_fd)) {
    if (tmp[0] == '#') {
      int ret = sscanf(tmp, "#\t[%04d]", &index);
      if (ret != 1 || index < 0 || index >= MAX_NPCGENER)
      {
        index = -1;
        continue;
      }

      for(; index < MAX_NPCGENER; index++)
      if (gener_list[index].mode == NPCG_EMPTY)
      {
        gener_list[index].mode = NPCG_CHARGED;
        break;
      }

      if (index == MAX_NPCGENER) break;

      continue;
    }

    if (tmp[0] == '#' || tmp[0] == '\n' || tmp[0] == '/')
	    continue;

    int ret = sscanf(tmp, " %127[^:]: %127[^\r\n]", cmd, val);

    if (ret != 2)
      continue;

    if (index != -1)
    {
      struct npcgener_st *npc_mob = &gener_list[index];
      if (strcmp(cmd, "Mob") == 0)
        load_npc(val, index);
      else
      {
        int value = atoi(val);
        if (strcmp(cmd, "RouteType") == 0)
          npc_mob->route_type = value;
        else if (strcmp(cmd, "MinuteGenerate") == 0)
          npc_mob->minute_generated = value;
        else if (strcmp(cmd, "MaxNumMob") == 0)
          npc_mob->max_num_mob = value;
        else if (strcmp(cmd, "StartX") == 0)
        {
          if (npc_mob->mob.name != NULL)
            npc_mob->start.position_x = value;
          else
            npc_mob->start.position_x = 0;
        }
        else if (strcmp(cmd, "StartY") == 0)
          npc_mob->start.position_y = value;
        else if (strcmp(cmd, "StartRange") == 0)
          npc_mob->start.range = value;
        else if (strcmp(cmd, "StartWait") == 0)
          npc_mob->start.wait = value;
        else if (strcmp(cmd, "DestX") == 0)
          npc_mob->dest.position_x = value;
        else if (strcmp(cmd, "DestY") == 0)
          npc_mob->dest.position_y = value;
        else if (strcmp(cmd, "DestRange") == 0)
          npc_mob->dest.range = value;
        else if (strcmp(cmd, "DestWait") == 0)
          npc_mob->dest.wait = value;
      }
    }
  }

  for (unsigned i = 0; i < load_npc_count; i++) {
    struct npcgener_st *npcgener = &gener_list[i];
    for (unsigned j = 0; j < npcgener->max_num_mob; j++) {
      npcgener->group[j] = npcgener->index + j;
    } 
  }

  fclose(npc_gener_fd);
}

void
load_npc(const char *name, int index)
{
  FILE *npc_fd = NULL;
  char file_path[1024] = { 0 };

  sprintf(file_path, "./npc/%s", name);

  npc_fd = fopen(file_path, "rb");

  if (npc_fd == NULL) {
    char error[1039] = { 0 };

    sprintf(error, "Could not load %s", file_path);
    fatal_error(error);
  }

  struct npcgener_st *npc_mob = &gener_list[index];
  struct mob_st new_mob = { 0 };

  fread(&new_mob, sizeof(struct mob_st), 1, npc_fd);

  if (new_mob.name[0] == '\0') return;

  memcpy(&npc_mob->mob, &new_mob, sizeof(struct mob_st));
  npc_mob->index = index;
  load_npc_count++;

  fclose(npc_fd);
}

void
load_mob_baby()
{
  static const char* baby_mob_name[MAX_MOB_BABY] = {
    "Condor", "Javali", "Lobo", "Urso", "Tigre",
    "Gorila", "Dragao_Negro", "Succubus", "Porco",
    "Javali_", "Lobo_", "Dragao_Menor", "Urso_",
    "Dente_de_Sabre", "Sem_Sela", "Fantasma", "Leve",
    "Equipado", "Andaluz", "Sem_Sela_", "Fantasma_",
    "Leve_", "Equipado_", "Andaluz_", "Fenrir", "Dragao",
    "Grande_Fenrir", "Tigre_de_Fogo", "Dragao_Vermelho",
    "Unicornio", "Pegasus", "Unisus", "Grifo", "Hippo_Grifo",
    "Grifo_Sangrento", "Svadilfari", "Sleipnir", ""
  };

  for (int i = 0; i < MAX_MOB_BABY; i++) {
    if (baby_mob_name[i][0] == '\0') continue;

    FILE *mob_baby_fd = NULL;
    char file_path[1024] = { 0 };

    snprintf(file_path, 1024, "./npc_base/%s", baby_mob_name[i]);

    struct mob_st *npc_mob = &baby_list[i];
    struct mob_st new_mob = { 0 };

    mob_baby_fd = fopen(file_path, "rb");

    if (mob_baby_fd == NULL) {
      char error[1039] = { 0 };

      sprintf(error, "Could not load %s", file_path);
      fatal_error(error);
    }

    fread(&new_mob, sizeof(struct mob_st), 1, mob_baby_fd);
    
    if (new_mob.name[0] == '\0') continue;

    memcpy(npc_mob, &new_mob, sizeof(struct mob_st));
  }
}
