/*
 * Commentaries with (USELESS) are PROBABLY useless declarations for this server project, as this file is basically copied from another server's base definitions file.
 * Therefore, those should be CALMLY revaluated as the project goes on.
 */

#ifndef __BASE_DEF_H__
#define __BASE_DEF_H__

#include <stdbool.h>
#include <time.h>

#include "../general-config.h"

#define ACCOUNTNAME_LENGTH	16
#define	ACCOUNTPASS_LENGTH	12
#define	SERVERNAME_LENGTH	16
#define	REALNAME_LENGTH		24
#define	EMAIL_LENGTH		48
#define	ADDRESS_LENGTH		80
#define	TELEPHONE_LENGTH	16

#define MAX_LEVEL		400
#define	MAX_LEVELCSH		200
#define	MOB_PER_ACCOUNT		4

#define	MAX_SUMMONLIST			4096
#define	MAX_GUILD						4096
#define	MAX_NPCGENERATOR		8192
#define MAX_SPAWN_LIST 			30000
#define MAX_INIT_ITEM_LIST 	4096
#define	MAX_STORAGE					128
#define	MAX_CLAN						26
#define	MAX_SEGMENT					5
#define	LAST_SEGMENT				4
#define	MAX_CLASS						4

#define	IDX_ITEM		10000
#define	MAX_ITEM		6500
#define	MAX_ITEMLIST		6500
#define	MAX_ITEM_DATA		6500
#define	ITEMNAME_LENGTH		28

#define	MAX_GRIDX		4096
#define	MAX_GRIDY		4096
#define	VIEW_GRIDX		23
#define	VIEW_GRIDY		23
#define	HALF_GRIDX		VIEW_GRIDX/2
#define	HALF_GRIDY		VIEW_GRIDY/2
#define	PRANAGRIDX		100
#define	PRANAGRIDY		100

#define	MAX_AFFECT              16
#define	MAX_DATABASE            6
#define	MAX_TELEPORT            256
#define	MAX_GAMESERVER          6
#define	MAX_GUILD_ZONE          5
#define	MAX_MOB_BABY            38
#define	MAX_PARTY               13
#define	MAX_EFFECT              12
#define	MAX_SIZENAME            64
#define	MAX_EFFECT_INDEX        120
#define	MAX_EFFECT_NAME         16
#define	MAX_MESH_BUFFER         16
#define	MAX_SCORE_BUFFER        32
#define	MAX_PARTY_LOAD          MAX_USERS_PER_CHANNEL
#define	EMU_VERSION             3.0

#define	CLIENT_VERSION		754

#define NOT_SESSION		0
#define LOGING_SESSION          1
#define CHARLIST_SESSION        2
#define CREATING_SESSION        4
#define DELETING_SESSION        8
#define ENTERING_SESSION        16
#define NUMERICA_SESSION        32
#define NUMERICA_SESSION2       34
#define GAME_SESSION            64

/* Sockets Index (USELESS) */
#define	SERVER_ID               0
#define	DATABASE_ID             1

/* Size and Types Server */
#define	BASE_CHARACTER          0
#define	BASE_MOB                1000

/* Actions */
#define	ACTION_MOVE		0
#define	ACTION_MOVE_RAND	1
#define	ACTION_BATTLE		2
#define	ACTION_MOVE_TO_SUMMONER	3
#define	NO_MORE_ACTION          -1

/* Spawn Types */
#define	SPAWN_NORMAL            0	/* Somente aparece */
#define	SPAWN_TELEPORT          2	/* Efeito usado quando o personagem nasce ou e teleportado */
#define	SPAWN_BABYGEN           10	/* Efeito de quando uma cria nasce (75x only) */

/* Delete  Mob Types */
#define	DELETE_NORMAL           0	/* Somente desaparece */
#define	DELETE_DEAD             1	/* Animacao da morte do spawn */
#define	DELETE_DISCONNECT       2	/* Efeito de quando o personagem sai do jogo */
#define	DELETE_UNSPAWN          3	/* Efeito quando os monstros ancts somem */

/* Move Types */
#define	ROUTE_NORMAL            0
#define	MOVE_NORMAL             0
#define	MOVE_TELEPORT           1

/* Slot Types */
#define	EQUIP_TYPE              0
#define	INV_TYPE                1
#define	STORAGE_TYPE            2

/* Equipment Slots */
#define	FACE_SLOT               0
#define	HELM_SLOT               1
#define	ARMA1_SLOT              6
#define	ARMA2_SLOT              7
#define	MOUNT_SLOT              14
#define	PET_SLOT		13
#define	CAPE_SLOT               15

/* Cities */
#define	ARMIA                   1
#define	ERION                   2
#define	AZRAN                   3
#define	NOATUN                  4
#define	GELO                    5

/* Classes */
#define	TRANSKNIGHT		0
#define	FOEMA			1
#define	BEASTMASTER		2
#define	HUNTRESS		3

/* Status Masterys (???) */
#define	WYD_fMaster             5
#define	WYD_sMaster             10
#define	WYD_tMaster             15

/* Movement Delay */
#define	AI_DELAY_MOVIMENTO      2000

/* Quests */
#define	QUEST_C_ARMIA_1         1
#define	QUEST_C_ARMIA_2         2
#define	QUEST_C_ARMIA_3         3
#define	QUEST_COMECO            4
#define	QUEST_MAGIC_BEAN	5
#define	QUEST_MOLAR		6
#define	QUEST_ARCH_LV355	7
#define	QUEST_CELE_LV40		8
#define	QUEST_CELE_LV90		9
#define	QUEST_CREATE_ARCH	10

/* Class Evolution */
#define	CLASS_MORTAL            1
#define	CLASS_ARCH              2
#define	CLASS_CELESTIAL         3
#define	CLASS_SUB_CELESTIAL     4
#define	CLASS_HARDCORE          5

/* BeastMaster Mutations */
#define	LOBISOMEM               1
#define	URSO                    2
#define	ASTAROTH                3
#define	TITA                    4
#define	EDEN                    5

/* BeastMaster Mutation Faces */
#define LOBISOMEM_FACE          22
#define URSO_FACE               23
#define ASTAROTH_FACE           24
#define TITA_FACE               25
#define EDEN_FACE               32

/* Buffs */
#define LENTIDAO                1
#define FM_VELOCIDADE           2
#define RESISTENCIA_N           3
#define EVASAO_N                5
#define POCAO_ATK               6
#define VELOCIDADE_N            7
#define ADD                     8
#define FM_BUFFATK              9
#define ATKMENOS                10
#define FM_ESCUDO_MAGICO        11
#define DEFESA_N                12
#define TK_ASSALTO              13
#define TK_POSSUIDO             14
#define FM_SKILLS               15
#define BM_MUTACAO              16
#define TK_AURAVIDA             17
#define FM_CONTROLE_MANA        18
#define HT_IMUNIDADE            19
#define VENENO                  20
#define HT_MEDITACAO            21
#define FM_TROVAO               22
#define BM_AURA_BESTIAL         23
#define TK_SAMARITANO           24
#define BM_PROTELEMENT          25
#define HT_EVASAO_APRIMORADA    26
#define HT_GELO                 27
#define HT_INIVISIBILIDADE      28
#define LIMITE_DA_ALMA          29
#define PvM                     30
#define HT_ESCUDO_DOURADO       31
#define CANCELAMENTO            32
#define MUTACAO2                33
#define COMIDA                  34
#define BONUS_HP_MP             35
#define HT_VENENO               36
#define HT_LIGACAO_ESPCTRAL     37
#define HT_TROCAESP             38
#define BAU_EXP                 39

/* Evocations and Mounts */
#define CONDOR                  0
#define JAVALI                  1
#define LOBO                    2
#define E_URSO                  3
#define TIGRE                   4
#define GORILA                  5
#define DRAGAO_NEGRO            6
#define SUCCUBUS                7
#define PORCO                   8
#define JAVALI_                 9
#define LOBO_                   10
#define DRAGAO_MENOR            11
#define URSO_                   12
#define DENTE_DE_SABRE          13
#define SEM_SELA                14
#define FANTASMA                15
#define LEVE                    16
#define EQUIPADO                17
#define ANDALUZ                 18
#define SEM_SELA_               19
#define FANTASMA_               20
#define LEVE_                   21
#define EQUIPADO_               22
#define ANDALUZ_                23
#define FENRIR                  24
#define DRAGAO                  25
#define GRANDE_FENRIR           26
#define TIGRE_DE_FOGO           27
#define DRAGAO_VERMELHO         28
#define UNICORNIO               29
#define PEGASUS                 30
#define UNISUS                  31
#define GRIFO                   32
#define HIPPO_GRIFO             33
#define GRIFO_SANGRENTO         34
#define SVADILFARI              35
#define SLEIPNIR                36

/* Pesadelos e Cartas */
#define PesaN                   0
#define PesaM                   1
#define PesaA                   2
#define CartaN                  0
#define CartaM                  1
#define CartaA                  2

//EMOTION 50 22
#define p39x_MISS               -3
#define p39x_NORMAL             0
#define p39x_D_NORMAL           1 // Divide o atk em 2
#define p39x_CRITICO            2
#define p39x_D_CRITICO          3 // Divide o atk em 2
#define p39x_DANO_INVISIVEL     4
#define p39x_DANO_ADMIN         5 // Confirmar, mas o dano aumenta absurdamente
#define p39x_DANO_ADMIN_CRIT    7
#define p39x_SUPER              8 //Atck com aquele shok de ARCH+
#define p39x_D_SUPER            9
#define p39x_S_CRITICO          10
#define p39x_D_S_CRITICO        11
#define p39x_S_DANO_INVISIVEL   12

#define DOOR_CITY_WAR           1
#define DOOR_COLISEUM           2
#define DOOR_CAMP               3
#define DOOR_QUEST_FJ           4
#define DOOR_IMP_CASTEL         5
#define DOOR_NOATUN             6

#define Effect_Atk		4,1
#define Effect_Atk_eff		4,3
#define Effect_Atk2		5,1
#define Effect_Atk_eff2		5,3
#define Effect_DeadPlus		11,3
#define Effect_Ressurect	12,1
#define Effect_LevelUp		14,3
#define Effect_ComemorarRVR	17,1

// Estruturas Base

struct item_st {
	short item_id;
	struct {
		unsigned char index;
		unsigned char value;
	} effect[3];
};

#define EF1             effect[0].index
#define EFV1            effect[0].value
#define EF2             effect[1].index
#define EFV2            effect[1].value
#define EF3             effect[2].index
#define EFV3            effect[2].value

struct position_st {
	short X;
	short Y;
};

struct affect_st {
	unsigned char index;
	unsigned char master;
	short value;
	unsigned time;
};

struct trade_st {
	struct item_st trade_item[15];
	unsigned char trade_item_slot[15];
	unsigned short other_client;
	int gold;
	bool confirm;
	bool waiting;
};

struct settings_st {
	int experience_rate;
	int drop_rate;
};

struct guild_st {
	short guild_id;		/* Guild ID */
	char name[16];		/* Nome da Guild */
	char leaders[4][16];	/* [0] = Nome dono; [1], [2], [3] = Nome subdonos; */
	char nicks[50][16];	/* Nomes dos membros */
	short city_own;		/* Cidade da qual a Guild é dona */
	short ally_guild;	/* Guild ID da guild aliada */
	short war_guild;	/* Guild ID da guild rival */
	short member_count;	/* Contagem de membros */
	int bet_armia;		/* Valor apostado em Armia */
	int bet_erion;		/* Valor apostado em Erion */
	int bet_azran;		/* Valor apostado em Azran */
	int bet_nippleheim;	/* Valor apostado em Nippleheim (Gelo) */
	int reputation;		/* Attack bstatus de todos os membros da Guild */
	int cape_color;		/* Cor da capa; 0 = NULL; 1 = Blue; 2 = Red; 3 = Andarilho */
};

struct guild_file_st {
	int channel;
	int city_own;
	int guild_id;
	char name[16];
	char sub_owners[3][12];
};

struct market_st {
	char title[27];
	struct item_st item[12];
	unsigned char slot[12];
	int gold[12];
	short unknown;
	short index;
};

struct pesadelo_st {
	int group[3];
	int time[3];
	struct position_st teleport[3];
	struct position_st maximum_area[3];
	struct position_st minimum_area[3];
	int mobs[3][9];
	int npcs[3][8];
	int npcs_alive[3];
	int status[3];
};

struct mount_info_st {
	int damage;
	int magic;
	int evasion;
	int immunity;
};

struct cubo_file_st {
	int sala[25];
	int current_question[25];
	int mobs[25];
	int time[25];
	int status[25];
	bool answers[25];
	char questions[25][128];
	struct position_st O[25];
	struct position_st X[25];
};

struct city_st {
	char guild_name[16];
	int guild_own;
	int gold;
	int bets;
	int taxes;
	int challenger;
	char guild_df[16];
	bool is_at_war;
};

struct city_manager_st {
	struct city_st armia;
	struct city_st erion;
	struct city_st azran;
	struct city_st noatun;
	struct city_st nippleheim;
};

struct status_st {
	unsigned short level;
	short defense;
	short attack;
	
	unsigned char merchant : 4;
	unsigned char direction : 4;
	unsigned char speed : 4;
	unsigned char ChaosRate : 4;

	unsigned short max_hp;
	unsigned short max_mp;
	unsigned short current_hp;
	unsigned short current_mp;

	short strength;
	short intelligence;
	short dexterity;
	short constitution;

	unsigned char w_master;
	unsigned char f_master;
	unsigned char s_master;
	unsigned char t_master;
};

struct duel_st {
	bool start;
	int time;
	int player_1;
	int player_2;
};

struct mob_st {
	char name[16];
	char cape;
	struct {
		unsigned char merchant : 6;
		unsigned char city_id : 2;
	} info;
	unsigned short guild_id;
	unsigned char class_info;
	struct {
		unsigned char slow_mov : 1;
		unsigned char drain_hp : 1;
		unsigned char vision_drop : 1;
		unsigned char evasion : 1;
		unsigned char snoop : 1;
		unsigned char speed_mov : 1;
		unsigned char skill_delay : 1;
		unsigned char resist : 1;
	} affect_info;
	int gold;
	unsigned int experience;
	struct position_st last_position;
	struct status_st b_status;
	struct status_st status;
	struct item_st equip[16];
	struct item_st inventory[64];
	int learn;			// ?
	unsigned short p_status;
	unsigned short p_skill;
	unsigned short p_master;
	unsigned char critical;
	unsigned char save_mana;
	char skill_bar_1[4];
	char guild_member_type;
	unsigned char unknown222;
	char regen_hp;
	char regen_mp;
	unsigned char resist[4];
	short slot_index;
	short client_index;
	unsigned int perforation;
	char skill_bar_2[16];
	int hold;
	char tab[26];
	unsigned int absorption;
	int timestamp;
	unsigned short atack_speed;
	int drain_hp;
	int arch_level;
	int cele_level;
	unsigned int quest_info;
	char guild_name[16];
	int deed_time;				/* Escritura */
	unsigned long long int deed_tick; 	/* Escritura */
	char unknown[148];
	struct position_st current;
	struct position_st dest;
	struct position_st gema;
	short evasion;
	short syub;
	int class_master;
	unsigned short acc_level;
	struct affect_st affect[MAX_AFFECT];
	unsigned short magic_increment;
	short mob_instance;
	int weapon_damage;
	char unknown2[50];
};

struct char_list_st {
	short position_x[MOB_PER_ACCOUNT];
	short position_y[MOB_PER_ACCOUNT];
	char name[MOB_PER_ACCOUNT][16];
	struct status_st status[MOB_PER_ACCOUNT];
	struct item_st equip[MOB_PER_ACCOUNT][16];
	unsigned short guild_id[MOB_PER_ACCOUNT];
	int gold[MOB_PER_ACCOUNT];
	unsigned int experience[MOB_PER_ACCOUNT];
};

struct party_st {
	short index;
	short leader_id;
	short players[MAX_PARTY];
	short count;
};

struct teleport_st {
	struct position_st from;
	struct position_st to;
	int price;
	int func;
};

struct gener_position_st {
	short position_x;
	short position_y;
	short range;
	short wait;
	char action[96];
};

struct subcelestial_st {
	struct status_st b_status;
	unsigned char face;
	unsigned char class_info;
	unsigned char Class_master;
	int experience;
	short p_status;
	short p_master;
	short p_skill;
	int learn;
	int hold;
	bool sub;
	char skill_bar_1[4];
	char skill_bar_2[16];
	struct affect_st affect[MAX_AFFECT];
};

struct ground_item_st {
	int item_index;
	int owner_id;
	struct item_st item_data;
	clock_t drop_time;
	short rotation;
	short status;
	short position_x;
	short position_y;
};

struct target_st {
	short target_id;
	short damage;
};

struct damage_st {
	short damage;
	char double_critical;
};

struct npcgener_st {
	int mode;
	int minute_generated;
	int max_num_mob;
	int current_num_mob;
	int index;
	int group[100];
	int max_segments;
	struct gener_position_st start;
	struct gener_position_st dest;
	struct gener_position_st segment_list[5];
	time_t death_time[100];
	int formation;
	int route_type;
	struct mob_st mob;
};

struct itemlist_st {
	char name[MAX_SIZENAME];
	short mesh_1;
	short texture;
	short unknown;
	short level;
	short strength;
	short intelligence;
	short dexterity;
	short constitution;
	struct {
		short index;
		short value;
	} effect [MAX_EFFECT];
	int price;
	short unique;
	short pos;
	short extreme;
	short grade;
};

struct skill_data_st {
	int points; 		/* 0 */
	int target; 		/* 4 */
	int mana; 		/* 8 */
	int delay; 		/* 12 */
	int range; 		/* 16 */
	int instance_type; 	/* 20 */
	int instance_value; 	/* 24 */
	int tick_type; 		/* 28 */
	int tick_value; 	/* 32 */
	int affect_type; 	/* 36 */
	int affect_value; 	/* 40 */
	int time; 		/* 44 */
	char act_1[8]; 		/* 52 */
	char act_2[8]; 		/* 60 */
	int instance_attribute; /* 64 */
	int tick_attribute;
	int aggressive;
	int max_target;
	int party_check;
	int affect_resist;
	int passive_check;
	int unknown;
};

struct guildzone_st {
	int owner_index;
	int chall_index;
	int area_guild_x, area_guild_y;
	int city_x, city_y;
	int city_min_x, city_min_y, city_max_x, city_max_y;
	int area_guild_min_x, area_guild_min_y, area_guild_max_x, area_guild_max_y;
	int war_min_x, war_min_y, war_max_x, war_max_y;
	int guilda_war_x, guilda_war_y;
	int guildb_war_x, guildb_war_y;
	int tax;
};

#endif
