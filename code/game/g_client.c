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

// g_client.c -- client functions that don't happen every frame

static vec3_t	playerMins = {-15, -15, -24};
static vec3_t	playerMaxs = {15, 15, 32};

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		//if ( hit->player && hit->player->ps.stats[STAT_HEALTH] > 0 ) {
		if ( hit->player) {
			return qtrue;
		}

	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint(qboolean isbot) {
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL && count < MAX_SPAWN_POINTS)
	{
		if(SpotWouldTelefrag(spot))
			continue;

		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			// spot is not for this human/bot player
			continue;
		}

		spots[count] = spot;
		count++;
	}

	if ( !count ) {	// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[MAX_SPAWN_POINTS];
	gentity_t	*list_spot[MAX_SPAWN_POINTS];
	int			numSpots, rnd, i, j;

	numSpots = 0;
	spot = NULL;

	while((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if(SpotWouldTelefrag(spot))
			continue;

		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			// spot is not for this human/bot player
			continue;
		}

		VectorSubtract( spot->s.origin, avoidPoint, delta );
		dist = VectorLength( delta );

		for (i = 0; i < numSpots; i++)
		{
			if(dist > list_dist[i])
			{
				if (numSpots >= MAX_SPAWN_POINTS)
					numSpots = MAX_SPAWN_POINTS - 1;
					
				for(j = numSpots; j > i; j--)
				{
					list_dist[j] = list_dist[j-1];
					list_spot[j] = list_spot[j-1];
				}
				
				list_dist[i] = dist;
				list_spot[i] = spot;
				
				numSpots++;
				break;
			}
		}
		
		if(i >= numSpots && numSpots < MAX_SPAWN_POINTS)
		{
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}
	
	if(!numSpots)
	{
		spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");

		if (!spot)
			G_Error( "Couldn't find a spawn point" );

		VectorCopy (spot->s.origin, origin);
		origin[2] += 9;
		VectorCopy (spot->s.angles, angles);
		return spot;
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * (numSpots / 2);

	VectorCopy (list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot ) {
	return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles, isbot );

	/*
	gentity_t	*spot;
	gentity_t	*nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint ( );
	if ( spot == nearestSpot ) {
		// roll again if it would be real close to point of death
		spot = SelectRandomDeathmatchSpawnPoint ( );
		if ( spot == nearestSpot ) {
			// last try
			spot = SelectRandomDeathmatchSpawnPoint ( );
		}		
	}

	// find a single player start spot
	if (!spot) {
		G_Error( "Couldn't find a spawn point" );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
	*/
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles, qboolean isbot ) {
	gentity_t	*spot;

	spot = NULL;
	
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			continue;
		}
		
		if((spot->spawnflags & 0x01))
			break;
	}

	if (!spot || SpotWouldTelefrag(spot))
		return SelectSpawnPoint(vec3_origin, origin, angles, isbot);

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodyQueFree

The body ques are never actually freed, they are just unlinked
=============
*/
void BodyQueFree( gentity_t *ent ) {
	trap_UnlinkEntity( ent );
	ent->physicsObject = qfalse;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {
#ifdef MISSIONPACK
	gentity_t	*e;
	int i;
#endif
	gentity_t		*body;
	int			contents;

	trap_UnlinkEntity (ent);

	// if player is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->s.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

	body->s = ent->s;
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc
#ifdef MISSIONPACK
	if ( ent->s.eFlags & EF_KAMIKAZE ) {
		body->s.eFlags |= EF_KAMIKAZE;

		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < level.num_entities; i++) {
			e = &g_entities[i];
			if (!e->inuse)
				continue;
			if (e->activator != ent)
				continue;
			if (strcmp(e->classname, "kamikaze timer"))
				continue;
			e->activator = body;
			break;
		}
	}
#endif
	body->s.powerups = 0;	// clear powerups
	body->s.loopSound = 0;	// clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		VectorCopy( ent->player->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.pos.trTime = level.time;
	body->s.event = 0;

	// change the animation to the last-frame only, so the sequence
	// doesn't repeat anew for the body
	switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
	case BOTH_DEATH1:
	case BOTH_DEAD1:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
		break;
	case BOTH_DEATH2:
	case BOTH_DEAD2:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
		break;
	case BOTH_DEATH3:
	case BOTH_DEAD3:
	default:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
		break;
	}

	body->r.svFlags = ent->r.svFlags;
	VectorCopy (ent->s.mins, body->s.mins);
	VectorCopy (ent->s.maxs, body->s.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.ownerNum = ent->s.number;

	body->nextthink = level.time + BODY_SINK_DELAY + BODY_SINK_TIME;
	body->think = BodyQueFree;

	body->die = body_die;

	// don't take more damage if already gibbed
	if ( ent->health <= GIB_HEALTH ) {
		body->s.eFlags |= EF_GIBBED;
		body->s.contents = 0;
		body->takedamage = qfalse;
	} else {
		body->s.contents = CONTENTS_CORPSE;
		body->takedamage = qtrue;
	}


	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity (body);
}

//======================================================================


/*
==================
SetPlayerViewAngle

==================
*/
void SetPlayerViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->player->ps.delta_angles[i] = cmdAngle - ent->player->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->player->ps.viewangles);
}

/*
================
PlayerRespawn
================
*/
void PlayerRespawn( gentity_t *ent ) {

	CopyToBodyQue (ent);
	PlayerSpawn(ent);
}

/*
================
TeamCount

Returns number of players on a team
================
*/
int TeamCount( int ignorePlayerNum, team_t team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxplayers ; i++ ) {
		if ( i == ignorePlayerNum ) {
			continue;
		}
		if ( level.players[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.players[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}

/*
================
TeamLeader

Returns the player number of the team leader
================
*/
int TeamLeader( int team ) {
	int		i;

	for ( i = 0 ; i < level.maxplayers ; i++ ) {
		if ( level.players[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.players[i].sess.sessionTeam == team ) {
			if ( level.players[i].sess.teamLeader )
				return i;
		}
	}

	return -1;
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignorePlayerNum ) {
	int		counts[TEAM_NUM_TEAMS];

	counts[TEAM_BLUE] = TeamCount( ignorePlayerNum, TEAM_BLUE );
	counts[TEAM_RED] = TeamCount( ignorePlayerNum, TEAM_RED );

	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
		return TEAM_RED;
	}
	return TEAM_BLUE;
}

/*
===========
PlayerCleanName
============
*/
static void PlayerCleanName(const char *in, char *out, int outSize)
{
	int outpos = 0, colorlessLen = 0, spaces = 0;

	// discard leading spaces
	for(; *in == ' '; in++);
	
	for(; *in && outpos < outSize - 1; in++)
	{
		out[outpos] = *in;

		if(*in == ' ')
		{
			// don't allow too many consecutive spaces
			if(spaces > 2)
				continue;
			
			spaces++;
		}
		else if(outpos > 0 && out[outpos - 1] == Q_COLOR_ESCAPE)
		{
			if(Q_IsColorString(&out[outpos - 1]))
			{
				colorlessLen--;
				
				if(ColorIndex(*in) == 0)
				{
					// Disallow color black in names to prevent players
					// from getting advantage playing in front of black backgrounds
					outpos--;
					continue;
				}
			}
			else
			{
				spaces = 0;
				colorlessLen++;
			}
		}
		else
		{
			spaces = 0;
			colorlessLen++;
		}
		
		outpos++;
	}

	out[outpos] = '\0';

	// don't allow empty names
	if( *out == '\0' || colorlessLen == 0)
		Q_strncpyz(out, DEFAULT_PLAYER_NAME, outSize );
}

/*
===========
PlayerHandicap
============
*/
float PlayerHandicap( gplayer_t *player ) {
	char	userinfo[MAX_INFO_STRING];
	float	handicap;

	if (!player) {
		return 100;
	}

	trap_GetUserinfo( player - level.players, userinfo, sizeof(userinfo) );

	handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
	if ( handicap < 1 || handicap > 100) {
		handicap = 100;
	}

	return handicap;
}


/*
===========
PlayerUserinfoChanged

Called from PlayerConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void PlayerUserinfoChanged( int playerNum ) {
	gentity_t *ent;
	int		teamTask, teamLeader;
	char	*s;
	char	model[MAX_QPATH];
	char	headModel[MAX_QPATH];
	char	oldname[MAX_STRING_CHARS];
	gplayer_t	*player;
	char	c1[MAX_INFO_STRING];
	char	c2[MAX_INFO_STRING];
	char	userinfo[MAX_INFO_STRING];

	ent = g_entities + playerNum;
	player = ent->player;

	trap_GetUserinfo( playerNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if ( !Info_Validate(userinfo) ) {
		strcpy (userinfo, "\\name\\badinfo");
		// don't keep those players and userinfo
		trap_DropPlayer(playerNum, "Invalid userinfo");
	}

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) ) {
		player->pers.predictItemPickup = qfalse;
	} else {
		player->pers.predictItemPickup = qtrue;
	}

	// check the anti lag
	s = Info_ValueForKey( userinfo, "cg_antiLag" );
	player->pers.antiLag = atoi( s );

	// set name
	Q_strncpyz ( oldname, player->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey (userinfo, "name");
	PlayerCleanName( s, player->pers.netname, sizeof(player->pers.netname) );

	if ( player->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( player->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			Q_strncpyz( player->pers.netname, "scoreboard", sizeof(player->pers.netname) );
		}
	}

	if ( player->pers.connected == CON_CONNECTED ) {
		if ( strcmp( oldname, player->pers.netname ) ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname, 
				player->pers.netname) );
		}
	}

	// set max health
#ifdef MISSIONPACK
	if (player->ps.powerups[PW_GUARD]) {
		player->pers.maxHealth = 200;
	} else {
		player->pers.maxHealth = PlayerHandicap( player );
	}
#else
	player->pers.maxHealth = PlayerHandicap( player );
#endif
	player->ps.stats[STAT_MAX_HEALTH] = player->pers.maxHealth;

	// set model
	if( g_gametype.integer >= GT_TEAM ) {
		Q_strncpyz( model, Info_ValueForKey (userinfo, "team_model"), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey (userinfo, "team_headmodel"), sizeof( headModel ) );
	} else {
		Q_strncpyz( model, Info_ValueForKey (userinfo, "model"), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey (userinfo, "headmodel"), sizeof( headModel ) );
	}

#ifdef MISSIONPACK
	if (g_gametype.integer >= GT_TEAM) {
		player->pers.teamInfo = qtrue;
	} else {
		s = Info_ValueForKey( userinfo, "teamoverlay" );
		if ( ! *s || atoi( s ) != 0 ) {
			player->pers.teamInfo = qtrue;
		} else {
			player->pers.teamInfo = qfalse;
		}
	}
#else
	// teamInfo
	s = Info_ValueForKey( userinfo, "teamoverlay" );
	if ( ! *s || atoi( s ) != 0 ) {
		player->pers.teamInfo = qtrue;
	} else {
		player->pers.teamInfo = qfalse;
	}
#endif
	/*
	s = Info_ValueForKey( userinfo, "cg_pmove_fixed" );
	if ( !*s || atoi( s ) == 0 ) {
		player->pers.pmoveFixed = qfalse;
	}
	else {
		player->pers.pmoveFixed = qtrue;
	}
	*/

	// team task (0 = none, 1 = offence, 2 = defence)
	teamTask = atoi(Info_ValueForKey(userinfo, "teamtask"));
	// team Leader (1 = leader, 0 is normal player)
	teamLeader = player->sess.teamLeader;

	// colors
	strcpy(c1, Info_ValueForKey( userinfo, "color1" ));
	strcpy(c2, Info_ValueForKey( userinfo, "color2" ));

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	if (ent->r.svFlags & SVF_BOT)
	{
		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d",
			player->pers.netname, player->sess.sessionTeam, model, headModel, c1, c2, 
			player->pers.maxHealth, player->sess.wins, player->sess.losses,
			Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader );
	}
	else
	{
		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d",
			player->pers.netname, player->sess.sessionTeam, model, headModel, c1, c2, 
			player->pers.maxHealth, player->sess.wins, player->sess.losses, teamTask, teamLeader);
	}

	trap_SetConfigstring( CS_PLAYERS+playerNum, s );

	// this is not the userinfo, more like the configstring actually
	G_LogPrintf( "PlayerUserinfoChanged: %i %s\n", playerNum, s );
}


/*
===========
PlayerConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the player should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the player will be sent the current gamestate
and will eventually get to PlayerBegin.

firstTime will be qtrue the very first time a player connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *PlayerConnect( int playerNum, qboolean firstTime, qboolean isBot, int connectionNum, int localPlayerNum ) {
	char		*value;
//	char		*areabits;
	gplayer_t	*player;
	char		userinfo[MAX_INFO_STRING];
	gentity_t	*ent;
	qboolean	firstConnectionPlayer;

	ent = &g_entities[ playerNum ];

	trap_GetUserinfo( playerNum, userinfo, sizeof( userinfo ) );

	// Check if it's the first player on the client (i.e. not a splitscreen player)
	firstConnectionPlayer = ( level.connections[connectionNum].numLocalPlayers == 0 );

	if ( firstConnectionPlayer ) {
		// IP filtering
		// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
		// recommanding PB based IP / GUID banning, the builtin system is pretty limited
		// check to see if they are on the banned IP list
		value = Info_ValueForKey (userinfo, "ip");
		if ( G_FilterPacket( value ) ) {
			return "You are banned from this server.";
		}

		// we don't check password for bots and local client
		// NOTE: local client <-> "ip" "localhost"
		//   this means this client is not running in our current process
		if ( !isBot && (strcmp(value, "localhost") != 0) ) {
			// check for a password
			value = Info_ValueForKey (userinfo, "password");
			if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
				strcmp( g_password.string, value) != 0) {
				return "Invalid password";
			}
		}
	} else {
		// Don't allow splitscreen players in single player.
		if ( g_singlePlayer.integer ) {
			return "Splitscreen not allowed in single player.";
		}
	}

	// if a player reconnects quickly after a disconnect, the player disconnect may never be called, thus flag can get lost in the ether
	if (ent->inuse) {
		G_LogPrintf("Forcing disconnect on active player: %i\n", playerNum);
		// so lets just fix up anything that should happen on a disconnect
		PlayerDisconnect(playerNum);
	}
	// they can connect
	ent->player = level.players + playerNum;
	player = ent->player;

//	areabits = player->areabits;

	memset( player, 0, sizeof(*player) );

	player->pers.connected = CON_CONNECTING;
	player->pers.initialSpawn = qtrue;

	// update player connection info
	level.connections[connectionNum].numLocalPlayers++;
	level.connections[connectionNum].localPlayerNums[localPlayerNum] = playerNum;
	player->pers.connectionNum = connectionNum;
	player->pers.localPlayerNum = localPlayerNum;

	// check for local client
	value = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( value, "localhost" ) ) {
		player->pers.localClient = qtrue;
	}

	if( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if( !G_BotConnect( playerNum, !firstTime ) ) {
			return "BotConnectfailed";
		}
	}

	// read or initialize the session data
	if ( firstTime || level.newSession ) {
		G_InitSessionData( player, userinfo );
	}
	G_ReadSessionData( player );

	// get and distribute relevent paramters
	G_LogPrintf( "PlayerConnect: %i\n", playerNum );
	PlayerUserinfoChanged( playerNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		if ( firstConnectionPlayer ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", player->pers.netname) );
		} else {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " dropped in\n\"", player->pers.netname) );
		}
	}

	// count current players and rank for scoreboard
	CalculateRanks();

	// for statistics
//	player->areabits = areabits;
//	if ( !player->areabits )
//		player->areabits = trap_Alloc( (trap_AAS_PointReachabilityAreaIndex( NULL ) + 7) / 8, NULL );

	return NULL;
}

/*
===========
PlayerBegin

called when a player has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void PlayerBegin( int playerNum ) {
	gentity_t	*ent;
	gplayer_t	*player;
	int			flags;

	ent = g_entities + playerNum;

	player = level.players + playerNum;

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->player = player;

	player->pers.connected = CON_CONNECTED;
	player->pers.enterTime = level.time;
	player->pers.teamState.state = TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = player->ps.eFlags;
	memset( &player->ps, 0, sizeof( player->ps ) );
	player->ps.eFlags = flags;

	// locate ent at a spawn point
	PlayerSpawn( ent );

	if ( player->pers.initialSpawn && !g_singlePlayer.integer ) {
		if ( g_gametype.integer != GT_TOURNAMENT ) {
			BroadcastTeamChange( player, -1 );
		}
	}
	player->pers.initialSpawn = qfalse;
	G_LogPrintf( "PlayerBegin: %i\n", playerNum );

	// count current players and rank for scoreboard
	CalculateRanks();
}

/*
===========
PlayerSpawn

Called every time a player is placed fresh in the world:
after the first PlayerBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void PlayerSpawn(gentity_t *ent) {
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gplayer_t	*player;
	int		i;
	playerPersistant_t	saved;
	playerSession_t		savedSess;
	int		persistant[MAX_PERSISTANT];
	gentity_t	*spawnPoint;
	gentity_t *tent;
	int		flags;
	int		savedPing;
//	char	*savedAreaBits;
	int		accuracy_hits, accuracy_shots;
	int		eventSequence;

	index = ent - g_entities;
	player = ent->player;

	VectorClear(spawn_origin);

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this player
	if ( player->sess.sessionTeam == TEAM_SPECTATOR ) {
		spawnPoint = SelectSpectatorSpawnPoint ( 
						spawn_origin, spawn_angles);
	} else if (g_gametype.integer >= GT_CTF ) {
		// all base oriented team games use the CTF spawn points
		spawnPoint = SelectCTFSpawnPoint ( 
						player->sess.sessionTeam, 
						player->pers.teamState.state, 
						spawn_origin, spawn_angles,
						!!(ent->r.svFlags & SVF_BOT));
	}
	else
	{
		// the first spawn should be at a good looking spot
		if ( player->pers.initialSpawn && player->pers.localClient )
		{
			spawnPoint = SelectInitialSpawnPoint(spawn_origin, spawn_angles,
							     !!(ent->r.svFlags & SVF_BOT));
		}
		else
		{
			// don't spawn near existing origin if possible
			spawnPoint = SelectSpawnPoint ( 
				player->ps.origin, 
				spawn_origin, spawn_angles, !!(ent->r.svFlags & SVF_BOT));
		}
	}
	player->pers.teamState.state = TEAM_ACTIVE;

	// always clear the kamikaze flag
	ent->s.eFlags &= ~EF_KAMIKAZE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->player->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// clear everything but the persistant data

	saved = player->pers;
	savedSess = player->sess;
	savedPing = player->ps.ping;
//	savedAreaBits = player->areabits;
	accuracy_hits = player->accuracy_hits;
	accuracy_shots = player->accuracy_shots;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = player->ps.persistant[i];
	}
	eventSequence = player->ps.eventSequence;

	Com_Memset (player, 0, sizeof(*player));

	player->pers = saved;
	player->sess = savedSess;
	player->ps.ping = savedPing;
//	player->areabits = savedAreaBits;
	player->accuracy_hits = accuracy_hits;
	player->accuracy_shots = accuracy_shots;
	player->lastkilled_player = -1;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		player->ps.persistant[i] = persistant[i];
	}
	player->ps.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	player->ps.persistant[PERS_SPAWN_COUNT]++;
	player->ps.persistant[PERS_TEAM] = player->sess.sessionTeam;

	player->airOutTime = level.time + 12000;

	// set max health
	player->pers.maxHealth = PlayerHandicap( player );
	// clear entity values
	player->ps.stats[STAT_MAX_HEALTH] = player->pers.maxHealth;
	player->ps.eFlags = flags;
	player->ps.contents = CONTENTS_BODY;
	player->ps.collisionType = ( g_playerCapsule.integer == 1 ) ? CT_CAPSULE : CT_AABB;

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->player = &level.players[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;
	
	VectorCopy (playerMins, player->ps.mins);
	VectorCopy (playerMaxs, player->ps.maxs);

	player->ps.playerNum = index;

	player->ps.stats[STAT_WEAPONS] = ( 1 << WP_MACHINEGUN );
	if ( g_gametype.integer == GT_TEAM ) {
		player->ps.ammo[WP_MACHINEGUN] = 50;
	} else {
		player->ps.ammo[WP_MACHINEGUN] = 100;
	}

	player->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GAUNTLET );
	player->ps.ammo[WP_GAUNTLET] = -1;
	player->ps.ammo[WP_GRAPPLING_HOOK] = -1;

	// health will count down towards max_health
	ent->health = player->ps.stats[STAT_HEALTH] = player->ps.stats[STAT_MAX_HEALTH] + 25;

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, player->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	player->ps.pm_flags |= PMF_RESPAWNED;

	trap_GetUsercmd( player - level.players, &ent->player->pers.cmd );
	SetPlayerViewAngle( ent, spawn_angles );
	// don't allow full run speed for a bit
	player->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	player->ps.pm_time = 100;

	player->respawnTime = level.time;
	player->inactivityTime = level.time + g_inactivity.integer * 1000;
	player->latched_buttons = 0;

	// set default animations
	player->ps.torsoAnim = TORSO_STAND;
	player->ps.legsAnim = LEGS_IDLE;

	if (!level.intermissiontime) {
		if (ent->player->sess.sessionTeam != TEAM_SPECTATOR) {
			G_KillBox(ent);
			// force the base weapon up
			player->ps.weapon = WP_MACHINEGUN;
			player->ps.weaponstate = WEAPON_READY;
			// fire the targets of the spawn point
			G_UseTargets(spawnPoint, ent);
			// select the highest weapon number available, after any spawn given items have fired
			player->ps.weapon = 1;

			for (i = WP_NUM_WEAPONS - 1 ; i > 0 ; i--) {
				if (player->ps.stats[STAT_WEAPONS] & (1 << i)) {
					player->ps.weapon = i;
					break;
				}
			}
			// positively link the player, even if the command times are weird
			VectorCopy(ent->player->ps.origin, ent->r.currentOrigin);

			tent = G_TempEntity(ent->player->ps.origin, EV_PLAYER_TELEPORT_IN);
			tent->s.playerNum = ent->s.playerNum;

			trap_LinkEntity (ent);
		}
	} else {
		// move players to intermission
		MovePlayerToIntermission(ent);
	}
	// run a player frame to drop exactly to the floor,
	// initialize animations and other things
	player->ps.commandTime = level.time - 100;
	ent->player->pers.cmd.serverTime = level.time;
	PlayerThink( ent-g_entities );
	// run the presend to set anything else, follow spectators wait
	// until all players have been reconnected after map_restart
	if ( ent->player->sess.spectatorState != SPECTATOR_FOLLOW ) {
		PlayerEndFrame( ent );
	}

	// clear entity state values
	BG_PlayerStateToEntityState( &player->ps, &ent->s, qtrue );

	// we don't want players being backward-reconciled to the place they died
	G_ResetHistory( ent );
}


/*
===========
PlayerDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropPlayer(), which will call this and do
server system housekeeping.
============
*/
void PlayerDisconnect( int playerNum ) {
	gentity_t	*ent;
	gentity_t	*tent;
	int			i;

	// cleanup if we are kicking a bot that
	// hasn't spawned yet
	G_RemoveQueuedBotBegin( playerNum );

	ent = g_entities + playerNum;
	if (!ent->player || ent->player->pers.connected == CON_DISCONNECTED) {
		return;
	}

	// stop any following players
	for ( i = 0 ; i < level.maxplayers ; i++ ) {
		if ( level.players[i].sess.sessionTeam == TEAM_SPECTATOR
			&& level.players[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& level.players[i].sess.spectatorPlayer == playerNum ) {
			StopFollowing( &g_entities[i] );
		}
	}

	// send effect if they were completely connected
	if ( ent->player->pers.connected == CON_CONNECTED 
		&& ent->player->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( ent->player->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.playerNum = ent->s.playerNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossPlayerItems( ent );
#ifdef MISSIONPACK
		TossPlayerPersistantPowerups( ent );
		if( g_gametype.integer == GT_HARVESTER ) {
			TossPlayerCubes( ent );
		}
#endif

	}

	G_LogPrintf( "PlayerDisconnect: %i\n", playerNum );

	// if we are playing in tourney mode and losing, give a win to the other player
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& !level.intermissiontime
		&& !level.warmupTime && level.sortedPlayers[1] == playerNum ) {
		level.players[ level.sortedPlayers[0] ].sess.wins++;
		PlayerUserinfoChanged( level.sortedPlayers[0] );
	}

	if( g_gametype.integer == GT_TOURNAMENT &&
		ent->player->sess.sessionTeam == TEAM_FREE &&
		level.intermissiontime ) {

		trap_Cmd_ExecuteText( EXEC_APPEND, "map_restart 0\n" );
		level.restarted = qtrue;
		level.changemap = NULL;
		level.intermissiontime = 0;
	}

	trap_UnlinkEntity (ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->player->pers.connected = CON_DISCONNECTED;
	ent->player->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->player->sess.sessionTeam = TEAM_FREE;

	trap_SetConfigstring( CS_PLAYERS + playerNum, "");

	CalculateRanks();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownPlayer( playerNum, qfalse );
	}

	// clear player connection info
	level.connections[ent->player->pers.connectionNum].numLocalPlayers--;
	level.connections[ent->player->pers.connectionNum].localPlayerNums[ent->player->pers.localPlayerNum] = -1;
	ent->player->pers.localPlayerNum = ent->player->pers.connectionNum = -1;
}


