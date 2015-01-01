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

#ifdef Q3_VM
// shut the hell up about
// warning: conversion from `pointer to void' to `pointer to void function(int)' is compiler dependent
#undef NULL
#define NULL 0
#endif

/*

	Menu action functions

*/

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


/*

	Menu item definitions

*/

#ifdef MISSIONPACK
menuitem_t mainmenu_items[] =
{
	{ MIF_BIGTEXT|MIF_HEADER, "TEAM ARENA", NULL, M_NONE, 20 },
	{ MIF_BIGTEXT|MIF_CALL, "Quake3", launchQ3, M_NONE, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Rules", NULL, M_RULES, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Credits", NULL, M_CREDITS_PAGE1, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Mods", NULL, M_MODS, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Cinematics", NULL, M_CINEMATICS, 0 },

	{ MIF_BIGTEXT|MIF_HEADER, "PLAY", NULL, M_NONE, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "SinglePlayer", NULL, M_SINGLEPLAYER, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "MultiPlayer", NULL, M_MULTIPLAYER, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "StartServer", NULL, M_START_SERVER, 0 },

	{ MIF_BIGTEXT|MIF_HEADER, "OPTIONS", NULL, M_NONE, 340 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Player", NULL, M_PLAYER, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Setup", NULL, M_SETUP, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Demos", NULL, M_DEMOS, 0 },

	{ MIF_BIGTEXT|MIF_HEADER|MIF_SUBMENU, "QUIT", NULL, M_EXIT, 0 }
};
#else
menuitem_t mainmenu_items[] =
{
	{ MIF_BIGTEXT|MIF_SUBMENU, "Single Player", NULL, M_SINGLEPLAYER, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Multiplayer", NULL, M_MULTIPLAYER, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Setup", NULL, M_SETUP, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Demos", NULL, M_DEMOS, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Cinematics", NULL, M_CINEMATICS, 0 },
	{ MIF_BIGTEXT|MIF_CALL, "Team Arena", launchTeamArena, M_NONE, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Mods", NULL, M_MODS, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Exit", NULL, M_EXIT, 0 }
};
#endif

cvarRangePair_t cr_skillPairs[] = {
{ 1, "I Can Win" },
{ 2, "Bring It On" },
{ 3, "Hurt Me Plenty" },
{ 4, "Hardcore" },
{ 5, "Nightmare!" }
};
cvarRange_t cr_skill = { 1, 5, qtrue, cr_skillPairs, ARRAY_LEN(cr_skillPairs) };

menuitem_t singleplayermenu_items[] =
{
	{ MIF_BIGTEXT|MIF_CALL, "Bot skill:", NULL, M_NONE, 0, "g_spSkill", &cr_skill },
	{ MIF_BIGTEXT|MIF_CALL, "New Game", newGame, M_NONE, 0 },
	{ MIF_BIGTEXT|MIF_CALL, "End Game", endGame, M_NONE, 0 },
	//{ MIF_BIGTEXT, "Level Select...", NULL, M_NONE, 0 },
};

menuitem_t setupmenu_items[] =
{
	{ MIF_BIGTEXT|MIF_SUBMENU, "Players", NULL, M_PLAYER, 0 }, // Player or Players in q3_ui depending on number of max players
	{ MIF_BIGTEXT|MIF_SUBMENU, "Controls", NULL, M_CONTROLS, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "System", NULL, M_SYSTEM, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Game Options", NULL, M_GAME_OPTIONS, 0 },
	{ MIF_BIGTEXT|MIF_SUBMENU, "Defaults", NULL, M_DEFAULTS, 0 }, // TODO: only show if not in-game?
};

menuitem_t demosmenu_items[] =
{
	{ MIF_BIGTEXT|MIF_CALL|MIF_NEXTBUTTON, "Play Demo", demoHandler, M_NONE, 0 },
};

menuitem_t cinematicsmenu_items[] =
{
	{ MIF_BIGTEXT|MIF_CALL|MIF_NEXTBUTTON, "Play Cinematic", cinematicHandler, M_NONE, 0 },
};

menuitem_t modsmenu_items[] =
{
	{ MIF_BIGTEXT|MIF_CALL|MIF_NEXTBUTTON, "Load Mod", modHandler, M_NONE, 0 },
};

menuitem_t exitmenu_items[] =
{
	{ MIF_BIGTEXT|MIF_CALL, "Yes", exitHandler, M_NONE, 0 },
	{ MIF_BIGTEXT, "/", NULL, M_NONE, 0 },
	{ MIF_BIGTEXT|MIF_CALL|MIF_POPMENU, "No", exitHandler, M_NONE, 0 },
};

menuitem_t defaultsmenu_items[] =
{
	{ MIF_BIGTEXT|MIF_CALL, "Yes", defaultsHandler, M_NONE, 0 },
	{ MIF_BIGTEXT, "/", NULL, M_NONE, 0 },
	{ MIF_BIGTEXT|MIF_CALL|MIF_POPMENU, "No", defaultsHandler, M_NONE, 0 },

	{ MIF_BIGTEXT|MIF_HEADER, "WARNING: This will reset *ALL*", NULL, M_NONE, 400 },
	{ MIF_BIGTEXT, "options to their default values.", NULL, M_NONE, 0 },
};


static cvarRangePair_t cr_teamoverlayPairs[] = { { 0, "off" }, { 1, "upper right" }, { 2, "lower right" }, { 3, "lower left" } };
static cvarRange_t cr_teamoverlay = { 0, 3, qtrue, cr_teamoverlayPairs, ARRAY_LEN(cr_teamoverlayPairs) };

static cvarRangePair_t cr_splitverticalPairs[] = { { 0, "horizontal" }, { 1, "vertical" } };
static cvarRange_t cr_splitvertical = { 0, 1, qtrue, cr_splitverticalPairs, ARRAY_LEN(cr_splitverticalPairs) };

static cvarRangePair_t cr_atmeffectsPairs[] = { { 0, "off" }, { 0.5f, "low" }, { 1, "high" } };
static cvarRange_t cr_atmeffects = { 0, 1, qfalse, cr_atmeffectsPairs, ARRAY_LEN(cr_atmeffectsPairs) };

static cvarRangePair_t cr_boolPairs[] = { { 0, "off" }, { 1, "on" } };
static cvarRange_t cr_bool = { 0, 1, qtrue, cr_boolPairs, ARRAY_LEN(cr_boolPairs) };

static cvarRangePair_t cr_boolInvertPairs[] = { { 0, "on" }, { 1, "off" } };
static cvarRange_t cr_boolInvert = { 0, 1, qtrue, cr_boolInvertPairs, ARRAY_LEN(cr_boolInvertPairs) };

static cvarRangePair_t cr_brassTimePairs[] = { { 0, "off" }, { 2500, "on" } };
static cvarRange_t cr_brassTime = { 0, 1, qtrue, cr_brassTimePairs, ARRAY_LEN(cr_brassTimePairs) };

menuitem_t gameoptionsmenu_items[] =
{
	//{ MIF_CALL, "Crosshair:",				NULL, M_NONE, 0, "cg_drawCrosshair", NULL }, // ZTM: TODO: draw crosshair shaders
	{ MIF_CALL, "View Bobbing:",			NULL, M_NONE, 0, "cg_viewbob", &cr_bool },
	{ MIF_CALL, "Simple Items:",			NULL, M_NONE, 0, "cg_simpleItems", &cr_bool },
	{ MIF_CALL, "Marks on Walls:",			NULL, M_NONE, 0, "cg_marks", &cr_bool },
	{ MIF_CALL, "Ejecting Brass:",			NULL, M_NONE, 0, "cg_brassTime", &cr_brassTime },
	{ MIF_CALL, "Dynamic Lights:",			NULL, M_NONE, 0, "r_dynamiclight", &cr_bool },
	{ MIF_CALL, "Identify Target:",			NULL, M_NONE, 0, "cg_drawCrosshairNames", &cr_bool },
	{ MIF_CALL, "High Quality Sky:",		NULL, M_NONE, 0, "r_fastsky", &cr_boolInvert },
	{ MIF_CALL, "Sync Every Frame:",		NULL, M_NONE, 0, "r_finish", &cr_bool },
	{ MIF_CALL, "Force Player Models:",		NULL, M_NONE, 0, "cg_forcemodel", &cr_bool },
	{ MIF_CALL, "Draw Team Overlay:",		NULL, M_NONE, 0, "cg_drawTeamOverlay", &cr_teamoverlay },
	{ MIF_CALL, "Automatic Downloading:",	NULL, M_NONE, 0, "cl_allowDownload", &cr_bool },
	{ MIF_CALL, "Splitscreen Mode:",		NULL, M_NONE, 0, "cg_splitviewVertical", &cr_splitvertical },
	{ MIF_CALL, "Snow/Rain:",				NULL, M_NONE, 0, "cg_atmosphericEffects", &cr_atmeffects },
};

menuitem_t creditsmenu_items[] =
{
	{ 0, "Programming: John Carmack", NULL, M_NONE, 0 },
	{ MIF_SWAPMENU|MIF_NEXTBUTTON, "Additional Credit", NULL, M_CREDITS_PAGE2, 0 },
};

menuitem_t credits2menu_items[] =
{
	{ 0, "SDL, jpeg, etc", NULL, M_NONE, 0 },
	{ MIF_SWAPMENU|MIF_NEXTBUTTON, "id Credit", NULL, M_CREDITS_PAGE1, 0 },
};

// Team Arena uses; Replay, Demo, Menu, Next
menuitem_t postgamemenu_items[] =
{
	{ MIF_BIGTEXT|MIF_CALL, "Menu", endGame, M_NONE, 0 },
	{ MIF_BIGTEXT|MIF_CALL|MIF_POPMENU, "Replay", restartMap, M_NONE, 0 },
	{ MIF_BIGTEXT|MIF_CALL|MIF_POPMENU, "Next", restartMap, M_NONE, 0 }, // TODO: add next map logic
};

menuitem_t stubmenu_items[] =
{
	{ 0, "STUB", NULL, M_NONE, 0 },
};


/*

	Menu definitions

*/

// quick menu definitions
#define QMENUDEF( name, flags, header ) { flags, header, name##_items, ARRAY_LEN( name##_items ) }
#define QMENUSTUB( name, flags, header ) { flags, header, stubmenu_items, ARRAY_LEN( stubmenu_items ) }

menudef_t ui_menus[M_NUM_MENUS] = {
	{ 0, NULL, NULL, 0 },								// M_NONE

	QMENUDEF( mainmenu, MF_MAINMENU, NULL ),			// M_MAIN
	QMENUDEF( singleplayermenu, 0, "Single Player" ),	// M_SINGLEPLAYER
	QMENUSTUB( multiplayermenu, 0, "Multiplayer" ),		// M_MULTIPLAYER
	QMENUDEF( setupmenu, 0, "Setup" ),					// M_SETUP
#ifdef MISSIONPACK
	QMENUDEF( demosmenu, 0, "Team Arena Demos" ),					// M_DEMOS
	QMENUDEF( cinematicsmenu, 0, "Team Arena Cinematics" ),			// M_CINEMATICS
	QMENUDEF( modsmenu, 0, "Team Arena Mods" ),						// M_MODS
	QMENUDEF( exitmenu, MF_DIALOG|MF_NOBACK, "Quit Team Arena?" ),	// M_EXIT
#else
	QMENUDEF( demosmenu, 0, "Demos" ),						// M_DEMOS
	QMENUDEF( cinematicsmenu, 0, "Cinematics" ),			// M_CINEMATICS
	QMENUDEF( modsmenu, 0, "Mods" ),						// M_MODS
	QMENUDEF( exitmenu, MF_DIALOG|MF_NOBACK, "Exit?" ),		// M_EXIT
#endif

	// Setup menus
	QMENUSTUB( playermenu, 0, "Player Setup" ),			// M_PLAYER
	QMENUSTUB( controlsmenu, 0, "Controls" ),			// M_CONTROLS
	QMENUSTUB( systemmenu, 0, "System" ),				// M_SYSTEM
	QMENUDEF( gameoptionsmenu, 0, "Game Options" ),	// M_GAME_OPTIONS
	QMENUDEF( defaultsmenu, MF_DIALOG|MF_NOBACK, "Set to defaults?" ),	// M_DEFAULTS

	// Team Arena stubs
	QMENUSTUB( rulesmenu, 0, "Rules" ),					// M_RULES
	QMENUSTUB( startservermenu, 0, "Start Server" ),	// M_START_SERVER

	// Team Arena credit menu
	QMENUDEF( creditsmenu, 0, "id Credit" ),				// M_CREDITS_PAGE1
	QMENUDEF( credits2menu,  0, "Additional Credit" ),		// M_CREDITS_PAGE2

	// in game menus
	QMENUDEF( postgamemenu, MF_POSTGAME|MF_NOESCAPE|MF_NOBACK, NULL ), // M_POSTGAME
};

