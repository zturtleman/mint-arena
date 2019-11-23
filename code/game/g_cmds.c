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
#include "g_local.h"

// for checking if EC is used by match templates
#include "ai_chat_sys.h"

#ifdef MISSIONPACK
#include "../../ui/menudef.h"			// for the voice chats
#endif

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[1024];
	char		string[1000];
	int			stringlength;
	int			i, j;
	gplayer_t	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;

	// don't send scores to bots (bots don't parse them)
	if ( ent->r.svFlags & SVF_BOT ) {
		return;
	}

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedPlayers;
	
	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.players[level.sortedPlayers[i]];

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		} else {
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		if( cl->accuracy_shots ) {
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedPlayers[i],
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedPlayers[i]].s.powerups, accuracy, 
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT], 
			cl->ps.persistant[PERS_DEFEND_COUNT], 
			cl->ps.persistant[PERS_ASSIST_COUNT], 
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
		j = strlen(entry);
		if (stringlength + j >= sizeof(string))
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i, 
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Cheats are not enabled on this server.\n\"");
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent-g_entities, "print \"You must be alive to use this command.\n\"");
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}


/*
==================
StringIsInteger
==================
*/
qboolean StringIsInteger( const char * s ) {
	int			i;
	int			len;
	qboolean	foundDigit;

	len = strlen( s );
	foundDigit = qfalse;

	for ( i=0 ; i < len ; i++ ) {
		if ( !isdigit( s[i] ) ) {
			return qfalse;
		}

		foundDigit = qtrue;
	}

	return foundDigit;
}


/*
==================
PlayerNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int PlayerNumberFromString( gentity_t *to, char *s, qboolean checkNums, qboolean checkNames ) {
	gplayer_t	*cl;
	int			idnum;
	char		cleanName[MAX_STRING_CHARS];

	if ( checkNums ) {
		// numeric values could be slot numbers
		if ( StringIsInteger( s ) ) {
			idnum = atoi( s );
			if ( idnum >= 0 && idnum < level.maxplayers ) {
				cl = &level.players[idnum];
				if ( cl->pers.connected == CON_CONNECTED ) {
					return idnum;
				}
			}
		}
	}

	if ( checkNames ) {
		// check for a name match
		for ( idnum=0,cl=level.players ; idnum < level.maxplayers ; idnum++,cl++ ) {
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			Q_strncpyz(cleanName, cl->pers.netname, sizeof(cleanName));
			Q_CleanStr(cleanName);
			if ( !Q_stricmp( cleanName, s ) ) {
				return idnum;
			}
		}
	}

	trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));

	return -1;
}

/*
==================
Cmd_Give_f

Give items to a player
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	name = ConcatArgs( 1 );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		ent->health = ent->player->ps.stats[STAT_MAX_HEALTH];
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->player->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 - 
			( 1 << WP_GRAPPLING_HOOK ) - ( 1 << WP_NONE );
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
			ent->player->ps.ammo[i] = 999;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		ent->player->ps.stats[STAT_ARMOR] = 200;

		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "excellent") == 0) {
		ent->player->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "impressive") == 0) {
		ent->player->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->player->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->player->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->player->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}


/*
==================
Cmd_God_f

Sets player to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets player to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->player->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->player->noclip = !ent->player->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f(gentity_t *ent)
{
	char		arg[MAX_TOKEN_CHARS];

	if(!ent->player->pers.localClient)
	{
		trap_SendServerCommand(ent-g_entities,
			"print \"The levelshot command must be executed by a local client\n\"");
		return;
	}

	if(!CheatsOk(ent))
		return;

	// doesn't work in single player
	if(g_gametype.integer == GT_SINGLE_PLAYER)
	{
		trap_SendServerCommand(ent-g_entities,
			"print \"Must not be in singleplayer mode for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	trap_Argv( 1, arg, sizeof( arg ) );
	trap_SendServerCommandEx( ent->player->pers.connectionNum, -1, va( "clientLevelShot %s", arg ) );
}


/*
==================
Cmd_TeamTask_f
==================
*/
void Cmd_TeamTask_f( gentity_t *ent ) {
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int playerNum = ent->player - level.players;

	if ( trap_Argc() != 2 ) {
		return;
	}
	trap_Argv( 1, arg, sizeof( arg ) );
	task = atoi( arg );

	trap_GetUserinfo(playerNum, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(playerNum, userinfo);
	PlayerUserinfoChanged(playerNum);
}


/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->player->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if (ent->health <= 0) {
		return;
	}
	ent->flags &= ~FL_GODMODE;
	ent->player->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

/*
=================
BroadcastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gplayer_t *player, int oldTeam )
{
	if ( player->sess.sessionTeam == oldTeam )
		return;

	if ( player->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " joined the spectators.\n\"",
		player->pers.netname));
	} else if ( player->sess.sessionTeam == TEAM_FREE ) {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " joined the battle.\n\"",
		player->pers.netname));
	} else if ( player->sess.sessionTeam == TEAM_RED ) {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " joined the red team.\n\"",
		player->pers.netname) );
	} else if ( player->sess.sessionTeam == TEAM_BLUE ) {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " joined the blue team.\n\"",
		player->pers.netname));
	} else {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " joined the %s team.\n\"",
		player->pers.netname, TeamName(player->sess.sessionTeam)));
	}
}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, const char *s ) {
	int					team, oldTeam;
	gplayer_t			*player;
	int					playerNum;
	spectatorState_t	specState;
	int					specPlayer;
	int					teamLeader;

	//
	// see what change is requested
	//
	player = ent->player;

	playerNum = player - level.players;
	specPlayer = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if ( !Q_stricmp( s, "follow1" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specPlayer = -1;
	} else if ( !Q_stricmp( s, "follow2" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specPlayer = -2;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			team = PickTeam( playerNum );
		}

		if ( g_teamForceBalance.integer && !player->pers.localClient && !( ent->r.svFlags & SVF_BOT ) ) {
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( playerNum, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( playerNum, TEAM_RED );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
				trap_SendServerCommand( playerNum, 
					"cp \"Red team has too many players.\n\"" );
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				trap_SendServerCommand( playerNum, 
					"cp \"Blue team has too many players.\n\"" );
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if ( (g_gametype.integer == GT_TOURNAMENT)
		&& level.numNonSpectatorPlayers >= 2 ) {
		team = TEAM_SPECTATOR;
	} else if ( g_maxGamePlayers.integer > 0 && 
		level.numNonSpectatorPlayers >= g_maxGamePlayers.integer ) {
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = player->sess.sessionTeam;
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

	// if the player was dead leave the body, but only if they're actually in game
	if ( player->ps.stats[STAT_HEALTH] <= 0 && player->pers.connected == CON_CONNECTED ) {
		CopyToBodyQue(ent);
	}

	// he starts at 'base'
	player->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->player->ps.stats[STAT_HEALTH] = ent->health = 0;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE_TEAM_CHANGE);

	}

	// they go to the end of the line for tournements
	if(team == TEAM_SPECTATOR && oldTeam != team)
		AddTournamentQueue(player);

	player->sess.sessionTeam = team;
	player->sess.spectatorState = specState;
	player->sess.spectatorPlayer = specPlayer;

	player->sess.teamLeader = qfalse;
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this player is not a bot
		if ( teamLeader == -1 || ( !(g_entities[playerNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			SetLeader( team, playerNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
		CheckTeamLeader( oldTeam );
	}

	// get and distribute relevant parameters
	PlayerUserinfoChanged( playerNum );

	// player hasn't spawned yet, they sent teampref or g_teamAutoJoin is enabled
	if ( player->pers.connected != CON_CONNECTED ) {
		return;
	}

	BroadcastTeamChange( player, oldTeam );

	PlayerBegin( playerNum );
}

/*
=================
StopFollowing

If the player being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	ent->player->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;	
	ent->player->sess.sessionTeam = TEAM_SPECTATOR;	
	ent->player->sess.spectatorState = SPECTATOR_FREE;
	ent->player->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->player->ps.playerNum = ent - g_entities;

	SetPlayerViewAngle( ent, ent->player->ps.viewangles );

	// don't use dead view angles
	if ( ent->player->ps.stats[STAT_HEALTH] <= 0 ) {
		ent->player->ps.stats[STAT_HEALTH] = 1;
	}
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	oldTeam = ent->player->sess.sessionTeam;

	if ( trap_Argc() != 2 ) {
		trap_SendServerCommand( ent-g_entities, va( "print \"You are on the %s team.\n\"", TeamName( oldTeam ) ) );
		return;
	}

	if ( ent->player->switchTeamTime > level.time ) {
		trap_SendServerCommand( ent-g_entities, "print \"May not switch teams more than once per 5 seconds.\n\"" );
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->player->sess.sessionTeam == TEAM_FREE ) {
		ent->player->sess.losses++;
	}

	trap_Argv( 1, s, sizeof( s ) );

	SetTeam( ent, s );

	if ( oldTeam != ent->player->sess.sessionTeam ) {
		ent->player->switchTeamTime = level.time + 5000;
	}
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		if ( ent->player->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = PlayerNumberFromString( ent, arg, qtrue, qtrue );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.players[ i ] == ent->player ) {
		return;
	}

	// can't follow another spectator
	if ( level.players[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	// don't follow one of their local players
	if ( level.players[ i ].pers.connectionNum == ent->player->pers.connectionNum ) {
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->player->sess.sessionTeam == TEAM_FREE ) {
		ent->player->sess.losses++;
	}

	// first set them to spectator
	if ( ent->player->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
	}

	ent->player->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->player->sess.spectatorPlayer = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		playerNum;
	int		original;

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->player->sess.sessionTeam == TEAM_FREE ) {
		ent->player->sess.losses++;
	}
	// first set them to spectator
	if ( ent->player->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	// if dedicated follow player, just switch between the two auto players
	if (ent->player->sess.spectatorPlayer < 0) {
		if (ent->player->sess.spectatorPlayer == -1) {
			ent->player->sess.spectatorPlayer = -2;
		} else if (ent->player->sess.spectatorPlayer == -2) {
			ent->player->sess.spectatorPlayer = -1;
		}
		return;
	}

	playerNum = ent->player->sess.spectatorPlayer;
	original = playerNum;
	do {
		playerNum += dir;
		if ( playerNum >= level.maxplayers ) {
			playerNum = 0;
		}
		if ( playerNum < 0 ) {
			playerNum = level.maxplayers - 1;
		}

		// can only follow connected players
		if ( level.players[ playerNum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.players[ playerNum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// don't follow one of their local players
		if ( level.players[ playerNum ].pers.connectionNum == ent->player->pers.connectionNum ) {
			continue;
		}

		// this is good, we can use it
		ent->player->sess.spectatorPlayer = playerNum;
		ent->player->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( playerNum != original );

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static qboolean G_SayTo( gentity_t *ent, gentity_t *other, int mode ) {
	if (!other) {
		return qfalse;
	}
	if (!other->inuse) {
		return qfalse;
	}
	if (!other->player) {
		return qfalse;
	}
	if ( other->player->pers.connected != CON_CONNECTED ) {
		return qfalse;
	}
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return qfalse;
	}
	// no chatting to players in tournements
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& other->player->sess.sessionTeam == TEAM_FREE
		&& ent && ent->player && ent->player->sess.sessionTeam != TEAM_FREE ) {
		return qfalse;
	}

	return qtrue;
}

// escape character for botfiles/match.c parsing
#define EC		"\x19"

static void G_RemoveChatEscapeChar( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (text[i] == '\x19')
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

// ent is NULL for messages from dedicated server
void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	static int	useChatEscapeCharacter = -1;
	int			i, j;
	gentity_t	*other;
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];
	char		*cmd, *str, *netname;
	int			playerNum;

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	if ( target && !G_SayTo( ent, target, mode ) ) {
		return;
	}

	if ( ent && ent->player ) {
		netname = ent->player->pers.netname;
		playerNum = ent->s.number;
	} else {
		netname = "server";
		playerNum = CHATPLAYER_SERVER;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", netname, text );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		cmd = "chat";
		break;
	case SAY_TEAM:
		G_LogPrintf( "sayteam: %s: %s\n", netname, text );
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC") (%s)"EC": ", 
				netname, Q_COLOR_ESCAPE, COLOR_WHITE, location);
		else
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
				netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_CYAN;
		cmd = "tchat";
		break;
	case SAY_TELL:
		if ( target && target->player ) {
			G_LogPrintf( "tell: %s to %s: %s\n", netname, target->player->pers.netname, text );
		}
		if (OnSameTeam(ent, target) && Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"] (%s)"EC": ", netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
		else
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_MAGENTA;
		cmd = "tell";
		break;
	}

	// if botfiles/match.c doesn't have EC (unpatched/demo Q3), it needs to be
	// removed for bots to be able to recognize the message.
	if ( useChatEscapeCharacter == -1 ) {
		useChatEscapeCharacter = BotMatchTemplatesContainsString( EC );
	}
	if ( !useChatEscapeCharacter ) {
		G_RemoveChatEscapeChar( name );
	}

	str = va( "%s \"%s%c%c%s\" %d", cmd, name, Q_COLOR_ESCAPE, color, text, playerNum );

	if ( target ) {
		trap_SendServerCommand( target-g_entities, str );

		// don't tell to the player self if it was already directed to this player
		// also don't send the chat back to a bot
		if ( ent && ent != target && !(ent->r.svFlags & SVF_BOT)) {
			trap_SendServerCommand( ent-g_entities, str );
		}
		return;
	}

	// send to everyone on team
	if ( mode == SAY_TEAM && ent && ent->player ) {
		G_TeamCommand( ent->player->sess.sessionTeam, str );
		return;
	}

	// send it to all the appropriate clients
	for (i = 0; i < level.maxconnections; i++) {
		for (  j = 0; j < MAX_SPLITVIEW; j++ ) {
			if ( level.connections[i].localPlayerNums[j] == -1 )
				continue;

			other = &g_entities[level.connections[i].localPlayerNums[j]];

			if ( !G_SayTo( ent, other, mode ) ) {
				break;
			}
		}

		if ( j == MAX_SPLITVIEW ) {
			trap_SendServerCommandEx( i, -1, str );
		}
	}
}

static void SanitizeChatText( char *text ) {
	int i;

	for ( i = 0; text[i]; i++ ) {
		if ( text[i] == '\n' || text[i] == '\r' ) {
			text[i] = ' ';
		}
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	SanitizeChatText( p );

	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 3 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: tell <player id> <message>\n\"" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = PlayerNumberFromString( ent, arg, qtrue, qtrue );
	if ( targetNum == -1 ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->player ) {
		return;
	}

	p = ConcatArgs( 2 );

	SanitizeChatText( p );

	G_Say( ent, target, SAY_TELL, p );
}


#ifdef MISSIONPACK
static qboolean G_VoiceTo( gentity_t *ent, gentity_t *other, int mode ) {
	if (!other) {
		return qfalse;
	}
	if (!other->inuse) {
		return qfalse;
	}
	if (!other->player) {
		return qfalse;
	}
	if ( mode == SAY_TEAM && !OnSameTeam(ent, other) ) {
		return qfalse;
	}
	// no chatting to players in tournements
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		return qfalse;
	}

	return qtrue;
}

void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly ) {
	int			i, j;
	gentity_t	*other;
	int			color;
	char		*cmd;
	char		*str;

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	if ( target && !G_VoiceTo( ent, target, mode ) ) {
		return;
	}

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "vchat: %s: %s\n", ent->player->pers.netname, id );
		color = COLOR_GREEN;
		cmd = "vchat";
		break;
	case SAY_TEAM:
		G_LogPrintf( "vtchat: %s: %s\n", ent->player->pers.netname, id );
		color = COLOR_CYAN;
		cmd = "vtchat";
		break;
	case SAY_TELL:
		if ( target && target->player ) {
			G_LogPrintf( "vtell: %s to %s: %s\n", ent->player->pers.netname, target->player->pers.netname, id );
		}
		color = COLOR_MAGENTA;
		cmd = "vtell";
		break;
	}

	str = va( "%s %d %d %d %s", cmd, voiceonly, ent->s.number, color, id );

	if ( target ) {
		trap_SendServerCommand( target-g_entities, str );

		// don't tell to the player self if it was already directed to this player
		// also don't send the chat back to a bot
		if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
			trap_SendServerCommand( ent-g_entities, str );
		}
		return;
	}

	// send to everyone on team
	if ( mode == SAY_TEAM ) {
		G_TeamCommand( ent->player->sess.sessionTeam, str );
		return;
	}

	// send it to all the appropriate clients
	for (i = 0; i < level.maxconnections; i++) {
		for (  j = 0; j < MAX_SPLITVIEW; j++ ) {
			if ( level.connections[i].localPlayerNums[j] == -1 )
				continue;

			other = &g_entities[level.connections[i].localPlayerNums[j]];

			if ( !G_VoiceTo( ent, other, mode ) ) {
				break;
			}
		}

		if ( j == MAX_SPLITVIEW ) {
			trap_SendServerCommandEx( i, -1, str );
		}
	}
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	SanitizeChatText( p );

	G_Voice( ent, NULL, mode, p, voiceonly );
}

/*
==================
Cmd_VoiceTell_f
==================
*/
static void Cmd_VoiceTell_f( gentity_t *ent, qboolean voiceonly ) {
	int			targetNum;
	gentity_t	*target;
	char		*id;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 3 ) {
		trap_SendServerCommand( ent-g_entities, va( "print \"Usage: %s <player id> <voice id>\n\"", voiceonly ? "votell" : "vtell" ) );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = PlayerNumberFromString( ent, arg, qtrue, qtrue );
	if ( targetNum == -1 ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->player ) {
		return;
	}

	id = ConcatArgs( 2 );

	SanitizeChatText( id );

	G_Voice( ent, target, SAY_TELL, id, voiceonly );
}


/*
==================
Cmd_VoiceTaunt_f
==================
*/
static void Cmd_VoiceTaunt_f( gentity_t *ent ) {
	gentity_t *who;
	int i;

	if (!ent->player) {
		return;
	}

	// insult someone who just killed you
	if (ent->enemy && ent->enemy->player && ent->enemy->player->lastkilled_player == ent->s.number) {
		// i am a dead corpse
		G_Voice( ent, ent->enemy, SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		ent->enemy = NULL;
		return;
	}
	// insult someone you just killed
	if (ent->player->lastkilled_player >= 0 && ent->player->lastkilled_player != ent->s.number) {
		who = g_entities + ent->player->lastkilled_player;
		if (who->player) {
			// who is the person I just killed
			if (who->player->lasthurt_mod == MOD_GAUNTLET) {
				G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );	// and I killed them with a gauntlet
			} else {
				G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );	// and I killed them with something else
			}
			ent->player->lastkilled_player = -1;
			return;
		}
	}

	if (g_gametype.integer >= GT_TEAM) {
		// praise a team mate who just got a reward
		for(i = 0; i < MAX_CLIENTS; i++) {
			who = g_entities + i;
			if (who->player && who != ent && who->player->sess.sessionTeam == ent->player->sess.sessionTeam) {
				if (who->player->rewardTime > level.time) {
					G_Voice( ent, who, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					return;
				}
			}
		}
	}

	// just say something
	G_Voice( ent, NULL, SAY_ALL, VOICECHAT_TAUNT, qfalse );
}
#endif



static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

static const int numgc_orders = ARRAY_LEN( gc_orders );

void Cmd_GameCommand_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	int			order;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 3 ) {
		trap_SendServerCommand( ent-g_entities, va( "print \"Usage: gc <player id> <order 0-%d>\n\"", numgc_orders - 1 ) );
		return;
	}

	trap_Argv( 2, arg, sizeof( arg ) );
	order = atoi( arg );

	if ( order < 0 || order >= numgc_orders ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Bad order: %i\n\"", order));
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = PlayerNumberFromString( ent, arg, qtrue, qtrue );
	if ( targetNum == -1 ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->player ) {
		return;
	}

	G_Say( ent, target, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos(ent->r.currentOrigin) ) );
}

/*
==================
Cmd_CallVote_f
==================
*/
#define CALLVOTE_ARG2_NONE 1
#define CALLVOTE_ARG2_INTREGAL 2
void Cmd_CallVote_f( gentity_t *ent ) {
	char*	c;
	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	int		arg2Flags, arg2RangeMin, arg2RangeMax;

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
		return;
	}

	if ( level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress.\n\"" );
		return;
	}
	if ( ent->player->pers.voteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of votes.\n\"" );
		return;
	}
	if ( ent->player->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	// check for command separators in arg2
	for( c = arg2; *c; ++c) {
		switch(*c) {
			case '\n':
			case '\r':
			case ';':
				trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
				return;
			break;
		}
	}

	arg2Flags = 0;
	arg2RangeMin = 0;
	arg2RangeMax = INT_MAX;

	if ( !Q_stricmp( arg1, "map_restart" ) ) {
		arg2Flags = CALLVOTE_ARG2_INTREGAL;
		arg2RangeMax = 60; // max 1 minute

		// default to 5 seconds if no argument was specified
		if ( !arg2[0] ) {
			Q_strncpyz( arg2, "5", sizeof( arg2 ) );
		}
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
		arg2Flags = CALLVOTE_ARG2_NONE;
	} else if ( !Q_stricmp( arg1, "map" ) ) {
		// string
	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
		arg2Flags = CALLVOTE_ARG2_INTREGAL;
		arg2RangeMax = GT_MAX_GAME_TYPE - 1;
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
		// string
	} else if ( !Q_stricmp( arg1, "kicknum" ) ) {
		arg2Flags = CALLVOTE_ARG2_INTREGAL;
		arg2RangeMax = MAX_CLIENTS;
	} else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
		arg2Flags = CALLVOTE_ARG2_INTREGAL;
		arg2RangeMax = 1;
	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
		arg2Flags = CALLVOTE_ARG2_INTREGAL;
		arg2RangeMax = 240; // 4 hours
	} else if ( !Q_stricmp( arg1, "fraglimit" ) ) {
		arg2Flags = CALLVOTE_ARG2_INTREGAL;
		arg2RangeMax = 999;
	} else if ( !Q_stricmp( arg1, "capturelimit" ) ) {
		arg2Flags = CALLVOTE_ARG2_INTREGAL;
		arg2RangeMax = 999;
	} else if ( !Q_stricmp( arg1, "g_instagib" ) ) {
		arg2Flags = CALLVOTE_ARG2_INTREGAL;
		arg2RangeMax = 1;
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, kicknum <playernum>, g_doWarmup <boolean>, timelimit <time>, fraglimit <frags>, capturelimit <captures>, g_instagib <boolean>.\n\"" );
		return;
	}

	if ( arg2Flags & CALLVOTE_ARG2_NONE ) {
		if ( arg2[0] != '\0' ) {
			trap_SendServerCommand( ent-g_entities, va( "print \"Vote command %s does not accept an argument.\n\"", arg1 ) );
			return;
		}
	}
	else if ( arg2[0] == '\0' ) {
		trap_SendServerCommand( ent-g_entities, va( "print \"Vote command %s requires an argument.\n\"", arg1 ) );
		return;
	}

	if ( arg2Flags & CALLVOTE_ARG2_INTREGAL ) {
		if ( !StringIsInteger( arg2 ) ) {
			trap_SendServerCommand( ent-g_entities, va( "print \"Vote command %s argument must be a number.\n\"", arg1 ) );
			return;
		}

		i = atoi( arg2 );
		if ( i < arg2RangeMin || i > arg2RangeMax ) {
			trap_SendServerCommand( ent-g_entities, va( "print \"Vote command %s argument must be %d to %d.\n\"", arg1, arg2RangeMin, arg2RangeMax ) );
			return;
		}
	}

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		// don't start a vote when map change or restart is in progress
		if ( !Q_stricmpn( level.voteString, "map", 3 )
			|| !Q_stricmpn( level.voteString, "nextmap", 7 ) ) {
			trap_SendServerCommand( ent-g_entities, "print \"Vote after map change.\n\"" );
			return;
		}

		level.voteExecuteTime = 0;
		trap_Cmd_ExecuteText( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) ) {
		i = atoi( arg2 );
		if( i == GT_SINGLE_PLAYER || i < 0 || i >= GT_MAX_GAME_TYPE) {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, bg_displayGametypeNames[i] );
	} else if ( !Q_stricmp( arg1, "map" ) ) {
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];
		char	filename[MAX_QPATH];

		if ( strstr( arg2, ".." ) || strstr( arg2, "::" ) || strlen( arg2 ) + 9 >= MAX_QPATH ) {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid map name.\n\"" );
			return;
		}
		Com_sprintf( filename, sizeof( filename ), "maps/%s.bsp", arg2 );
		if ( trap_FS_FOpenFile( filename, NULL, FS_READ ) <= 0 ) {
			trap_SendServerCommand( ent-g_entities, "print \"Map not found.\n\"" );
			return;
		}

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (*s) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	} else if ( !Q_stricmp( arg1, "kicknum" ) || !Q_stricmp( arg1, "kick" ) ) {
		i = PlayerNumberFromString( ent, arg2, !Q_stricmp( arg1, "kicknum" ), !Q_stricmp( arg1, "kick" ) );
		if ( i == -1 ) {
			return;
		}

		if ( level.players[i].pers.localClient ) {
			trap_SendServerCommand( ent-g_entities, "print \"Cannot kick host client.\n\"" );
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "kicknum %d", i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "kick %s" , level.players[i].pers.netname );
	} else {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	G_LogPrintf( "callvote: %s: %s\n", ent->player->pers.netname, level.voteDisplayString );

	trap_SendServerCommand( -1, va("print \"%s called a vote.\n\"", ent->player->pers.netname ) );

	// start the voting, the caller automatically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxplayers ; i++ ) {
		level.players[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->player->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	if ( !level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
		return;
	}
	if ( ent->player->ps.eFlags & EF_VOTED ) {
		trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
		return;
	}
	if ( ent->player->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );

	ent->player->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( tolower( msg[0] ) == 'y' || msg[0] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
		G_LogPrintf( "vote: %s: yes\n", ent->player->pers.netname );
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
		G_LogPrintf( "vote: %s: no\n", ent->player->pers.netname );
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	char*	c;
	int		i, team, cs_offset;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	team = ent->player->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
		return;
	}

	if ( level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, "print \"A team vote is already in progress.\n\"" );
		return;
	}
	if ( ent->player->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of team votes.\n\"" );
		return;
	}
	if ( ent->player->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	arg2[0] = '\0';
	for ( i = 2; i < trap_Argc(); i++ ) {
		if (i > 2)
			strcat(arg2, " ");
		trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
	}

	// check for command separators in arg2
	for( c = arg2; *c; ++c) {
		switch(*c) {
			case '\n':
			case '\r':
			case ';':
				trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
				return;
			break;
		}
	}

	if ( !Q_stricmp( arg1, "leader" ) ) {
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if ( !arg2[0] ) {
			i = ent->player->ps.playerNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if ( i >= 3 || !arg2[i]) {
				i = atoi( arg2 );
				if ( i < 0 || i >= level.maxplayers ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Bad player slot: %i\n\"", i) );
					return;
				}

				if ( !g_entities[i].inuse ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Player %i is not active\n\"", i) );
					return;
				}
			}
			else {
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for ( i = 0 ; i < level.maxplayers ; i++ ) {
					if ( level.players[i].pers.connected == CON_DISCONNECTED )
						continue;
					if (level.players[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.players[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if ( !Q_stricmp(netname, leader) ) {
						break;
					}
				}
				if ( i >= level.maxplayers ) {
					trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

	G_LogPrintf( "callteamvote: %s on %s team: %s\n", ent->player->pers.netname, TeamName( team ), level.teamVoteString[cs_offset] );

	for ( i = 0 ; i < level.maxplayers ; i++ ) {
		if ( level.players[i].pers.connected == CON_DISCONNECTED )
			continue;
		if (level.players[i].sess.sessionTeam == team)
			trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->player->pers.netname ) );
	}

	// start the voting, the caller automatically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for ( i = 0 ; i < level.maxplayers ; i++ ) {
		if (level.players[i].sess.sessionTeam == team)
			level.players[i].ps.eFlags &= ~EF_TEAMVOTED;
	}
	ent->player->ps.eFlags |= EF_TEAMVOTED;

	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
	trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	int			team, cs_offset;
	char		msg[64];

	team = ent->player->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, "print \"No team vote in progress.\n\"" );
		return;
	}
	if ( ent->player->ps.eFlags & EF_TEAMVOTED ) {
		trap_SendServerCommand( ent-g_entities, "print \"Team vote already cast.\n\"" );
		return;
	}
	if ( ent->player->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"Team vote cast.\n\"" );

	ent->player->ps.eFlags |= EF_TEAMVOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( tolower( msg[0] ) == 'y' || msg[0] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
		G_LogPrintf( "teamvote: %s on %s team: yes\n", ent->player->pers.netname, TeamName( team ) );
	} else {
		level.teamVoteNo[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );	
		G_LogPrintf( "teamvote: %s on %s team: no\n", ent->player->pers.netname, TeamName( team ) );
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Cheats are not enabled on this server.\n\"");
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent-g_entities, "print \"usage: setviewpos x y z yaw\n\"");
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}



/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent ) {
/*
	int max, n, i;

	max = trap_AAS_PointReachabilityAreaIndex( NULL );

	n = 0;
	for ( i = 0; i < max; i++ ) {
		if ( ent->player->areabits[i >> 3] & (1 << (i & 7)) )
			n++;
	}

	//trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
*/
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int connectionNum ) {
	gentity_t *ent;
	gconnection_t *connection;
	int		localPlayerNum;
	int		playerNum;
	char	*cmd;
	char	buf[MAX_TOKEN_CHARS];

	connection = &level.connections[connectionNum];

	trap_Argv( 0, buf, sizeof( buf ) );

	cmd = &buf[0];

	// Commands for extra local players.
	// 2team, 2give, 2teamtask, ...
	if (cmd[0] >= '2' && cmd[0] <= '0'+MAX_SPLITVIEW) {
		localPlayerNum = cmd[0]-'1';

		cmd++;

		if ( connection->localPlayerNums[localPlayerNum] == -1 ) {
			trap_SendServerCommandEx( connectionNum, -1, va("print \"unknown cmd %s\n\"", buf ) );
			return;
		}

		playerNum = connection->localPlayerNums[localPlayerNum];
	} else {
		for ( localPlayerNum = 0; localPlayerNum < MAX_SPLITVIEW; localPlayerNum++ ) {
			if ( connection->localPlayerNums[localPlayerNum] != -1 ) {
				playerNum = connection->localPlayerNums[localPlayerNum];
				break;
			}
		}
		if ( localPlayerNum == MAX_SPLITVIEW ) {
			//G_Printf("No Local player connected from connection %d!\n", connectionNum);
			return;
		}
	}

	ent = g_entities + playerNum;
	if (!ent->player || ent->player->pers.connected != CON_CONNECTED) {
		return;		// not fully in game yet
	}

	if (Q_stricmp (cmd, "say") == 0) {
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0) {
		Cmd_Say_f (ent, SAY_TEAM, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "tell") == 0) {
		Cmd_Tell_f ( ent );
		return;
	}
#ifdef MISSIONPACK
	if (Q_stricmp (cmd, "vsay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vsay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vtell") == 0) {
		Cmd_VoiceTell_f ( ent, qfalse );
		return;
	}
	if (Q_stricmp (cmd, "vosay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "vosay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "votell") == 0) {
		Cmd_VoiceTell_f ( ent, qtrue );
		return;
	}
	if (Q_stricmp (cmd, "vtaunt") == 0) {
		Cmd_VoiceTaunt_f ( ent );
		return;
	}
#endif
	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime) {
		trap_SendServerCommand( playerNum, va("print \"%s command not allowed at intermission.\n\"", buf ) );
		return;
	}

	if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "teamtask") == 0)
		Cmd_TeamTask_f (ent);
	else if (Q_stricmp (cmd, "levelshot") == 0)
		Cmd_LevelShot_f (ent);
	else if (Q_stricmp (cmd, "follow") == 0)
		Cmd_Follow_f (ent);
	else if (Q_stricmp (cmd, "follownext") == 0)
		Cmd_FollowCycle_f (ent, 1);
	else if (Q_stricmp (cmd, "followprev") == 0)
		Cmd_FollowCycle_f (ent, -1);
	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "callvote") == 0)
		Cmd_CallVote_f (ent);
	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);
	else if (Q_stricmp (cmd, "callteamvote") == 0)
		Cmd_CallTeamVote_f (ent);
	else if (Q_stricmp (cmd, "teamvote") == 0)
		Cmd_TeamVote_f (ent);
	else if (Q_stricmp (cmd, "gc") == 0)
		Cmd_GameCommand_f( ent );
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
	else if (Q_stricmp (cmd, "stats") == 0)
		Cmd_Stats_f( ent );
	else
		trap_SendServerCommand( playerNum, va("print \"unknown cmd %s\n\"", buf ) );
}
