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
#define ITEM_ACTION_SOUND	"sound/misc/kcaction.wav" // from TA .menu item action scripts
#define ITEM_FOCUS_SOUND	"sound/misc/menu2.wav" // from TA main.menu itemFocusSound
#define ITEM_WARN_SOUND	"sound/misc/menu4.wav" // FIXME: what sound for TA? does it use this behavior?
#define MENU_POP_SOUND	"sound/misc/tim_pump.wav" // from TA main.menu onOpen script

#define MENU_COPYRIGHT "Quake III: Team Arena Copyright 2000 Id Software, Inc. All rights reserved"

//#define Q3UIFONTS // luls

#else

#define CURSOR_SHADER "menu/art/3_cursor2"
//#define CURSOR_SIZE_SHADER CURSOR_SHADER
//#define CURSOR_SELECT_SHADER CURSOR_SHADER

//#define MAIN_MUSIC		"music/sonic2.wav" // NOTE: Q3 doesn't actually have menu music
#define ITEM_ACTION_SOUND	"sound/misc/menu1.wav" // q3_ui: menu_in_sound
#define ITEM_FOCUS_SOUND	"sound/misc/menu2.wav" // q3_ui: menu_move_sound
#define ITEM_WARN_SOUND	"sound/misc/menu4.wav" // q3_ui: menu_buzz_sound
#define MENU_POP_SOUND	"sound/misc/menu3.wav" // q3_ui: menu_out_sound

#define MENU_COPYRIGHT "Quake III Arena(c) 1999-2000, Id Software, Inc.  All Rights Reserved"

#define Q3UIFONTS // use banner and proportional fonts

#endif

// color bar
#define ART_FX_BASE			"menu/art/fx_base"
#define ART_FX_BLUE			"menu/art/fx_blue"
#define ART_FX_CYAN			"menu/art/fx_cyan"
#define ART_FX_GREEN		"menu/art/fx_grn"
#define ART_FX_RED			"menu/art/fx_red"
#define ART_FX_TEAL			"menu/art/fx_teal"
#define ART_FX_WHITE		"menu/art/fx_white"
#define ART_FX_YELLOW		"menu/art/fx_yel"
#define ART_FX_ORANGE		"menu/art/fx_orange"
#define ART_FX_LIME			"menu/art/fx_lime"
#define ART_FX_VIVIDGREEN	"menu/art/fx_vividgreen"
#define ART_FX_LIGHTBLUE	"menu/art/fx_lightblue"
#define ART_FX_PURPLE		"menu/art/fx_purple"
#define ART_FX_PINK			"menu/art/fx_pink"

#define NUM_COLOR_EFFECTS 13

/*
	List of Widgets
*/
typedef enum {
	UIW_GENERIC, // text button
	UIW_BITMAP, // bitmap button
	UIW_SLIDER,
	UIW_COLORBAR,
	UIW_RADIO,
	UIW_LISTBOX,

	UIW_NUM_WIDGETS
} uiWidgetType_t;

/*
	List of menus
*/
typedef enum {
	M_NONE,

	M_ERROR, // Display com_errorMessage

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
	M_CREDITS,
	M_DEFAULTS,

	// only referenced by team arena right now
	M_RULES,
	M_START_SERVER,

	M_INGAME,
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

#define		MIF_BIGTEXT		0x0080	// use a larger font for this item
#define		MIF_NEXTBUTTON	0x0100	// this itemshould have 'next' button graphic and placement. It should be the last item in list, so arrow keys work correct.
#define		MIF_HEADER		0x0200	// A Team Arena main menu header
#define		MIF_PANEL		0x0400	// Begin a 'tab' aka 'panel' of a menu
#define		MIF_BACKBUTTON	0x0800	// automatically set on back button
#define		MIF_FILELIST	0x1000	// create cvarPairs using file list information in extData

//#define		MIF_CVAR		0x1000
//#define		MIF_CONTROL		0x2000	// y is PC_* (PlayerControl Index)

#define		MIF_SELECTABLE (MIF_SUBMENU|MIF_CALL|MIF_POPMENU|MIF_SWAPMENU|MIF_PANEL)

// cvar value type
typedef enum {
	CVT_NONE,		// terminates list of cvar value pairs
	CVT_INT,
	CVT_FLOAT,
	CVT_STRING,
	CVT_CMD			// command string to execute
} cvarValueType_e;

typedef struct {
	cvarValueType_e	type;
	const char *value;		// cvar value
	const char *string;		// display text
} cvarValuePair_t;

typedef struct {
	float min;
	float max;
	float stepSize;
} cvarRange_t;

typedef struct {
	int flags;
	const char *caption;
	void (*action)(int item); // used for MIF_CALL
	menuId_t menuid; // used for MIF_SUBMENU and MIF_SWAPMENU
	const char *extData; // info string containing extra rarely specified data.
						 // for any: "widget"
						 // for everything: "x" and "y" (in pixels)
						 // for file list: "dir", "ext", "empty"
						 // for list box: "width" (in pixels), "listboxheight" (in number of text lines)

	const char	*cvarName;

	// if cvarPairs is set, item will loop through values and display text from cvarPairs
	// otherwise if cvarRange, it's treated as a slider (min, max, stepSize)
	cvarValuePair_t	*cvarPairs;
	cvarRange_t		*cvarRange;

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
	float x, y, width, height;
} region_t;

typedef struct {
	// copied from menuitem_t (although it's generated for back button)
	int				flags;
	const char		*caption;
	void			(*action)(int item);
	menuId_t		menuid;
	const char		*cvarName;
	cvarRange_t		*cvarRange;
	cvarValuePair_t *cvarPairs;

	// unique
	region_t		captionPos;
	region_t		clickPos;	// bbox for slider or same as caption rect
	//float			x, y, width, height;
	//float			cvarValue;	// if cvarRange->numPairs > 0 it's an index, otherwise it's the clamped value for slider
	vmCvar_t		vmCvar;
	int				cvarPair;
	int				numPairs;
	int				bitmapIndex;

	uiWidgetType_t	widgetType;
#if 0
	union {
		struct {
			qboolean	colorBar;
		} slider;
	} widget;
#endif

} currentMenuItem_t;

#define MAX_MENU_DEPTH	20
#define MAX_MENU_ITEMS	64
typedef struct {
	struct {
		menuId_t	menu;
		int			panel;
		int			selectedItem;

	} stack[MAX_MENU_DEPTH];
	int			numStacked;

	menuId_t	menu;
	int			panel;
	int			selectedItem;
	int			mouseItem; // item mouse points to. -1 if none.
	keyNum_t	mouseClickDown; // for click and drag sliders, mouseItem will be set to item

	// info generated from menudef_t
	currentMenuItem_t	header;
	currentMenuItem_t	items[MAX_MENU_ITEMS];
	int numItems;

	// file list information
	cvarValuePair_t filePairs[1024];
	int		numFilePairs;
	char	fileText[4096];
	int		fileTextLength;

} currentMenu_t;

/*

	Widgets

*/

typedef struct {
	//void	(*cache)(); // called once at UI load
	void	(*init)( currentMenuItem_t *item, const char *extData );
	void	(*draw)( currentMenuItem_t *item, vec4_t color, int style );
	void	(*mouseAction)( currentMenuItem_t *item );

} uiWidget_t;

extern uiWidget_t ui_widgets[UIW_NUM_WIDGETS];

qboolean UI_ItemIsSlider( currentMenuItem_t *item );
qboolean UI_ItemIsRadioButton( currentMenuItem_t *item );


/*

	Main UI system

*/

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
	qhandle_t itemWarnSound;
	qhandle_t menuPopSound;

	qboolean startedMusic;

	int currentResPair; // current index in cp_resolution

} uiStatic_t;

typedef struct {
	qhandle_t menuBackground;
	qhandle_t menuBackgroundNoLogo;
	qhandle_t connectBackground;

#ifdef MISSIONPACK
	qhandle_t menuBackgroundB;
	qhandle_t menuBackgroundC;
	qhandle_t menuBackgroundD;
	qhandle_t menuBackgroundE;
	qhandle_t levelShotDetail;

	qhandle_t gradientBar;
	qhandle_t lightningShader;
#else
	qhandle_t bannerModel;

	qhandle_t frameLeft;
	qhandle_t frameLeftFilled; // player model select menu
	qhandle_t frameRight;
#endif

	qhandle_t sliderBar;
	qhandle_t sliderButton;
	qhandle_t sliderButtonSelected;

	qhandle_t radioButtonOff;
	qhandle_t radioButtonOn;

	qhandle_t dialogSmallBackground;
	qhandle_t dialogLargeBackground;

	qhandle_t fxBasePic;
	qhandle_t fxPic[NUM_COLOR_EFFECTS];

} uiAssets_t;

typedef struct {
	const char	*text;

	// only used for back and next buttons
	int			horizontalPad;
	int			verticialPad;

	const char	*offName;
	int			offWidth;
	int			offHeight;

	const char	*onName;
	int			onWidth;
	int			onHeight;

	// filled in at run time
	qhandle_t	offShader;
	qhandle_t	onShader;
} uiBitmap_t;

extern uiStatic_t uis;
extern uiAssets_t uiAssets;
extern uiBitmap_t ui_bitmaps[];

// ui_main.c
#define MAX_RESOLUTIONS	32
extern cvarValuePair_t cp_resolution[MAX_RESOLUTIONS];
void UI_GetResolutions( void );

// ui_menus.c
extern menudef_t ui_menus[M_NUM_MENUS];

// ui_draw.c
void UI_LoadAssets( void );
void UI_BuildCurrentMenu( currentMenu_t *current );
void UI_DrawCurrentMenu( currentMenu_t *current );
void UI_DrawConnectBackground( void );

// ui_logic.c
typedef enum {
	MACTION_PRESS,
	MACTION_DRAG,
	MACTION_RELEASE,

} mouseActionState_t;

void UI_SetMenu( currentMenu_t *current, menuId_t menu );
void UI_PushMenu( currentMenu_t *current, menuId_t menu );
void UI_PopMenu( currentMenu_t *current );
void UI_MenuAdjustCursor( currentMenu_t *current, int dir );
void UI_MenuCursorPoint( currentMenu_t *current, int x, int y );
void UI_MenuAction( currentMenu_t *current, int itemNum, int dir );
qboolean UI_MenuMouseAction( currentMenu_t *current, int itemNum, int x, int y, mouseActionState_t state );
qboolean UI_MenuItemChangeValue( currentMenu_t *current, int itemNum, int dir );
int UI_NumCvarPairs( cvarValuePair_t *cvarPairs );
void UI_RegisterMenuCvars( currentMenu_t *current );
void UI_UpdateMenuCvars( currentMenu_t *current );
void UI_InitFileList( currentMenu_t *current, currentMenuItem_t *item, const char *extData );


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

side bar could be a 'container menu', though might be best to reference from the menudef_t
other wise keeping track of it in the stack and going to a menu directly is harder.

panel could be an inline horizonal menu referenced from the menu

==========================

thinking about how to handle items have display / edit a console variable

most have a list of possible values, others use slider with min and max, or bool ratio button

typedef struct {
	float	value;
	char *name;
} cvarPossValue_t;

char *cvarName; // hmm, or reference vmCvar_t ???
float value;
float			min, max; // slider
cvarPossValue_t *valueNames;
int				numValueNames;

get range check values from engine? -- would be nice if engine knew bool.

typedef struct {
	float value;
	const char *string;

} cvarValuePair_t;

typedef struct {
	float min, max;
	qboolean intergal;

	cvarValuePair_t *pairs;
	int numPairs;
} cvarRangeInfo_t;

cvarRangeInfo_t *cvarRange;

cvarRangeInfo_t cv_boolRatio = { 0, 1, qtrue, NULL };

vmCvar_t	*cvar;
float		value;	// used for draw / logic, but might be clamped etc which don't want to go into affect without it actually being changed in menu?
cvarRangeInfo_t *cvarRange;
int			index; // index in 



*/

