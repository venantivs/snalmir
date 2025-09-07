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
#include "../network/socket-utils.h"
#include "npc.h"
#include "mob.h"
#include "world.h"
#include "base_functions.h"
#include "utils.h"

struct npcgener_st g_gener_list[MAX_NPCGENERATOR];
struct mob_st baby_list[MAX_MOB_BABY];
unsigned load_npc_count = 0;
unsigned spawn_count = 0;

void
load_npcs()
{
  memset(g_gener_list, 0, sizeof(struct npcgener_st) * MAX_NPCGENERATOR);
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
      if (g_gener_list[index].mode == NPCG_EMPTY) {
        g_gener_list[index].mode = NPCG_CHARGED;
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
      struct npcgener_st *npc_mob = &g_gener_list[index];

      if (strcmp(cmd, "Leader") == 0) {
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
    struct npcgener_st *npcgener = &g_gener_list[i];
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

  struct npcgener_st *npc_mob = &g_gener_list[index];
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
    struct npcgener_st *npc_mob = &g_gener_list[i];
    struct mob_st *mob = &npc_mob->mob;

    bool is_pesa_mob = false;
    // TODO: IMPLEMENTAR ISSO AQUI
    // struct pesadelo_st *pesadelo = &GLOBALpesadelo;
    // if ((i >= pesadelo->g_mobs[PesaN][0] && i <= pesadelo->g_mobs[PesaN][8]) ||
    //     (i >= pesadelo->g_mobs[PesaM][0] && i <= pesadelo->g_mobs[PesaM][8]) ||
    //     (i >= pesadelo->g_mobs[PesaA][0] && i <= pesadelo->g_mobs[PesaA][8])) {
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
          if (current_time < (npc_mob->death_time[death_id] + 5))
            continue;
        } else {
          if (current_time < (npc_mob->death_time[death_id] + MOB_RESPAWN_DELAY))
            continue;
        }

        int index = get_spawn_empty_index();
        if (!index)
          break;

        if (!update_world(index, &npc_mob->start.position, WORLD_MOB)) {
          printf("%d\n", index);
          // fatal_error("Erro ao executar #update_world.\n");
          continue;
        }

        struct mob_server_st *current_mob = &g_mobs[index];
        memset(&current_mob->mob, 0, sizeof(struct mob_st));
        memcpy(&current_mob->mob, &g_gener_list[i].mob, sizeof(struct mob_st));

        current_mob->mode = MOB_IDLE;
        current_mob->mob.client_index = index;
        current_mob->next_action = (get_clock() + 800);
        current_mob->spawn_type = SPAWN_TELEPORT;
        current_mob->generate_index = i;
        current_mob->mob.last_position = npc_mob->start.position;
        current_mob->mob.current = npc_mob->start.position;
        current_mob->mob.dest = npc_mob->start.position;
        current_mob->death_id = death_id;

        if (is_pesa_mob) {
          current_mob->mob_type = MOB_TYPE_PESA_MOB;
        } else {
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
	for (size_t i = (BASE_MOB + 1); i <= spawn_count; i++) {
    struct mob_server_st *mob = &g_mobs[i];

    clock_t opa = get_clock();
		if (is_dead(*mob)) {
      continue;
    } else if (opa > mob->next_action) {
			int action = standby_processor(mob);
      struct position_st new_position = { 0 };

			switch (action) {
			case ACTION_MOVE:
				new_position.X = (g_gener_list[mob->generate_index].dest.position.X + (rand() % 3));
				new_position.Y = (g_gener_list[mob->generate_index].dest.position.Y + (rand() % 3));
				movement(mob, new_position, MOVE_NORMAL);
				break;
			
			case ACTION_MOVE_RAND:
				new_position.X = (g_gener_list[mob->generate_index].start.position.X + (rand() % 3));
				new_position.Y = (g_gener_list[mob->generate_index].start.position.Y + (rand() % 3));
				movement(mob, new_position, MOVE_NORMAL);
				break;

			case ACTION_MOVE_TO_SUMMONER:
				new_position.X = (g_mobs[mob->summoner].mob.current.X);
				new_position.Y = (g_mobs[mob->summoner].mob.current.Y);
				movement(mob, new_position, MOVE_TELEPORT);
				break;

			case ACTION_BATTLE:
        battle_processor(mob);
				break;
			}
		}

		if (mob->mob.status.merchant == 0)
			processor_sec_timer_mob(mob, sec_counter);
	}
}

void
first_trainer(int user_index, int npc_index)
{
  struct mob_server_st *mob = &g_mobs[user_index];
  if (mob->mob.b_status.level > 34) {
    send_chat(npc_index, "Seu nivel e inadequado para esta area.");
    return;
  }

  if ((mob->mob.quest_info & (1 << (QUEST_C_ARMIA_1))) != 0) {
    send_chat(npc_index, "Voce ja completou essa quest!");
    return;
  }

  if (mob->mob.class_master != CLASS_MORTAL) {
    send_chat(npc_index, "Somente personagens mortais podem participar desta quest.");
    return;
  }

  int slot = get_item_slot(user_index, 451, INV_TYPE);
  if (slot < 0 || slot > 63) {
    send_chat(npc_index, "Traga-me a chave da primeira porta.");
    return;
  }

  struct item_st prize = { 0 };
  prize.item_id = 428;
  prize.effect[0].index = 61;
  prize.effect[0].value = 120;
  memcpy(&mob->mob.inventory[slot], &prize, sizeof(struct item_st));

  mob->mob.quest_info |= 1 << QUEST_C_ARMIA_1;

  send_refresh_inventory(user_index);
  send_all_packets(user_index);
}

void
second_trainer(int user_index, int npc_index)
{
  struct mob_server_st *user = &g_mobs[user_index];
  if (user->mob.b_status.level > 34) {
    send_chat(npc_index, "Seu nivel e inadequado para esta area.");
    return;
  }

  if (user->mob.class_master != CLASS_MORTAL) {
    send_chat(npc_index, "Somente para mortais.");
    return;
  }

  short slot = get_item_slot(user_index, 452, INV_TYPE);
  if (slot < 0 || slot > 62) {
    send_chat(npc_index, "Traga-me a chave da segunda porta.");
    return;
  }

  if ((user->mob.quest_info & (1 << QUEST_C_ARMIA_2)) != 0) {
    send_chat(npc_index, "Voce ja completou esta quest!");
    return;
  }

  short _num = abs(rand()) % 3;
  short _ref = abs(rand()) % 6;

  int add[] = { 26, 54, 74 };
  int val[] = { 36, 8, 32 };

  if (user->mob.equip[6].item_id > 0) {
    user->mob.equip[6].effect[0].index = 43;
    user->mob.equip[6].effect[0].value = _ref;
    user->mob.equip[6].effect[1].index = add[_num];
    user->mob.equip[6].effect[1].value = val[_num];

    send_create_item(user_index, EQUIP_TYPE, 6, &user->mob.equip[6]);
    memset(&user->mob.inventory[slot], 0, sizeof(struct item_st));

    user->mob.quest_info |= 1 << QUEST_C_ARMIA_2;

    send_refresh_inventory(user_index);
    send_chat(npc_index, "Sua arma foi aprimorada, parabens bravo guerreiro.");
    send_all_packets(user_index);
    return;
  }

  send_chat(npc_index, "Sua arma nao esta equipada!");
}

void
third_trainer(int user_index, int npc_index)
{
  struct mob_server_st *user = &g_mobs[user_index];

  if (user->mob.b_status.level > 34) {
    send_chat(npc_index, "Seu nivel e inadequado para esta area.");
    return;
  }

  if (user->mob.class_master != CLASS_MORTAL) {
    send_chat(npc_index, "Somente para mortais.");
    return;
  }

  short slot = get_item_slot(user_index, 453, INV_TYPE);
  if (slot < 0 || slot > 62) {
    send_chat(npc_index, "Traga-me a chave da segunda porta.");
    return;
  }

  if ((user->mob.quest_info & (1 << QUEST_C_ARMIA_3)) != 0) {
    send_chat(npc_index, "Voce ja completou esta quest!");
    return;
  }

  memset(&user->mob.inventory[slot], 0, sizeof(struct item_st));
  for (size_t i = 0; i < 5; i++) {
    short free_slot = get_item_slot(user_index, 0, INV_TYPE);
    user->mob.inventory[free_slot].item_id = 4016;
  }

  user->mob.quest_info |= 1 << QUEST_C_ARMIA_3;
  send_refresh_inventory(user_index);
  send_chat(npc_index, "Faca bom uso destes itens.");
  send_all_packets(user_index);
}

void
mortal_quests(int user_index, int npc_index)
{
  if (user_index < 0 || user_index > MAX_USERS_PER_CHANNEL)
    return;

  if (npc_index < BASE_MOB || npc_index > MAX_SPAWN_LIST)
    return;

  struct mob_server_st *user_mob = &g_mobs[user_index];
  // struct mob_server_st *npc_mob = &g_mobs[npc_index];

  if (user_mob->mob.class_master != CLASS_MORTAL) {
    send_chat(npc_index, "Somente mortais!");
    return;
  }

  if (user_mob->mob.b_status.level > 59 && user_mob->mob.b_status.level < 115) {
    send_teleport(user_index, (struct position_st) { 2398, 2104 });
    send_client_message("Bem vindo a quest Nv.[60~115]", user_index);
    return;
  }

  if (user_mob->mob.b_status.level >= 115 && user_mob->mob.b_status.level < 190) {
    send_teleport(user_index, (struct position_st) { 2237, 1713 });
    send_client_message("Bem vindo a quest Nv.[116~190]", user_index);
    return;
  }

  if (user_mob->mob.b_status.level >= 190 && user_mob->mob.b_status.level < 265) {
    send_teleport(user_index, (struct position_st) { 467, 3901 });
    send_client_message("Bem vindo a quest Nv.[191~265]", user_index);
    return;
  }

  if (user_mob->mob.b_status.level >= 265 && user_mob->mob.b_status.level < 320) {
    send_teleport(user_index, (struct position_st) { 668, 3755 });
    send_client_message("Bem vindo a quest Nv.[266~320]", user_index);
    return;
  }

  if (user_mob->mob.b_status.level >= 320 && user_mob->mob.b_status.level < 399) {
    send_teleport(user_index, (struct position_st) { 1325, 4042 });
    send_client_message("Bem vindo a quest Nv.[321~399]", user_index);
    return;
  }

  send_client_message("Voce nao possui mais quests.", user_index);
}