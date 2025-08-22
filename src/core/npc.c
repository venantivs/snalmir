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
#include <time.h>

#include "../network/server.h"
#include "npc.h"
#include "mob.h"
#include "world.h"
#include "base_functions.h"
#include "utils.h"

struct npcgener_st gener_list[MAX_NPCGENERATOR];
struct mob_st baby_list[MAX_MOB_BABY];
unsigned load_npc_count = 0;
unsigned spawn_count = 0;

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

  npc_gener_fd = fopen("./bin/NPCGener.txt", "rt");

  if (npc_gener_fd == NULL)
    fatal_error("Could not load NPCGener.txt");

  int index = -1;
  char tmp[1024], cmd[128], val[128];

  while(fgets(tmp, sizeof(tmp), npc_gener_fd)) {
    if (tmp[0] == '#') {
      int ret = sscanf(tmp, "#\t[%04d]", &index);
      if (ret != 1 || index < 0 || index >= MAX_NPCGENER) {
        index = -1;
        continue;
      }

      for(; index < MAX_NPCGENER; index++)
      if (gener_list[index].mode == NPCG_EMPTY) {
        gener_list[index].mode = NPCG_CHARGED;
        break;
      }

      if (index == MAX_NPCGENER)
        break;

      continue;
    }

    if (tmp[0] == '#' || tmp[0] == '\n' || tmp[0] == '/')
	    continue;

    int ret = sscanf(tmp, " %127[^:]: %127[^\r\n]", cmd, val);

    if (ret != 2)
      continue;

    if (index != -1) {
      struct npcgener_st *npc_mob = &gener_list[index];

      if (strcmp(cmd, "Mob") == 0) {
        load_npc(val, index);
      } else {
        int value = atoi(val);
        if (strcmp(cmd, "RouteType") == 0)
          npc_mob->route_type = value;
        else if (strcmp(cmd, "MinuteGenerate") == 0)
          npc_mob->minute_generated = value;
        else if (strcmp(cmd, "MaxNumMob") == 0)
          npc_mob->max_num_mob = value;
        else if (strcmp(cmd, "StartX") == 0) {
          if (npc_mob->mob.name[0] != '\0')
            npc_mob->start.position.X = value;
          else
            npc_mob->start.position.X = 0;
        }
        else if (strcmp(cmd, "StartY") == 0)
          npc_mob->start.position.Y = value;
        else if (strcmp(cmd, "StartRange") == 0)
          npc_mob->start.range = value;
        else if (strcmp(cmd, "StartWait") == 0)
          npc_mob->start.wait = value;
        else if (strcmp(cmd, "DestX") == 0)
          npc_mob->dest.position.X = value;
        else if (strcmp(cmd, "DestY") == 0)
          npc_mob->dest.position.Y = value;
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

  sprintf(file_path, "./bin/npc/%s", name);

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

    snprintf(file_path, 1024, "./bin/npc_base/%s", baby_mob_name[i]);

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

void
spawn_mobs()
{
  for (size_t i = 0; i < load_npc_count; i++) {
    struct npcgener_st *npc_mob = &gener_list[i];
    struct mob_st *mob = &npc_mob->mob;

    bool is_pesa_mob = false;
    // IMPLEMENTAR ISSO AQUI
    // struct pesadelo_st *pesadelo = &GLOBALpesadelo;
    // if ((i >= pesadelo->mobs[PesaN][0] && i <= pesadelo->mobs[PesaN][8]) ||
    //     (i >= pesadelo->mobs[PesaM][0] && i <= pesadelo->mobs[PesaM][8]) ||
    //     (i >= pesadelo->mobs[PesaA][0] && i <= pesadelo->mobs[PesaA][8])) {
    //       is_pesa_mob = true;
    //     }

    if (npc_mob->max_num_mob == 0)
      continue;

    if (mob->name[0] == '\0' || npc_mob->start.position.X == 0)
      continue;

    if (npc_mob->minute_generated >= 0 && npc_mob->current_num_mob < npc_mob->max_num_mob) {
      time_t current_time = time(NULL);
      for (size_t j = npc_mob->current_num_mob; j < npc_mob->max_num_mob; j++) {
        int death_id = npc_mob->group[j] - npc_mob->index; // Se der negativo...
        if (is_pesa_mob) {
          if (current_time < (npc_mob->death_time[death_id] + 5)) continue;
        } else {
          if (current_time < (npc_mob->death_time[death_id] + MOB_RESPAWN_DELAY)) continue;
        }

        int index = get_spawn_empty_index();
        if (!index)
          break;

        short position_x = npc_mob->start.position.X;
        short position_y = npc_mob->start.position.Y;

        if (!update_world(index, &position_x, &position_y, WORLD_MOB)) continue;

        struct mob_server_st *current_mob = &mobs[index];
        memset(&current_mob->mob, 0, sizeof(struct mob_st));
        memcpy(&current_mob->mob, &gener_list[i].mob, sizeof(struct mob_st));

        current_mob->mode = MOB_IDLE;
        current_mob->mob.client_index = index;
        current_mob->next_action = (clock() + 800);
        current_mob->spawn_type = SPAWN_TELEPORT;
        current_mob->generate_index = i;
        current_mob->mob.last_position.X = position_x;
        current_mob->mob.last_position.Y = position_y;
        current_mob->mob.current.X = position_x;
        current_mob->mob.current.Y = position_y;
        current_mob->mob.dest.X = position_x;
        current_mob->mob.dest.Y = position_y;
        current_mob->death_id = death_id;

        if (is_pesa_mob)
          current_mob->mob_type = MOB_TYPE_PESA_MOB;
        else {
          if (current_mob->mob.cape == 3 && current_mob->mob.b_status.merchant == 0)
            current_mob->mob_type = MOB_TYPE_GUARD;
          else if (current_mob->mob.cape == 3 && current_mob->mob.b_status.merchant > 0)
            current_mob->mob_type = MOB_TYPE_NPC;
          else
            current_mob->mob_type = MOB_TYPE_MONSTER;
        }
      
        current_mob->mob.client_index = index;
        current_mob->mob.status = current_mob->mob.b_status;
        current_mob->mob.status.speed = 3;

        npc_mob->current_num_mob++;

        get_current_score(index);

        current_mob->mob.status.current_hp = current_mob->mob.status.max_hp;
        current_mob->mob.status.current_mp = current_mob->mob.status.max_mp;

        npc_mob->death_time[death_id] = 0;

        send_grid_mob(index);

        current_mob->spawn_type = SPAWN_NORMAL;

        if (index > spawn_count)
          spawn_count = index;
      }
    }
  }
}

void
action_mob(int sec_counter)
{
  short newPosX = 0;
	short newPosY = 0;

	for (size_t i = (BASE_MOB + 1); i <= spawn_count; i++) {
    struct mob_server_st *mob = &mobs[i];

		if (is_dead(*mob))
      continue;
		else if (clock() > mob->next_action) {
			int action = standby_processor(mob);
			switch (action) {
			case ACTION_MOVE:
				newPosX = (gener_list[mob->generate_index].dest.position.X + (rand() % 3));
				newPosY = (gener_list[mob->generate_index].dest.position.Y + (rand() % 3));
				movement(mob, newPosX, newPosY, MOVE_NORMAL);
				break;
			
			case ACTION_MOVE_RAND:
				newPosX = (gener_list[mob->generate_index].start.position.X + (rand() % 3));
				newPosY = (gener_list[mob->generate_index].start.position.Y + (rand() % 3));
				movement(mob, newPosX, newPosY, MOVE_NORMAL);
				break;

			case ACTION_MOVE_TO_SUMMONER:
				newPosX = (mobs[mob->summoner].mob.current.X);
				newPosY = (mobs[mob->summoner].mob.current.Y);
				movement(mob, newPosX, newPosY, MOVE_TELEPORT);
				break;

			case ACTION_BATTLE:
        fprintf(stderr, "ENTROU NO ACTION BATTLE PROCESSOR, NÃO DEVERIA ESTAR AQUI!!!!");
        // TODO: IMPLEMENTAR
				//c->BattleProcessor();
				break;
			}
		}

		if (mob->mob.status.merchant == 0)
			processor_sec_timer_mob(mob, sec_counter);
	}
}
