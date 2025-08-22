/*
 * Licença: GPLv3
 * Autor: callixtvs
 * Data: Dezembro de 2023
 * Atualização: Dezembro de 2023
 * Arquivo: core/base_functions_send.c
 * Descrição: Arquivo onde são implementadas algumas funções setters úteis.
 */

#include "base-def.h"
#include "../network/server.h"
#include "base_functions.h"
#include "world.h"
#include "../network/socket-utils.h"

#include <stdio.h>
#include <string.h>

void
send_grid_mob(int index)
{
  if (index <= 0 || index >= MAX_SPAWN_LIST)
    return;

  struct mob_st *npc_mob = &mobs[index].mob;

  short posX = npc_mob->current.X, posY = npc_mob->current.Y;

  int vision_x = VIEW_GRIDX, vision_y = VIEW_GRIDY,
      min_position_x = (posX - HALF_GRIDX),
      min_position_y = (posY - HALF_GRIDY);

  if ((min_position_x + vision_x) >= MAX_GRIDX)
    vision_x = (vision_x - (vision_x + min_position_x - MAX_GRIDX));

  if ((min_position_y + vision_y) >= MAX_GRIDY)
    vision_y = (vision_y - (vision_y + min_position_y - MAX_GRIDY));

  if (min_position_x < 0) {
    min_position_x = 0;
    vision_x = (vision_x + min_position_x);
  }

  if (min_position_y < 0) {
    min_position_y = 0;
    vision_y = (vision_y + min_position_y);
  }

  int max_position_x = (min_position_x + vision_x),
    max_position_y = (min_position_y + vision_y);

  for (int y = min_position_y; y < max_position_y; y++) {
    for (int x = min_position_x; x < max_position_x; x++) {
      short mob_id = mob_grid[y][x];
      short item_id = item_grid[y][x];

      if (x > min_position_x || x <= max_position_x || y > min_position_y || y <= max_position_y)  {
        if (item_id > 0 && index <= MAX_USERS_PER_CHANNEL)
          get_create_item(item_id);
      }

      if (mob_id <= 0 || index == mob_id)
        continue;

      if (mob_id <= MAX_USERS_PER_CHANNEL)
        send_create_mob(mob_id, index);

      if (index <= MAX_USERS_PER_CHANNEL)
        send_create_mob(index, mob_id);
    }
  }
}

void
send_grid_item(int index, void *packet)
{
  if (index <= 0 || index >= MAX_INIT_ITEM_LIST)
    return;

  short posX = ground_items[index].position.X,
    posY = ground_items[index].position.Y;

  int VisX = VIEW_GRIDX, VisY = VIEW_GRIDY,
    minPosX = (posX - HALF_GRIDX),
    minPosY = (posY - HALF_GRIDY);

  if ((minPosX + VisX) >= MAX_GRIDX)
    VisX = (VisX - (VisX + minPosX - MAX_GRIDX));

  if ((minPosY + VisY) >= MAX_GRIDY)
    VisY = (VisY - (VisY + minPosY - MAX_GRIDY));

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

      if (mob_id <= MAX_USERS_PER_CHANNEL)
        send_one_message((unsigned char*) packet, xlen(packet), mob_id);
    }
  }
}

void
send_create_mob(int user_index, int mob_index)
{
  if (user_index <= MAX_USERS_PER_CHANNEL) {
    if (mobs[mob_index].mode == MOB_MARKET) {
      // CREATES A MARKET MOB, TODO: Implement
    } else {
      get_create_mob(user_index, mob_index);
    }
  }
}

void 
send_score(short index)
{
	if (index <= 0 || index >= MAX_SPAWN_LIST)
		return;

	struct packet_refresh_score refresh_score;
	refresh_score.header.size = sizeof(struct packet_refresh_score);
	refresh_score.header.operation_code = 0x336;
	refresh_score.header.index = index;

  struct mob_server_st user = mobs[index];

	if (index < MAX_USERS_PER_CHANNEL) {
		refresh_score.current_hp = user.mob.status.current_hp;
		refresh_score.current_mp = user.mob.status.current_mp;
	}

	refresh_score.critical = mobs[index].mob.critical;
	refresh_score.save_mana = mobs[index].mob.save_mana;

	if (user.guild_disable == 0) {
		refresh_score.guild_index = user.mob.guild_id;
		refresh_score.guild_member_type = user.mob.guild_member_type * 0x40;
	} else {
		refresh_score.guild_index = 0;
		refresh_score.guild_member_type = 0;
	}

	refresh_score.resist_1 = user.mob.resist[0];
	refresh_score.resist_2 = user.mob.resist[1];
	refresh_score.resist_3 = user.mob.resist[2];
	refresh_score.resist_4 = user.mob.resist[3];
	refresh_score.regen_hp = user.mob.regen_hp;
	refresh_score.regen_mp = user.mob.regen_mp;

	int m = user.mob.magic_increment;
	if (m >= 255)
		m = 254;

	refresh_score.magic_increment = m;
	refresh_score.status = user.mob.status;

  memset(&refresh_score.special, 0x0, 4);
	int offset = (int)((int) refresh_score.affect - (int) &refresh_score); 
  get_affect(offset, user.mob.affect, (unsigned char*) &refresh_score);
	send_grid_multicast(user.mob.current.X, user.mob.current.Y, &refresh_score, 0); // Envia para todos os Indexes
}

void 
send_etc(short index)
{
	if (index <= 0 || index >= MAX_USERS_PER_CHANNEL)
		return;

  struct mob_st mob = mobs[index].mob;

	struct packet_refresh_etc refresh_etc;
	refresh_etc.header.size = sizeof(struct packet_refresh_etc);
	refresh_etc.header.operation_code = 0x337;
	refresh_etc.header.index = index;
	refresh_etc.hold = mob.hold;
	refresh_etc.exp = mob.experience;
	refresh_etc.learn = mob.learn;
	refresh_etc.status = mob.p_status;
	refresh_etc.master = mob.p_master;
	refresh_etc.skills = mob.p_skill;
	refresh_etc.magic = mob.magic_increment;
	refresh_etc.gold = mob.gold;

	add_client_message((unsigned char*) &refresh_etc, xlen(&refresh_etc), index);
}

void
send_affects(int index)
{
	if (index > MAX_USERS_PER_CHANNEL)
		return;

  struct mob_st user = mobs[index].mob;

  struct packet_affect send_affect;
	send_affect.header.operation_code = 0x3B9;
	send_affect.header.size = 140;
	send_affect.header.index = index;

	for (size_t i = 0; i < MAX_AFFECT; i++) {
		if (user.affect[i].time >= 0 && user.affect[i].index != 0) {
			send_affect.affect[i].index = user.affect[i].index;
			send_affect.affect[i].master = user.affect[i].master;
			send_affect.affect[i].value = user.affect[i].value;
			send_affect.affect[i].time = user.affect[i].time;
		} else {
			send_affect.affect[i].index = 0;
			send_affect.affect[i].master = 0;
			send_affect.affect[i].value = 0;
			send_affect.affect[i].time = 0;

			user.affect[i].index = 0;
			user.affect[i].master = 0;
			user.affect[i].value = 0;
			user.affect[i].time = 0;
		}
	}

	add_client_message((unsigned char*) &send_affect, xlen(&send_affect), index);
}

void
send_emotion(int index, int effect_type, int effect_value)
{
  struct packet_emotion emotion;
	emotion.header.size = sizeof(struct packet_emotion);
	emotion.header.operation_code = 0x36A;
	emotion.header.index = index;
	emotion.effect_type = effect_type;
	emotion.effect_value = effect_value;
	emotion.unknown = 0;

  struct mob_st user = mobs[index].mob;

	send_grid_multicast(user.current.X, user.current.Y, &emotion, 0);
}

// TODO: REFATORAR
void 
send_grid_multicast(short position_x, short position_y, void *packet, int index)
{
	int VisX = VIEW_GRIDX, VisY = VIEW_GRIDY,
		minPosX = (position_x - HALF_GRIDX),
		minPosY = (position_y - HALF_GRIDY);

	if ((minPosX + VisX) >= MAX_GRIDX)
		VisX = (VisX - (VisX + minPosX - MAX_GRIDX));

	if ((minPosY + VisY) >= MAX_GRIDY)
		VisY = (VisY - (VisY + minPosY - MAX_GRIDY));

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

			if (mob_id <= 0 || index == mob_id)
				continue;

			if (packet == NULL || mob_id >= MAX_USERS_PER_CHANNEL)
				continue;

      send_one_message((unsigned char*) packet, xlen(packet), index);
		}
	}
}

void
send_grid_multicast_with_packet(int mob_index, short position_x, short position_y, unsigned char *packet)
{
	if (mob_index <= 0 || mob_index >= MAX_SPAWN_LIST)
		return;

	struct mob_st *mob = &mobs[mob_index].mob;

	int VisX = VIEW_GRIDX, VisY = VIEW_GRIDY,
		minPosX = (mob->current.X - HALF_GRIDX),
		minPosY = (mob->current.Y - HALF_GRIDY);

	if ((minPosX + VisX) >= MAX_GRIDX)
		VisX = (VisX - (VisX + minPosX - MAX_GRIDX));

	if ((minPosY + VisY) >= MAX_GRIDY)
		VisY = (VisY - (VisY + minPosY - MAX_GRIDY));

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

	int dVisX = VIEW_GRIDX, dVisY = VIEW_GRIDY,
		dminPosX = (position_x - HALF_GRIDX),
		dminPosY = (position_y - HALF_GRIDY);

	if ((dminPosX + dVisX) >= MAX_GRIDX)
		dVisX = (dVisX - (dVisX + dminPosX - MAX_GRIDX));

	if ((dminPosY + dVisY) >= MAX_GRIDY)
		dVisY = (dVisY - (dVisY + dminPosY - MAX_GRIDY));

	if (dminPosX < 0) {
		dminPosX = 0;
		dVisX = (dVisX + dminPosX);
	}

	if (dminPosY < 0) {
		dminPosY = 0;
		dVisY = (dVisY + dminPosY);
	}

	int dmaxPosX = (dminPosX + dVisX),
		dmaxPosY = (dminPosY + dVisY);

	for (int nY = minPosY; nY < maxPosY; nY++) {
		for (int nX = minPosX; nX < maxPosX; nX++) {
			int mob_id = mob_grid[nY][nX];
			
			if (mob_id <= 0 || mob_index == mob_id)
				continue;
			
			if (packet != NULL && mob_id <= MAX_USERS_PER_CHANNEL)
				send_one_message((unsigned char*) packet, xlen(packet), mob_id);

			if (nX < dminPosX || nX >= dmaxPosX || nY < dminPosY || nY >= dmaxPosY) {
				if (mob_id <= MAX_USERS_PER_CHANNEL) {
					struct packet_signal_parm signal_parm;
					signal_parm.header.size = sizeof(struct packet_signal_parm);
					signal_parm.header.index = mob_index;
					signal_parm.header.operation_code = 0x165;
					signal_parm.data = DELETE_NORMAL;

					send_one_message((unsigned char*) &signal_parm, sizeof(struct packet_signal_parm), mob_id);
				}

				if (mob_index <= MAX_USERS_PER_CHANNEL) {
					struct packet_signal_parm signal_parm;
					signal_parm.header.size = sizeof(struct packet_signal_parm);
					signal_parm.header.index = mob_id;
					signal_parm.header.operation_code = 0x165;
					signal_parm.data = DELETE_NORMAL;

					send_one_message((unsigned char*) &signal_parm, sizeof(struct packet_signal_parm), mob_index);
				}
			}
		}
	}

	mob->current.X = position_x;
	mob->current.Y = position_y;

	for (int nY = dminPosY; nY < dmaxPosY; nY++) {
		for (int nX = dminPosX; nX < dmaxPosX; nX++) {
			short init_id = item_grid[nY][nX];
			short mob_id = mob_grid[nY][nX];

			if (nX < minPosX || nX >= maxPosX || nY < minPosY || nY >= maxPosY) {
				if (mob_id > 0 && mob_index != mob_id) {
					if (mob_id <= MAX_USERS_PER_CHANNEL)
						send_create_mob(mob_id, mob_index);
					if (mob_index <= MAX_USERS_PER_CHANNEL)
						send_create_mob(mob_index, mob_id);
					if (packet != NULL && mob_id <= MAX_USERS_PER_CHANNEL)
						send_one_message((unsigned char*) packet, xlen(packet), mob_id);
				}

				if (init_id > 0) {
					if (mob_index <= MAX_USERS_PER_CHANNEL) {
						get_create_item(init_id);

						struct ground_item_st init = ground_items[init_id];

						if (init.status == 1 || init.status == 4) {

							if (get_door_type(init.position) == 0)
								continue;

							struct packet_door door;
							door.header.operation_code = 0x374;
							door.header.index = 30000;
							door.header.size = sizeof(struct packet_door);
							door.status = init.status;
							door.init_id = init_id + 10000;

							if (mob_id <= MAX_USERS_PER_CHANNEL)
								send_one_message((unsigned char*) &door, sizeof(struct packet_door), mob_id);

							if (mob_index <= MAX_USERS_PER_CHANNEL)
								send_one_message((unsigned char*) &door, xlen(&door), mob_index);
						}
					}
				}
			}
		}
	}

	mob->current.X = position_x;
	mob->current.Y = position_y;
}

void
send_remove_mob(int mob_index, int to_remove_index, int delete_type)
{
	struct packet_signal_parm remove_mob_signal;
	remove_mob_signal.header.size = sizeof(struct packet_signal_parm);
	remove_mob_signal.header.index = to_remove_index;
	remove_mob_signal.header.operation_code = 0x165;
	remove_mob_signal.data = delete_type;

	send_grid_multicast_with_packet(0, mobs[mob_index].mob.current.X, mobs[mob_index].mob.current.Y, (unsigned char*) &remove_mob_signal);
}

void
send_teleport(int mob_index, struct position_st destination)
{
	struct mob_st *mob = &mobs[mob_index].mob;

	if (mob->current.X == destination.X && mob->current.Y == destination.Y)
		return;

	update_world(mob_index, &destination.X, &destination.Y, WORLD_MOB);

	mob->last_position.X = mob->current.X;
	mob->last_position.Y = mob->current.Y;

	get_action(mob_index, destination.X, destination.Y, MOVE_TELEPORT, NULL);
}