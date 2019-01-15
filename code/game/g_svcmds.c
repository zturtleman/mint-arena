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

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"


/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and you can use '*' to match any value
so you can specify an entire class C network with "addip 192.246.40.*"

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.

TTimo NOTE: for persistence, bans are stored in g_banIPs cvar MAX_CVAR_VALUE_STRING
The size of the cvar string buffer is limiting the banning to around 20 masks
this could be improved by putting some g_banIPs2 g_banIps3 etc. maybe
still, you should rely on PB for banning instead

==============================================================================
*/

typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			if (*s == '*') // 'match any'
			{
				// b[i] and m[i] to 0
				s++;
				if (!*s)
					break;
				s++;
				continue;
			}
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4] = {0};
	byte	m[4] = {0};
	int		i,j;
	char	iplist_final[MAX_CVAR_VALUE_STRING] = {0};
	char	ip[64] = {0};

	*iplist_final = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		*(unsigned *)m = ipFilters[i].mask;
		*ip = 0;
		for (j = 0 ; j < 4 ; j++)
		{
			if (m[j]!=255)
				Q_strcat(ip, sizeof(ip), "*");
			else
				Q_strcat(ip, sizeof(ip), va("%i", b[j]));
			Q_strcat(ip, sizeof(ip), (j<3) ? "." : " ");
		}		
		if (strlen(iplist_final)+strlen(ip) < MAX_CVAR_VALUE_STRING)
		{
			Q_strcat( iplist_final, sizeof(iplist_final), ip);
		}
		else
		{
			Com_Printf("g_banIPs overflowed at MAX_CVAR_VALUE_STRING\n");
			break;
		}
	}

	trap_Cvar_Set( "g_banIPs", iplist_final );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4] = {0};
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t;
	char		str[MAX_CVAR_VALUE_STRING];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t );
		t = s;
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: addip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	G_Printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities;
	for (e = 0; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_GRAPPLE:
			G_Printf("ET_GRAPPLE          ");
			break;
		case ET_CORONA:
			G_Printf("ET_CORONA           ");
			break;
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

int PlayerForString( const char *s ) {
	gplayer_t	*cl;
	int			idnum;
	char		cleanName[MAX_NETNAME];

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

	G_Printf( "User %s is not on the server\n", s );

	return -1;
}

/*
===================
G_Field_CompletePlayerName
===================
*/
void G_Field_CompletePlayerName( void ) {
	gplayer_t	*cl;
	int			idnum;
	char		cleanName[MAX_NETNAME];
	char		list[MAX_CLIENTS * MAX_NETNAME];
	int			listTotalLength;

	// ZTM: FIXME: have to clear whole list because BG_AddStringToList doesn't properly terminate list
	memset( list, 0, sizeof( list ) );
	listTotalLength = 0;

	for ( idnum=0,cl=level.players ; idnum < level.maxplayers ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		Q_strncpyz(cleanName, cl->pers.netname, sizeof(cleanName));
		Q_CleanStr(cleanName);

		// Use quotes if there is a space in the name
		if ( strchr( cleanName, ' ' ) != NULL ) {
			BG_AddStringToList( list, sizeof( list ), &listTotalLength, va( "\"%s\"", cleanName ) );
		} else {
			BG_AddStringToList( list, sizeof( list ), &listTotalLength, cleanName );
		}
	}

	if ( listTotalLength > 0 ) {
		list[listTotalLength++] = 0;
		trap_Field_CompleteList( list );
	}
}

/*
===================
Svcmd_ForceTeam_f

forceTeam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	int			playerNum;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 3 ) {
		G_Printf("Usage: forceTeam <player> <team>\n");
		return;
	}

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	playerNum = PlayerForString( str );
	if ( playerNum == -1 ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[playerNum], str );
}

/*
===================
Svcmd_ForceTeamComplete
===================
*/
void	Svcmd_ForceTeamComplete( char *args, int argNum ) {
	if ( argNum == 2 ) {
		G_Field_CompletePlayerName();
	} else if ( argNum == 3 ) {
		trap_Field_CompleteList( "blue\0follow1\0follow2\0free\0red\0scoreboard\0spectator\0" );
	}
}

/*
===================
Svcmd_Teleport_f

teleport <player> <x> <y> <z> [yaw]
===================
*/
void	Svcmd_Teleport_f( void ) {
	int			playerNum;
	gentity_t	*ent;
	char		str[MAX_TOKEN_CHARS];
	vec3_t		position, angles;

	if ( !g_cheats.integer ) {
		G_Printf("Cheats are not enabled on this server.\n");
		return;
	}

	if ( trap_Argc() < 3 ) {
		G_Printf("Usage: teleport <player> <x> <y> <z> [yaw]\n");
		return;
	}

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	playerNum = PlayerForString( str );
	if ( playerNum == -1 ) {
		return;
	}

	// set the position
	trap_Argv( 2, str, sizeof( str ) );
	position[0] = atoi( str );

	trap_Argv( 3, str, sizeof( str ) );
	position[1] = atoi( str );

	trap_Argv( 4, str, sizeof( str ) );
	position[2] = atoi( str );

	ent = &g_entities[playerNum];
	VectorCopy( ent->s.angles, angles );

	if ( trap_Argc() > 5 ) {
		trap_Argv( 5, str, sizeof( str ) );
		angles[YAW] = atoi( str );
	}

	TeleportPlayer( ent, position, angles );
}

/*
===================
Svcmd_TeleportComplete
===================
*/
void	Svcmd_TeleportComplete( char *args, int argNum ) {
	if ( argNum == 2 ) {
		G_Field_CompletePlayerName();
	}
}

/*
===================
Svcmd_ListIPs_f
===================
*/
void	Svcmd_ListIPs_f( void ) {
	trap_Cmd_ExecuteText( EXEC_NOW, "g_banIPs\n" );
}

/*
===================
Svcmd_Say_f
===================
*/
void	Svcmd_Say_f( void ) {
	char		*p;

	if ( trap_Argc() < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

	G_Say( NULL, NULL, SAY_ALL, p );
}

/*
===================
Svcmd_Tell_f
===================
*/
void	Svcmd_Tell_f( void ) {
	char		arg[MAX_TOKEN_CHARS];
	int			playerNum;
	gentity_t	*target;
	char		*p;

	if ( trap_Argc() < 3 ) {
		G_Printf( "Usage: tell <player id> <message>\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	playerNum = PlayerForString( arg );
	if ( playerNum == -1 ) {
		return;
	}

	target = &level.gentities[playerNum];
	if ( !target->inuse || !target->player ) {
		return;
	}

	p = ConcatArgs( 2 );

	G_Say( NULL, target, SAY_TELL, p );
}

/*
===================
Svcmd_TellComplete
===================
*/
void	Svcmd_TellComplete( char *args, int argNum ) {
	if ( argNum == 2 ) {
		G_Field_CompletePlayerName();
	}
}

struct svcmd
{
  char     *cmd;
  qboolean dedicated;
  void     ( *function )( void );
  void     ( *complete )( char *, int );
} svcmds[ ] = {
  { "abort_podium", qfalse, Svcmd_AbortPodium_f },
  { "addbot", qfalse, Svcmd_AddBot_f, Svcmd_AddBotComplete },
  { "addip", qfalse, Svcmd_AddIP_f },
  { "botlist", qfalse, Svcmd_BotList_f },
  { "botreport", qfalse, Svcmd_BotTeamplayReport_f },
  { "entityList", qfalse, Svcmd_EntityList_f },
  { "forceTeam", qfalse, Svcmd_ForceTeam_f, Svcmd_ForceTeamComplete },
  { "listip", qfalse, Svcmd_ListIPs_f },
  { "removeip", qfalse, Svcmd_RemoveIP_f },
  { "say", qtrue, Svcmd_Say_f },
  { "teleport", qfalse, Svcmd_Teleport_f, Svcmd_TeleportComplete },
  { "tell", qtrue, Svcmd_Tell_f, Svcmd_TellComplete },
};

const size_t numSvCmds = ARRAY_LEN(svcmds);

/*
=================
G_ConsoleCommand

=================
*/
qboolean	G_ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];
	struct	svcmd *command;

	trap_Argv( 0, cmd, sizeof( cmd ) );

	command = bsearch( cmd, svcmds, numSvCmds, sizeof( struct svcmd ), cmdcmp );

	if( !command )
	{
		if( g_dedicated.integer )
			G_Printf( "unknown command: %s\n", cmd );

		return qfalse;
	}

	if( command->dedicated && !g_dedicated.integer )
		return qfalse;

	command->function( );
	return qtrue;
}

/*
=================
G_ConsoleCompleteArgument

=================
*/
qboolean	G_ConsoleCompleteArgument( int completeArgument ) {
	char	args[BIG_INFO_STRING];
	char	cmd[MAX_TOKEN_CHARS];
	struct	svcmd *command;

	trap_Argv( 0, cmd, sizeof( cmd ) );

	command = bsearch( cmd, svcmds, numSvCmds, sizeof( struct svcmd ), cmdcmp );

	if( !command || !command->complete )
		return qfalse;

	if( command->dedicated && !g_dedicated.integer )
		return qfalse;

	trap_LiteralArgs( args, sizeof ( args ) );

	command->complete( args, completeArgument );
	return qtrue;
}

void G_RegisterCommands( void )
{
	int i;

	for( i = 0; i < numSvCmds; i++ )
	{
		if( svcmds[ i ].dedicated && !g_dedicated.integer )
			continue;
		trap_AddCommand( svcmds[ i ].cmd );
	}
}
