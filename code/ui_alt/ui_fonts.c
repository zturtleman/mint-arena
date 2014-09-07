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
// Q3 banner font and proportional font code

#include "ui_local.h"

#ifdef Q3UIFONTS

#define PROP_SPACE_WIDTH 8
#define PROP_GAP_WIDTH 3

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

#define PROPB_GAP_WIDTH		4
#define PROPB_SPACE_WIDTH	12
#define PROPB_HEIGHT		36

/*
=================
UI_DrawBannerString
=================
*/
static void UI_DrawBannerString2( int x, int y, const char* str, vec4_t color )
{
	const char* s;
	unsigned char	ch;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	agapwidth;
	float	aspacewidth;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	// draw the colored text
	trap_R_SetColor( color );
	
	ax = x;
	ay = y;
	agapwidth = PROPB_GAP_WIDTH;
	ah = PROPB_HEIGHT;
	CG_AdjustFrom640( &ax, &ay, &agapwidth, &ah );

	aspacewidth = PROPB_SPACE_WIDTH;
	CG_AdjustFrom640( NULL, NULL, &aspacewidth, NULL );

	s = str;
	while ( *s )
	{
		ch = toupper( *s & 127 );
		if ( ch == ' ' ) {
			aw = aspacewidth;
		}
		else if ( ch >= 'A' && ch <= 'Z' ) {
			ch -= 'A';
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2];
			CG_AdjustFrom640( NULL, NULL, &aw, NULL );
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, uis.charsetPropB );
		}

		ax += (aw + agapwidth);
		s++;
	}

	trap_R_SetColor( NULL );
}

int UI_BannerStringWidth( const char* str ) {
	const char *	s;
	int				ch;
	int				width;

	// find the width of the drawn text
	s = str;
	width = 0;
	while ( *s ) {
		ch = toupper( *s );
		if ( ch == ' ' ) {
			width += PROPB_SPACE_WIDTH;
		}
		else if ( ch >= 'A' && ch <= 'Z' ) {
			width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
		}
		s++;
	}
	width -= PROPB_GAP_WIDTH;

	return width;
}

void UI_DrawBannerString( int x, int y, const char* str, vec4_t color ) {
	vec4_t			drawcolor;

	if ( 0 ) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawBannerString2( x+2, y+2, str, drawcolor );
	}

	UI_DrawBannerString2( x, y, str, color );
}


int UI_ProportionalStringWidth( const char* str ) {
	const char *	s;
	int				ch;
	int				charWidth;
	int				width;

	s = str;
	width = 0;
	while ( *s ) {
		ch = *s & 127;
		charWidth = propMap[ch][2];
		if ( charWidth != -1 ) {
			width += charWidth;
			width += PROP_GAP_WIDTH;
		}
		s++;
	}

	width -= PROP_GAP_WIDTH;
	return width;
}

static void UI_DrawProportionalString2( int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset )
{
	const char* s;
	unsigned char	ch;
	float	ax;
	float	ay;
	float	aw = 0;
	float	ah;
	float	agapwidth;
	float	aspacewidth;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	// draw the colored text
	trap_R_SetColor( color );
	
	ax = x;
	ay = y;
	agapwidth = PROP_GAP_WIDTH * sizeScale;
	ah = PROP_HEIGHT * sizeScale;
	CG_AdjustFrom640( &ax, &ay, &agapwidth, &ah );

	aspacewidth = PROP_SPACE_WIDTH * sizeScale;
	CG_AdjustFrom640( NULL, NULL, &aspacewidth, NULL );

	s = str;
	while ( *s )
	{
		ch = *s & 127;
		if ( ch == ' ' ) {
			aw = aspacewidth;
		}
		else if ( propMap[ch][2] != -1 ) {
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2] * sizeScale;
			CG_AdjustFrom640( NULL, NULL, &aw, NULL );
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
		}

		ax += (aw + agapwidth);
		s++;
	}

	trap_R_SetColor( NULL );
}

/*
=================
UI_ProportionalSizeScale
=================
*/
float UI_ProportionalSizeScale( int style ) {
	if(  style & UI_SMALLFONT ) {
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
	vec4_t	drawcolor;
	int		width;
	float	sizeScale;

	sizeScale = UI_ProportionalSizeScale( style );

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			width = UI_ProportionalStringWidth( str ) * sizeScale;
			x -= width / 2;
			break;

		case UI_RIGHT:
			width = UI_ProportionalStringWidth( str ) * sizeScale;
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if ( style & UI_DROPSHADOW ) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x+2, y+2, str, drawcolor, sizeScale, uis.charsetProp );
	}

	if ( style & UI_INVERSE ) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, uis.charsetProp );
		return;
	}

	if ( style & UI_PULSE ) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x, y, str, color, sizeScale, uis.charsetProp );

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin( cg.realTime / PULSE_DIVISOR );
		UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, uis.charsetPropGlow );
		return;
	}

	UI_DrawProportionalString2( x, y, str, color, sizeScale, uis.charsetProp );
}

#endif // Q3UIFONTS

