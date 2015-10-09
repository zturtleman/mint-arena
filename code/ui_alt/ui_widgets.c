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

// ZTM: FIXME: ### randomly writing this here
// sliders are currently invisible (need to port code to 'widget' system)
// listboxs are not centered like they were before I'm not sure why.

// ui_draw.c
void UI_DrawRadioButton( currentMenuItem_t *item, float *x );
void UI_DrawSlider( float x, float y, float min, float max, float value, int style, float *drawcolor, qboolean colorBar );

/*
============================================================================
Common
============================================================================
*/
static void Caption_Init( currentMenuItem_t *item, const char *extData ) {
	if ( item->flags & MIF_BIGTEXT ) {
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
}

static float Caption_Draw( currentMenuItem_t *item, vec4_t drawcolor, int style, qboolean selectionMarker ) {
#ifdef Q3UIFONTS
	if ( item->flags & MIF_BIGTEXT ) {
		UI_DrawProportionalString( item->captionPos.x, item->captionPos.y, item->caption, UI_DROPSHADOW|style, drawcolor );
	} else
#endif
	{
		CG_DrawString( item->captionPos.x, item->captionPos.y, item->caption, UI_DROPSHADOW|style, drawcolor );
	}

	// Q3A bitmap prop font doesn't have this glyph
	if ( selectionMarker && !( item->flags & MIF_BIGTEXT ) ) {
		char cursorStr[2] = { 13, '\0' };
		float x = item->captionPos.x + item->captionPos.width + 2;

#ifdef Q3UIFONTS
		if ( item->flags & MIF_BIGTEXT ) {
			UI_DrawProportionalString( x, item->captionPos.y, cursorStr, UI_BLINK|style, drawcolor );
		} else
#endif
		{
			CG_DrawString( x, item->captionPos.y, cursorStr, UI_BLINK|style, drawcolor );
		}
	}

	return item->captionPos.x + item->captionPos.width + BIGCHAR_WIDTH;
}


/*
============================================================================
Text button or list item with value
============================================================================
*/
static void Generic_Init( currentMenuItem_t *item, const char *extData ) {
	Caption_Init( item, extData );

	item->clickPos.x = 0;
	item->clickPos.y = 0;
	item->clickPos.width = item->captionPos.width;
	item->clickPos.height = item->captionPos.height;
}

static void Generic_Draw( currentMenuItem_t *item, vec4_t drawcolor, int style ) {
	float x;

	x = Caption_Draw( item, drawcolor, style, ( item->cvarName && ( style & UI_PULSE ) ) );

	// draw cvar value
	if ( item->cvarName ) {
		const char *string;

		if ( item->cvarPairs && item->numPairs > 0 ) {
			string = item->cvarPairs[ item->cvarPair ].string;
		} else {
			string = item->vmCvar.string;
		}

		if ( item->widgetType == UIW_RADIO ) {
			UI_DrawRadioButton( item, &x );
		}

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


/*
============================================================================
Bitmap button
============================================================================
*/
static void Bitmap_Init( currentMenuItem_t *item, const char *extData ) {
	item->clickPos.x = 0;
	item->clickPos.y = 0;
	item->clickPos.width = item->captionPos.width;
	item->clickPos.height = item->captionPos.height;
}


/*
============================================================================
Horizontal slider bar
============================================================================
*/
static void Slider_Init( currentMenuItem_t *item, const char *extData ) {
	Caption_Init( item, extData );

	item->clickPos.x = 0;
	item->clickPos.y = 0;
	item->clickPos.width = item->captionPos.width + BIGCHAR_WIDTH + 96;
	item->clickPos.height = MAX( item->captionPos.height, 20 );
}

static void Slider_Draw( currentMenuItem_t *item, vec4_t drawcolor, int style ) {
	qboolean itemSelected = !!( style & UI_PULSE );
	float x;

	x = Caption_Draw( item, drawcolor, style, itemSelected );

	UI_DrawSlider( x, item->captionPos.y,
					item->cvarRange->min, item->cvarRange->max,
					item->vmCvar.value, style, drawcolor, qfalse );
}


/*
============================================================================
Color effects bar
============================================================================
*/
static void ColorBar_Init( currentMenuItem_t *item, const char *extData ) {
	Caption_Init( item, extData );

	item->clickPos.x = 0;
	item->clickPos.y = 0;
	item->clickPos.width = item->captionPos.width + BIGCHAR_WIDTH + 128;
	item->clickPos.height = MAX( item->captionPos.height, 12 );
}

static void ColorBar_Draw( currentMenuItem_t *item, vec4_t drawcolor, int style ) {
	qboolean itemSelected = !!( style & UI_PULSE );
	float x;

	x = Caption_Draw( item, drawcolor, style, itemSelected );

	UI_DrawSlider( x, item->captionPos.y,
					item->cvarRange->min, item->cvarRange->max,
					item->vmCvar.value, style, drawcolor, qtrue );
}


/*
============================================================================
Radio button
============================================================================
*/
static void Radio_Init( currentMenuItem_t *item, const char *extData ) {
	Generic_Init( item, extData );
}

static void Radio_Draw( currentMenuItem_t *item, vec4_t drawcolor, int style ) {
	Generic_Draw( item, drawcolor, style );
}


/*
============================================================================
List Box
============================================================================
*/
static void ListBox_Init( currentMenuItem_t *item, const char *extData ) {
	Caption_Init( item, extData );

#if 0 // caption above list box
	item->clickPos.x = 0;
	item->clickPos.y = item->captionPos.height + 2;
#else // caption left of list box
	item->clickPos.x = item->captionPos.width + BIGCHAR_WIDTH;
	item->clickPos.y = 0;
#endif

	item->clickPos.width = atoi( Info_ValueForKey( extData, "width" ) );
	if ( item->clickPos.width <= 0 ) {
		item->clickPos.width = 280;
	}

	item->clickPos.height = atoi( Info_ValueForKey( extData, "listboxheight" ) );
	if ( item->clickPos.height > 0 ) {
		item->clickPos.height *= BIGCHAR_HEIGHT + 2;
	} else {
		item->clickPos.height = 8 * ( BIGCHAR_HEIGHT + 2 );
	}
}

static void ListBox_Draw( currentMenuItem_t *item, vec4_t drawcolor, int style ) {
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


/*
============================================================================

============================================================================
*/
uiWidget_t ui_widgets[UIW_NUM_WIDGETS] = {
	{ Generic_Init, Generic_Draw, NULL },			// UIW_GENERIC
	{ Bitmap_Init, NULL, NULL },					// UIW_BITMAP, drawing is handled in UI_DrawCurrentMenu
	{ Slider_Init, Slider_Draw, NULL },				// UIW_SLIDER
	{ ColorBar_Init, ColorBar_Draw, NULL },			// UIW_COLORBAR
	{ Radio_Init, Radio_Draw, NULL },				// UIW_RADIO
	{ ListBox_Init, ListBox_Draw, NULL },			// UIW_LISTBOX
};

