/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/game_skills.c
 * Descrição: Arquivo onde são implementadas as funções que carregam as skills do jogo.
 */

#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "game_skills.h"

struct skill_data_st g_skill_data[MAX_SKILL_DATA];

void
load_skill_data()
{
  FILE *skill_data_fd = fopen("./bin/SkillData.csv", "rt");

  if (skill_data_fd == NULL)
    fatal_error("Could not load SkillData.csv");
  
  char tmp[1024] = { 0 };
  while(fgets(tmp, sizeof(tmp), skill_data_fd)) {
    int index = -1;
    struct skill_data_st spell = { 0 };

    if (tmp[0] == '\n' || tmp[0] == '#') continue;

    // TODO: REVER SE TEM NECESSIDADE DISSO
    char *p = tmp;
    while(p[0] != '\0') {
      if (p[0] == ',')
        p[0] = ' ';
      p++;
    }

    sscanf(tmp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %*s %*s %d %d %d %d %d %d",
      &index, &spell.points, &spell.target, &spell.mana, &spell.delay, &spell.range,
      &spell.instance_type, &spell.instance_value, &spell.tick_type, &spell.tick_value,
      &spell.affect_type, &spell.affect_value, &spell.time, &spell.instance_attribute,
      &spell.tick_attribute, &spell.aggressive, &spell.max_target, &spell.party_check,
      &spell.affect_resist, &spell.passive_check, &spell.unknown);

    if (index < 0 || index >= MAX_SKILL_DATA) continue;

    memcpy(&g_skill_data[index], &spell, sizeof(struct skill_data_st));
  }

  fclose(skill_data_fd);
}

void 
load_game_skills()
{
  memset(g_skill_data, 0, sizeof(struct skill_data_st) * MAX_SKILL_DATA);

  load_skill_data();
}