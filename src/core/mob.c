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

#include "../network/server.h"
#include "../network/packet-def.h"
#include "utils.h"
#include "user.h"
#include "world.h"
#include "base_functions.h"

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

    fread(&base_char_mobs[i], sizeof(struct mob_st), 1, user_mob);
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

  struct account_file_st *account_file = &users_db[user_index];
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

  struct account_file_st *account_file = &users_db[user_index];
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

  struct account_file_st *account_file = &users_db[user_index];
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
			if ((is_summon(*mob) && (mob->enemy_list[i] < MAX_USERS_PER_CHANNEL)) || (is_summon(*mob) && is_summon(mobs[mob->enemy_list[i]]))) {
				if (check_pvp_area(mob->enemy_list[i]) == 0) remove_enemy_list(mob, mob->enemy_list[i]);
				else if (check_pvp_area(mob->mob.client_index) == 0) remove_enemy_list(mob, mob->enemy_list[i]);
			}
			else if (is_dead(mobs[mob->enemy_list[i]]))
				remove_enemy_list(mob, mob->enemy_list[i]);
			else if (is_summon(*mob) && mob->enemy_list[i] == mob->summoner)
				remove_enemy_list(mob, mob->enemy_list[i]);
			else if (get_distance(mob->mob.current, mobs[mob->enemy_list[i]].mob.current) > 7)
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
          if (is_dead(mobs[mob->max_damage])) {
            remove_enemy_list(mob, mob->max_damage);
            break;
          } else if (mob->max_damage > BASE_MOB && !is_summon(mobs[mob->max_damage]) && !is_summon(*mob)) {
            remove_enemy_list(mob, mob->max_damage);
            break;
          } else if (get_distance(mob->mob.current, mobs[mob->max_damage].mob.current) > 7) {
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
          if (is_dead(mobs[mob->low_hp])) {
            remove_enemy_list(mob, mob->low_hp);
            break;
          } else if (mob->low_hp > BASE_MOB && !is_summon(mobs[mob->low_hp]) && !is_summon(*mob)) {
            remove_enemy_list(mob, mob->low_hp);
            break;
          } else if (get_distance(mob->mob.current, mobs[mob->low_hp].mob.current) > 7) {
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
          if (is_dead(mobs[mob->low_defense])) {
            remove_enemy_list(mob, mob->low_defense);
            break;
          } else if (mob->low_defense > BASE_MOB && !is_summon(mobs[mob->low_defense]) && !is_summon(*mob)) {
            remove_enemy_list(mob, mob->low_defense);
            break;
          } else if (get_distance(mob->mob.current, mobs[mob->low_defense].mob.current) > 7) {
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
          if (is_dead(mobs[mob->low_level])) {
            remove_enemy_list(mob, mob->low_level);
            continue;
          } else if (mob->low_level > BASE_MOB && ! is_summon(mobs[mob->low_level]) && !is_summon(*mob)) {
            remove_enemy_list(mob, mob->low_level);
            continue;
          } else if (get_distance(mob->mob.current, mobs[mob->low_level].mob.current) > 7) {
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
			if (is_dead(mobs[mob->enemy_list[i]])) {
        remove_enemy_list(mob, mob->enemy_list[i]);
        continue;
      } else if (mob->enemy_list[i] > BASE_MOB && !is_summon(mobs[mob->enemy_list[i]]) && !is_summon(*mob)) {
        remove_enemy_list(mob, mob->enemy_list[i]);
        continue;
      } else if (get_distance(mob->mob.current, mobs[mob->enemy_list[i]].mob.current) > 7) {
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
			short mob_id = mob_grid[nY][nX];

			if (mob_id <= 0)
        continue;

			if (mob_id > BASE_MOB) {
				if (!is_summon(mobs[mob_id]))
					continue;
				else if(mobs[mob_id].mob_type != MOB_TYPE_GUARD) 
					continue;
			}

			if (is_dead(mobs[mob_id]))
        continue;

			if (get_distance(mob->mob.current, mobs[mob_id].mob.current) > 3)
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
			short mob_id = mob_grid[nY][nX];

      if (mob_id <= 0)
        continue;

      if (mob_id > BASE_MOB) {
				if (mobs[mob_id].mob_type == mob_type) {
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

	int	Rst4 = (sec_counter & 0x03);
	int	Rst5 = (sec_counter & 0x04);
	int	Rst8 = (sec_counter & 0x07);
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
			if (is_dead(mobs[mob->current_target])) {
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

		distance = get_distance(mob->mob.current, gener_list[mob->generate_index].start.position);

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
		distance = get_distance(mob->mob.current, gener_list[mob->generate_index].start.position);
		
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

		distance = get_distance(mob->mob.current, gener_list[mob->generate_index].start.position);

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

    distance = get_distance(mob->mob.current, gener_list[mob->summoner].start.position);

		if (distance > 14) {
			ret_action = ACTION_MOVE_TO_SUMMONER;
			mob->next_action = clock() + 2000 + abs(rand() % 2000);
			mob->mode = MOB_IDLE;
			break;
		} else {
			if (mob->current_target != 0) {
				if (is_dead(mobs[mob->current_target])) {
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

		distance = get_distance(mob->mob.current, gener_list[mob->generate_index].start.position);

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

		distance = get_distance(mob->mob.current, gener_list[mob->generate_index].start.position);

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
