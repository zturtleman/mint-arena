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

#define MAIN_MUSIC		"music/sonic1.wav"
#define ITEM_ACTION_SOUND	"sound/misc/kcaction.wav"
#define ITEM_FOCUS_SOUND	"sound/misc/menu2.wav" // TA .menu: itemFocusSound
#define MENU_POP_SOUND	"sound/misc/menu3.wav" // FIXME: what sound to play for TA?

#define MENU_COPYRIGHT "Quake III: Team Arena Copyright 2000 Id Software, Inc. All rights reserved"

//#define Q3UIFONTS // luls

#else

#define CURSOR_SHADER "menu/art/3_cursor2"
//#define CURSOR_SIZE_SHADER CURSOR_SHADER
//#define CURSOR_SELECT_SHADER CURSOR_SHADER

//#define MAIN_MUSIC		"music/sonic2.wav" // NOTE: Q3 doesn't actually have menu music
#define ITEM_ACTION_SOUND	"sound/misc/menu1.wav" // q3_ui: menu_in_sound
#define ITEM_FOCUS_SOUND	"sound/misc/menu2.wav" // q3_ui: menu_move_sound
#define MENU_POP_SOUND	"sound/misc/menu3.wav" // q3_ui: menu_out_sound

//	menu_in_sound	= trap_S_RegisterSound( "sound/misc/menu1.wav", qfalse );
//	menu_move_sound	= trap_S_RegisterSound( "sound/misc/menu2.wav", qfalse );
//	menu_out_sound	= trap_S_RegisterSound( "sound/misc/menu3.wav", qfalse );
//	menu_buzz_sound	= trap_S_RegisterSound( "sound/misc/menu4.wav", qfalse );

#define MENU_COPYRIGHT "Quake III Arena(c) 1999-2000, Id Software, Inc.  All Rights Reserved"

#define Q3UIFONTS // use banner and proportional fonts

#endif


/*
	List of menus
*/
typedef enum {
	M_NONE,

	M_MAIN,
	M_SINGLEPLAYER,
	M_MULTIPLAYER,
	M_SETUP,
	M_DEMOS,
	M_CINEMATICS,
	M_MODS,
	M_EXIT, // exit confim dialog

	M_PLAYER,
	M_CONTROLS,
	M_SYSTEM,
	M_GAME_OPTIONS,
	M_DEFAULTS,

	// only referenced by team arena right now
	M_RULES,
	M_START_SERVER,

	M_CREDITS_PAGE1,
	M_CREDITS_PAGE2,

	M_POSTGAME,

	M_NUM_MENUS

} menuId_t;


/*

	Menu Info

*/

// Menu Item Flags
#define		MIF_POPMENU		0x0001	// on action, return to previous menu
#define		MIF_SWAPMENU	0x0002	// on action, change to new menu without adding calling menu to stack
#define		MIF_SUBMENU		0x0004	// on action, change to new menu (adds previous menu to stack)
#define		MIF_CALL		0x0008	// on action, call the item's action function with item number
//#define		MIF_CALLPLAYER	0x0010	// on action, create a menu to select a player (listing Player 1, Player 2, etc) that will call the item's action function with local player number

#define		MIF_NEXTBUTTON	0x0100	// this itemshould have 'next' button graphic and placement. It should be the last item in list, so arrow keys work correct.
#define		MIF_HEADER		0x0200	// A Team Arena main menu header

//#define		MIF_CVAR		0x1000
//#define		MIF_CONTROL		0x2000	// y is PC_* (PlayerControl Index)

#define		MIF_SELECTABLE (MIF_SUBMENU|MIF_CALL|MIF_POPMENU|MIF_SWAPMENU)

typedef struct {
	int flags;
	const char *caption;
	void (*action)(int item); // used for MIF_CALL
	menuId_t menuid; // used for MIF_SUBMENU and MIF_SWAPMENU
	int y;

} menuitem_t;

// Menu Flags
#define		MF_MAINMENU		0x1		// special main menu handling
#define		MF_DIALOG		0x2		// draw dialog box with header (the message text) inside the box and items horizonally before it
#define		MF_NOESCAPE		0x4		// menu must be closed using menu buttons, not escape key
#define		MF_NOBACK		0x8		// don't add back button, but still allows escape key to pop menu -- seems like poor UI tbh
#define		MF_POSTGAME		0x10	// hacks for post game menu... draw items horizonally at the bottom of the screen and no background in-game

typedef struct {
	int			menuFlags;
	const char	*header;
	menuitem_t	*items;
	int			numItems;

} menudef_t;


/*

	The Active Data

*/

typedef struct {
	int flags;
	const char *caption;
	float x, y, width, height;
	void (*action)(int item);
	menuId_t menuid;

} currentMenuItem_t;

#define MAX_MENU_DEPTH	20
#define MAX_MENU_ITEMS	64
typedef struct {
	struct {
		menuId_t	menu;
		int			selectedItem;

	} stack[MAX_MENU_DEPTH];
	int			numStacked;

	menuId_t	menu;
	int			selectedItem;
	int			mouseItem; // item mouse points to. -1 if none.

	// info generated from menudef
	currentMenuItem_t	header;
	currentMenuItem_t	items[MAX_MENU_ITEMS];
	int numItems;

} currentMenu_t;

typedef struct {
	int x, y;
	qhandle_t shader;
	qboolean show;

} cursor_t;

typedef struct {
#ifdef Q3UIFONTS
	fontInfo_t fontProp;
	fontInfo_t fontPropGlow;
	fontInfo_t fontPropB;
#endif

	qhandle_t cursorShader;
	cursor_t cursors[MAX_SPLITVIEW];

	qhandle_t itemActionSound;
	qhandle_t itemFocusSound;
	qhandle_t menuPopSound;

	qboolean startedMusic;

} uiStatic_t;

extern uiStatic_t uis;

// ui_menus.c
extern menudef_t ui_menus[M_NUM_MENUS];

// ui_draw.c
void UI_LoadAssets( void );
void UI_BuildCurrentMenu( currentMenu_t *current );
void UI_DrawCurrentMenu( currentMenu_t *current );

// ui_logic.c
void UI_SetMenu( currentMenu_t *current, menuId_t menu );
void UI_PushMenu( currentMenu_t *current, menuId_t menu );
void UI_PopMenu( currentMenu_t *current );
void UI_MenuAdjustCursor( currentMenu_t *current, int dir );
void UI_MenuCursorPoint( currentMenu_t *current, int x, int y );
void UI_MenuAction( currentMenu_t *current, int itemNum );


// ui_fonts.c
#ifdef Q3UIFONTS
void UI_InitBannerFont( fontInfo_t *font );
int UI_BannerStringWidth( const char* str );
void UI_DrawBannerString( int x, int y, const char* str, int style, vec4_t color );
void UI_InitPropFont( fontInfo_t *font, qboolean glow );
int UI_ProportionalStringWidth( const char* str );
float UI_ProportionalSizeScale( int style );
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
#endif

/*

thinking about how to handle TA main menu

make the horizontal menus be MIF_INLINEMENU
store which item to change to when up/down/left/right are pressed.

==========================

thinking about how to handle TA setup menu
would probably be nice for Q3 as well (especially in wide screen when we for sure have the space any way...)

[1  ]	2
[   ]	
[   ]	
[   ]	
		[3  ]  [  ]  [  ]

1 = side bar
2 = menu content
3 = menu panel select

3 swaps menu
1 swaps menu and panel

side bar could be a 'container menu', though might be best to reference from the menudef
other wise keeping track of it in the stack and going to a menu directly is harder.

panel could be an inline horizonal menu referenced from the menu

*/

