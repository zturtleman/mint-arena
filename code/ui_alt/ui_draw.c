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

#include "ui_local.h"

vec4_t color_header           = {1.00f, 1.00f, 1.00f, 1.00f};	// bright white
vec4_t color_copyright        = {0.50f, 0.00f, 0.00f, 1.00f};	// dim red
#ifdef MISSIONPACK
vec4_t color_smalltext        = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_smallSelected    = {1.00f, 0.75f, 0.00f, 1.00f};
vec4_t color_bigtext          = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_bigSelected      = {1.00f, 0.75f, 0.00f, 1.00f};
#else
//vec4_t text_color_disabled  = {0.50f, 0.50f, 0.50f, 1.00f};	// light gray
vec4_t color_smalltext        = {1.00f, 0.43f, 0.00f, 1.00f};	// light orange // text_color_normal
vec4_t color_smallSelected    = {1.00f, 1.00f, 0.00f, 1.00f};	// bright yellow // text_color_highlight
vec4_t color_bigtext          = {0.70f, 0.00f, 0.00f, 1.00f};	// bright red with UI_INVERSE effect already applied
vec4_t color_bigSelected      = {1.00f, 0.00f, 0.00f, 1.00f};	// bright red // text_big_color
#endif

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

} uiAssets_t;

// Bitmap buttons
struct ui_bitmap_s {
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
} ui_bitmaps[] = {
#ifdef MISSIONPACK
	{ "Back", 16, 6, "ui/assets/backarrow", 50, 50, "ui/assets/backarrow_alt", 54, 54 },
	{ "Next", 16, 6, "ui/assets/forwardarrow", 50, 50, "ui/assets/forwardarrow_alt", 54, 54 },
#else
	{ "Accept", 0, 0, "menu/art/accept_0", 128, 64, "menu/art/accept_1", 128, 64 },
	{ "Back", 0, 0, "menu/art/back_0", 128, 64, "menu/art/back_1", 128, 64 },
	{ "Create", 0, 0, "menu/art/create_0", 128, 64, "menu/art/create_1", 128, 64 },
	{ "Delete", 0, 0, "menu/art/delete_0", 128, 64, "menu/art/delete_1", 128, 64 },
	{ "Fight", 0, 0, "menu/art/fight_0", 128, 64, "menu/art/fight_1", 128, 64 },
	{ "Load", 0, 0, "menu/art/load_0", 128, 64, "menu/art/load_1", 128, 64 },
	{ "Menu", 0, 0, "menu/art/menu_0", 128, 64, "menu/art/menu_1", 128, 64 },
	{ "Model", 0, 0, "menu/art/model_0", 128, 64, "menu/art/model_1", 128, 64 },
	{ "Next", 0, 0, "menu/art/next_0", 128, 64, "menu/art/next_1", 128, 64 },
	{ "Play", 0, 0, "menu/art/play_0", 128, 64, "menu/art/play_1", 128, 64 },
	{ "Refresh", 0, 0, "menu/art/refresh_0", 128, 64, "menu/art/refresh_1", 128, 64 },
	{ "Replay", 0, 0, "menu/art/replay_0", 128, 64, "menu/art/replay_1", 128, 64 },
	{ "Reset", 0, 0, "menu/art/reset_0", 128, 64, "menu/art/reset_1", 128, 64 },
	{ "Save", 0, 0, "menu/art/save_0", 128, 64, "menu/art/save_1", 128, 64 },
	{ "Skirmish", 0, 0, "menu/art/skirmish_0", 128, 64, "menu/art/skirmish_1", 128, 64 },
	{ "Specify", 0, 0, "menu/art/specify_0", 128, 64, "menu/art/specify_1", 128, 64 },

	// duplicate buttons with different text...
	{ "Play Cinematic", 0, 0, "menu/art/play_0", 128, 64, "menu/art/play_1", 128, 64 },
	{ "Play Demo", 0, 0, "menu/art/play_0", 128, 64, "menu/art/play_1", 128, 64 },
	{ "Load Mod", 0, 0, "menu/art/load_0", 128, 64, "menu/art/load_1", 128, 64 },
#endif
	{ NULL, 0, 0, NULL, 0, 0, NULL, 0, 0 }
};

uiAssets_t uiAssets;

void UI_LoadAssets( void ) {
	int i;

	Com_Memset( &uiAssets, 0, sizeof ( uiAssets ) );

#ifdef MISSIONPACK
	uiAssets.menuBackground = trap_R_RegisterShaderNoMip( "menuback_a" );
	uiAssets.menuBackgroundB = trap_R_RegisterShaderNoMip( "menuback_b" );
	uiAssets.menuBackgroundC = trap_R_RegisterShaderNoMip( "menuback_c" );
	uiAssets.menuBackgroundD = trap_R_RegisterShaderNoMip( "menuback_d" );
	uiAssets.menuBackgroundE = trap_R_RegisterShaderNoMip( "menuback_e" );
	uiAssets.levelShotDetail = trap_R_RegisterShaderNoMip( "levelshotdetail" );
	uiAssets.menuBackgroundNoLogo = uiAssets.menuBackground;
	uiAssets.connectBackground = trap_R_RegisterShaderNoMip( "ui/assets/backscreen" );
	uiAssets.sliderBar = trap_R_RegisterShaderNoMip( "ui/assets/slider2" );
	uiAssets.sliderButton = trap_R_RegisterShaderNoMip( "ui/assets/sliderbutt_1" );
	uiAssets.sliderButtonSelected = uiAssets.sliderButton;
	uiAssets.gradientBar = trap_R_RegisterShaderNoMip( "ui/assets/gradientbar2" );
	uiAssets.lightningShader = trap_R_RegisterShaderNoMip( "lightningkc" );
#else
	uiAssets.bannerModel = trap_R_RegisterModel( "models/mapobjects/banner/banner5.md3" );
	uiAssets.menuBackground = trap_R_RegisterShaderNoMip( "menuback" );
	uiAssets.connectBackground = uiAssets.menuBackground;
	uiAssets.menuBackgroundNoLogo = trap_R_RegisterShaderNoMip( "menubacknologo" );

	uiAssets.frameLeft = trap_R_RegisterShaderNoMip( "menu/art/frame2_l" );
	uiAssets.frameLeftFilled = trap_R_RegisterShaderNoMip( "menu/art/frame1_l" );
	uiAssets.frameRight = trap_R_RegisterShaderNoMip( "menu/art/frame1_r" );
	uiAssets.sliderBar = trap_R_RegisterShaderNoMip( "menu/art/slider2" );
	uiAssets.sliderButton = trap_R_RegisterShaderNoMip( "menu/art/sliderbutt_0" );
	uiAssets.sliderButtonSelected = trap_R_RegisterShaderNoMip( "menu/art/sliderbutt_1" );

	uiAssets.radioButtonOff = trap_R_RegisterShaderNoMip( "menu/art/switch_off" );
	uiAssets.radioButtonOn = trap_R_RegisterShaderNoMip( "menu/art/switch_on" );
#endif

	uiAssets.dialogSmallBackground = trap_R_RegisterShaderNoMip( "menu/art/cut_frame" );
	uiAssets.dialogLargeBackground = trap_R_RegisterShaderNoMip( "menu/art/addbotframe" );

	for ( i = 0; ui_bitmaps[i].text; ++i ) {
		ui_bitmaps[i].offShader = trap_R_RegisterShaderNoMip( ui_bitmaps[i].offName );
		ui_bitmaps[i].onShader = trap_R_RegisterShaderNoMip( ui_bitmaps[i].onName );
	}
}

int UI_FindBitmap( const currentMenuItem_t *item, const char *bitmapName ) {
	int i;

	if ( !item || !bitmapName || !( item->flags & MIF_BIGTEXT ) ) {
		return -1;
	}

	for ( i = 0; ui_bitmaps[i].text; ++i ) {
		if ( Q_stricmp( ui_bitmaps[i].text, bitmapName ) ) {
			continue;
		}

		if ( ui_bitmaps[i].offShader && ui_bitmaps[i].onShader ) {
			return i;
		} else {
			// just fall back to text rendering
			return -1;
		}
	}

	return -1;
}

void UI_DrawConnectBackground( void ) {
	CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uiAssets.connectBackground );
}

void UI_DrawMainMenuBackground( void ) {
#ifdef MISSIONPACK
	CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uiAssets.menuBackground );

	CG_DrawPic( 0, 163, SCREEN_WIDTH, 153, uiAssets.menuBackgroundE ); // light flash
	CG_DrawPic( 0, 163, SCREEN_WIDTH, 153, uiAssets.menuBackgroundB ); // blue
		// FIXME?: menuback_b has bordercolor 0.5 0.5 0.5 .7

	// FIXME: rect 390 163 250 155 cinematic "mpintro.roq" backcolor 1 1 1 .25

	CG_DrawPic( 0, 163, 255, 155, uiAssets.menuBackgroundD ); // flashing team arena text

	CG_DrawPic( 0, 0, SCREEN_WIDTH, 240, uiAssets.levelShotDetail );
	CG_DrawPic( 0, 240, SCREEN_WIDTH, 240, uiAssets.levelShotDetail );

	CG_DrawPic( 205, 123, 235, 235, uiAssets.menuBackgroundC ); // center
#else
	refdef_t		refdef;
	refEntity_t		ent;
	vec3_t			origin;
	vec3_t			angles;
	float			adjust;
	float			x, y, w, h;

	// draw menuback
	CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uiAssets.menuBackground );

	// setup the refdef

	memset( &refdef, 0, sizeof( refdef ) );

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	x = 0;
	y = 0;
	w = 640;
	h = 120;
	CG_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	adjust = 0; // JDC: Kenneth asked me to stop this 1.0 * sin( (float)uis.realtime / 1000 );
	refdef.fov_x = 60 + adjust;
	refdef.fov_y = 19.6875 + adjust;

	refdef.time = cg.realTime;

	origin[0] = 300;
	origin[1] = 0;
	origin[2] = -32;

	trap_R_ClearScene();

	// add the model

	memset( &ent, 0, sizeof(ent) );

	adjust = 5.0 * sin( (float)cg.realTime / 5000 );
	VectorSet( angles, 0, 180 + adjust, 0 );
	AnglesToAxis( angles, ent.axis );
	ent.hModel = uiAssets.bannerModel;
	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent.origin, ent.oldorigin );

	trap_R_AddRefEntityToScene( &ent );

	trap_R_RenderScene( &refdef );
#endif

	CG_DrawString( SCREEN_WIDTH / 2, SCREEN_HEIGHT - SMALLCHAR_HEIGHT*2, MENU_COPYRIGHT, UI_CENTER|UI_SMALLFONT, color_copyright );
}

#ifdef MISSIONPACK
// returns qtrue if any visable mouse cursor is in bounds
qboolean UI_CursorInRect( int x, int y, int width, int height ) {
	int i;

	for ( i = 0; i < MAX_SPLITVIEW; i++ ) {
		if ( uis.cursors[i].show
			&& uis.cursors[i].x >= x && uis.cursors[i].x <= x + width
			&& uis.cursors[i].y >= y && uis.cursors[i].y <= y + height ) {
			// inside rectangle
			return qtrue;
		}
	}

	return qfalse;
}

// TODO: highlight if selected item is in bounds? (doesn't need to literally be based on bounds check)
void UI_DrawGradientBar( int x, int y, int width, int height, qboolean highlight ) {
	vec4_t	baseColor = { 0, 0, 0.75f, 0.5f };
	vec4_t	highlightColor = { 0.75f, 0, 0, 0.5f };
	vec4_t	borderColor = { 0.5f, 0.5f, 0.5f, 0.5f };
	int		borderSize = 2;
	float *color;

	if ( highlight && UI_CursorInRect( x, y, width, height ) ) {
		color = highlightColor;
	} else {
		color = baseColor;
	}

	// draw gradient body
	trap_R_SetColor( color );
	CG_DrawPic( x, y, width, height, uiAssets.gradientBar );

	// draw top and bottom border
	trap_R_SetColor( borderColor );
	CG_DrawPic( x, y-borderSize, width, borderSize, uiAssets.gradientBar );
	CG_DrawPic( x, y+height, width, borderSize, uiAssets.gradientBar );

	trap_R_SetColor( NULL );
}
#endif

// testing 2D polys API...
void UI_BuiltinSliderBar( float x, float y, float width, float height, float *drawcolor ) {
	polyVert_t verts[3];

	CG_AdjustFrom640( &x, &y, &width, &height );

	VectorSet( verts[0].xyz, x, y+height, 0 ); // left lower
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;

	VectorSet( verts[1].xyz, x+width, y, 0 ); // right upper
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;

	VectorSet( verts[2].xyz, x+width, y+height, 0 ); // right upper
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;

	verts[0].modulate[0] = drawcolor[0] * 0xFF;
	verts[0].modulate[1] = drawcolor[1] * 0xFF;
	verts[0].modulate[2] = drawcolor[2] * 0xFF;
	verts[0].modulate[3] = 0;

	verts[1].modulate[0] = drawcolor[0] * 0xFF;
	verts[1].modulate[1] = drawcolor[1] * 0xFF;
	verts[1].modulate[2] = drawcolor[2] * 0xFF;
	verts[1].modulate[3] = 0xE0;

	verts[2].modulate[0] = drawcolor[0] * 0xFF;
	verts[2].modulate[1] = drawcolor[1] * 0xFF;
	verts[2].modulate[2] = drawcolor[2] * 0xFF;
	verts[2].modulate[3] = 0xE0;

	trap_R_Add2dPolys( verts, 3, cgs.media.whiteShader );
}

void UI_BuiltinCircle( float x, float y, float width, float height, float *drawcolor ) {
#define CIRCLE_VERTS	21
	polyVert_t verts[CIRCLE_VERTS];
	int i;
	float horizontalRadius, verticialRadius;

	CG_AdjustFrom640( &x, &y, &width, &height );

	horizontalRadius = width * 0.5f;
	verticialRadius = height * 0.5f;

	// move to center of circle
	x += horizontalRadius;
	y += verticialRadius;

	// full bright point at center
	i = 0;
	verts[i].xyz[0] = x;
	verts[i].xyz[1] = y;
	verts[i].xyz[2] = 0;

	verts[i].st[0] = 0.5f;
	verts[i].st[1] = 0.5f;

	verts[i].modulate[0] = drawcolor[0] * 0xFF;
	verts[i].modulate[1] = drawcolor[1] * 0xFF;
	verts[i].modulate[2] = drawcolor[2] * 0xFF;
	verts[i].modulate[3] = 0xFF;

	for ( i = 1; i < CIRCLE_VERTS; i++ ) {
		float theta = 2.0f * M_PI * (float)(i-1) / (float)(CIRCLE_VERTS-2);

		verts[i].xyz[0] = x + cos( theta ) * horizontalRadius;
		verts[i].xyz[1] = y + sin( theta ) * verticialRadius;
		verts[i].xyz[2] = 0;

		// these are wrong.. but drawing blank image anyway
		verts[i].st[0] = 0.5f + ( verts[i].xyz[0] - x ) / horizontalRadius;
		verts[i].st[1] = 0.5f + ( verts[i].xyz[1] - y ) / verticialRadius;

		verts[i].modulate[0] = drawcolor[0] * 0x40;
		verts[i].modulate[1] = drawcolor[1] * 0x40;
		verts[i].modulate[2] = drawcolor[2] * 0x40;
		verts[i].modulate[3] = 0xFF;
	}

	trap_R_Add2dPolys( verts, CIRCLE_VERTS, cgs.media.whiteShader );
}

void UI_DrawSlider( float x, float y, float min, float max, float value, int style, float *drawcolor ) {
	float frac, sliderValue, buttonX;
	qhandle_t hShader;

	trap_R_SetColor( drawcolor );

	// draw the background
	if ( uiAssets.sliderBar ) {
		CG_DrawPic( x, y, 96, 16, uiAssets.sliderBar );
	} else {
		UI_BuiltinSliderBar( x, y, 96, 16, drawcolor );
	}

	// draw the button
	if ( style & UI_PULSE )
		hShader = uiAssets.sliderButtonSelected;
	else
		hShader = uiAssets.sliderButton;

	// clamp slider value so it doesn't go off when value is out of range
	// but still display the real value
	if ( min > max ) {
		sliderValue = Com_Clamp( max, min, value );
	} else {
		sliderValue = Com_Clamp( min, max, value );
	}

	// position of slider button
	frac = ( sliderValue - min ) / ( max - min );

	// ZTM: NOTE: team arena colorizes it, q3 does not iirc
	buttonX = x + 8 + ( 96 - 16 ) * frac - 2;
	if ( hShader ) {
		CG_DrawPic( buttonX, y - 2, 12, 20, hShader );
	} else {
		UI_BuiltinCircle( buttonX, y - 2, 12, 20, drawcolor );
	}

	trap_R_SetColor( NULL );

	// draw value
#ifdef Q3UIFONTS
	if ( style & UI_GIANTFONT ) {
		UI_DrawProportionalString( x + 96 + 8, y, va("%g", value), style, drawcolor );
	} else
#endif
	{
		CG_DrawString( x + 96 + 8, y, va("%g", value), UI_DROPSHADOW|style, drawcolor );
	}
}

// draw radio button if item should have one
// TODO: Should "is this a radio button" be determined in UI_BuildCurrentMenu instead of checking each item each frame?
static void UI_DrawRadioButton( currentMenuItem_t *item, float *x ) {
	qhandle_t hShader;
	int valuePair0, valuePair1, offPair, picY;

	if ( !item->cvarPairs || item->numPairs != 2 || item->cvarPairs[ 0 ].type != CVT_INT || item->cvarPairs[ 1 ].type != CVT_INT
		|| !uiAssets.radioButtonOff || !uiAssets.radioButtonOn ) {
		// not a radio button or missing shader
		return;
	}

	valuePair0 = atoi( item->cvarPairs[ 0 ].value );
	valuePair1 = atoi( item->cvarPairs[ 1 ].value );

	if ( valuePair0 == 0 && valuePair1 == 1 ) {
		offPair = 0;
	} else if ( valuePair0 == 1 && valuePair1 == 0 ) {
		// reversed case
		offPair = 1;
	} else {
		// not a radio button
		return;
	}

	if ( Q_stricmp( item->cvarPairs[ offPair ].string, "off" ) || Q_stricmp( item->cvarPairs[ !offPair ].string, "on" ) ) {
		// not a radio button
		return;
	}

	hShader = ( item->cvarPair == offPair ) ? uiAssets.radioButtonOff : uiAssets.radioButtonOn;

	// center ratio button on box and move down 2 pixels at 16 pt size
	picY = item->captionPos.y + item->captionPos.height / 2 - 16 / 2
				+ 2.0f * SMALLCHAR_HEIGHT / 16.0f;

	CG_DrawPic( *x, picY, 16, 16, hShader );
	*x += 16;
}

int UI_CvarPairsStringCompare( const void *a, const void *b ) {
	return Q_stricmp( ((const cvarValuePair_t*)a)->string, ((const cvarValuePair_t*)b)->string );
}

void UI_InitListBox( currentMenuItem_t *item, const char *extData ) {
	// ZTM: FIXME: there can only be one file list displayed at a time. They should probably at least be in currentMenu_t?
	#define MAX_LB_FILES 1024
	static cvarValuePair_t filePairs[MAX_LB_FILES];
	static char	fileNames[4096];
	static char	defaultMessage[128];
	char	dirName[MAX_QPATH], extension[MAX_QPATH];
	char	*name;
	int		i, len, file, numFilenames;

	if ( item->cvarPairs ) {
		return;
	}

	Q_strncpyz( dirName, Info_ValueForKey( extData, "dir" ), sizeof (dirName) );
	Q_strncpyz( extension, Info_ValueForKey( extData, "ext" ), sizeof (extension) );
	// defaultMessage cannot be local memory...
	Q_strncpyz( defaultMessage, Info_ValueForKey( extData, "empty" ), sizeof (defaultMessage) );

	numFilenames = trap_FS_GetFileList( dirName, extension, fileNames, sizeof ( fileNames ) );

	// mod list is "gamedir\0description\0"
	if ( !Q_stricmp( dirName, "$modlist" ) ) {
		numFilenames /= 2;
		if (numFilenames > MAX_LB_FILES) {
			numFilenames = MAX_LB_FILES;
		}
		name = fileNames;
		for ( i = 0; i < numFilenames; i++ ) {
			filePairs[i].type = CVT_STRING;

			// gamedir
			filePairs[i].value = name;
			name += strlen( name ) + 1;

			// description
			filePairs[i].string = name;
			name += strlen( name ) + 1;
		}

		// sort list by displayed names
		qsort( filePairs, numFilenames, sizeof ( filePairs[0] ), UI_CvarPairsStringCompare );
	} else {
		if (numFilenames > MAX_LB_FILES) {
			numFilenames = MAX_LB_FILES;
		}
		name = fileNames;
		for ( file = 0, i = 0; file < numFilenames; file++ ) {
			len = strlen( name );

			// special handling for list of directories
			// ZTM: FIXME: directories in pk3dirs do not have slash at end, but from pk3s do. causes duplicates if exist in both
			if ( *extension == '/' ) {
				if ( name[ len - 1 ] == '/' ) {
					name[ len - 1 ] = 0;
				} else if ( Q_stricmp( name, "." ) == 0 || Q_stricmp( name, ".." ) == 0 ) {
					name[0] = '\0';
				}
			} else {
				COM_StripExtension( name, name, len+1 );
			}

			if ( *name ) {
				filePairs[i].type = CVT_STRING;
				filePairs[i].value = name;
				filePairs[i].string = name;
				i++;
			}

			name += len + 1;
		}
	}

	//
	if (!numFilenames) {
		filePairs[numFilenames].type = CVT_STRING;
		filePairs[numFilenames].value = "";
		filePairs[numFilenames].string = defaultMessage;
		numFilenames++;
		item->flags &= ~MIF_SELECTABLE;
	}

	filePairs[numFilenames].type = CVT_NONE;
	filePairs[numFilenames].value = NULL;
	filePairs[numFilenames].string = NULL;

	item->cvarPairs = filePairs;
}

void UI_DrawListBox( currentMenuItem_t *item, float *drawcolor, int style ) {
	int i, x, y, width, height, starty, edgeSize;
	const char *string;
	qboolean selected = ( style & UI_PULSE );

	if ( item->caption && *item->caption ) {
#ifdef Q3UIFONTS
		if ( item->flags & MIF_BIGTEXT ) {
			UI_DrawProportionalString( item->captionPos.x, item->captionPos.y, item->caption, UI_DROPSHADOW|style, drawcolor );
		} else
#endif
		{
			CG_DrawString( item->captionPos.x, item->captionPos.y, item->caption, UI_DROPSHADOW|style, drawcolor );
		}
	}

	CG_DrawRect( item->clickPos.x, item->clickPos.y, item->clickPos.width, item->clickPos.height, 1, drawcolor );

	edgeSize = 2;
	starty = y = item->clickPos.y + edgeSize;
	x = item->clickPos.x + edgeSize;
	width = item->clickPos.width - edgeSize * 2;
	height = item->clickPos.height - edgeSize * 2;

	for ( i = item->cvarPair; i < item->numPairs; i++ ) {
		// highlight line of selected item in list box
		if ( item->cvarPair == i ) {
			vec4_t color;

			VectorCopy( drawcolor, color );
			color[3] = 0.5f;

			trap_R_SetColor( color );
			CG_DrawPic( x, y, width, BIGCHAR_HEIGHT, cgs.media.whiteShader );
			trap_R_SetColor( NULL );
		}

		if ( item->cvarPair == i && selected ) {
			style |= UI_PULSE;
		} else {
			style &= ~UI_PULSE;
		}

		if ( item->cvarPairs && item->numPairs > 0 ) {
			string = item->cvarPairs[ i ].string;
		} else {
			string = item->vmCvar.string;
		}

#ifdef Q3UIFONTS
		if ( item->flags & MIF_BIGTEXT ) {
			UI_DrawProportionalString( x, y, string, UI_DROPSHADOW|style, drawcolor );
		} else
#endif
		{
			CG_DrawString( x, y, string, UI_DROPSHADOW|style, drawcolor );
		}

		y += BIGCHAR_HEIGHT+2;

		if ( y > starty + height ) {
			break;
		}
	}
}

void UI_DrawCurrentMenu( currentMenu_t *current ) {
	int i;
	qboolean drawFramePics = qtrue;
	vec4_t drawcolor;
	int style, panelNum;
	menudef_t	*menuInfo;
	currentMenuItem_t *item;
#ifdef MISSIONPACK
	qboolean	drawStatusBar;
#endif

	if ( !current->menu ) {
		return;
	}

	UI_UpdateMenuCvars( current );

	menuInfo = &ui_menus[current->menu];

	if ( cg.connected ) {
		drawFramePics = qfalse;

		if ( !( menuInfo->menuFlags & ( MF_DIALOG | MF_POSTGAME ) ) ) {
			CG_DrawPic( 320-233, 240-166, 466, 332, uiAssets.dialogLargeBackground );
		}
	} else {
		if ( menuInfo->menuFlags & MF_MAINMENU ) {
			UI_DrawMainMenuBackground();
			drawFramePics = qfalse;
		} else {
			CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uiAssets.menuBackgroundNoLogo );
		}
	}

	if ( menuInfo->menuFlags & MF_DIALOG ) {
		CG_DrawPic( 142, 118, 359, 256, uiAssets.dialogSmallBackground );
	}
	else if ( drawFramePics ) {
#ifndef MISSIONPACK
		CG_DrawPic( 0, 78, 256, 329, uiAssets.frameLeft );
		// CG_DrawPic( 0, 78, 256, 329, uiAssets.frameLeftFilled ); // for player model select menu
		CG_DrawPic( 376, 76, 256, 334, uiAssets.frameRight );
#endif
	}

	// doesn't have a special header handling, use generic
	if ( menuInfo->header ) {
#ifdef MISSIONPACK
		UI_DrawGradientBar( 0, current->header.captionPos.y - 6, SCREEN_WIDTH, current->header.captionPos.height + 12, qfalse );
#endif

#ifdef Q3UIFONTS
		if ( menuInfo->menuFlags & MF_DIALOG ) {
			UI_DrawProportionalString( current->header.captionPos.x, current->header.captionPos.y, menuInfo->header, UI_DROPSHADOW|UI_GIANTFONT, color_bigtext );
		} else {
			UI_DrawBannerString( current->header.captionPos.x, current->header.captionPos.y, menuInfo->header, 0, color_header );
		}
#else
		CG_DrawString( current->header.captionPos.x, current->header.captionPos.y, menuInfo->header, UI_DROPSHADOW|UI_GIANTFONT, color_header );
#endif
	}

#ifdef MISSIONPACK
	// draw status bar background if not dialog and there is a back button (i.e. not a top level menu)
	drawStatusBar = ( !( menuInfo->menuFlags & (MF_DIALOG|MF_NOBACK) ) && current->numStacked > 0 );
	if ( drawStatusBar ) {
		UI_DrawGradientBar( 0, SCREEN_HEIGHT-46, SCREEN_WIDTH, 30, qtrue );
	}
#endif

	panelNum = 0;
	for ( i = 0, item = current->items; i < current->numItems; i++, item++ ) {
		style = 0;

#ifdef MISSIONPACK
		if ( item->flags & MIF_HEADER ) {
			UI_DrawGradientBar( 0, item->captionPos.y, SCREEN_WIDTH, item->captionPos.height, qtrue );
		}
#endif

		if ( item->flags & MIF_BIGTEXT ) {
			Vector4Copy( color_bigtext, drawcolor );
			style |= UI_GIANTFONT;
		} else {
			Vector4Copy( color_smalltext, drawcolor );
			style |= UI_SMALLFONT;
		}

		if ( !( item->flags & MIF_SELECTABLE ) ) {
			VectorScale( drawcolor, 0.7f, drawcolor );
		} else if ( current->selectedItem == i ) {
			style |= UI_PULSE;

			if ( item->flags & MIF_BIGTEXT ) {
				Vector4Copy( color_bigSelected, drawcolor );
			} else {
				Vector4Copy( color_smallSelected, drawcolor );
			}
		}

		if ( item->flags & MIF_PANEL ) {
			panelNum++;

			if ( panelNum == current->panel ) {
				// highlight button for displayed tab
				if ( item->flags & MIF_BIGTEXT ) {
					Vector4Copy( color_bigSelected, drawcolor );
				} else {
					Vector4Copy( color_smallSelected, drawcolor );
				}

				// don't have button for displayed tab pulse
				style &= ~UI_PULSE;
			}
		}

		if ( item->flags & MIF_LISTBOX ) {
			UI_DrawListBox( item, drawcolor, style );
			continue;
		}

		if ( item->bitmapIndex != -1 ) {
			CG_DrawPic( item->captionPos.x, item->captionPos.y, item->captionPos.width, item->captionPos.height, ui_bitmaps[item->bitmapIndex].offShader );

			// if selected draw 'on' image over the top
			// ZTM: FIXME? Team Arena back/next don't need the offShader drawn.
			if ( style & UI_PULSE ) {
				vec4_t color = {1,1,1,1};
				region_t	onImage;

				onImage.width = ui_bitmaps[item->bitmapIndex].onWidth;
				onImage.height = ui_bitmaps[item->bitmapIndex].onHeight;

				// center selected image over the top of unselected image
				// FIXME?: this assumes 'on' image size is >= to 'off' image size
				onImage.x = item->captionPos.x - ( onImage.width - item->captionPos.width ) / 2.0f;
				onImage.y = item->captionPos.y - ( onImage.height - item->captionPos.height ) / 2.0f;

#ifdef MISSIONPACK
				// back and next don't pulse in Team Arena
				// ZTM: FIXME: I don't think there are other bitmap buttons in Team Arena, so could just disable it always in Team Arena?
				if ( !(item->flags & (MIF_BACKBUTTON|MIF_NEXTBUTTON) ) ) {
#endif
					color[3] = 0.5+0.5*sin(cg.realTime/PULSE_DIVISOR);
#ifdef MISSIONPACK
				}
#endif

				trap_R_SetColor( color );
				CG_DrawPic( onImage.x, onImage.y, onImage.width, onImage.height, ui_bitmaps[item->bitmapIndex].onShader );
				trap_R_SetColor( NULL );

#ifdef MISSIONPACK
				// only draw lightning or caption if status bar was drawn
				if ( !drawStatusBar ) {
					continue;
				}

				// if next button is selected draw lightning between back button and caption, and caption and next button
				if ( item->flags & MIF_NEXTBUTTON ) {
					int captionWidth = CG_DrawStrlen( item->caption, style );
					int lightningOverlap = 16; // lightning doesn't do to the edge of the image so overlay onto buttons and caption
					int padSize = ui_bitmaps[item->bitmapIndex].offWidth + ui_bitmaps[item->bitmapIndex].horizontalPad - lightningOverlap;
					int lightningWidth = ( SCREEN_WIDTH - captionWidth ) / 2 - padSize + lightningOverlap;

					CG_DrawPic( padSize, 385, lightningWidth, 128, uiAssets.lightningShader );
					CG_DrawPic( SCREEN_WIDTH - lightningWidth - padSize, 385, lightningWidth, 128, uiAssets.lightningShader );
				}

				// if it's a back or next button, display the caption
				if ( item->flags & (MIF_BACKBUTTON|MIF_NEXTBUTTON) ) {
					// this has white text, instead of selected color
					// TODO: use unselected small/big color?
					CG_DrawString( SCREEN_WIDTH / 2, onImage.y + onImage.height / 2 - GIANTCHAR_HEIGHT / 2, item->caption, UI_CENTER|UI_DROPSHADOW|style, colorWhite );
				}
#endif
			}
		} else
#ifdef Q3UIFONTS
		if ( item->flags & MIF_BIGTEXT ) {
			UI_DrawProportionalString( item->captionPos.x, item->captionPos.y, item->caption, UI_DROPSHADOW|style, drawcolor );
		} else
#endif
		{
			CG_DrawString( item->captionPos.x, item->captionPos.y, item->caption, UI_DROPSHADOW|style, drawcolor );
		}

		// draw cvar value
		if ( item->cvarName ) {
			float x;
			const char *string;

			// Q3A bitmap prop font doesn't have this glyph
			if ( current->selectedItem == i && !( item->flags & MIF_BIGTEXT ) ) {
				char cursorStr[2] = { 13, '\0' };

				x = item->captionPos.x + item->captionPos.width + 2;

#ifdef Q3UIFONTS
				if ( item->flags & MIF_BIGTEXT ) {
					UI_DrawProportionalString( x, item->captionPos.y, cursorStr, UI_BLINK|style, drawcolor );
				} else
#endif
				{
					CG_DrawString( x, item->captionPos.y, cursorStr, UI_BLINK|style, drawcolor );
				}
			}

			// x position for cvar value
			x = item->captionPos.x + item->captionPos.width + BIGCHAR_WIDTH;

			if ( UI_ItemIsSlider( item ) ) {
				UI_DrawSlider( x, item->captionPos.y,
						item->cvarRange->min, item->cvarRange->max,
						item->vmCvar.value, style, drawcolor );
				continue;
			}

			if ( item->cvarPairs && item->numPairs > 0 ) {
				string = item->cvarPairs[ item->cvarPair ].string;
			} else {
				string = item->vmCvar.string;
			}

			UI_DrawRadioButton( item, &x );

			// HACK for M_ERROR com_errorMessage cvar centering
			if ( !item->caption && !( item->flags & MIF_SELECTABLE ) ) {
				style = UI_CENTER;
				x = 320;
			}

#ifdef Q3UIFONTS
			if ( item->flags & MIF_BIGTEXT ) {
				UI_DrawProportionalString( x, item->captionPos.y, string, UI_DROPSHADOW|style, drawcolor );
			} else
#endif
			{
				CG_DrawString( x, item->captionPos.y, string, UI_DROPSHADOW|style, drawcolor );
			}
		}
	}
}

// big text menu items (q3 main menu / setup menu) have extra vertical gap
static int UI_ItemVerticalGap( currentMenuItem_t *item ) {
	if ( ( item->flags & MIF_BIGTEXT ) && !( item->flags & (MIF_PANEL|MIF_HEADER) ) ) {
		return 7;
	} else {
		return 2;
	}
}

// TODO: Fix font heights in UI_BuildCurrentMenu with Q3UIFONTS defined
#define MAX_MENU_HEADERS	8
// this is used for drawing and logic. it's closely related to UI_DrawCurrentMenu.
void UI_BuildCurrentMenu( currentMenu_t *current ) {
	int i, horizontalGap = BIGCHAR_WIDTH;
	int	numHeaders = 0, totalWidth[MAX_MENU_HEADERS] = {0}, totalHeight = 0, totalPanelHeight = 0;
	int panelItemX, panelItemY, itemX, itemY, curX, curY;
	int headerBottom = 0;
	qboolean horizontalMenu = qfalse;
	menudef_t	*menuInfo;
	menuitem_t	*itemInfo;
	currentMenuItem_t *item;
	int panelNum;

	if ( !current->menu ) {
		return;
	}

	menuInfo = &ui_menus[current->menu];

	if ( menuInfo->header ) {
#ifdef Q3UIFONTS // ug, dialogs need to use prop font
		if ( menuInfo->menuFlags & MF_DIALOG ) {
			current->header.captionPos.width = UI_ProportionalStringWidth( menuInfo->header );
			current->header.captionPos.height = PROP_HEIGHT;
		} else {
			current->header.captionPos.width = UI_BannerStringWidth( menuInfo->header );
			current->header.captionPos.height = 36; // ZTM: FIXME: Put PROPB_HEIGHT in a header
		}
#else
		current->header.captionPos.width = CG_DrawStrlen( menuInfo->header, UI_GIANTFONT );
		current->header.captionPos.height = BIGCHAR_HEIGHT;
#endif
		current->header.captionPos.x = ( SCREEN_WIDTH - current->header.captionPos.width ) / 2;
		current->header.captionPos.y = 16; // ZTM: banner.generic.y is always 16

		// ZTM: Disabled, verticial center in Q3A ignores header
		//headerBottom = current->header.captionPos.y + current->header.captionPos.height;
	} else {
		Com_Memset( &current->header, 0, sizeof ( current->header ) );
	}

/* ZTM: Using fixed Y in main menu item def now
#ifndef MISSIONPACK
	if ( !cg.connected && ( menuInfo->menuFlags & MF_MAINMENU ) ) {
		// Q3 banner model
		headerBottom = 120;
	}
#endif
*/

	if ( menuInfo->menuFlags & MF_DIALOG ) {
		current->header.captionPos.y = 204;
		headerBottom = 265;
		horizontalMenu = qtrue;
	} else if ( menuInfo->menuFlags & MF_POSTGAME ) {
		horizontalMenu = qtrue;
#ifdef Q3UIFONTS
		headerBottom = SCREEN_HEIGHT - PROP_HEIGHT - 10;
#else
		headerBottom = SCREEN_HEIGHT - BIGCHAR_HEIGHT - 10;
#endif
	}

	//
	// Convert item info
	//
	item = current->items;
	itemInfo = menuInfo->items;
	panelNum = 0;
	for ( i = 0, current->numItems = 0; i < menuInfo->numItems; i++, itemInfo++ ) {
		item->flags = itemInfo->flags;
		item->action = itemInfo->action;
		item->menuid = itemInfo->menuid;
		item->cvarName = itemInfo->cvarName;
		item->cvarRange = itemInfo->cvarRange;
		item->cvarPairs = itemInfo->cvarPairs;
		item->numPairs = 0; // this is set later
		item->caption = itemInfo->caption;
		item->captionPos.y = atoi( Info_ValueForKey( itemInfo->extData, "y" ) );
		item->captionPos.x = atoi( Info_ValueForKey( itemInfo->extData, "x" ) );
		item->bitmapIndex = -1;

		if ( item->flags & MIF_PANEL ) {
			panelNum++;
		} else {
			// if not part of current panel, don't show it
			if ( panelNum > 0 && panelNum != current->panel )
			{
				continue;
			}
		}

		if ( item->flags & MIF_LISTBOX ) {
			UI_InitListBox( item, itemInfo->extData );
		}
#ifdef MISSIONPACK
		else if ( item->flags & MIF_NEXTBUTTON ) {
			item->bitmapIndex = UI_FindBitmap( item, "Next" );
		}
#endif
		else
		{
			item->bitmapIndex = UI_FindBitmap( item, item->caption );
		}

		if ( item->bitmapIndex != -1 ) {
			item->captionPos.width = ui_bitmaps[item->bitmapIndex].offWidth;
			item->captionPos.height = ui_bitmaps[item->bitmapIndex].offHeight;
		} else if ( item->flags & MIF_BIGTEXT ) {
#ifdef Q3UIFONTS
			item->captionPos.width = UI_ProportionalStringWidth( item->caption );
			item->captionPos.height = PROP_HEIGHT;
#else
			item->captionPos.width = CG_DrawStrlen( item->caption, UI_GIANTFONT );
			item->captionPos.height = GIANTCHAR_HEIGHT;
#endif
		} else {
			item->captionPos.width = CG_DrawStrlen( item->caption, UI_SMALLFONT );
			item->captionPos.height = SMALLCHAR_HEIGHT;
		}

		if ( item->flags & MIF_LISTBOX ) {
#if 0 // caption above list box
			item->clickPos.x = 0;
			item->clickPos.y = item->captionPos.height + 2;
#else // caption left of list box
			item->clickPos.x = item->captionPos.width + BIGCHAR_WIDTH;
			item->clickPos.y = 0;
#endif

			item->clickPos.width = atoi( Info_ValueForKey( itemInfo->extData, "width" ) );
			if ( item->clickPos.width <= 0 ) {
				item->clickPos.width = 280;
			}

			item->clickPos.height = atoi( Info_ValueForKey( itemInfo->extData, "listboxheight" ) );
			if ( item->clickPos.height > 0 ) {
				item->clickPos.height *= BIGCHAR_HEIGHT + 2;
			} else {
				item->clickPos.height = 8 * ( BIGCHAR_HEIGHT + 2 );
			}
		} else if ( UI_ItemIsSlider( item ) ) {
			item->clickPos.x = 0;
			item->clickPos.y = 0;
			item->clickPos.width = item->captionPos.width + BIGCHAR_WIDTH + 96;
			item->clickPos.height = 20;
		} else {
			item->clickPos.x = 0;
			item->clickPos.y = 0;
			item->clickPos.width = item->captionPos.width;
			item->clickPos.height = item->captionPos.height;
		}

		if ( item->flags & MIF_NEXTBUTTON ) {
			// Fixed place in lower right
			item->captionPos.x = SCREEN_WIDTH - item->captionPos.width;
			item->captionPos.y = SCREEN_HEIGHT - item->captionPos.height;

			if ( item->bitmapIndex != -1 ) {
				item->captionPos.y -= ui_bitmaps[item->bitmapIndex].verticialPad;
				item->captionPos.x -= ui_bitmaps[item->bitmapIndex].horizontalPad;
			} else {
				// move away from the screen edges
				item->captionPos.x -= 10;
				item->captionPos.y -= 10;
			}

			item->clickPos.x = item->captionPos.x;
			item->clickPos.y = item->captionPos.y;
		}

		item++;
		current->numItems++;
	}

	//
	// Calculate total width/height
	//
	numHeaders = 0;
	for ( i = 0, item = current->items; i < current->numItems; i++, item++ ) {
		if ( item->flags & MIF_NEXTBUTTON ) {
			// Fixed place in lower right
			continue;
		}

		if ( item->flags & MIF_HEADER ) {
			numHeaders++;

			if ( !( item->flags & MIF_SELECTABLE ) ) {
				continue;
			}
		}

		totalWidth[numHeaders] += MAX( item->captionPos.width, item->clickPos.x + item->clickPos.width );
		if ( i != current->numItems - 1 && !(current->items[i+1].flags & MIF_HEADER) ) {
			totalWidth[numHeaders] += horizontalGap;
		}

		// FIXME: this macro usage is bad (multiple function calls)
		if ( item->flags & MIF_PANEL ) {
			totalPanelHeight += MAX( item->captionPos.height + UI_ItemVerticalGap( item ), item->clickPos.y + item->clickPos.height );
		} else {
			totalHeight += MAX( item->captionPos.height + UI_ItemVerticalGap( item ), item->clickPos.y + item->clickPos.height );
		}
	}

	//
	// Set item x and y
	//
	numHeaders = 0;
	panelItemX = -1;
	panelItemY = -1;
	itemX = -1;
	itemY = -1;
	for ( i = 0, item = current->items; i < current->numItems; i++, item++ ) {
		int sectionHeight;
		qboolean centerX;

		if ( item->flags & MIF_NEXTBUTTON ) {
			// Fixed place in lower right
			continue;
		}

		centerX = !horizontalMenu;

		// panel tabs/buttons are separate from list items
		if ( item->flags & MIF_PANEL ) {
			curX = panelItemX;
			curY = panelItemY;
			sectionHeight = totalPanelHeight;
			centerX = qfalse; // left aligned
		} else {
			curX = itemX;
			curY = itemY;
			sectionHeight = totalHeight;
		}

		if ( item->flags & MIF_HEADER ) {
			numHeaders++;
			centerX = qtrue;
		} else if ( numHeaders ) {
			centerX = qfalse;
		}

		if ( item->captionPos.y != 0 ) {
			curY = item->captionPos.y;
		} else if ( item->flags & MIF_HEADER ) {
			if ( curY == -1 )
				curY = headerBottom;
			else
				curY += item->captionPos.height + UI_ItemVerticalGap( item );
		} else if ( horizontalMenu && curY == -1 ) {
			curY = headerBottom;
		} else if ( curY == -1 ) {
			// center Y
			curY = headerBottom + (SCREEN_HEIGHT - headerBottom - sectionHeight) / 2;
		}

		if ( item->flags & MIF_PANEL ) {
			// right align
			curX = 216 - item->captionPos.width;
		} if ( !panelNum && item->cvarName && centerX && !(item->flags & MIF_LISTBOX) ) {
			// right align HACK for M_GAME_OPTIONS
			curX = SCREEN_WIDTH / 2 + totalWidth[0] / menuInfo->numItems / 6 - item->captionPos.width;
		} else if ( horizontalMenu && curX == -1 ) {
			// center line
			curX = (SCREEN_WIDTH - totalWidth[numHeaders]) / 2;
		} else if ( centerX ) {
			// center item
			curX = (SCREEN_WIDTH - MAX( item->captionPos.width, item->clickPos.x + item->clickPos.width ) ) / 2;

			// ZTM: TODO: properly deal with changing the center x?
			if ( panelNum ) {
				curX = 216 + 12;
			}
		}

		// uses absolute position because x/y might already be set to absolute
		item->captionPos.x = curX;
		item->captionPos.y = curY;

		// clickable position is relative
		item->clickPos.x += curX;
		item->clickPos.y += curY;

		// move draw point for next item
		// ZTM: FIXME: what the hell have I done.
		if ( ( horizontalMenu && !numHeaders ) || ( numHeaders && !( item->flags & MIF_HEADER ) ) ) {
			curX += MAX( item->captionPos.width, item->clickPos.x - item->captionPos.x + item->clickPos.width ) + horizontalGap;
		} else {
			curY += MAX( item->captionPos.height, item->clickPos.y - item->captionPos.y + item->clickPos.height ) + UI_ItemVerticalGap( item );
		}

		if ( item->flags & MIF_HEADER ) {
			// center next line
			curX = (SCREEN_WIDTH - totalWidth[numHeaders]) / 2;
		}

		if ( item->flags & MIF_PANEL ) {
			panelItemX = curX;
			panelItemY = curY;
		} else {
			itemX = curX;
			itemY = curY;
		}
	}

	// add back button
	if ( current->numStacked && !( menuInfo->menuFlags & MF_NOBACK ) ) {
		item = &current->items[current->numItems];
		item->flags = MIF_BIGTEXT|MIF_POPMENU|MIF_BACKBUTTON;
		item->action = NULL;
		item->menuid = M_NONE;
		item->cvarName = NULL;
		item->cvarRange = NULL;
		item->cvarPairs = NULL;
		item->numPairs = 0;
		item->bitmapIndex = UI_FindBitmap( item, "Back" );

#ifdef MISSIONPACK
		if ( current->numStacked == 1 )
			item->caption = "Exit to Main Menu";
		else
#endif
		item->caption = "Back";

		if ( item->bitmapIndex != -1 ) {
			item->captionPos.width = ui_bitmaps[item->bitmapIndex].offWidth;
			item->captionPos.height = ui_bitmaps[item->bitmapIndex].offHeight;

			item->captionPos.y = SCREEN_HEIGHT - item->captionPos.height - ui_bitmaps[item->bitmapIndex].verticialPad;
			item->captionPos.x = ui_bitmaps[item->bitmapIndex].horizontalPad;
		} else {
			// note: assumes MIF_BIGTEXT
#ifdef Q3UIFONTS
			item->captionPos.width = UI_ProportionalStringWidth( item->caption );
			item->captionPos.height = PROP_HEIGHT;
#else
			item->captionPos.width = CG_DrawStrlen( item->caption, UI_GIANTFONT );
			item->captionPos.height = GIANTCHAR_HEIGHT;
#endif

			item->captionPos.y = SCREEN_HEIGHT - item->captionPos.height - 10;
			item->captionPos.x = 10;
		}

		item->clickPos.x = item->captionPos.x;
		item->clickPos.y = item->captionPos.y;
		item->clickPos.width = item->captionPos.width;
		item->clickPos.height = item->captionPos.height;

		i++;
		current->numItems++;
	}

	//
	// setup cvar value name indexes
	//
	UI_RegisterMenuCvars( current );
}

