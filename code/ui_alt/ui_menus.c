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

void launchTeamArena( int item ) {
#ifdef MISSIONPACK
	trap_Cvar_Set( "fs_game", BASEGAME );
#else
	trap_Cvar_Set( "fs_game", BASETA );
#endif
	trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
}

void quitGame( int item ) {
	trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
}

void newGame( int item ) {
	trap_Cvar_SetValue( "ui_singlePlayerActive", 1 );
	trap_Cvar_SetValue( "g_gametype", GT_SINGLE_PLAYER );
	trap_Cmd_ExecuteText( EXEC_APPEND, "map q3dm0\n" );
}

void endGame( int item ) {
	trap_Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
}

menuitem_t mainmenu_items[] =
{
	{ MIF_CENTER|MIF_SUBMENU, "Single Player", (void*)&singleplayermenu, 0 },
	{ MIF_CENTER, "Multiplayer", NULL, 0 },
	{ MIF_CENTER, "Setup", NULL, 0 },
	{ MIF_CENTER, "Demos", NULL, 0 },
	{ MIF_CENTER, "Cinematics", NULL, 0 },
#ifdef MISSIONPACK
	{ MIF_CENTER|MIF_CALL, "Quake III Arena", (void*)launchTeamArena, 0 },
#else
	{ MIF_CENTER|MIF_CALL, "Team Arena", (void*)launchTeamArena, 0 },
#endif
	{ MIF_CENTER, "Mods", NULL, 0 },
	{ MIF_CENTER|MIF_CALL, "Exit", (void*)quitGame, 0 }
};

menuitem_t singleplayermenu_items[] =
{
	{ MIF_CENTER|MIF_CALL, "New Game", (void*)newGame, 0 },
	{ MIF_CENTER|MIF_CALL, "End Game", (void*)endGame, 0 },
	{ MIF_CENTER, "Level Select...", NULL, 0 },
};

menudef_t mainmenu = { MENUTYPE_MAIN, "Main Menu", mainmenu_items, ARRAY_LEN( mainmenu_items ) };
menudef_t singleplayermenu = { MENUTYPE_GENERIC, "Single Player", singleplayermenu_items, ARRAY_LEN( singleplayermenu_items ) };

void UI_InitMenus( void ) {
}

