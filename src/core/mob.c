/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/mob.c
 * Descrição: Arquivo onde são implementadas as funções que correspondem ao mobs (chars, npcs, monstros, etc) em específico.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../general-config.h"
#include "../network/server.h"
#include "../network/packet-def.h"
#include "../network/socket-utils.h"
#include "utils.h"
#include "user.h"
#include "world.h"
#include "base_functions.h"
#include "experience_table.h"

void
load_base_char_mobs()
{
  static const char *class_mobs_paths[] = {
    "./bin/char_base/TK",
    "./bin/char_base/FM",
    "./bin/char_base/BM",
    "./bin/char_base/HT"
  };

  for (size_t i = 0; i < MOB_PER_ACCOUNT; i++) {
    FILE *user_mob = NULL;

    user_mob = fopen(class_mobs_paths[i], "rb");

    if (user_mob == NULL) {
      char error[1024] = { 0 };

      sprintf(error, "Could not load %s", class_mobs_paths[i]);
      fatal_error(error);
    }

    fread(&g_base_char_mobs[i], sizeof(struct mob_st), 1, user_mob);
    fclose(user_mob);
  }
}

void
create_mob(const char* name, int user_index)
{
  FILE *user_mob = NULL;
  char file_path[1024] = { 0 };

  sprintf(file_path, "./bin/char/%s", name);

  user_mob = fopen(file_path, "wb");

  if (user_mob == NULL)
    return;

  struct account_file_st *account_file = &g_users_db[user_index];
  unsigned char_index = account_file->profile.sel_char;

  fwrite(&account_file->mob_account[char_index], sizeof(struct mob_st), 1, user_mob);
  fwrite(&account_file->subcelestials[char_index], sizeof(struct subcelestial_st), 1, user_mob);
  fclose(user_mob);
}

void
load_mob(int char_index, int user_index)
{
  FILE *user_mob = NULL;
  char file_path[1024] = { 0 };

  struct account_file_st *account_file = &g_users_db[user_index];
  const char *nickname = account_file->profile.mob_name[char_index];

  // CHECAR O QUE EXATAMENTE FAZ
  if (!(account_file->profile.char_info & (1 << char_index)))
	  return;

  memset(&account_file->mob_account[char_index], 0, sizeof(struct mob_st));
  memset(&account_file->subcelestials[char_index], 0, sizeof(struct subcelestial_st));

  sprintf(file_path, "./bin/char/%s", nickname); // Passar para snprintf

  user_mob = fopen(file_path, "rb");
  if (user_mob == NULL) // Char não existe
    return;

  fread(&account_file->mob_account[char_index],sizeof(struct mob_st), 1, user_mob);
  fread(&account_file->subcelestials[char_index], sizeof(struct subcelestial_st), 1, user_mob);
  fclose(user_mob);

  return;
}

/* Muita coisa esquisita rolando aqui, rever pra ver se não tem como melhorar. */
void
save_mob(int char_index, bool delete_mob, int user_index)
{
  FILE *user_mob = NULL;
  char file_path[1024] = { 0 };

  struct account_file_st *account_file = &g_users_db[user_index];
  struct mob_st *mob = &account_file->mob_account[char_index];
  const char *name = account_file->profile.mob_name[char_index];

  sprintf(file_path, "./bin/char/%s", name);

  if (mob->name[0] == '\0')
    return;

  user_mob = fopen(file_path, "wb");

  if (!delete_mob) {
    if (user_mob == NULL) {
      if (name[0] != '\0')
        return create_mob(name, user_index);
      else
        return;
    }

    fwrite(&account_file->mob_account[char_index], sizeof(struct mob_st), 1, user_mob);
    fwrite(&account_file->subcelestials[char_index], sizeof(struct subcelestial_st), 1, user_mob);
    fclose(user_mob);
  } else {
    if (user_mob == NULL)
      return;

    fclose(user_mob);
    if (remove(file_path) == -1) {
      char error[1041] = { 0 }; // Caracteres adicionados para mitigar chance de overflow.

      sprintf(error, "Could not delete %s", file_path);
      fatal_error(error);
    }
    account_file->profile.mob_name[char_index][0] = '\0';
    save_account(user_index);
  }
}

void
load_selchar(struct mob_st _char[4], struct char_list_st *char_list)
{
  for (size_t i = 0; i < 4; i++)
  {
    struct mob_st *mob = &_char[i];
    
    memcpy(char_list->equip[i], mob->equip, sizeof(mob->equip));
    for (int x = 0; x < MAX_AFFECT; x++)
    {
      if (mob->affect[x].index == BM_MUTACAO)
      {
        char_list->equip[i][0].item_id = mob->equip[0].EFV2;
        char_list->equip[i][0].EFV1 = 0;
      }
    }

    char_list->guild_id[i] = mob->guild_id;
    char_list->gold[i] = mob->gold;
    char_list->experience[i] = mob->experience;
    char_list->position_x[i] = mob->last_position.X;
    char_list->position_y[i] = mob->last_position.Y;

    memcpy(char_list->name[i], mob->name, sizeof(mob->name));
    memcpy(&char_list->status[i], &mob->status, sizeof(mob->status));
  }
}

void
clear_property(struct mob_server_st *mob)
{
  memset(&mob->mob, 0, sizeof(struct mob_st));
  memset(&mob->enemy_list, 0, sizeof(int) * MAX_ENEMY);
  memset(&mob->baby_mob, 0, sizeof(int) * MAX_PARTY);
  memset(&mob->buffer_time, 0, sizeof(int) * MAX_AFFECT);
  memset(&mob->trade_info, 0, sizeof(struct trade_st));

  mob->mode = MOB_EMPTY;
  mob->evoc_time = 0;
  mob->mob_type = 0;
  mob->summoner = 0;
  mob->guild_disable = 0;
  mob->generate_index = 0;
  mob->enemy_count = 0;
  mob->party_index = -1;
  mob->next = 0;
  mob->in_party = false;
  mob->in_arena = false;
  mob->in_pvp = false;
  mob->is_trading = false;
}

bool
is_dead(struct mob_server_st mob)
{
  return mob.mode == MOB_EMPTY || mob.mob.status.current_hp == 0;
}

bool
is_summon(struct mob_server_st mob)
{
  return mob.mob_type == MOB_TYPE_SUMMON;
}

void
add_enemy_list(struct mob_server_st *mob, short target_id)
{
	if (target_id <= 0)
    return;

	if (is_summon(*mob) && target_id == mob->summoner)
    return;

	for (size_t i = 0; i < MAX_ENEMY; i++) {
		if (mob->enemy_list[i] == target_id)
      return;
	}

	size_t i;
	for (i = 0; i < MAX_ENEMY; i++) {
		if (mob->enemy_list[i] == 0)
      break;
	}

	if (i == MAX_ENEMY)
    return;

	mob->enemy_list[i] = target_id;
}

void
remove_enemy_list(struct mob_server_st *mob, short target_id)
{
	if (target_id <= 0)
    return;

	for (size_t i = 0; i < MAX_ENEMY; i++) {
		if (mob->enemy_list[i] == target_id) { 
      mob->enemy_list[i] = 0;
      break;
    }
	}
}

void
refresh_enemy(struct mob_server_st *mob)
{
	mob->enemy_count = 0;

	for (size_t i = 0; i < MAX_ENEMY; ++i) {
		if (mob->enemy_list[i] > 0) {
			if ((is_summon(*mob) && (mob->enemy_list[i] < MAX_USERS_PER_CHANNEL)) || (is_summon(*mob) && is_summon(g_mobs[mob->enemy_list[i]]))) {
				if (check_pvp_area(mob->enemy_list[i]) == 0) remove_enemy_list(mob, mob->enemy_list[i]);
				else if (check_pvp_area(mob->mob.client_index) == 0) remove_enemy_list(mob, mob->enemy_list[i]);
			}
			else if (is_dead(g_mobs[mob->enemy_list[i]]))
				remove_enemy_list(mob, mob->enemy_list[i]);
			else if (is_summon(*mob) && mob->enemy_list[i] == mob->summoner)
				remove_enemy_list(mob, mob->enemy_list[i]);
			else if (get_distance(mob->mob.current, g_mobs[mob->enemy_list[i]].mob.current) > 7)
				remove_enemy_list(mob, mob->enemy_list[i]);
			else
				mob->enemy_count++;
		}
	}

	if (mob->enemy_count == 0) {
		mob->max_damage = 0;
		mob->low_hp = 0;
		mob->low_defense = 0;
		mob->low_level = 0;
		mob->max_damage_value= 0;
		mob->low_hp_value = 0;
		mob->low_defense_value = 0;
		mob->low_level_value = 0;
	}
}

bool
select_target_from_enemy_list(struct mob_server_st *mob)
{
	int es = (rand() % 6);
	if (mob->enemy_count == 0)
		return false;

	switch (es) {
	case 1: //Max Damage
	{
    if (mob->max_damage < 0)
      break;
    else {
      for (int x = 0; x < MAX_ENEMY; x++) {
        if (mob->enemy_list[x] == mob->max_damage) {
          if (is_dead(g_mobs[mob->max_damage])) {
            remove_enemy_list(mob, mob->max_damage);
            break;
          } else if (mob->max_damage > BASE_MOB && !is_summon(g_mobs[mob->max_damage]) && !is_summon(*mob)) {
            remove_enemy_list(mob, mob->max_damage);
            break;
          } else if (get_distance(mob->mob.current, g_mobs[mob->max_damage].mob.current) > 7) {
            remove_enemy_list(mob, mob->max_damage);
            break;
          } else {
            mob->current_target = mob->max_damage;
            return true;
          }
        }
      }
      break;
    }
	}
	case 2: //Low HP
	{
    if (mob->low_hp < 0)
      break;
    else {
      for (int x = 0; x < MAX_ENEMY; x++) {
        if (mob->enemy_list[x] == mob->low_hp) {
          if (is_dead(g_mobs[mob->low_hp])) {
            remove_enemy_list(mob, mob->low_hp);
            break;
          } else if (mob->low_hp > BASE_MOB && !is_summon(g_mobs[mob->low_hp]) && !is_summon(*mob)) {
            remove_enemy_list(mob, mob->low_hp);
            break;
          } else if (get_distance(mob->mob.current, g_mobs[mob->low_hp].mob.current) > 7) {
            remove_enemy_list(mob, mob->low_hp);
            break;
          } else {
            mob->current_target = mob->low_hp;
            return true;
          }
        }
      }
      break;
    }
	}
	case 3: //Low Defense
	{
    if (mob->low_defense < 0)
      break;
    else {
      for (int x = 0; x < MAX_ENEMY; x++) {
        if (mob->enemy_list[x] == mob->low_defense) {
          if (is_dead(g_mobs[mob->low_defense])) {
            remove_enemy_list(mob, mob->low_defense);
            break;
          } else if (mob->low_defense > BASE_MOB && !is_summon(g_mobs[mob->low_defense]) && !is_summon(*mob)) {
            remove_enemy_list(mob, mob->low_defense);
            break;
          } else if (get_distance(mob->mob.current, g_mobs[mob->low_defense].mob.current) > 7) {
            remove_enemy_list(mob, mob->low_defense);
            break;
          } else {
            mob->current_target = mob->low_defense;
            return true;
          }
        }
      }
      break;
    }
	}
	case 4: //Low Level
	{
    if (mob->low_level < 0)
      break;
    else {
      for (int x = 0; x < MAX_ENEMY; x++) {
        if (mob->enemy_list[x] == mob->low_level) {
          if (is_dead(g_mobs[mob->low_level])) {
            remove_enemy_list(mob, mob->low_level);
            continue;
          } else if (mob->low_level > BASE_MOB && ! is_summon(g_mobs[mob->low_level]) && !is_summon(*mob)) {
            remove_enemy_list(mob, mob->low_level);
            continue;
          } else if (get_distance(mob->mob.current, g_mobs[mob->low_level].mob.current) > 7) {
            remove_enemy_list(mob, mob->low_level);
            continue;
          } else {
            mob->current_target = mob->low_level;
            return true;
          }
        }
      }
      break;
    }
	}
	}

	for (int i = 0; i < MAX_ENEMY; i++) {
		if (mob->enemy_list[i] > 0) {
			if (is_dead(g_mobs[mob->enemy_list[i]])) {
        remove_enemy_list(mob, mob->enemy_list[i]);
        continue;
      } else if (mob->enemy_list[i] > BASE_MOB && !is_summon(g_mobs[mob->enemy_list[i]]) && !is_summon(*mob)) {
        remove_enemy_list(mob, mob->enemy_list[i]);
        continue;
      } else if (get_distance(mob->mob.current, g_mobs[mob->enemy_list[i]].mob.current) > 7) {
        remove_enemy_list(mob, mob->enemy_list[i]);
        continue;
      } else {
        mob->current_target = mob->enemy_list[i];
        return true;
      }
		}
	}

	return false;
}

bool
get_enemy_from_view(struct mob_server_st *mob)
{
	refresh_enemy(mob);

	int VisX = 6, VisY = 6,
		minPosX = (mob->mob.current.X - (VisX / 2)),
		minPosY = (mob->mob.current.Y - (VisY / 2));

	if ((minPosX + VisX) >= 4096)
		VisX = (VisX - (VisX + minPosX - 4096));

	if ((minPosY + VisY) >= 4096)
		VisY = (VisY - (VisY + minPosY - 4096));

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
			short mob_id = g_mob_grid[nY][nX];

			if (mob_id <= 0)
        continue;

			if (mob_id > BASE_MOB) {
				if (!is_summon(g_mobs[mob_id]))
					continue;
				else if(g_mobs[mob_id].mob_type != MOB_TYPE_GUARD) 
					continue;
			}

			if (is_dead(g_mobs[mob_id]))
        continue;

			if (get_distance(mob->mob.current, g_mobs[mob_id].mob.current) > 3)
        continue;

			add_enemy_list(mob, mob_id);

			return true;
		}
	}
	return false;
}

bool
get_enemy_from_view_by_mob_type(struct mob_server_st *mob, int mob_type)
{
	refresh_enemy(mob);

	int VisX = 6, VisY = 6,
		minPosX = (mob->mob.current.X - (VisX / 2)),
		minPosY = (mob->mob.current.Y - (VisY / 2));

	if ((minPosX + VisX) >= 4096)
		VisX = (VisX - (VisX + minPosX - 4096));

	if ((minPosY + VisY) >= 4096)
		VisY = (VisY - (VisY + minPosY - 4096));

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
			short mob_id = g_mob_grid[nY][nX];

      if (mob_id <= 0)
        continue;

      if (mob_id > BASE_MOB) {
				if (g_mobs[mob_id].mob_type == mob_type) {
					add_enemy_list(mob, mob_id);
					return true;
				}
			}
		}
	}

	return false;
}

void
movement(struct mob_server_st *mob, int x, int y, int type)
{
	if (x <= 0 || x >= MAX_GRIDX)
		return;

	if (y <= 0 || y >= MAX_GRIDY)
		return;

	short posX = x;
	short posY = y;

	mob->mob.dest.X = x;
	mob->mob.dest.Y = y;

	if (!update_world(mob->mob.client_index, &posX, &posY, WORLD_MOB))
    return;

	mob->mob.last_position.X = mob->mob.current.X;
	mob->mob.last_position.Y = mob->mob.current.Y;

	get_action(mob->mob.client_index, posX, posY, type, NULL);
}

void
processor_sec_timer_mob(struct mob_server_st *mob, int sec_counter)
{
	if (is_dead(*mob))
    return;

	// int	Rst4 = (sec_counter & 0x03);
	// int	Rst5 = (sec_counter & 0x04);
	// int	Rst8 = (sec_counter & 0x07);
	int Rst16 = (sec_counter & 0x0F);
	bool ret = false;

	if (Rst16 == 0) {
		int RegenHP = mob->mob.regen_hp - mob->mob.drain_hp;
		int RegenMP = mob->mob.regen_mp;
		if (RegenHP < 0) {
			if ((mob->mob.status.current_hp - mob->mob.regen_hp) <= 1)
				mob->mob.status.current_hp = 1;
			else
				mob->mob.status.current_hp -= RegenHP;
		} else {
			if ((mob->mob.status.current_hp + RegenHP) >= mob->mob.status.max_hp)
				mob->mob.status.current_hp = mob->mob.status.max_hp;
			else
				mob->mob.status.current_hp += RegenHP;
		}

		if ((mob->mob.status.current_mp + RegenMP) >= mob->mob.status.max_mp)
			mob->mob.status.current_mp = mob->mob.status.max_mp;
		else
			mob->mob.status.current_mp += RegenMP;

		ret = true;
	}

	for (size_t i = 0; i < 16; i++) {
		if (mob->mob.affect[i].index != 0) {
			if (mob->buffer_time[i] > 0) {
				mob->buffer_time[i]--;

				if (mob->buffer_time[i] <= 1) {
					mob->mob.affect[i].index = 0;
					mob->mob.affect[i].time = 0;
					mob->mob.affect[i].master = 0;
					mob->mob.affect[i].value = 0;
					mob->buffer_time[i] = 0;
					ret = true;
				} else {
          mob->mob.affect[i].time = (mob->buffer_time[i] / 8);
        }
			}
		}
	}

	if (ret) {
    get_current_score(mob->mob.client_index);
    send_score(mob->mob.client_index);
	}
}

int
standby_processor(struct mob_server_st *mob)
{
  bool searching_enemy = false;
	int ret_action = NO_MORE_ACTION;
	double distance = 0;

	if (mob->mob_type != MOB_TYPE_NPC && mob->mob_type != MOB_TYPE_SUMMON) {
		if (mob->current_target != 0) {
			if (is_dead(g_mobs[mob->current_target])) {
				refresh_enemy(mob);

				if (select_target_from_enemy_list(mob)) {
					mob->mode = MOB_COMBAT;
					mob->next_action = clock() + 1000 + abs(rand() % 1000);
					return ACTION_BATTLE;
				}
			} else {
				mob->mode = MOB_COMBAT;
				mob->next_action = clock() + 1000 + abs(rand() % 1000);
				return ACTION_BATTLE;
			}
		}

		refresh_enemy(mob);

		if (select_target_from_enemy_list(mob)) {
			mob->mode = MOB_COMBAT;
			mob->next_action = clock() + 1000 + abs(rand() % 1000);
			return ACTION_BATTLE;
		}
	}

	switch(mob->mob_type) {
	case MOB_TYPE_MONSTER:
  case MOB_TYPE_PESA_BOSS:
  case MOB_TYPE_AGUA:
		searching_enemy = get_enemy_from_view(mob);
		if (searching_enemy) {
			if (select_target_from_enemy_list(mob)) {
				ret_action = ACTION_BATTLE;
				mob->mode = MOB_COMBAT;
				mob->next_action = clock() + 1000 + abs(rand() % 1000);
				break;
			} else {
				refresh_enemy(mob);
			}	
		}

		distance = get_distance(mob->mob.current, g_gener_list[mob->generate_index].start.position);

		if (mob->next >= 13 || distance > 5) {
			ret_action = ACTION_MOVE_RAND;
			mob->next = 0;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		} else {
			ret_action = ACTION_MOVE;
			mob->next++;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		}
		break;

	case MOB_TYPE_NPC:
		distance = get_distance(mob->mob.current, g_gener_list[mob->generate_index].start.position);
		
		if (mob->next >= 13 || distance > 5) {
			ret_action = ACTION_MOVE_RAND;
			mob->next = 0;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		} else {
			ret_action = ACTION_MOVE;
			mob->next++;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		}
		break;

	case MOB_TYPE_GUARD:
		searching_enemy = get_enemy_from_view_by_mob_type(mob, MOB_TYPE_MONSTER);
		
		if (searching_enemy) {
			if (select_target_from_enemy_list(mob)) {
				ret_action = ACTION_BATTLE;
				mob->mode = MOB_COMBAT;
				mob->next_action = clock() + 1000 + abs(rand() % 1000);
				break;
			} else {
				refresh_enemy(mob);
			}
		}

		distance = get_distance(mob->mob.current, g_gener_list[mob->generate_index].start.position);

		if (mob->next >= 13 || distance > 5) {
			ret_action = ACTION_MOVE_RAND;
			mob->next = 0;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		} else {
			ret_action = ACTION_MOVE;
			mob->next++;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		}
		break;

	case MOB_TYPE_SUMMON:
		if (mob->summoner >= MAX_USERS_PER_CHANNEL || mob->summoner <= 0) {
			remove_object(mob->mob.client_index, mob->mob.current, WORLD_MOB);
			clear_property(mob);
			return NO_MORE_ACTION;
		}

    distance = get_distance(mob->mob.current, g_gener_list[mob->summoner].start.position);

		if (distance > 14) {
			ret_action = ACTION_MOVE_TO_SUMMONER;
			mob->next_action = clock() + 2000 + abs(rand() % 2000);
			mob->mode = MOB_IDLE;
			break;
		} else {
			if (mob->current_target != 0) {
				if (is_dead(g_mobs[mob->current_target])) {
					refresh_enemy(mob);
					if (select_target_from_enemy_list(mob)) {
						ret_action = ACTION_BATTLE;
						mob->mode = MOB_COMBAT;
						mob->next_action = clock() + 1000 + abs(rand() % 1000);
						break;
					}
				} else {
					ret_action = ACTION_BATTLE;
					mob->mode = MOB_COMBAT;
					mob->next_action = clock() + 1000 + abs(rand() % 1000);
					break;
				}
			}

			refresh_enemy(mob);

			if (select_target_from_enemy_list(mob)) {
				ret_action = ACTION_BATTLE;
				mob->mode = MOB_COMBAT;
				mob->next_action = clock() + 1000 + abs(rand() % 1000);
				break;
			}

			searching_enemy = get_enemy_from_view_by_mob_type(mob, MOB_TYPE_MONSTER);

			if (searching_enemy) {
				if (select_target_from_enemy_list(mob)) {
					ret_action = ACTION_BATTLE;
					mob->mode = MOB_COMBAT;
					mob->next_action = clock() + 1000 + abs(rand() % 1000);
					break;
				} else
					refresh_enemy(mob);
			} else {
				ret_action = ACTION_MOVE_RAND;
				mob->next_action = clock() + 3000 + abs(rand() % 7000);
				mob->mode = MOB_IDLE;
				break;
			}
		}
		break;

	case MOB_TYPE_PESA_MOB:
		if (abs(rand() % 2) == 2) {
			searching_enemy = get_enemy_from_view_by_mob_type(mob, MOB_TYPE_PESA_NPC);
			if (searching_enemy) {
				if (select_target_from_enemy_list(mob)) {
					ret_action = ACTION_BATTLE;
					mob->mode = MOB_COMBAT;
					mob->next_action = clock() + 1000 + abs(rand() % 1000);
					break;
				} else {
					refresh_enemy(mob);
				}
			}
		} else {
			searching_enemy = get_enemy_from_view(mob);
			if (searching_enemy) {
				if (select_target_from_enemy_list(mob)) {
					ret_action = ACTION_BATTLE;
					mob->mode = MOB_COMBAT;
					mob->next_action = clock() + 1000 + abs(rand() % 1000);
					break;
				} else {
					refresh_enemy(mob);
				}
			}
		}

		distance = get_distance(mob->mob.current, g_gener_list[mob->generate_index].start.position);

		if (mob->next >= 13 || distance > 5) {
			ret_action = ACTION_MOVE_RAND;
			mob->next = 0;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		} else {
			ret_action = ACTION_MOVE;
			mob->next++;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		}
		break;

	case MOB_TYPE_PESA_NPC:
		searching_enemy = get_enemy_from_view_by_mob_type(mob, MOB_TYPE_PESA_MOB);
		if (searching_enemy) {
			if (select_target_from_enemy_list(mob)) {
				ret_action = ACTION_BATTLE;
				mob->mode = MOB_COMBAT;
				mob->next_action = clock() + 1000 + abs(rand() % 1000);
				break;
			} else {
				refresh_enemy(mob);
			}
		}

		distance = get_distance(mob->mob.current, g_gener_list[mob->generate_index].start.position);

		if (mob->next >= 13 || distance > 5) {
			ret_action = ACTION_MOVE_RAND;
			mob->next = 0;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		} else {
			ret_action = ACTION_MOVE;
			mob->next++;
			mob->next_action = clock() + 3000 + abs(rand() % 7000);
			mob->mode = MOB_IDLE;
			break;
		}
		break;
	}
	return ret_action;
}

int
battle_processor(struct mob_server_st *mob)
{
	if (is_summon(*mob)) {
		if (mob->summoner >= MAX_USERS_PER_CHANNEL || mob->summoner <= 0) {
			remove_object(mob->mob.client_index, mob->mob.current, WORLD_MOB);
			clear_property(mob);
			return 0;
		}

		struct mob_server_st *summoner_mob = &g_mobs[mob->summoner];
		bool valid = false;
		for (size_t i = 0; i < MAX_PARTY; i++) {
			if (summoner_mob->baby_mob[i] == mob->mob.client_index) {
				valid = true;
				break;
			}
		}

		if (!valid) {
			remove_object(mob->mob.client_index, mob->mob.current, WORLD_MOB);
			clear_property(mob);
			return 0;
		}

		double distance = get_distance(mob->mob.current, summoner_mob->mob.current);
		if (distance > 14) {
			short newPosX = (summoner_mob->mob.current.X);
			short newPosY = (summoner_mob->mob.current.Y);
			movement(mob, newPosX, newPosY, MOVE_TELEPORT);
			return NO_MORE_ACTION;
		}
	}

	if (mob->current_target == 0) {
		refresh_enemy(mob);

		if (!select_target_from_enemy_list(mob)){
			mob->mode = MOB_IDLE;
			mob->current_target = 0;
			return NO_MORE_ACTION;
		}
	} else {
		double distance = get_distance(mob->mob.current, g_mobs[mob->current_target].mob.current);
		if (distance > 2 && distance < 6) {
			short newPosX = g_mobs[mob->current_target].mob.current.X;
			short newPosY = g_mobs[mob->current_target].mob.current.Y;
			movement(mob, newPosX, newPosY, MOVE_NORMAL);
			return NO_MORE_ACTION;
		}

		double distance_2 = get_distance(mob->mob.current, g_mobs[mob->current_target].mob.current);
		if (distance_2 <= 2) {
			send_attack(mob->mob.client_index, mob->current_target);
			add_enemy_list(&g_mobs[mob->current_target], mob->mob.client_index);
			return NO_MORE_ACTION;
		}
	}

	return NO_MORE_ACTION;
}

void
level_up(struct mob_server_st *mob)
{
	if (mob->mob.b_status.level > 399 && mob->mob.class_master <= CLASS_ARCH) {
		mob->mob.b_status.level = 399;
		return;
	} else if (mob->mob.b_status.level > 199 && mob->mob.class_master >= CLASS_CELESTIAL) {
		mob->mob.b_status.level = 199;
		return;
	}

	if (mob->mob.class_master >= CLASS_CELESTIAL && mob->mob.experience >= experience_celestial_subcelestial[mob->mob.b_status.level + 1]) {
		// TODO: IMPLEMENTAR
	}
	
	if (mob->mob.experience >= experience_mortal_arch[mob->mob.b_status.level + 1]) {
		if (mob->mob.b_status.level <= 398) {
			if (mob->mob.class_master == CLASS_MORTAL) {
				mob->mob.b_status.level += 1;
				if (mob->mob.class_info == 3)
					mob->mob.b_status.attack += 3;
				else
					mob->mob.b_status.attack += 1;

				if (mob->mob.class_info == 2)//bm
					mob->mob.b_status.defense += 3;
				else if (mob->mob.class_info == 0)//tk
					mob->mob.b_status.defense += 2;
				else
					mob->mob.b_status.defense += 1;

				if (mob->mob.class_info == 1)//foema
					mob->mob.b_status.max_mp += 3;
				else
					mob->mob.b_status.max_mp += 1;

				if (mob->mob.class_info == 0)//tk
					mob->mob.b_status.max_hp += 3;
				else
					mob->mob.b_status.max_hp += 1;

				mob->mob.hold = 0;
				get_bonus_score_points(&mob->mob);
				get_bonus_master_points(&mob->mob);
				get_bonus_skill_points(&mob->mob);
				mob->mob.status.current_hp = mob->mob.status.max_hp;
				mob->mob.status.current_mp = mob->mob.status.max_mp;
				get_current_score(mob->mob.client_index);
				send_etc(mob->mob.client_index);
				send_score(mob->mob.client_index);
				send_affects(mob->mob.client_index);
				send_client_message("+ + + Level UP + + +", mob->mob.client_index);
				send_emotion(mob->mob.client_index, Effect_LevelUp);
				send_all_packets(mob->mob.client_index);
				return;
			} else if (mob->mob.class_master == CLASS_ARCH) {
				mob->mob.b_status.level += 1;
				mob->mob.arch_level = mob->mob.b_status.level;
				if (mob->mob.class_info == 3)
					mob->mob.b_status.attack += 4;
				else
					mob->mob.b_status.attack += 2;

				if (mob->mob.class_info == 2)//bm
					mob->mob.b_status.defense += 4;
				else if (mob->mob.class_info == 0)//tk
					mob->mob.b_status.defense += 2;
				else
					mob->mob.b_status.defense += 1;

				if (mob->mob.class_info == 1 || mob->mob.class_info == 2)//foema
					mob->mob.b_status.max_mp += 4;
				else
					mob->mob.b_status.max_mp += 2;

				if (mob->mob.class_info == 0)//tk
					mob->mob.b_status.max_hp += 4;
				else
					mob->mob.b_status.max_hp += 2;

				mob->mob.hold = 0;
				get_bonus_score_points(&mob->mob);
				get_bonus_master_points(&mob->mob);
				get_bonus_skill_points(&mob->mob);
				mob->mob.status.current_hp = mob->mob.status.max_hp;
				mob->mob.status.current_mp = mob->mob.status.max_mp;
				get_current_score(mob->mob.client_index);
				send_etc(mob->mob.client_index);
				send_score(mob->mob.client_index);
				send_affects(mob->mob.client_index);
				send_client_message("+ + + Level UP + + +", mob->mob.client_index);
				send_emotion(mob->mob.client_index, Effect_LevelUp);

				send_all_packets(mob->mob.client_index);
				return;
			}
		}
	}
}

void 
mob_drop(struct mob_server_st *user, int mob_index)
{
	struct mob_server_st *mob = &g_mobs[mob_index];
	if (strcmp(mob->mob.name, "Runas") == 0) {
		int rune_id = ((rand() % 23) + 5110);
		int prob_drop = (rand() % 100);
		
		if (prob_drop < 25) {
			struct item_st rune = { 0 };
			rune.EF1 = 43;

			if (rune_id == 5115)
				rune_id = ((rand() % 23) + 5110);

			rune.item_id = rune_id;

			int empty_slot = get_item_slot(user->mob.client_index, 0, INV_TYPE);
			if (empty_slot != -1) {
				memcpy(&user->mob.inventory[empty_slot], &rune, sizeof(struct item_st));
				send_create_item(user->mob.client_index, INV_TYPE, empty_slot, &user->mob.inventory[empty_slot]);
				send_client_message(".:: PARABENS ::.", user->mob.client_index);
			}
		}
	} else {
		int max_item_drop = 1 + DROP_RATE;
		if (max_item_drop > 12)
			max_item_drop = 12;
		
		int drop_gold = (mob->mob.gold * DROP_RATE);
		if (drop_gold > 2000000000)
			drop_gold = 2000000000;
		
		int current_item_drop = 0;
		int armor_level = rand() % 6;
		int weapon_level = rand() % 6;

		if (armor_level <= 4)
			armor_level = 0;
		else
			armor_level = 1;

		if (weapon_level <= 4)
			weapon_level = 0;
		else
			weapon_level = 1;

		int item_boss = (rand() % 100);
		int drop_rate = DROP_RATE + user->drop_bonus;

		if (user->mob.gold + drop_gold > 2000000000)
			user->mob.gold = 2000000000;
		else
			user->mob.gold += drop_gold;

		send_etc(user->mob.client_index);
		for (size_t x = 0; x <= 62 && current_item_drop < max_item_drop; x++) {
			int slot_index = x % 8;
			int slot_type = x / 8;

			if ((current_item_drop >= max_item_drop) && (slot_type != 7) && slot_index != 0)
				continue;

			struct item_st drop_temp;
			memcpy(&drop_temp, &mob->mob.inventory[x], sizeof(struct item_st));
			if (drop_temp.item_id <= 0)
				continue;

			bool drop = false;
			int css = rand() % 100;
			if (css >(50 + (armor_level * 25)))
				continue;

			int chance = 0;
			if ((slot_type == 0 || slot_type == 2) && armor_level == 0) {
				chance = (14 + drop_rate);
			} else if (slot_type == 1 && item_boss <= 25) { //(boos) (1/4)
				chance = (3 + drop_rate);
			} else if (slot_type == 3 && weapon_level == 0) {
				chance = (13 + drop_rate);
			} else if ((slot_type == 4 || slot_type == 5) && armor_level == 1) {
				chance = (8 + drop_rate);
			} else if (slot_type == 6 && weapon_level == 1) {
				chance = (8 + drop_rate);
			} else if (slot_type == 7) { // Special Slots
				if (slot_index == 0) //slot 56 == 100% Confirmado TMSRV
					chance = 100;
				else if (slot_index == 1 || slot_index == 2) //slot 57 e 58 == 50%
					chance = (20 + drop_rate);
				else if (slot_index == 3) // slot 59 == 3%
					chance = (2 + drop_rate);
				else if (slot_index == 4) // slot 60 == 10%
					chance = (5 + drop_rate);
				else if (slot_index == 5 || slot_index == 6) //slot 61 e 62 == 5%
					chance = (3 + drop_rate);
			} else {
				chance = 0;
			}

			if (chance == 0) {
				drop = false;
			} else {
				int success = 0;
				int tx = 50;
				tx += 150; // ????
				success = (rand() % tx);
				drop = (success <= chance);
			}

			if (drop) {
				int empty_slot = get_item_slot(user->mob.client_index, 0, INV_TYPE);
				if (empty_slot == -1) {
					send_client_message("Inventario cheio!", user->mob.client_index);
					break;
				}

				memcpy(&user->mob.inventory[empty_slot], &drop_temp, sizeof(struct item_st));
				send_create_item(user->mob.client_index, INV_TYPE, empty_slot, &user->mob.inventory[empty_slot]);
				current_item_drop++;
			}
		}

		int rate_bonus = abs(rand() % 120);
		if (rate_bonus <= 3) {
			struct item_st drop_temp;
			memset(&drop_temp, 0, sizeof(struct item_st));
			drop_temp.item_id = 4026;

			int empty_slot = get_item_slot(user->mob.client_index, 0, INV_TYPE);
			if (empty_slot == -1) {
				send_client_message("Inventario cheio!", user->mob.client_index);
				return;
			}

			memcpy(&user->mob.inventory[empty_slot], &drop_temp, sizeof(struct item_st));
			send_create_item(user->mob.client_index, INV_TYPE, empty_slot, &user->mob.inventory[empty_slot]);
		}

		rate_bonus = abs(rand() % 120);
		if (rate_bonus <= 2) {
			struct item_st drop_temp;
			memset(&drop_temp, 0, sizeof(struct item_st));
			drop_temp.item_id = 4027;

			int empty_slot = get_item_slot(user->mob.client_index, 0, INV_TYPE);
			if (empty_slot == -1) {
				send_client_message("Inventario cheio!", user->mob.client_index);
				return;			
			}

			memcpy(&user->mob.inventory[empty_slot], &drop_temp, sizeof(struct item_st));
			send_create_item(user->mob.client_index, INV_TYPE, empty_slot, &user->mob.inventory[empty_slot]);
		}
	}
}

