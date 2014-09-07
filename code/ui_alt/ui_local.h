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

//#define Q3UIFONTS // use banner and proportional fonts

#endif


/*

	Menu Info

*/

#define	MIF_SUBMENU		0x01	// data is menudef_t // on action, change to new menu (adds previous menu to stack)
//#define	MIF_CVAR		0x02	// data is a cvar name.
#define	MIF_CALL		0x04	// data is functionu8_t.
//#define	MIF_CONTROL		0x08	// y is PC_* (PlayerControl Index)
#define	MIF_SPACE		0x10	// Skip line, no action. But still draw text.

//#define	MIF_HIDE		0x20	// Hide MenuItem.
//#define	MIF_CENTER		0x40	// Center x of MenuItem on the screen.
//#define	MIF_YELLOW		0x80	// Text is always yellow.

// "Fake" flags.
//#define	MIF_SETY		(MIF_HIDE|MIF_SPACE)
//#define	MIF_YELLOWSKIP	(MIF_SPACE|MIF_YELLOW)

#define		MIF_POPMENU		0x100	// on action, return to previous menu
#define		MIF_SWAPMENU	0x200	// on action, change to new menu without adding calling menu to stack
#define		MIF_NEXTBUTTON	0x400	// this should have 'next' button graphic and placement. should be last in menudef_t item list.
#define		MIF_HEADER		0x800	// A Team Arena main menu header

#define		MIF_SELECTABLE (MIF_SUBMENU|MIF_CALL|MIF_POPMENU|MIF_SWAPMENU)

typedef struct {
	int flags;
	const char *caption;
	void *data;
	int y;

} menuitem_t;

typedef enum {
	MENUTYPE_GENERIC,
	MENUTYPE_DIALOG,	// yes/no dialog
	MENUTYPE_MAINMENU,	// special main menu handling
	MENUTYPE_DEMOS,
	MENUTYPE_CINEMATICS,
	MENUTYPE_MODS,

} menuType_t;

typedef struct {
	menuType_t	menuType;
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
	void *data;

} currentMenuItem_t;

#define MAX_MENU_DEPTH	20
#define MAX_MENU_ITEMS	64
typedef struct {
	struct {
		menudef_t	*menu;
		int			selectedItem;

	} stack[MAX_MENU_DEPTH];
	int			numStacked;

	menudef_t	*menu;
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
	qhandle_t charsetProp;
	qhandle_t charsetPropGlow;
	qhandle_t charsetPropB;
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
extern menudef_t mainmenu;

// ui_draw.c
void UI_LoadAssets( void );
void UI_BuildCurrentMenu( currentMenu_t *current );
void UI_DrawCurrentMenu( currentMenu_t *current );

// ui_logic.c
void UI_SetMenu( currentMenu_t *current, menudef_t *menu );
void UI_PushMenu( currentMenu_t *current, menudef_t *menu );
void UI_PopMenu( currentMenu_t *current );
void UI_MenuAdjustCursor( currentMenu_t *current, int dir );
void UI_MenuCursorPoint( currentMenu_t *current, int x, int y );
void UI_MenuAction( currentMenu_t *current, int itemNum );


// ui_fonts.c
#ifdef Q3UIFONTS
int UI_BannerStringWidth( const char* str );
void UI_DrawBannerString( int x, int y, const char* str, vec4_t color );
int UI_ProportionalStringWidth( const char* str );
float UI_ProportionalSizeScale( int style );
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
#endif

