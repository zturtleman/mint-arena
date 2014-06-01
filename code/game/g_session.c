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


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WritePlayerSessionData

Called on game shutdown
================
*/
void G_WritePlayerSessionData( gplayer_t *player ) {
	const char	*s;
	const char	*var;

	s = va("%i %i %i %i %i %i %i", 
		player->sess.sessionTeam,
		player->sess.spectatorNum,
		player->sess.spectatorState,
		player->sess.spectatorPlayer,
		player->sess.wins,
		player->sess.losses,
		player->sess.teamLeader
		);

	var = va( "session%i", (int)(player - level.players) );

	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gplayer_t *player ) {
	char	s[MAX_STRING_CHARS];
	const char	*var;
	int teamLeader;
	int spectatorState;
	int sessionTeam;

	var = va( "session%i", (int)(player - level.players) );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i",
		&sessionTeam,
		&player->sess.spectatorNum,
		&spectatorState,
		&player->sess.spectatorPlayer,
		&player->sess.wins,
		&player->sess.losses,
		&teamLeader
		);

	player->sess.sessionTeam = (team_t)sessionTeam;
	player->sess.spectatorState = (spectatorState_t)spectatorState;
	player->sess.teamLeader = (qboolean)teamLeader;
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gplayer_t *player, char *userinfo ) {
	playerSession_t	*sess;
	const char		*value;

	sess = &player->sess;

	// initial team determination
	if ( g_gametype.integer >= GT_TEAM ) {
		// always spawn as spectator in team games
		sess->sessionTeam = TEAM_SPECTATOR;
		sess->spectatorState = SPECTATOR_FREE;

		// allow specifying team, mainly for bots (and humans via start server menu)
		value = Info_ValueForKey( userinfo, "teampref" );

		if ( value[0] || g_teamAutoJoin.integer ) {
			SetTeam( &g_entities[player - level.players], value );
		}
	} else {
		value = Info_ValueForKey( userinfo, "teampref" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if ( g_maxGamePlayers.integer > 0 && 
					level.numNonSpectatorPlayers >= g_maxGamePlayers.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorPlayers >= 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			}
		}

		sess->spectatorState = SPECTATOR_FREE;
	}

	AddTournamentQueue(player);

	G_WritePlayerSessionData( player );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int			gt;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// player sessions
	if ( g_gametype.integer != gt ) {
		level.newSession = qtrue;
		G_Printf( "Gametype changed, clearing session data.\n" );
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	trap_Cvar_SetValue( "session", g_gametype.integer );

	for ( i = 0 ; i < level.maxplayers ; i++ ) {
		if ( level.players[i].pers.connected == CON_CONNECTED ) {
			G_WritePlayerSessionData( &level.players[i] );
		}
	}
}
