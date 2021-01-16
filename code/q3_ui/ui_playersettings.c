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
#include "ui_local.h"

#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"
#define ART_MODEL0			"menu/art/model_0"
#define ART_MODEL1			"menu/art/model_1"
#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"
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

#define ID_NAME			10
#define ID_HANDICAP		11
#define ID_EFFECTS		12
#define ID_BACK			13
#define ID_MODEL		14
#define ID_EFFECTS2		15

#define MAX_NAMELENGTH	20


typedef struct {
	menuframework_s		menu;

	menutext_s			banner;
	menubitmap_s		framel;
	menubitmap_s		framer;
	menubitmap_s		player;

	menufield_s			name;
	menulist_s			handicap;
	menulist_s			effects;
	menulist_s			effects2;

	menubitmap_s		back;
	menubitmap_s		model;
	menubitmap_s		item_null;

	qhandle_t			fxBasePic;
	qhandle_t			fxPic[NUM_COLOR_EFFECTS];
	uiPlayerInfo_t		playerinfo;
	int					current_fx;
	char				playerModel[MAX_QPATH];
	char				playerHead[MAX_QPATH];
	int					localPlayerNum;
	char				bannerString[32];
} playersettings_t;

static playersettings_t	s_playersettings;

static int gamecodetoui[NUM_COLOR_EFFECTS] = {8,4,6,0,10,2,12,1,3,5,7,9,11};
static int uitogamecode[NUM_COLOR_EFFECTS] = {4,8,6,9,2,10,3,11,1,12,5,13,7};

static const char *handicap_items[] = {
	"None",
	"95",
	"90",
	"85",
	"80",
	"75",
	"70",
	"65",
	"60",
	"55",
	"50",
	"45",
	"40",
	"35",
	"30",
	"25",
	"20",
	"15",
	"10",
	"5",
	NULL
};


/*
=================
PlayerSettings_DrawName
=================
*/
static void PlayerSettings_DrawName( void *self ) {
	menufield_s		*f;
	qboolean		focus;
	int				style;
	float			*color;
	int				x, y;
	char			name[32];

	f = (menufield_s*)self;
	x = f->generic.x;
	y = f->generic.y;
	focus = (f->generic.parent->cursor == f->generic.menuPosition);

	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;
	if( focus ) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	UI_DrawProportionalString( x, y, "Name", style, color );

	// draw the actual name
	x += 64;
	y += PROP_HEIGHT;

	if ( focus ) {
		style |= UI_FORCECOLOR;
	}
	UI_MField_Draw( &f->field, x, y, style, colorWhite, focus );

	// draw at bottom also using proportional font
	Q_strncpyz( name, MField_Buffer( &f->field ), sizeof(name) );
	Q_CleanStr( name );
	UI_DrawProportionalString( 320, 440, name, UI_CENTER|UI_BIGFONT, text_color_normal );
}


/*
=================
PlayerSettings_DrawHandicap
=================
*/
static void PlayerSettings_DrawHandicap( void *self ) {
	menulist_s		*item;
	qboolean		focus;
	int				style;
	float			*color;

	item = (menulist_s *)self;
	focus = (item->generic.parent->cursor == item->generic.menuPosition);

	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;
	if( focus ) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	UI_DrawProportionalString( item->generic.x, item->generic.y, "Handicap", style, color );
	UI_DrawProportionalString( item->generic.x + 64, item->generic.y + PROP_HEIGHT, handicap_items[item->curvalue], style, color );
}


/*
=================
PlayerSettings_DrawEffects
=================
*/
static void PlayerSettings_DrawEffects( void *self ) {
	menulist_s		*item;
	qboolean		focus;
	int				focusedID;
	int				style;
	float			*color;
	float			xOffset;
	qhandle_t		colorShader;

	item = (menulist_s *)self;
	focus = (item->generic.parent->cursor == item->generic.menuPosition);
	focusedID = ((menulist_s *)item->generic.parent->items[item->generic.parent->cursor])->generic.id;

	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;
	// header pulses if either color bar is in forcus
	if( item->generic.id == ID_EFFECTS && ( focusedID == ID_EFFECTS || focusedID == ID_EFFECTS2 ) ) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	if ( item->generic.id == ID_EFFECTS ) {
		UI_DrawProportionalString( item->generic.x, item->generic.y, "Effects", style, color );
	}

	xOffset = 128.0f / (NUM_COLOR_EFFECTS + 1);

	CG_DrawPic( item->generic.x + 64, item->generic.y + PROP_HEIGHT + 8, 128, 8, s_playersettings.fxBasePic );

	colorShader = s_playersettings.fxPic[item->curvalue];
	if ( !colorShader ) {
		vec4_t picColor;

		colorShader = s_playersettings.fxPic[NUM_COLOR_EFFECTS-1]; // white
		if ( !colorShader )
			colorShader = uis.whiteShader;

		CG_PlayerColorFromIndex( uitogamecode[item->curvalue], picColor );
		picColor[3] = 1;
		trap_R_SetColor( picColor );
	}
	CG_DrawPic( item->generic.x + 64 + item->curvalue * xOffset + xOffset * 0.5f, item->generic.y + PROP_HEIGHT + 6, 16, 12, colorShader );
	trap_R_SetColor( NULL );

	if ( focus ) {
		float color[4];

		if ( item->curvalue == 2 ) {
			// don't draw yellow on yellow
			color[0] = color[1] = color[2] = 1.0f; // white
		} else {
			VectorCopy(text_color_highlight, color); // yellow
		}

		color[3] = 0.5 + 0.5 * sin( uis.realtime / PULSE_DIVISOR );

		CG_DrawRect( item->generic.x + 64 + item->curvalue * xOffset + xOffset * 0.5f, item->generic.y + PROP_HEIGHT + 6, 16, 12, 2, color );
	}
}


/*
=================
PlayerSettings_DrawPlayer
=================
*/
static void PlayerSettings_DrawPlayer( void *self ) {
	menubitmap_s	*b;
	vec3_t			viewangles;
	char			model[MAX_QPATH], headmodel[MAX_QPATH];

	trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "model"), model, sizeof( model ) );
	trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "headmodel"), headmodel, sizeof ( headmodel ) );
	if ( strcmp( model, s_playersettings.playerModel ) != 0 || strcmp( headmodel, s_playersettings.playerHead ) != 0 ) {
		UI_PlayerInfo_SetModel( &s_playersettings.playerinfo, model, headmodel, NULL );
		strcpy( s_playersettings.playerModel, model );
		strcpy( s_playersettings.playerHead, headmodel );

		viewangles[YAW]   = 180 - 30;
		viewangles[PITCH] = 0;
		viewangles[ROLL]  = 0;
		UI_PlayerInfo_SetInfo( &s_playersettings.playerinfo, s_playersettings.localPlayerNum, LEGS_IDLE, TORSO_STAND, viewangles, vec3_origin, WP_MACHINEGUN, qfalse );
	}

	b = (menubitmap_s*) self;
	UI_DrawPlayer( b->generic.x, b->generic.y, b->width, b->height, &s_playersettings.playerinfo, uis.realtime/2 );
}


/*
=================
PlayerSettings_SaveChanges
=================
*/
static void PlayerSettings_SaveChanges( void ) {
	// name
	trap_Cvar_Set( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "name"),
			MField_Buffer( &s_playersettings.name.field ) );

	// handicap
	trap_Cvar_SetValue( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "handicap"),
			100 - s_playersettings.handicap.curvalue * 5 );

	// effects color
	trap_Cvar_SetValue( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "color1"),
			uitogamecode[s_playersettings.effects.curvalue] );
	trap_Cvar_SetValue( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "color2"),
			uitogamecode[s_playersettings.effects2.curvalue] );
}


/*
=================
PlayerSettings_MenuKey
=================
*/
static sfxHandle_t PlayerSettings_MenuKey( int key ) {
	if( key == K_MOUSE2 || key == K_ESCAPE ) {
		PlayerSettings_SaveChanges();
	}
	return Menu_DefaultKey( &s_playersettings.menu, key );
}


/*
=================
PlayerSettings_SetMenuItems
=================
*/
static void PlayerSettings_SetMenuItems( void ) {
	vec3_t	viewangles;
	int		c;
	int		h;
	char	model[MAX_QPATH], headmodel[MAX_QPATH];

	// name
	MField_SetText( &s_playersettings.name.field, CG_Cvar_VariableString(
			Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "name")) );

	// effects color
	c = trap_Cvar_VariableValue( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "color1") ) - 1;
	if( c < 0 || c > NUM_COLOR_EFFECTS-1 ) {
		c = NUM_COLOR_EFFECTS-1;
	}
	s_playersettings.effects.curvalue = gamecodetoui[c];

	c = trap_Cvar_VariableValue( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "color2") ) - 1;
	if( c < 0 || c > NUM_COLOR_EFFECTS-1 ) {
		c = NUM_COLOR_EFFECTS-1;
	}
	s_playersettings.effects2.curvalue = gamecodetoui[c];

	// model/skin
	memset( &s_playersettings.playerinfo, 0, sizeof(uiPlayerInfo_t) );
	
	viewangles[YAW]   = 180 - 30;
	viewangles[PITCH] = 0;
	viewangles[ROLL]  = 0;

	trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "model"), model, sizeof( model ) );
	trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "headmodel"), headmodel, sizeof ( headmodel ) );

	UI_PlayerInfo_SetModel( &s_playersettings.playerinfo, model, headmodel, NULL );
	UI_PlayerInfo_SetInfo( &s_playersettings.playerinfo, s_playersettings.localPlayerNum, LEGS_IDLE, TORSO_STAND, viewangles, vec3_origin, WP_MACHINEGUN, qfalse );

	// handicap
	h = Com_Clamp( 5, 100, trap_Cvar_VariableValue(Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "handicap")) );
	s_playersettings.handicap.curvalue = 20 - h / 5;
}


/*
=================
PlayerSettings_MenuEvent
=================
*/
static void PlayerSettings_MenuEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_HANDICAP:
		trap_Cvar_Set( Com_LocalPlayerCvarName(s_playersettings.localPlayerNum, "handicap"),
				va( "%i", 100 - 25 * s_playersettings.handicap.curvalue ) );
		break;

	case ID_EFFECTS:
		PlayerSettings_SaveChanges();
		UI_PlayerInfo_UpdateColor( &s_playersettings.playerinfo );
		break;

	case ID_MODEL:
		PlayerSettings_SaveChanges();
		UI_PlayerModelMenu(s_playersettings.localPlayerNum);
		break;

	case ID_BACK:
		PlayerSettings_SaveChanges();
		UI_PopMenu();
		break;
	}
}


/*
=================
PlayerSettings_StatusBar
=================
*/
static void PlayerSettings_StatusBar( void *ptr ) {
	switch( ((menucommon_s*)ptr)->id ) {
	case ID_HANDICAP:
		UI_DrawString( 320, 400, "Lower handicap makes you weaker", UI_CENTER|UI_SMALLFONT, colorWhite );
		UI_DrawString( 320, 420, "giving you more of a challenge", UI_CENTER|UI_SMALLFONT, colorWhite );
		break;
	case ID_EFFECTS:
		UI_DrawString( 320, 410, "Color of railgun core", UI_CENTER|UI_SMALLFONT, colorWhite );
		break;
	case ID_EFFECTS2:
		UI_DrawString( 320, 410, "Color of railgun disks", UI_CENTER|UI_SMALLFONT, colorWhite );
		break;
	default:
		break;
	}
}


/*
=================
PlayerSettings_MenuInit
=================
*/
static void PlayerSettings_MenuInit( int localPlayerNum )
{
	int		y;

	memset(&s_playersettings,0,sizeof(playersettings_t));

	s_playersettings.localPlayerNum = localPlayerNum;
	if (UI_MaxSplitView() == 1) {
		Com_sprintf(s_playersettings.bannerString, sizeof (s_playersettings.bannerString), "PLAYER SETTINGS");
	} else {
		Com_sprintf(s_playersettings.bannerString, sizeof (s_playersettings.bannerString), "PLAYER %d SETTINGS", s_playersettings.localPlayerNum+1);
	}

	PlayerSettings_Cache();

	s_playersettings.menu.key        = PlayerSettings_MenuKey;
	s_playersettings.menu.wrapAround = qtrue;
	s_playersettings.menu.fullscreen = qtrue;

	// Q3 bitmap banner font is missing numbers, fall back to PTEXT
	if (uis.bannerNumbers || UI_MaxSplitView() == 1) {
		s_playersettings.banner.generic.type  = MTYPE_BTEXT;
	} else {
		s_playersettings.banner.generic.type  = MTYPE_PTEXT;
	}
	s_playersettings.banner.generic.flags = QMF_INACTIVE;
	s_playersettings.banner.generic.x     = 320;
	s_playersettings.banner.generic.y     = 16;
	s_playersettings.banner.string        = s_playersettings.bannerString;
	s_playersettings.banner.color         = text_banner_color;
	s_playersettings.banner.style         = UI_CENTER;

	s_playersettings.framel.generic.type  = MTYPE_BITMAP;
	s_playersettings.framel.generic.name  = ART_FRAMEL;
	s_playersettings.framel.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_playersettings.framel.generic.x     = 0;
	s_playersettings.framel.generic.y     = 78;
	s_playersettings.framel.width         = 256;
	s_playersettings.framel.height        = 329;

	s_playersettings.framer.generic.type  = MTYPE_BITMAP;
	s_playersettings.framer.generic.name  = ART_FRAMER;
	s_playersettings.framer.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_playersettings.framer.generic.x     = 376;
	s_playersettings.framer.generic.y     = 76;
	s_playersettings.framer.width         = 256;
	s_playersettings.framer.height        = 334;

	y = 144;
	s_playersettings.name.generic.type			= MTYPE_FIELD;
	s_playersettings.name.generic.flags			= QMF_NODEFAULTINIT;
	s_playersettings.name.generic.ownerdraw		= PlayerSettings_DrawName;
	s_playersettings.name.field.widthInChars	= MAX_NAMELENGTH;
	s_playersettings.name.field.maxchars		= MAX_NAMELENGTH;
	s_playersettings.name.generic.x				= 192;
	s_playersettings.name.generic.y				= y;
	s_playersettings.name.generic.left			= 192 - 8;
	s_playersettings.name.generic.top			= y - 8;
	s_playersettings.name.generic.right			= 192 + 200;
	s_playersettings.name.generic.bottom		= y + 2 * PROP_HEIGHT;

	y += 3 * PROP_HEIGHT;
	s_playersettings.handicap.generic.type		= MTYPE_SPINCONTROL;
	s_playersettings.handicap.generic.flags		= QMF_NODEFAULTINIT;
	s_playersettings.handicap.generic.id		= ID_HANDICAP;
	s_playersettings.handicap.generic.ownerdraw	= PlayerSettings_DrawHandicap;
	s_playersettings.handicap.generic.statusbar = PlayerSettings_StatusBar;
	s_playersettings.handicap.generic.x			= 192;
	s_playersettings.handicap.generic.y			= y;
	s_playersettings.handicap.generic.left		= 192 - 8;
	s_playersettings.handicap.generic.top		= y - 8;
	s_playersettings.handicap.generic.right		= 192 + 200;
	s_playersettings.handicap.generic.bottom	= y + 2 * PROP_HEIGHT;
	s_playersettings.handicap.numitems			= 20;

	y += 3 * PROP_HEIGHT;
	s_playersettings.effects.generic.type		= MTYPE_SPINCONTROL;
	s_playersettings.effects.generic.flags		= QMF_NODEFAULTINIT;
	s_playersettings.effects.generic.id			= ID_EFFECTS;
	s_playersettings.effects.generic.callback	= PlayerSettings_MenuEvent;
	s_playersettings.effects.generic.ownerdraw	= PlayerSettings_DrawEffects;
	s_playersettings.effects.generic.statusbar  = PlayerSettings_StatusBar;
	s_playersettings.effects.generic.x			= 192;
	s_playersettings.effects.generic.y			= y;
	s_playersettings.effects.generic.left		= 192 - 8;
	s_playersettings.effects.generic.top		= y - 8;
	s_playersettings.effects.generic.right		= 192 + 200;
	s_playersettings.effects.generic.bottom		= y + 2* PROP_HEIGHT;
	s_playersettings.effects.numitems			= NUM_COLOR_EFFECTS;

	y += 1 * PROP_HEIGHT;
	s_playersettings.effects2.generic.type		= MTYPE_SPINCONTROL;
	s_playersettings.effects2.generic.flags		= QMF_NODEFAULTINIT;
	s_playersettings.effects2.generic.id		= ID_EFFECTS2;
	s_playersettings.effects2.generic.ownerdraw	= PlayerSettings_DrawEffects;
	s_playersettings.effects2.generic.statusbar  = PlayerSettings_StatusBar;
	s_playersettings.effects2.generic.x			= 192;
	s_playersettings.effects2.generic.y			= y;
	s_playersettings.effects2.generic.left		= 192 - 8;
	s_playersettings.effects2.generic.top		= y - 8;
	s_playersettings.effects2.generic.right		= 192 + 200;
	s_playersettings.effects2.generic.bottom	= y + 2* PROP_HEIGHT;
	s_playersettings.effects2.numitems			= NUM_COLOR_EFFECTS;

	s_playersettings.model.generic.type			= MTYPE_BITMAP;
	s_playersettings.model.generic.name			= ART_MODEL0;
	s_playersettings.model.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_playersettings.model.generic.id			= ID_MODEL;
	s_playersettings.model.generic.callback		= PlayerSettings_MenuEvent;
	s_playersettings.model.generic.x			= 640;
	s_playersettings.model.generic.y			= 480-64;
	s_playersettings.model.width				= 128;
	s_playersettings.model.height				= 64;
	s_playersettings.model.focuspic				= ART_MODEL1;

	s_playersettings.player.generic.type		= MTYPE_BITMAP;
	s_playersettings.player.generic.flags		= QMF_INACTIVE;
	s_playersettings.player.generic.ownerdraw	= PlayerSettings_DrawPlayer;
	s_playersettings.player.generic.x			= 400;
	s_playersettings.player.generic.y			= -40;
	s_playersettings.player.width				= 32*10;
	s_playersettings.player.height				= 56*10;

	s_playersettings.back.generic.type			= MTYPE_BITMAP;
	s_playersettings.back.generic.name			= ART_BACK0;
	s_playersettings.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_playersettings.back.generic.id			= ID_BACK;
	s_playersettings.back.generic.callback		= PlayerSettings_MenuEvent;
	s_playersettings.back.generic.x				= 0;
	s_playersettings.back.generic.y				= 480-64;
	s_playersettings.back.width					= 128;
	s_playersettings.back.height				= 64;
	s_playersettings.back.focuspic				= ART_BACK1;

	s_playersettings.item_null.generic.type		= MTYPE_BITMAP;
	s_playersettings.item_null.generic.flags	= QMF_LEFT_JUSTIFY|QMF_MOUSEONLY|QMF_SILENT;
	s_playersettings.item_null.generic.x		= 0;
	s_playersettings.item_null.generic.y		= 0;
	s_playersettings.item_null.width			= 640;
	s_playersettings.item_null.height			= 480;

	Menu_AddItem( &s_playersettings.menu, &s_playersettings.banner );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.framel );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.framer );

	Menu_AddItem( &s_playersettings.menu, &s_playersettings.name );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.handicap );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.effects );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.effects2 );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.model );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.back );

	Menu_AddItem( &s_playersettings.menu, &s_playersettings.player );

	Menu_AddItem( &s_playersettings.menu, &s_playersettings.item_null );

	PlayerSettings_SetMenuItems();
}


/*
=================
PlayerSettings_Cache
=================
*/
void PlayerSettings_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_MODEL0 );
	trap_R_RegisterShaderNoMip( ART_MODEL1 );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );

	s_playersettings.fxBasePic = trap_R_RegisterShaderNoMip( ART_FX_BASE );
	s_playersettings.fxPic[0] = trap_R_RegisterShaderNoMip( ART_FX_RED );
	s_playersettings.fxPic[1] = trap_R_RegisterShaderNoMip( ART_FX_ORANGE );
	s_playersettings.fxPic[2] = trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
	s_playersettings.fxPic[3] = trap_R_RegisterShaderNoMip( ART_FX_LIME );
	s_playersettings.fxPic[4] = trap_R_RegisterShaderNoMip( ART_FX_GREEN );
	s_playersettings.fxPic[5] = trap_R_RegisterShaderNoMip( ART_FX_VIVIDGREEN );
	s_playersettings.fxPic[6] = trap_R_RegisterShaderNoMip( ART_FX_TEAL );
	s_playersettings.fxPic[7] = trap_R_RegisterShaderNoMip( ART_FX_LIGHTBLUE );
	s_playersettings.fxPic[8] = trap_R_RegisterShaderNoMip( ART_FX_BLUE );
	s_playersettings.fxPic[9] = trap_R_RegisterShaderNoMip( ART_FX_PURPLE );
	s_playersettings.fxPic[10] = trap_R_RegisterShaderNoMip( ART_FX_CYAN );
	s_playersettings.fxPic[11] = trap_R_RegisterShaderNoMip( ART_FX_PINK );
	s_playersettings.fxPic[12] = trap_R_RegisterShaderNoMip( ART_FX_WHITE );
}


/*
=================
UI_PlayerSettingsMenu
=================
*/
void UI_PlayerSettingsMenu( int localPlayerNum ) {
	PlayerSettings_MenuInit( localPlayerNum );
	UI_PushMenu( &s_playersettings.menu );
}
