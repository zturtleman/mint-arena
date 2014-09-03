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

#include "../cgame/cg_local.h"
#include "../ui/ui_public.h"

/*

	Assets

*/

#ifdef MISSIONPACK

#define CURSOR_SHADER "ui/assets/3_cursor3"

// ZTM: I'm pretty sure these aren't used...
//#define CURSOR_SIZE_SHADER "ui/assets/sizecursor"
//#define CURSOR_SELECT_SHADER "ui/assets/selectcursor"

//#define MAIN_BANNER_MODEL				"models/mapobjects/banner/banner5.md3"
#define MAIN_MUSIC		"music/sonic1.wav"
#define ITEM_ACTION_SOUND	"sound/misc/kcaction.wav"
#define ITEM_FOCUS_SOUND	"sound/misc/menu2.wav" // TA .menu: itemFocusSound
#define MENU_POP_SOUND	"sound/misc/menu3.wav" // FIXME: what sound to play for TA?

#define MENU_COPYRIGHT "Quake III: Team Arena Copyright 2000 Id Software, Inc. All rights reserved"

#else

#define CURSOR_SHADER "menu/art/3_cursor2"
//#define CURSOR_SIZE_SHADER CURSOR_SHADER
//#define CURSOR_SELECT_SHADER CURSOR_SHADER

#define MAIN_BANNER_MODEL				"models/mapobjects/banner/banner5.md3"
//#define MAIN_MUSIC		"music/sonic2.wav" // NOTE: Q3 doesn't actually have menu music
#define ITEM_ACTION_SOUND	"sound/misc/menu1.wav" // q3_ui: menu_in_sound
#define ITEM_FOCUS_SOUND	"sound/misc/menu2.wav" // q3_ui: menu_move_sound
#define MENU_POP_SOUND	"sound/misc/menu3.wav" // q3_ui: menu_out_sound

//	menu_in_sound	= trap_S_RegisterSound( "sound/misc/menu1.wav", qfalse );
//	menu_move_sound	= trap_S_RegisterSound( "sound/misc/menu2.wav", qfalse );
//	menu_out_sound	= trap_S_RegisterSound( "sound/misc/menu3.wav", qfalse );
//	menu_buzz_sound	= trap_S_RegisterSound( "sound/misc/menu4.wav", qfalse );

#define MENU_COPYRIGHT "Quake III Arena(c) 1999-2000, Id Software, Inc.  All Rights Reserved"

#endif


/*

	Menu Info

*/

#define	MIF_SUBMENU		0x01	// data is menudef_t
//#define	MIF_CVAR		0x02	// data is a cvar name.
#define	MIF_CALL		0x04	// data is functionu8_t.
//#define	MIF_CONTROL		0x08	// y is PC_* (PlayerControl Index)
#define	MIF_SPACE		0x10	// Skip line, no action. But still draw text.

#define	MIF_HIDE		0x20	// Hide MenuItem.
#define	MIF_CENTER		0x40	// Center x of MenuItem on the screen.
//#define	MIF_YELLOW		0x80	// Text is always yellow.

// "Fake" flags.
#define	MIF_SETY		(MIF_HIDE|MIF_SPACE)
//#define	MIF_YELLOWSKIP	(MIF_SPACE|MIF_YELLOW)

typedef struct {
	int flags;
	const char *caption;
	void *data;
	int y;

} menuitem_t;

#define MENUTYPE_GENERIC	0
#define MENUTYPE_MAIN		1	// main menu is treated special

typedef struct {
	int			menuType;
	const char	*header;
	menuitem_t	*items;
	int			numItems;

} menudef_t;


/*

	The Active Data

*/

#define MAX_MENU_DEPTH	20
typedef struct {
	struct {
		menudef_t	*menu;
		int			selectedItem;

	} stack[MAX_MENU_DEPTH];
	int			numStacked;

	menudef_t	*menu;
	int			selectedItem;

} currentMenu_t;

typedef struct {
	int x, y;
	qhandle_t shader;
	qboolean show;

} cursor_t;

typedef struct {

	qhandle_t cursorShader;
	cursor_t cursors[MAX_SPLITVIEW];

#ifndef MISSIONPACK
	qhandle_t bannerModel;
#endif

	qhandle_t itemActionSound;
	qhandle_t itemFocusSound;
	qhandle_t menuPopSound;

	qboolean startedMusic;

} uiStatic_t;

extern uiStatic_t uis;

// ui_menus.c
void UI_InitMenus( void );

// ui_draw.c
void UI_DrawCurrentMenu( currentMenu_t *current );

// ui_logic.c
void UI_SetMenu( currentMenu_t *current, menudef_t *menu );
void UI_PushMenu( currentMenu_t *current, menudef_t *menu );
void UI_PopMenu( currentMenu_t *current );
void UI_MenuAdjustCursor( currentMenu_t *current, int dir );
void UI_MenuAction( currentMenu_t *current, int itemNum );



