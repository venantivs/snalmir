/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Julho de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/mob.c
 * Descrição: Arquivo onde são implementadas as funções que correspondem ao mobs (chars, npcs, monstros, etc) em específico.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../network/server.h"
#include "../network/packet-def.h"
#include "utils.h"
#include "user.h"

void
load_base_char_mobs()
{
  static const char *class_mobs_paths[] = {
    "./npc/TK",
    "./npc/FM",
    "./npc/BM",
    "./npc/HT"
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

  sprintf(file_path, "./char/%s", name);

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

  sprintf(file_path, "./char/%s", nickname); // Passar para snprintf

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

  sprintf(file_path, "./char/%s", name);

  if (mob->name == NULL)
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
