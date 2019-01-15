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
/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"

uiStatic_t		uis;
qboolean		m_entersound;		// after a frame, so caching won't disrupt the sound

/*
=================
UI_MaxSplitView
=================
*/
int UI_MaxSplitView(void) {
	return uis.maxSplitView;
}

/*
=================
UI_PushMenu
=================
*/
void UI_PushMenu( menuframework_s *menu )
{
	int				i;
	menucommon_s*	item;

	// avoid stacking menus invoked by hotkeys
	for (i=0 ; i<uis.menusp ; i++)
	{
		if (uis.stack[i] == menu)
		{
			uis.menusp = i;
			break;
		}
	}

	if (i == uis.menusp)
	{
		if (uis.menusp >= MAX_MENUDEPTH)
			trap_Error("UI_PushMenu: menu stack overflow");

		uis.stack[uis.menusp++] = menu;
	}

	uis.activemenu = menu;

	// default cursor position
	menu->cursor      = 0;
	menu->cursor_prev = 0;

	m_entersound = qtrue;

	Key_SetCatcher( KEYCATCH_UI );

	// force first available item to have focus
	for (i=0; i<menu->nitems; i++)
	{
		item = (menucommon_s *)menu->items[i];
		if (!(item->flags & (QMF_GRAYED|QMF_MOUSEONLY|QMF_INACTIVE)))
		{
			menu->cursor_prev = -1;
			Menu_SetCursor( menu, i );
			break;
		}
	}

	uis.firstdraw = qtrue;
}

/*
=================
UI_PopMenu
=================
*/
void UI_PopMenu (void)
{
	trap_S_StartLocalSound( menu_out_sound, CHAN_LOCAL_SOUND );

	uis.menusp--;

	if (uis.menusp < 0)
		trap_Error ("UI_PopMenu: menu stack underflow");

	if (uis.menusp) {
		uis.activemenu = uis.stack[uis.menusp-1];
		uis.firstdraw = qtrue;
	}
	else {
		UI_ForceMenuOff ();
	}
}

void UI_ForceMenuOff (void)
{
	uis.menusp     = 0;
	uis.activemenu = NULL;

	Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
	trap_Cvar_SetValue( "cl_paused", 0 );
}

/*
=================
UI_DrawProportionalString2
=================
*/
static int	propMap[128][3] = {
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, PROP_SPACE_WIDTH},		// SPACE
{11, 122, 7},	// !
{154, 181, 14},	// "
{55, 122, 17},	// #
{79, 122, 18},	// $
{101, 122, 23},	// %
{153, 122, 18},	// &
{9, 93, 7},		// '
{207, 122, 8},	// (
{230, 122, 9},	// )
{177, 122, 18},	// *
{30, 152, 18},	// +
{85, 181, 7},	// ,
{34, 93, 11},	// -
{110, 181, 6},	// .
{130, 152, 14},	// /

{22, 64, 17},	// 0
{41, 64, 12},	// 1
{58, 64, 17},	// 2
{78, 64, 18},	// 3
{98, 64, 19},	// 4
{120, 64, 18},	// 5
{141, 64, 18},	// 6
{204, 64, 16},	// 7
{162, 64, 17},	// 8
{182, 64, 18},	// 9
{59, 181, 7},	// :
{35,181, 7},	// ;
{203, 152, 14},	// <
{56, 93, 14},	// =
{228, 152, 14},	// >
{177, 181, 18},	// ?

{28, 122, 22},	// @
{5, 4, 18},		// A
{27, 4, 18},	// B
{48, 4, 18},	// C
{69, 4, 17},	// D
{90, 4, 13},	// E
{106, 4, 13},	// F
{121, 4, 18},	// G
{143, 4, 17},	// H
{164, 4, 8},	// I
{175, 4, 16},	// J
{195, 4, 18},	// K
{216, 4, 12},	// L
{230, 4, 23},	// M
{6, 34, 18},	// N
{27, 34, 18},	// O

{48, 34, 18},	// P
{68, 34, 18},	// Q
{90, 34, 17},	// R
{110, 34, 18},	// S
{130, 34, 14},	// T
{146, 34, 18},	// U
{166, 34, 19},	// V
{185, 34, 29},	// W
{215, 34, 18},	// X
{234, 34, 18},	// Y
{5, 64, 14},	// Z
{60, 152, 7},	// [
{106, 151, 13},	// '\'
{83, 152, 7},	// ]
{128, 122, 17},	// ^
{4, 152, 21},	// _

{134, 181, 5},	// '
{5, 4, 18},		// A
{27, 4, 18},	// B
{48, 4, 18},	// C
{69, 4, 17},	// D
{90, 4, 13},	// E
{106, 4, 13},	// F
{121, 4, 18},	// G
{143, 4, 17},	// H
{164, 4, 8},	// I
{175, 4, 16},	// J
{195, 4, 18},	// K
{216, 4, 12},	// L
{230, 4, 23},	// M
{6, 34, 18},	// N
{27, 34, 18},	// O

{48, 34, 18},	// P
{68, 34, 18},	// Q
{90, 34, 17},	// R
{110, 34, 18},	// S
{130, 34, 14},	// T
{146, 34, 18},	// U
{166, 34, 19},	// V
{185, 34, 29},	// W
{215, 34, 18},	// X
{234, 34, 18},	// Y
{5, 64, 14},	// Z
{153, 152, 13},	// {
{11, 181, 5},	// |
{180, 152, 13},	// }
{79, 93, 17},	// ~
{0, 0, -1}		// DEL
};

static int propMapB[26][3] = {
{11, 12, 33},
{49, 12, 31},
{85, 12, 31},
{120, 12, 30},
{156, 12, 21},
{183, 12, 21},
{207, 12, 32},

{13, 55, 30},
{49, 55, 13},
{66, 55, 29},
{101, 55, 31},
{135, 55, 21},
{158, 55, 40},
{204, 55, 32},

{12, 97, 31},
{48, 97, 31},
{82, 97, 30},
{118, 97, 30},
{153, 97, 30},
{185, 97, 25},
{213, 97, 30},

{11, 139, 32},
{42, 139, 51},
{93, 139, 32},
{126, 139, 31},
{158, 139, 25},
};

void UI_InitBannerFont( fontInfo_t *font ) {
	int			i, aw, xSkip;
	float		fcol, frow, fwidth, fheight;
	const char 	*shaderName;
	qhandle_t	hShader;

	shaderName = "menu/art/font2_prop";
	hShader = trap_R_RegisterShaderNoMip( shaderName );

	Q_strncpyz( font->name, "bitmapbannerfont", sizeof ( font->name ) );
	font->glyphScale = 48.0f / PROPB_HEIGHT;

	for ( i = 0; i < GLYPHS_PER_FONT; i++ ) {
		if ( ( i >= 'A' && i <= 'Z' ) || ( i >= 'a' && i <= 'z' ) ) {
			int ch = toupper( i ) - 'A';
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2];
			xSkip = aw + PROPB_GAP_WIDTH;
		} else if ( i == ' ' ) {
			fcol = 0;
			frow = 0;
			fwidth = 0;
			fheight = 0;
			aw = PROPB_SPACE_WIDTH;
			xSkip = PROPB_SPACE_WIDTH;
		} else {
			// does not exist in font
			fcol = 0;
			frow = 0;
			fwidth = 0;
			fheight = 0;
			aw = 0;
			xSkip = 0;
		}

		font->glyphs[i].height = PROPB_HEIGHT;
		font->glyphs[i].top = PROPB_HEIGHT;
		font->glyphs[i].left = 0;
		font->glyphs[i].pitch = aw;
		font->glyphs[i].xSkip = xSkip;
		font->glyphs[i].imageWidth = aw;
		font->glyphs[i].imageHeight = PROPB_HEIGHT;
		font->glyphs[i].s = fcol;
		font->glyphs[i].t = frow;
		font->glyphs[i].s2 = fcol + fwidth;
		font->glyphs[i].t2 = frow + fheight;
		font->glyphs[i].glyph = hShader;
		Q_strncpyz( font->glyphs[i].shaderName, shaderName, sizeof ( font->glyphs[i].shaderName ) );
	}
}

void UI_DrawBannerString( int x, int y, const char* str, int style, vec4_t color ) {
	float			decent;
	int				width;

	// find the width of the drawn text
	width = Text_Width( str, &uis.fontPropB, PROPB_HEIGHT / 48.0f, 0 );

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			x -= width / 2;
			break;

		case UI_RIGHT:
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	//
	// This function expects that y is top of line, text_paint expects at baseline
	//
	decent = -uis.fontPropB.glyphs[(int)'g'].top + uis.fontPropB.glyphs[(int)'g'].height;
	y = y + PROPB_HEIGHT - decent * PROPB_HEIGHT / 48.0f * uis.fontPropB.glyphScale;

	Text_Paint( x, y, &uis.fontPropB, PROPB_HEIGHT / 48.0f, color, str, 0, 0, ( style & UI_DROPSHADOW ) ? 2 : 0, 0, !!( style & UI_FORCECOLOR ), !!( style & UI_INMOTION ) );
}


int UI_ProportionalStringWidth( const char* str ) {
	return Text_Width( str, &uis.fontProp, PROP_HEIGHT / 48.0f, 0 );
}

void UI_InitPropFont( fontInfo_t *font, qboolean glow ) {
	int			i, aw, xSkip;
	float		fcol, frow, fwidth, fheight;
	const char 	*shaderName;
	qhandle_t	hShader;

	if ( glow ) {
		shaderName = "menu/art/font1_prop_glo";
	} else {
		shaderName = "menu/art/font1_prop";
	}
	hShader = trap_R_RegisterShaderNoMip( shaderName );

	Q_strncpyz( font->name, "bitmappropfont", sizeof ( font->name ) );
	font->glyphScale = 48.0f / PROP_HEIGHT;

	for ( i = 0; i < GLYPHS_PER_FONT; i++ ) {
		int ch = i;
		if ( i == ' ' ) {
			fcol = 0;
			frow = 0;
			fwidth = 0;
			fheight = 0;
			aw = PROP_SPACE_WIDTH;
			xSkip = PROP_SPACE_WIDTH;
		}
		else if ( ch < ARRAY_LEN( propMap ) && propMap[ch][2] != -1 ) {
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2];
			xSkip = aw + PROP_GAP_WIDTH;
		} else {
			// does not exist in font
			fcol = 0;
			frow = 0;
			fwidth = 0;
			fheight = 0;
			aw = 0;
			xSkip = 0;
		}

		font->glyphs[i].height = PROP_HEIGHT;
		font->glyphs[i].top = PROP_HEIGHT;
		font->glyphs[i].left = 0;
		font->glyphs[i].pitch = aw;
		font->glyphs[i].xSkip = xSkip;
		font->glyphs[i].imageWidth = aw;
		font->glyphs[i].imageHeight = PROP_HEIGHT;
		font->glyphs[i].s = fcol;
		font->glyphs[i].t = frow;
		font->glyphs[i].s2 = fcol + fwidth;
		font->glyphs[i].t2 = frow + fheight;
		font->glyphs[i].glyph = hShader;
		Q_strncpyz( font->glyphs[i].shaderName, shaderName, sizeof ( font->glyphs[i].shaderName ) );
	}
}

/*
=================
UI_ProportionalSizeScale
=================
*/
float UI_ProportionalSizeScale( int style ) {
	if( ( style & UI_FONTMASK ) == UI_SMALLFONT ) {
		return PROP_SMALL_SIZE_SCALE;
	}

	return 1.00;
}


/*
=================
UI_DrawProportionalString
=================
*/
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color ) {
	float	decent;
	int		glowY;
	vec4_t	drawcolor;
	int		width;
	int		charh;
	float	propScale;
	float	scale;

	if( !str ) {
		return;
	}

	propScale = UI_ProportionalSizeScale( style );

	charh = propScale * PROP_HEIGHT;
	scale = propScale * PROP_HEIGHT / 48.0f;

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			width = UI_ProportionalStringWidth( str ) * propScale;
			x -= width / 2;
			break;

		case UI_RIGHT:
			width = UI_ProportionalStringWidth( str ) * propScale;
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	//
	// This function expects that y is top of line, text_paint expects at baseline
	//
	// glow font
	decent = -uis.fontPropGlow.glyphs[(int)'g'].top + uis.fontPropGlow.glyphs[(int)'g'].height;
	glowY = y + charh - decent * scale * uis.fontPropGlow.glyphScale;
	if ( decent != 0 ) {
		// Make TrueType fonts line up with font1_prop bitmap font which has 4 transparent pixels above glyphs at 16 point font size
		glowY += 3.0f * propScale;
	}

	// normal font
	decent = -uis.fontProp.glyphs[(int)'g'].top + uis.fontProp.glyphs[(int)'g'].height;
	y = y + charh - decent * scale * uis.fontProp.glyphScale;
	if ( decent != 0 ) {
		// Make TrueType fonts line up with font1_prop bitmap font which has 4 transparent pixels above glyphs at 16 point font size
		y += 3.0f * propScale;
	}

	if ( style & UI_INVERSE ) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
		Text_Paint( x, y, &uis.fontProp, scale, drawcolor, str, 0, 0, ( style & UI_DROPSHADOW ) ? 2 : 0, 0, !!( style & UI_FORCECOLOR ), !!( style & UI_INMOTION ) );
		return;
	}

	if ( style & UI_PULSE ) {
		Text_Paint( x, y, &uis.fontProp, scale, color, str, 0, 0, ( style & UI_DROPSHADOW ) ? 2 : 0, 0, !!( style & UI_FORCECOLOR ), !!( style & UI_INMOTION ) );

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin( uis.realtime / PULSE_DIVISOR );
		Text_Paint( x, glowY, &uis.fontPropGlow, scale, drawcolor, str, 0, 0, 0, 0, !!( style & UI_FORCECOLOR ), !!( style & UI_INMOTION ) );
		return;
	}

	Text_Paint( x, y, &uis.fontProp, scale, color, str, 0, 0, ( style & UI_DROPSHADOW ) ? 2 : 0, 0, !!( style & UI_FORCECOLOR ), !!( style & UI_INMOTION ) );
}

/*
=================
UI_DrawProportionalString_Wrapped
=================
*/
void UI_DrawProportionalString_AutoWrapped( int x, int y, int xmax, int ystep, const char* str, int style, vec4_t color ) {
	int width;
	char *s1,*s2,*s3;
	char c_bcp;
	char buf[1024];
	float   sizeScale;

	if (!str || str[0]=='\0')
		return;
	
	sizeScale = UI_ProportionalSizeScale( style );
	
	Q_strncpyz(buf, str, sizeof(buf));
	s1 = s2 = s3 = buf;

	while (1) {
		do {
			s3++;
		} while (*s3!=' ' && *s3!='\0');
		c_bcp = *s3;
		*s3 = '\0';
		width = UI_ProportionalStringWidth(s1) * sizeScale;
		*s3 = c_bcp;
		if (width > xmax) {
			if (s1==s2)
			{
				// fuck, don't have a clean cut, we'll overflow
				s2 = s3;
			}
			*s2 = '\0';
			UI_DrawProportionalString(x, y, s1, style, color);
			y += ystep;
			if (c_bcp == '\0')
      {
        // that was the last word
        // we could start a new loop, but that wouldn't be much use
        // even if the word is too long, we would overflow it (see above)
        // so just print it now if needed
        s2++;
        if (*s2 != '\0') // if we are printing an overflowing line we have s2 == s3
          UI_DrawProportionalString(x, y, s2, style, color);
				break; 
      }
			s2++;
			s1 = s2;
			s3 = s2;
		}
		else
		{
			s2 = s3;
			if (c_bcp == '\0') // we reached the end
			{
				UI_DrawProportionalString(x, y, s1, style, color);
				break;
			}
		}
	}
}

/*
=================
UI_FontForStyle
=================
*/
const fontInfo_t *UI_FontForStyle( int style ) {
	const fontInfo_t *font;

	switch (style & UI_FONTMASK)
	{
		case UI_SMALLFONT:
			font = &uis.smallFont;
			break;

		case UI_BIGFONT:
		default:
			font = &uis.textFont;
			break;

		case UI_CONSOLEFONT:
			font = &cgs.media.consoleFont;
			break;
	}

	return font;
}

/*
=================
UI_DrawString
=================
*/
void UI_DrawString( int x, int y, const char* str, int style, vec4_t color )
{
	CG_DrawStringCommon( x, y, str, style, UI_FontForStyle( style ), color, 0, 0, 0, 0, -1, -1, 0 );
}

/*
=================
UI_DrawStrlen
=================
*/
float UI_DrawStrlen( const char *str, int style )
{
	return CG_DrawStrlenCommon( str, style, UI_FontForStyle( style ), 0 );
}

/*
=================
UI_MField_Draw
=================
*/
void UI_MField_Draw( mfield_t *edit, int x, int y, int style, vec4_t color, qboolean drawCursor ) {
	MField_Draw( edit, x, y, style, UI_FontForStyle( style ), color, drawCursor );
}

/*
=================
UI_DrawChar
=================
*/
void UI_DrawChar( int x, int y, int ch, int style, vec4_t color )
{
	char	buff[2];

	buff[0] = ch;
	buff[1] = '\0';

	UI_DrawString( x, y, buff, style, color );
}

qboolean UI_IsFullscreen( void ) {
	if ( uis.activemenu && ( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return uis.activemenu->fullscreen;
	}

	return qfalse;
}

void UI_SetActiveMenu( uiMenuCommand_t menu ) {
	switch ( menu ) {
	case UIMENU_NONE:
		UI_ForceMenuOff();
		return;
	case UIMENU_MAIN:
		UI_MainMenu();
		return;
	case UIMENU_INGAME:
		trap_Cvar_SetValue( "cl_paused", 1 );
		UI_InGameMenu();
		return;
	case UIMENU_TEAM:
	case UIMENU_POSTGAME:
	default:
		Com_Printf( "UI_SetActiveMenu: unknown enum %d\n", menu );
	}
}

/*
=================
UI_KeyEvent
=================
*/
void UI_KeyEvent( int key, qboolean down ) {
	sfxHandle_t		s;

	if (!uis.activemenu) {
		return;
	}

	if (!down) {
		return;
	}

	if (uis.activemenu->key)
		s = uis.activemenu->key( key );
	else
		s = Menu_DefaultKey( uis.activemenu, key );

	if ((s > 0) && (s != menu_null_sound))
		trap_S_StartLocalSound( s, CHAN_LOCAL_SOUND );
}

/*
=================
UI_MouseEvent
=================
*/
void UI_MouseEvent( int localPlayerNum, int dx, int dy )
{
	float			ax, ay, aw, ah;
	int				xbias, ybias;
	int				i;
	menucommon_s*	m;

	if ( localPlayerNum != 0 ) {
		// q3_ui currently only supports one cursor
		return;
	}

	if (!uis.activemenu)
		return;

	ax = 0;
	ay = 0;
	aw = 1;
	ah = 1;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	xbias = ax/aw;
	ybias = ay/ah;

	// update mouse screen position
	uis.cursorx = Com_Clamp( -xbias, SCREEN_WIDTH+xbias, uis.cursorx + dx );
	uis.cursory = Com_Clamp( -ybias, SCREEN_HEIGHT+ybias, uis.cursory + dy );

	// region test the active menu items
	for (i=0; i<uis.activemenu->nitems; i++)
	{
		m = (menucommon_s*)uis.activemenu->items[i];

		if (m->flags & (QMF_GRAYED|QMF_INACTIVE))
			continue;

		if ((uis.cursorx < m->left) ||
			(uis.cursorx > m->right) ||
			(uis.cursory < m->top) ||
			(uis.cursory > m->bottom))
		{
			// cursor out of item bounds
			continue;
		}

		// set focus to item at cursor
		if (uis.activemenu->cursor != i)
		{
			Menu_SetCursor( uis.activemenu, i );
			((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor_prev]))->flags &= ~QMF_HASMOUSEFOCUS;

			if ( !(((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor]))->flags & QMF_SILENT ) ) {
				trap_S_StartLocalSound( menu_move_sound, CHAN_LOCAL_SOUND );
			}
		}

		((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor]))->flags |= QMF_HASMOUSEFOCUS;
		return;
	}  

	if (uis.activemenu->nitems > 0) {
		// out of any region
		((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor]))->flags &= ~QMF_HASMOUSEFOCUS;
	}
}

/*
=================
UI_GetCursorPos
=================
*/
void UI_GetCursorPos( int localPlayerNum, int *x, int *y )
{
	if (localPlayerNum != 0) {
		// ui currently only supports one cursor
		if ( x ) *x = 0;
		if ( y ) *y = 0;
	}

	if ( x ) *x = uis.cursorx;
	if ( y ) *y = uis.cursory;
}

/*
=================
UI_SetCursorPos
=================
*/
void UI_SetCursorPos( int localPlayerNum, int x, int y )
{
	if (localPlayerNum != 0) {
		// ui currently only supports one cursor
		return;
	}

	if ( uis.cursorx == x && uis.cursory == y ) {
		// ignore duplicate events
		return;
	}

	uis.cursorx = x;
	uis.cursory = y;
	UI_MouseEvent( localPlayerNum, 0, 0 );
}

/*
=================
UI_Cache
=================
*/
void UI_Cache_f( void ) {
	MainMenu_Cache();
	InGame_Cache();
	InSelectPlayer_Cache();
	ConfirmMenu_Cache();
	PlayerModel_Cache();
	PlayerSettings_Cache();
	Controls_Cache();
	UI_Joystick_Cache();
	Demos_Cache();
	UI_CinematicsMenu_Cache();
	Preferences_Cache();
	ServerInfo_Cache();
	SpecifyServer_Cache();
	ArenaServers_Cache();
	StartServer_Cache();
	ServerOptions_Cache();
	DriverInfo_Cache();
	GraphicsOptions_Cache();
	UI_DisplayOptionsMenu_Cache();
	UI_SoundOptionsMenu_Cache();
	UI_NetworkOptionsMenu_Cache();
	UI_SPLevelMenu_Cache();
	UI_SPSkillMenu_Cache();
	UI_SPPostgameMenu_Cache();
	TeamMain_Cache();
	UI_AddBots_Cache();
	UI_RemoveBots_Cache();
	UI_SetupMenu_Cache();
	UI_SelectPlayer_Cache();
//	UI_LoadConfig_Cache();
//	UI_SaveConfigMenu_Cache();
	UI_BotSelectMenu_Cache();
	UI_ModsMenu_Cache();

}


consoleCommand_t	ui_commands[] = {
	{ "iamacheater", UI_SPUnlock_f, 0 },
	{ "iamamonkey", UI_SPUnlockMedals_f, 0 },
	{ "levelselect", UI_SPLevelMenu_f, 0 },
	{ "postgame", UI_SPPostgameMenu_f, CMD_INGAME },
	{ "spdevmap", UI_SPMap_f, 0 },
	{ "spmap", UI_SPMap_f, 0 },
	{ "ui_cache", UI_Cache_f, 0 },
	{ "ui_cinematics", UI_CinematicsMenu_f, 0 },
	{ "ui_teamOrders", UI_TeamOrdersMenu_f, CMD_INGAME }
};

int ui_numCommands = ARRAY_LEN( ui_commands );

/*
=================
UI_ConsoleCommand

update frame time, commands are executed by CG_ConsoleCommand
=================
*/
void UI_ConsoleCommand( int realTime ) {
	uis.frametime = realTime - uis.realtime;
	uis.realtime = realTime;
}

/*
=================
UI_Shutdown
=================
*/
void UI_Shutdown( void ) {
}

/*
=================
UI_Init
=================
*/
void UI_Init( qboolean inGameLoad, int maxSplitView ) {
	uis.maxSplitView = Com_Clamp(1, MAX_SPLITVIEW, maxSplitView);
	
	UI_RegisterCvars();

	UI_InitGameinfo();

	// initialize the menu system
	Menu_Cache();

	uis.activemenu = NULL;
	uis.menusp     = 0;
}

/*
=================
UI_Refresh
=================
*/
void UI_Refresh( int realtime )
{
	uis.frametime = realtime - uis.realtime;
	uis.realtime  = realtime;

	if ( !( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	UI_UpdateCvars();

	if ( uis.activemenu )
	{
		if (uis.activemenu->fullscreen)
		{
			CG_ClearViewport();

			// draw the background
			if( uis.activemenu->showlogo ) {
				CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackShader );
			}
			else {
				CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackNoLogoShader );
			}
		}

		if (uis.activemenu->draw)
			uis.activemenu->draw();
		else
			Menu_Draw( uis.activemenu );

		if( uis.firstdraw ) {
			int i;

			for (i = 0; i < UI_MaxSplitView(); ++i) {
				UI_MouseEvent( i, 0, 0 );
			}
			uis.firstdraw = qfalse;
		}
	}

	// draw cursor
	trap_R_SetColor( NULL );
	CG_DrawPic( uis.cursorx-16, uis.cursory-16, 32, 32, uis.cursor);

	if (uis.debug)
	{
		// cursor coordinates
		UI_DrawString( 0, 0, va("(%d,%d)",uis.cursorx,uis.cursory), UI_LEFT|UI_SMALLFONT, colorRed );
	}

	// delay playing the enter sound until after the
	// menu has been drawn, to avoid delay while
	// caching images
	if (m_entersound)
	{
		trap_S_StartLocalSound( menu_in_sound, CHAN_LOCAL_SOUND );
		m_entersound = qfalse;
	}
}

qboolean UI_CursorInRect (int x, int y, int width, int height)
{
	if (uis.cursorx < x ||
		uis.cursory < y ||
		uis.cursorx > x+width ||
		uis.cursory > y+height)
		return qfalse;

	return qtrue;
}
