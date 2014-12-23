/*
===========================================================================
Copyright (C) 2014 Zack Middleton

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
// defines the game menus

#include "ui_local.h"

extern menudef_t mainmenu;
extern menudef_t singleplayermenu;
extern menudef_t multiplayermenu;
extern menudef_t setupmenu;
extern menudef_t demosmenu;
extern menudef_t cinematicsmenu;
extern menudef_t modsmenu;
extern menudef_t exitmenu;

// setup
extern menudef_t playermenu;
extern menudef_t controlsmenu;
extern menudef_t systemmenu;
extern menudef_t gameoptionsmenu;
extern menudef_t defaultsmenu;

// only referenced by team arena right now
extern menudef_t rulesmenu;
extern menudef_t startservermenu;

//
extern menudef_t creditsmenu;
extern menudef_t credits2menu;

//
extern menudef_t postgamemenu;

#ifdef MISSIONPACK
void launchQ3( int item ) {
	trap_Cvar_Set( "fs_game", BASEQ3 );
	trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
}
#else
void launchTeamArena( int item ) {
	trap_Cvar_Set( "fs_game", BASETA );
	trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
}
#endif

void demoHandler( int item ) {
	CG_Printf("DEBUG: TODO: play demo... got %d\n", item);
}

void cinematicHandler( int item ) {
	CG_Printf("DEBUG: TODO: play cinematic... got %d\n", item);
}

void modHandler( int item ) {
	CG_Printf("DEBUG: TODO: load mod... got %d\n", item);
}

// 0 = yes, 1 = no
void exitHandler( int item ) {
	if ( item == 0 ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	} else {
		CG_Printf("DEBUG: canceled exiting...\n");
	}
}

// 0 = yes, 1 = no
void defaultsHandler( int item ) {
	if ( item == 0 ) {
	// should put cvar_restart first? sometimes people put cvars in it -- this would apply to q3_ui too
	trap_Cmd_ExecuteText( EXEC_APPEND, "exec default.cfg\n");
	trap_Cmd_ExecuteText( EXEC_APPEND, "cvar_restart\n");
	trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
	} else {
		CG_Printf("DEBUG: canceled reseting to defaults...\n");
	}
}

void newGame( int item ) {
	trap_Cvar_SetValue( "ui_singlePlayerActive", 1 );
	trap_Cvar_SetValue( "g_doWarmup", 0 );
	trap_Cvar_SetValue( "g_gametype", GT_SINGLE_PLAYER );
	trap_Cmd_ExecuteText( EXEC_APPEND, "map q3dm0\n" );
}

void endGame( int item ) {
	trap_Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
}

void restartMap( int item ) {
	trap_Cmd_ExecuteText( EXEC_APPEND, "map_restart 0\n" );
}


#ifdef MISSIONPACK
menuitem_t mainmenu_items[] =
{
	{ MIF_HEADER, "TEAM ARENA", NULL, 20 },
	{ MIF_CALL, "Quake3", launchQ3, 0 },
	{ MIF_SUBMENU, "Rules", &rulesmenu, 0 },
	{ MIF_SUBMENU, "Credits", &creditsmenu, 0 },
	{ MIF_SUBMENU, "Mods", &modsmenu, 0 },
	{ MIF_SUBMENU, "Cinematics", &cinematicsmenu, 0 },

	{ MIF_HEADER, "PLAY", NULL, 0 },
	{ MIF_SUBMENU, "SinglePlayer", &singleplayermenu, 0 },
	{ MIF_SUBMENU, "MultiPlayer", &multiplayermenu, 0 },
	{ MIF_SUBMENU, "StartServer", &startservermenu, 0 },

	{ MIF_HEADER, "OPTIONS", NULL, 340 },
	{ MIF_SUBMENU, "Player", &playermenu, 0 },
	{ MIF_SUBMENU, "Setup", &setupmenu, 0 },
	{ MIF_SUBMENU, "Demos", &demosmenu, 0 },

	{ MIF_HEADER|MIF_SUBMENU, "QUIT", &exitmenu, 0 }
};
#else
menuitem_t mainmenu_items[] =
{
	{ MIF_SUBMENU, "Single Player", &singleplayermenu, 0 },
	{ MIF_SUBMENU, "Multiplayer", &multiplayermenu, 0 },
	{ MIF_SUBMENU, "Setup", &setupmenu, 0 },
	{ MIF_SUBMENU, "Demos", &demosmenu, 0 },
	{ MIF_SUBMENU, "Cinematics", &cinematicsmenu, 0 },
	{ MIF_CALL, "Team Arena", launchTeamArena, 0 },
	{ MIF_SUBMENU, "Mods", &modsmenu, 0 },
	{ MIF_SUBMENU, "Exit", &exitmenu, 0 }
};
#endif

menuitem_t singleplayermenu_items[] =
{
	{ MIF_CALL, "New Game", newGame, 0 },
	{ MIF_CALL, "End Game", endGame, 0 },
	//{ 0, "Level Select...", NULL, 0 },
};

menuitem_t setupmenu_items[] =
{
	{ MIF_SUBMENU, "Players", &playermenu, 0 }, // Player or Players in q3_ui depending on number of max players
	{ MIF_SUBMENU, "Controls", &controlsmenu, 0 },
	{ MIF_SUBMENU, "System", &systemmenu, 0 },
	{ MIF_SUBMENU, "Game Options", &gameoptionsmenu, 0 },
	{ MIF_SUBMENU, "Defaults", &defaultsmenu, 0 }, // TODO: only show if not in-game?
};

menuitem_t demosmenu_items[] =
{
	{ MIF_CALL|MIF_NEXTBUTTON, "Play Demo", demoHandler, 0 },
};

menuitem_t cinematicsmenu_items[] =
{
	{ MIF_CALL|MIF_NEXTBUTTON, "Play Cinematic", cinematicHandler, 0 },
};

menuitem_t modsmenu_items[] =
{
	{ MIF_CALL|MIF_NEXTBUTTON, "Load Mod", modHandler, 0 },
};

menuitem_t exitmenu_items[] =
{
	{ MIF_CALL, "Yes", exitHandler, 0 },
	{ 0, "/", NULL, 0 },
	{ MIF_CALL|MIF_POPMENU, "No", exitHandler, 0 },
};

menuitem_t defaultsmenu_items[] =
{
	{ MIF_CALL, "Yes", defaultsHandler, 0 },
	{ 0, "/", NULL, 0 },
	{ MIF_CALL|MIF_POPMENU, "No", defaultsHandler, 0 },

	{ MIF_HEADER, "WARNING: This will reset *ALL*", NULL, 400 },
	{ 0, "options to their default values.", NULL, 0 },
};

menuitem_t creditsmenu_items[] =
{
	{ 0, "Programming: John Carmack", NULL, 0 },
	{ MIF_SWAPMENU|MIF_NEXTBUTTON, "Additional Credit", &credits2menu, 0 },
};

menuitem_t credits2menu_items[] =
{
	{ 0, "SDL, jpeg, etc", NULL, 0 },
	{ MIF_SWAPMENU|MIF_NEXTBUTTON, "id Credit", &creditsmenu, 0 },
};

// Team Arena uses; Replay, Demo, Menu, Next
menuitem_t postgamemenu_items[] =
{
	{ MIF_CALL, "Menu", endGame, 0 },
	{ MIF_CALL|MIF_POPMENU, "Replay", restartMap, 0 },
	{ MIF_CALL|MIF_POPMENU, "Next", restartMap, 0 }, // TODO: add next map logic
};

menuitem_t stubmenu_items[] =
{
	{ 0, "STUB", NULL, 0 },
};

menudef_t mainmenu = { MF_MAINMENU, NULL, mainmenu_items, ARRAY_LEN( mainmenu_items ) };
menudef_t singleplayermenu = { 0, "Single Player", singleplayermenu_items, ARRAY_LEN( singleplayermenu_items ) };
menudef_t multiplayermenu = { 0, "Multiplayer", stubmenu_items, ARRAY_LEN( stubmenu_items ) };
menudef_t setupmenu = { 0, "Setup", setupmenu_items, ARRAY_LEN( setupmenu_items ) };
#ifdef MISSIONPACK
menudef_t demosmenu = { 0, "Team Arena Demos", demosmenu_items, ARRAY_LEN( demosmenu_items ) };
menudef_t cinematicsmenu = { 0, "Team Arena Cinematics", cinematicsmenu_items, ARRAY_LEN( cinematicsmenu_items ) };
menudef_t modsmenu = { 0, "Team Arena Mods", modsmenu_items, ARRAY_LEN( modsmenu_items ) };
menudef_t exitmenu = { MF_DIALOG|MF_NOBACK, "Quit Team Arena?", exitmenu_items, ARRAY_LEN( exitmenu_items ) };
#else
menudef_t demosmenu = { 0, "Demos", demosmenu_items, ARRAY_LEN( demosmenu_items ) };
menudef_t cinematicsmenu = { 0, "Cinematics", cinematicsmenu_items, ARRAY_LEN( cinematicsmenu_items ) };
menudef_t modsmenu = { 0, "Mods", modsmenu_items, ARRAY_LEN( modsmenu_items ) };
menudef_t exitmenu = { MF_DIALOG|MF_NOBACK, "Exit?", exitmenu_items, ARRAY_LEN( exitmenu_items ) };
#endif

// Setup menus
menudef_t playermenu = { 0, "Player Setup", stubmenu_items, ARRAY_LEN( stubmenu_items ) };
menudef_t controlsmenu = { 0, "Controls", stubmenu_items, ARRAY_LEN( stubmenu_items ) };
menudef_t systemmenu = { 0, "System", stubmenu_items, ARRAY_LEN( stubmenu_items ) };
menudef_t gameoptionsmenu = { 0, "Game Options", stubmenu_items, ARRAY_LEN( stubmenu_items ) };
menudef_t defaultsmenu = { MF_DIALOG|MF_NOBACK, "Set to defaults?", defaultsmenu_items, ARRAY_LEN( defaultsmenu_items ) };

// Team Arena stubs
menudef_t rulesmenu = { 0, "Rules", stubmenu_items, ARRAY_LEN( stubmenu_items ) };
menudef_t startservermenu = { 0, "Start Server", stubmenu_items, ARRAY_LEN( stubmenu_items ) };

// Team Arena credit menu
menudef_t creditsmenu = { 0, "id Credit", creditsmenu_items, ARRAY_LEN( creditsmenu_items ) };
menudef_t credits2menu = { 0, "Additional Credit", credits2menu_items, ARRAY_LEN( credits2menu_items ) };

// in game menus
menudef_t postgamemenu = { MF_POSTGAME|MF_NOESCAPE|MF_NOBACK, NULL, postgamemenu_items, ARRAY_LEN( postgamemenu_items ) };


