/*
===========================================================================
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of Spearmint Source Code.

Spearmint Source Code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Spearmint Source Code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Spearmint Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, Spearmint Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following
the terms and conditions of the GNU General Public License.  If not, please
request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional
terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc.,
Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//

/*****************************************************************************
 * name:		ai_chat.c
 *
 * desc:		Quake3 bot AI
 *
 * $Archive: /MissionPack/code/game/ai_chat.c $
 *
 *****************************************************************************/

#include "g_local.h"
#include "../botlib/botlib.h"
#include "../botlib/be_aas.h"
//
#include "ai_char.h"
#include "ai_chat_sys.h"
#include "ai_ea.h"
#include "ai_gen.h"
#include "ai_goal.h"
#include "ai_move.h"
#include "ai_weap.h"
//
#include "ai_main.h"
#include "ai_dmq3.h"
#include "ai_chat.h"
#include "ai_cmd.h"
#include "ai_vcmd.h"
#include "ai_dmnet.h"
#include "ai_team.h"
//
#include "chars.h"				//characteristics
#include "inv.h"				//indexes into the inventory
#include "syn.h"				//synonyms
#include "match.h"				//string matching types and vars

#define TIME_BETWEENCHATTING	25


/*
==================
BotNumActivePlayers
==================
*/
int BotNumActivePlayers(void) {
	int i, num;
	char buf[MAX_INFO_STRING];

	num = 0;
	for (i = 0; i < level.maxplayers; i++) {
		trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) continue;
		//
		num++;
	}
	return num;
}

/*
==================
BotIsFirstInRankings
==================
*/
int BotIsFirstInRankings(bot_state_t *bs) {
	int i, score;
	char buf[MAX_INFO_STRING];
	playerState_t ps;

	score = bs->cur_ps.persistant[PERS_SCORE];
	for (i = 0; i < level.maxplayers; i++) {
		trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) continue;
		//
		if (BotAI_GetPlayerState(i, &ps) && score < ps.persistant[PERS_SCORE]) {
			return qfalse;
		}
	}
	return qtrue;
}

/*
==================
BotIsLastInRankings
==================
*/
int BotIsLastInRankings(bot_state_t *bs) {
	int i, score;
	char buf[MAX_INFO_STRING];
	playerState_t ps;

	score = bs->cur_ps.persistant[PERS_SCORE];
	for (i = 0; i < level.maxplayers; i++) {
		trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) continue;
		//
		if (BotAI_GetPlayerState(i, &ps) && score > ps.persistant[PERS_SCORE]) {
			return qfalse;
		}
	}
	return qtrue;
}

/*
==================
BotFirstPlayerInRankings
==================
*/
char *BotFirstPlayerInRankings(void) {
	int i, bestscore, bestplayer;
	char buf[MAX_INFO_STRING];
	static char name[32];
	playerState_t ps;

	bestscore = -999999;
	bestplayer = 0;
	for (i = 0; i < level.maxplayers; i++) {
		trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) continue;
		//
		if (BotAI_GetPlayerState(i, &ps) && ps.persistant[PERS_SCORE] > bestscore) {
			bestscore = ps.persistant[PERS_SCORE];
			bestplayer = i;
		}
	}
	EasyPlayerName(bestplayer, name, 32);
	return name;
}

/*
==================
BotLastPlayerInRankings
==================
*/
char *BotLastPlayerInRankings(void) {
	int i, worstscore, bestplayer;
	char buf[MAX_INFO_STRING];
	static char name[32];

	playerState_t ps;

	worstscore = 999999;
	bestplayer = 0;
	for (i = 0; i < level.maxplayers; i++) {
		trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) continue;
		//
		if (BotAI_GetPlayerState(i, &ps) && ps.persistant[PERS_SCORE] < worstscore) {
			worstscore = ps.persistant[PERS_SCORE];
			bestplayer = i;
		}
	}
	EasyPlayerName(bestplayer, name, 32);
	return name;
}

/*
==================
BotRandomOpponentName
==================
*/
char *BotRandomOpponentName(bot_state_t *bs) {
	int i, count;
	char buf[MAX_INFO_STRING];
	int opponents[MAX_CLIENTS], numopponents;
	static char name[32];

	numopponents = 0;
	opponents[0] = 0;
	for (i = 0; i < level.maxplayers; i++) {
		if (i == bs->playernum) continue;
		//
		trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) continue;
		//skip team mates
		if (BotSameTeam(bs, i)) continue;
		//
		opponents[numopponents] = i;
		numopponents++;
	}
	count = random() * numopponents;
	for (i = 0; i < numopponents; i++) {
		count--;
		if (count <= 0) {
			EasyPlayerName(opponents[i], name, sizeof(name));
			return name;
		}
	}
	EasyPlayerName(opponents[0], name, sizeof(name));
	return name;
}

/*
==================
BotMapTitle
==================
*/

char *BotMapTitle(void) {
	char info[1024];
	static char mapname[128];

	trap_GetServerinfo(info, sizeof(info));

	strncpy(mapname, Info_ValueForKey( info, "mapname" ), sizeof(mapname)-1);
	mapname[sizeof(mapname)-1] = '\0';

	return mapname;
}


/*
==================
BotWeaponNameForMeansOfDeath
==================
*/
char *BotWeaponNameForMeansOfDeath(int mod) {
	weapon_t	weapon;
	gitem_t		*item;

	switch(mod) {
		case MOD_SHOTGUN:
			weapon = WP_SHOTGUN;
			break;
		case MOD_GAUNTLET:
			weapon = WP_GAUNTLET;
			break;
		case MOD_MACHINEGUN:
			weapon = WP_MACHINEGUN;
			break;
		case MOD_GRENADE:
		case MOD_GRENADE_SPLASH:
			weapon = WP_GRENADE_LAUNCHER;
			break;
		case MOD_ROCKET:
		case MOD_ROCKET_SPLASH:
			weapon = WP_ROCKET_LAUNCHER;
			break;
		case MOD_PLASMA:
		case MOD_PLASMA_SPLASH:
			weapon = WP_PLASMAGUN;
			break;
		case MOD_RAILGUN:
			weapon = WP_RAILGUN;
			break;
		case MOD_LIGHTNING:
			weapon = WP_LIGHTNING;
			break;
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			weapon = WP_BFG;
			break;
#ifdef MISSIONPACK
		case MOD_NAIL:
			weapon = WP_NAILGUN;
			break;
		case MOD_CHAINGUN:
			weapon = WP_CHAINGUN;
			break;
		case MOD_PROXIMITY_MINE:
			weapon = WP_PROX_LAUNCHER;
			break;
		case MOD_KAMIKAZE:
			return "Kamikaze";
		case MOD_JUICED:
			return "Prox mine";
#endif
		case MOD_GRAPPLE:
			return "Grapple";
		default:
			return "[unknown weapon]";
	}

	item = BG_FindItemForWeapon( weapon );

	if ( item ) {
		return item->pickup_name;
	}

	return "[unknown weapon]";
}

/*
==================
BotRandomWeaponName
==================
*/
char *BotRandomWeaponName(void) {
	int rnd;
	gitem_t	*item;

#if 0
	rnd = 1 + random() * ( (float)WP_NUM_WEAPONS - 1.1f );
#else
	// skip grapple
	rnd = 1 + random() * ( (float)WP_NUM_WEAPONS - 2.1f );

	// only happens if there is a weapon after grapple, like in Team Arena
	if ( rnd >= WP_GRAPPLING_HOOK ) {
		rnd++;
	}
#endif

	item = BG_FindItemForWeapon( rnd );

	if ( item ) {
		return item->pickup_name;
	}

	return "[unknown weapon]";
}

/*
==================
BotVisibleEnemies
==================
*/
int BotVisibleEnemies(bot_state_t *bs) {
	float vis;
	int i;
	aas_entityinfo_t entinfo;

	for (i = 0; i < MAX_CLIENTS; i++) {

		if (i == bs->playernum) continue;
		//
		BotEntityInfo(i, &entinfo);
		//
		if (!entinfo.valid) continue;
		//if the enemy isn't dead and the enemy isn't the bot self
		if (EntityIsDead(&entinfo) || entinfo.number == bs->entitynum) continue;
		//if the enemy is invisible
		if (EntityIsInvisible(&entinfo)) {
			continue;
		}
		//if on the same team
		if (BotSameTeam(bs, i)) continue;
		//check if the enemy is visible
		vis = BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, i);
		if (vis > 0) return qtrue;
	}
	return qfalse;
}

/*
==================
BotValidChatPosition
==================
*/
int BotValidChatPosition(bot_state_t *bs) {
	vec3_t point, start, end, mins, maxs;
	bsp_trace_t trace;

	//if the bot is dead all positions are valid
	if (BotIsDead(bs)) return qtrue;
	//never start chatting with a powerup
	if (bs->inventory[INVENTORY_QUAD] ||
		bs->inventory[INVENTORY_ENVIRONMENTSUIT] ||
		bs->inventory[INVENTORY_HASTE] ||
		bs->inventory[INVENTORY_INVISIBILITY] ||
		bs->inventory[INVENTORY_REGEN] ||
		bs->inventory[INVENTORY_FLIGHT]) return qfalse;
	//must be on the ground
	//if (bs->cur_ps.groundEntityNum != ENTITYNUM_NONE) return qfalse;
	//do not chat if in lava or slime
	VectorCopy(bs->origin, point);
	point[2] -= 24;
	if (trap_PointContents(point,bs->entitynum) & (CONTENTS_LAVA|CONTENTS_SLIME)) return qfalse;
	//do not chat if under water
	VectorCopy(bs->origin, point);
	point[2] += 32;
	if (trap_PointContents(point,bs->entitynum) & MASK_WATER) return qfalse;
	//must be standing on the world entity
	VectorCopy(bs->origin, start);
	VectorCopy(bs->origin, end);
	start[2] += 1;
	end[2] -= 10;
	trap_AAS_PresenceTypeBoundingBox(PRESENCE_CROUCH, mins, maxs);
	BotAI_Trace(&trace, start, mins, maxs, end, bs->playernum, MASK_SOLID);
	if (trace.entityNum != ENTITYNUM_WORLD) return qfalse;
	//the bot is in a position where it can chat
	return qtrue;
}

/*
==================
BotChat_EnterGame
==================
*/
int BotChat_EnterGame(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	//don't chat in teamplay
	if (TeamPlayIsOn()) return qfalse;
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_ENTEREXITGAME, 0, 1);
	if (!bot_fastchat.integer) {
		if (random() > rnd) return qfalse;
	}
	if (BotNumActivePlayers() <= 1) return qfalse;
	if (!BotValidChatPosition(bs)) return qfalse;
	BotAI_BotInitialChat(bs, "game_enter",
				EasyPlayerName(bs->playernum, name, 32),	// 0
				BotRandomOpponentName(bs),				// 1
				"[invalid var]",						// 2
				"[invalid var]",						// 3
				BotMapTitle(),							// 4
				NULL);
	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
==================
BotChat_ExitGame
==================
*/
int BotChat_ExitGame(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	//don't chat in teamplay
	if (TeamPlayIsOn()) return qfalse;
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_ENTEREXITGAME, 0, 1);
	if (!bot_fastchat.integer) {
		if (random() > rnd) return qfalse;
	}
	if (BotNumActivePlayers() <= 1) return qfalse;
	//
	BotAI_BotInitialChat(bs, "game_exit",
				EasyPlayerName(bs->playernum, name, 32),	// 0
				BotRandomOpponentName(bs),				// 1
				"[invalid var]",						// 2
				"[invalid var]",						// 3
				BotMapTitle(),							// 4
				NULL);
	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
==================
BotChat_StartLevel
==================
*/
int BotChat_StartLevel(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) return qfalse;
	if (BotIsObserver(bs)) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	//don't chat in teamplay
	if (TeamPlayIsOn()) {
#ifdef MISSIONPACK
	    EA_Command(bs->playernum, "vtaunt");
#endif
	    return qfalse;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_STARTENDLEVEL, 0, 1);
	if (!bot_fastchat.integer) {
		if (random() > rnd) return qfalse;
	}
	if (BotNumActivePlayers() <= 1) return qfalse;
	BotAI_BotInitialChat(bs, "level_start",
				EasyPlayerName(bs->playernum, name, 32),	// 0
				NULL);
	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
==================
BotChat_EndLevel
==================
*/
int BotChat_EndLevel(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) return qfalse;
	if (BotIsObserver(bs)) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	// teamplay
	if (TeamPlayIsOn()) 
	{
#ifdef MISSIONPACK
		if (BotIsFirstInRankings(bs)) {
			EA_Command(bs->playernum, "vtaunt");
		}
#endif
		return qtrue;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_STARTENDLEVEL, 0, 1);
	if (!bot_fastchat.integer) {
		if (random() > rnd) return qfalse;
	}
	if (BotNumActivePlayers() <= 1) return qfalse;
	//
	if (BotIsFirstInRankings(bs)) {
		BotAI_BotInitialChat(bs, "level_end_victory",
				EasyPlayerName(bs->playernum, name, 32),	// 0
				BotRandomOpponentName(bs),				// 1
				"[invalid var]",						// 2
				BotLastPlayerInRankings(),				// 3
				BotMapTitle(),							// 4
				NULL);
	}
	else if (BotIsLastInRankings(bs)) {
		BotAI_BotInitialChat(bs, "level_end_lose",
				EasyPlayerName(bs->playernum, name, 32),	// 0
				BotRandomOpponentName(bs),				// 1
				BotFirstPlayerInRankings(),				// 2
				"[invalid var]",						// 3
				BotMapTitle(),							// 4
				NULL);
	}
	else {
		BotAI_BotInitialChat(bs, "level_end",
				EasyPlayerName(bs->playernum, name, 32),	// 0
				BotRandomOpponentName(bs),				// 1
				BotFirstPlayerInRankings(),				// 2
				BotLastPlayerInRankings(),				// 3
				BotMapTitle(),							// 4
				NULL);
	}
	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
==================
BotChat_Death
==================
*/
int BotChat_Death(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_DEATH, 0, 1);
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	//if fast chatting is off
	if (!bot_fastchat.integer) {
		if (random() > rnd) return qfalse;
	}
	if (BotNumActivePlayers() <= 1) return qfalse;
	//
	if (bs->lastkilledby >= 0 && bs->lastkilledby < MAX_CLIENTS)
		EasyPlayerName(bs->lastkilledby, name, 32);
	else
		strcpy(name, "[world]");
	//
	if (TeamPlayIsOn() && BotSameTeam(bs, bs->lastkilledby)) {
		if (bs->lastkilledby == bs->playernum) return qfalse;
		BotAI_BotInitialChat(bs, "death_teammate", name, NULL);
		bs->chatto = CHAT_TEAM;
	}
	else
	{
		//teamplay
		if (TeamPlayIsOn()) {
#ifdef MISSIONPACK
			EA_Command(bs->playernum, "vtaunt");
#endif
			return qtrue;
		}
		//
		if (bs->botdeathtype == MOD_WATER)
			BotAI_BotInitialChat(bs, "death_drown", BotRandomOpponentName(bs), NULL);
		else if (bs->botdeathtype == MOD_SLIME)
			BotAI_BotInitialChat(bs, "death_slime", BotRandomOpponentName(bs), NULL);
		else if (bs->botdeathtype == MOD_LAVA)
			BotAI_BotInitialChat(bs, "death_lava", BotRandomOpponentName(bs), NULL);
		else if (bs->botdeathtype == MOD_FALLING)
			BotAI_BotInitialChat(bs, "death_cratered", BotRandomOpponentName(bs), NULL);
		else if (bs->botsuicide || //all other suicides by own weapon
				bs->botdeathtype == MOD_CRUSH ||
				bs->botdeathtype == MOD_SUICIDE ||
				bs->botdeathtype == MOD_TARGET_LASER ||
				bs->botdeathtype == MOD_TRIGGER_HURT ||
				bs->botdeathtype == MOD_UNKNOWN)
			BotAI_BotInitialChat(bs, "death_suicide", BotRandomOpponentName(bs), NULL);
		else if (bs->botdeathtype == MOD_TELEFRAG)
			BotAI_BotInitialChat(bs, "death_telefrag", name, NULL);
#ifdef MISSIONPACK
		else if (bs->botdeathtype == MOD_KAMIKAZE && BotNumInitialChats(bs->cs, "death_kamikaze"))
			BotAI_BotInitialChat(bs, "death_kamikaze", name, NULL);
#endif
		else {
			if ((bs->botdeathtype == MOD_GAUNTLET ||
				bs->botdeathtype == MOD_RAILGUN ||
				bs->botdeathtype == MOD_BFG ||
				bs->botdeathtype == MOD_BFG_SPLASH) && random() < 0.5) {

				if (bs->botdeathtype == MOD_GAUNTLET)
					BotAI_BotInitialChat(bs, "death_gauntlet",
							name,												// 0
							BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
							NULL);
				else if (bs->botdeathtype == MOD_RAILGUN)
					BotAI_BotInitialChat(bs, "death_rail",
							name,												// 0
							BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
							NULL);
				else
					BotAI_BotInitialChat(bs, "death_bfg",
							name,												// 0
							BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
							NULL);
			}
			//choose between insult and praise
			else if (random() < Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_INSULT, 0, 1)) {
				BotAI_BotInitialChat(bs, "death_insult",
							name,												// 0
							BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
							NULL);
			}
			else {
				BotAI_BotInitialChat(bs, "death_praise",
							name,												// 0
							BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
							NULL);
			}
		}
		bs->chatto = CHAT_ALL;
	}
	bs->lastchat_time = FloatTime();
	return qtrue;
}

/*
==================
BotChat_Kill
==================
*/
int BotChat_Kill(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_KILL, 0, 1);
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	//if fast chat is off
	if (!bot_fastchat.integer) {
		if (random() > rnd) return qfalse;
	}
	if (bs->lastkilledplayer == bs->playernum) return qfalse;
	if (BotNumActivePlayers() <= 1) return qfalse;
	if (!BotValidChatPosition(bs)) return qfalse;
	//
	if (BotVisibleEnemies(bs)) return qfalse;
	//
	EasyPlayerName(bs->lastkilledplayer, name, 32);
	//
	bs->chatto = CHAT_ALL;
	if (TeamPlayIsOn() && BotSameTeam(bs, bs->lastkilledplayer)) {
		BotAI_BotInitialChat(bs, "kill_teammate", name, NULL);
		bs->chatto = CHAT_TEAM;
	}
	else
	{
		//don't chat in teamplay
		if (TeamPlayIsOn()) {
#ifdef MISSIONPACK
			EA_Command(bs->playernum, "vtaunt");
#endif
			return qfalse;			// don't wait
		}
		//
		if (bs->enemydeathtype == MOD_GAUNTLET) {
			BotAI_BotInitialChat(bs, "kill_gauntlet", name, NULL);
		}
		else if (bs->enemydeathtype == MOD_RAILGUN) {
			BotAI_BotInitialChat(bs, "kill_rail", name, NULL);
		}
		else if (bs->enemydeathtype == MOD_TELEFRAG) {
			BotAI_BotInitialChat(bs, "kill_telefrag", name, NULL);
		}
#ifdef MISSIONPACK
		else if (bs->botdeathtype == MOD_KAMIKAZE && BotNumInitialChats(bs->cs, "kill_kamikaze"))
			BotAI_BotInitialChat(bs, "kill_kamikaze", name, NULL);
#endif
		//choose between insult and praise
		else if (random() < Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_INSULT, 0, 1)) {
			BotAI_BotInitialChat(bs, "kill_insult", name, NULL);
		}
		else {
			BotAI_BotInitialChat(bs, "kill_praise", name, NULL);
		}
	}
	bs->lastchat_time = FloatTime();
	return qtrue;
}

/*
==================
BotChat_EnemySuicide
==================
*/
int BotChat_EnemySuicide(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	if (BotNumActivePlayers() <= 1) return qfalse;
	//
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_ENEMYSUICIDE, 0, 1);
	//don't chat in teamplay
	if (TeamPlayIsOn()) return qfalse;
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	//if fast chat is off
	if (!bot_fastchat.integer) {
		if (random() > rnd) return qfalse;
	}
	if (!BotValidChatPosition(bs)) return qfalse;
	//
	if (BotVisibleEnemies(bs)) return qfalse;
	//
	if (bs->enemy >= 0) EasyPlayerName(bs->enemy, name, 32);
	else strcpy(name, "");
	BotAI_BotInitialChat(bs, "enemy_suicide", name, NULL);
	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
==================
BotChat_HitTalking
==================
*/
int BotChat_HitTalking(bot_state_t *bs) {
	char name[32], *weap;
	int lasthurt_player;
	float rnd;

	if (bot_nochat.integer) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	if (BotNumActivePlayers() <= 1) return qfalse;
	lasthurt_player = g_entities[bs->playernum].player->lasthurt_player;
	if (!lasthurt_player) return qfalse;
	if (lasthurt_player == bs->playernum) return qfalse;
	//
	if (lasthurt_player < 0 || lasthurt_player >= MAX_CLIENTS) return qfalse;
	//
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_HITTALKING, 0, 1);
	//don't chat in teamplay
	if (TeamPlayIsOn()) return qfalse;
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	//if fast chat is off
	if (!bot_fastchat.integer) {
		if (random() > rnd * 0.5) return qfalse;
	}
	if (!BotValidChatPosition(bs)) return qfalse;
	//
	PlayerName(g_entities[bs->playernum].player->lasthurt_player, name, sizeof(name));
	weap = BotWeaponNameForMeansOfDeath(g_entities[bs->playernum].player->lasthurt_mod);
	//
	BotAI_BotInitialChat(bs, "hit_talking", name, weap, NULL);
	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
==================
BotChat_HitNoDeath
==================
*/
int BotChat_HitNoDeath(bot_state_t *bs) {
	char name[32], *weap;
	float rnd;
	int lasthurt_player;
	aas_entityinfo_t entinfo;

	lasthurt_player = g_entities[bs->playernum].player->lasthurt_player;
	if (!lasthurt_player) return qfalse;
	if (lasthurt_player == bs->playernum) return qfalse;
	//
	if (lasthurt_player < 0 || lasthurt_player >= MAX_CLIENTS) return qfalse;
	//
	if (bot_nochat.integer) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	if (BotNumActivePlayers() <= 1) return qfalse;
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_HITNODEATH, 0, 1);
	//don't chat in teamplay
	if (TeamPlayIsOn()) return qfalse;
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	//if fast chat is off
	if (!bot_fastchat.integer) {
		if (random() > rnd * 0.5) return qfalse;
	}
	if (!BotValidChatPosition(bs)) return qfalse;
	//
	if (BotVisibleEnemies(bs)) return qfalse;
	//
	BotEntityInfo(bs->enemy, &entinfo);
	if (EntityIsShooting(&entinfo)) return qfalse;
	//
	PlayerName(lasthurt_player, name, sizeof(name));
	weap = BotWeaponNameForMeansOfDeath(g_entities[bs->playernum].player->lasthurt_mod);
	//
	BotAI_BotInitialChat(bs, "hit_nodeath", name, weap, NULL);
	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
==================
BotChat_HitNoKill
==================
*/
int BotChat_HitNoKill(bot_state_t *bs) {
	char name[32], *weap;
	float rnd;
	aas_entityinfo_t entinfo;

	if (bot_nochat.integer) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	if (BotNumActivePlayers() <= 1) return qfalse;
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_HITNOKILL, 0, 1);
	//don't chat in teamplay
	if (TeamPlayIsOn()) return qfalse;
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	//if fast chat is off
	if (!bot_fastchat.integer) {
		if (random() > rnd * 0.5) return qfalse;
	}
	if (!BotValidChatPosition(bs)) return qfalse;
	//
	if (BotVisibleEnemies(bs)) return qfalse;
	//
	BotEntityInfo(bs->enemy, &entinfo);
	if (EntityIsShooting(&entinfo)) return qfalse;
	//
	PlayerName(bs->enemy, name, sizeof(name));
	weap = BotWeaponNameForMeansOfDeath(g_entities[bs->enemy].player->lasthurt_mod);
	//
	BotAI_BotInitialChat(bs, "hit_nokill", name, weap, NULL);
	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
==================
BotChat_Random
==================
*/
int BotChat_Random(bot_state_t *bs) {
	float rnd;
	char name[32];

	if (bot_nochat.integer) return qfalse;
	if (BotIsObserver(bs)) return qfalse;
	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) return qfalse;
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) return qfalse;
	//don't chat when doing something important :)
	if (bs->ltgtype == LTG_TEAMHELP ||
		bs->ltgtype == LTG_TEAMACCOMPANY ||
		bs->ltgtype == LTG_RUSHBASE) return qfalse;
	//
	rnd = Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_RANDOM, 0, 1);
	if (random() > bs->thinktime * 0.1) return qfalse;
	if (!bot_fastchat.integer) {
		if (random() > rnd) return qfalse;
		if (random() > 0.25) return qfalse;
	}
	if (BotNumActivePlayers() <= 1) return qfalse;
	//
	if (!BotValidChatPosition(bs)) return qfalse;
	//
	if (BotVisibleEnemies(bs)) return qfalse;
	//
	if (bs->lastkilledplayer == bs->playernum) {
		strcpy(name, BotRandomOpponentName(bs));
	}
	else {
		EasyPlayerName(bs->lastkilledplayer, name, sizeof(name));
	}
	if (TeamPlayIsOn()) {
#ifdef MISSIONPACK
		EA_Command(bs->playernum, "vtaunt");
#endif
		return qfalse;			// don't wait
	}
	//
	if (random() < Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_MISC, 0, 1)) {
		BotAI_BotInitialChat(bs, "random_misc",
					BotRandomOpponentName(bs),	// 0
					name,						// 1
					"[invalid var]",			// 2
					"[invalid var]",			// 3
					BotMapTitle(),				// 4
					BotRandomWeaponName(),		// 5
					NULL);
	}
	else {
		BotAI_BotInitialChat(bs, "random_insult",
					BotRandomOpponentName(bs),	// 0
					name,						// 1
					"[invalid var]",			// 2
					"[invalid var]",			// 3
					BotMapTitle(),				// 4
					BotRandomWeaponName(),		// 5
					NULL);
	}
	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
==================
BotChatTime
==================
*/
float BotChatTime(bot_state_t *bs) {
	//int cpm;

	//cpm = Characteristic_BInteger(bs->character, CHARACTERISTIC_CHAT_CPM, 1, 4000);

	return 2.0;	//(float) BotChatLength(bs->cs) * 30 / cpm;
}

/*
==================
BotChatTest
==================
*/
void BotChatTest(bot_state_t *bs) {

	char name[32];
	char *weap;
	int num, i;

	num = BotNumInitialChats(bs->cs, "game_enter");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "game_enter",
					EasyPlayerName(bs->playernum, name, 32),	// 0
					BotRandomOpponentName(bs),				// 1
					"[invalid var]",						// 2
					"[invalid var]",						// 3
					BotMapTitle(),							// 4
					NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "game_exit");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "game_exit",
					EasyPlayerName(bs->playernum, name, 32),	// 0
					BotRandomOpponentName(bs),				// 1
					"[invalid var]",						// 2
					"[invalid var]",						// 3
					BotMapTitle(),							// 4
					NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "level_start");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "level_start",
					EasyPlayerName(bs->playernum, name, 32),	// 0
					NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "level_end_victory");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "level_end_victory",
				EasyPlayerName(bs->playernum, name, 32),	// 0
				BotRandomOpponentName(bs),				// 1
				BotFirstPlayerInRankings(),				// 2
				BotLastPlayerInRankings(),				// 3
				BotMapTitle(),							// 4
				NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "level_end_lose");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "level_end_lose",
				EasyPlayerName(bs->playernum, name, 32),	// 0
				BotRandomOpponentName(bs),				// 1
				BotFirstPlayerInRankings(),				// 2
				BotLastPlayerInRankings(),				// 3
				BotMapTitle(),							// 4
				NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "level_end");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "level_end",
				EasyPlayerName(bs->playernum, name, 32),	// 0
				BotRandomOpponentName(bs),				// 1
				BotFirstPlayerInRankings(),				// 2
				BotLastPlayerInRankings(),				// 3
				BotMapTitle(),							// 4
				NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	EasyPlayerName(bs->lastkilledby, name, sizeof(name));
	num = BotNumInitialChats(bs->cs, "death_drown");
	for (i = 0; i < num; i++)
	{
		//
		BotAI_BotInitialChat(bs, "death_drown", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_slime");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_slime", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_lava");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_lava", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_cratered");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_cratered", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_suicide");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_suicide", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_telefrag");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_telefrag", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_gauntlet");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_gauntlet",
				name,												// 0
				BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
				NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_rail");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_rail",
				name,												// 0
				BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
				NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_bfg");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_bfg",
				name,												// 0
				BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
				NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_insult");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_insult",
					name,												// 0
					BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
					NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "death_praise");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "death_praise",
					name,												// 0
					BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
					NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	//
	EasyPlayerName(bs->lastkilledplayer, name, 32);
	//
	num = BotNumInitialChats(bs->cs, "kill_gauntlet");
	for (i = 0; i < num; i++)
	{
		//
		BotAI_BotInitialChat(bs, "kill_gauntlet", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "kill_rail");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "kill_rail", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "kill_telefrag");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "kill_telefrag", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "kill_insult");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "kill_insult", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "kill_praise");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "kill_praise", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "enemy_suicide");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "enemy_suicide", name, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	PlayerName(g_entities[bs->playernum].player->lasthurt_player, name, sizeof(name));
	weap = BotWeaponNameForMeansOfDeath(g_entities[bs->playernum].player->lasthurt_player);
	num = BotNumInitialChats(bs->cs, "hit_talking");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "hit_talking", name, weap, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "hit_nodeath");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "hit_nodeath", name, weap, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "hit_nokill");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "hit_nokill", name, weap, NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	//
	if (bs->lastkilledplayer == bs->playernum) {
		strcpy(name, BotRandomOpponentName(bs));
	}
	else {
		EasyPlayerName(bs->lastkilledplayer, name, sizeof(name));
	}
	//
	num = BotNumInitialChats(bs->cs, "random_misc");
	for (i = 0; i < num; i++)
	{
		//
		BotAI_BotInitialChat(bs, "random_misc",
					BotRandomOpponentName(bs),	// 0
					name,						// 1
					"[invalid var]",			// 2
					"[invalid var]",			// 3
					BotMapTitle(),				// 4
					BotRandomWeaponName(),		// 5
					NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
	num = BotNumInitialChats(bs->cs, "random_insult");
	for (i = 0; i < num; i++)
	{
		BotAI_BotInitialChat(bs, "random_insult",
					BotRandomOpponentName(bs),	// 0
					name,						// 1
					"[invalid var]",			// 2
					"[invalid var]",			// 3
					BotMapTitle(),				// 4
					BotRandomWeaponName(),		// 5
					NULL);
		BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
}
