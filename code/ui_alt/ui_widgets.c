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

void UI_DrawRadioButton( currentMenuItem_t *item, float *x );
void UI_DrawSlider( currentMenuItem_t *item, float x, float y, int style, float *drawcolor, qboolean colorBar );

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
	const char *value;
	float x;

	if ( item->cvarPairs && item->numPairs > 0 ) {
		value = item->cvarPairs[ item->cvarPair ].string;
	} else if ( item->cvarName ) {
		value = item->vmCvar.string;
	} else {
		value = NULL;
	}

	x = Caption_Draw( item, drawcolor, style, ( value && ( style & UI_PULSE ) ) );

	if ( !value ) {
		return;
	}

	if ( item->widgetType == UIW_RADIO ) {
		UI_DrawRadioButton( item, &x );
	}

	// HACK for M_ERROR com_errorMessage cvar centering
	if ( !item->caption && !( item->flags & MIF_SELECTABLE ) ) {
		style = UI_CENTER;
		x = 320;
	}

	// draw cvar value
#ifdef Q3UIFONTS
	if ( item->flags & MIF_BIGTEXT ) {
		UI_DrawProportionalString( x, item->captionPos.y, value, UI_DROPSHADOW|style, drawcolor );
	} else
#endif
	{
		CG_DrawString( x, item->captionPos.y, value, UI_DROPSHADOW|style, drawcolor );
	}
}


/*
============================================================================
Bitmap button
============================================================================
*/
static void Bitmap_Init( currentMenuItem_t *item, const char *extData ) {
	item->captionPos.width = ui_bitmaps[item->bitmapIndex].offWidth;
	item->captionPos.height = ui_bitmaps[item->bitmapIndex].offHeight;

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
static void UI_BuiltinSliderBar( float x, float y, float width, float height, float *drawcolor ) {
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

static void UI_BuiltinCircle( float x, float y, float width, float height, float *drawcolor ) {
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

void UI_DrawSlider( currentMenuItem_t *item, float x, float y, int style, float *drawcolor, qboolean colorBar ) {
	float frac, sliderValue, buttonX;
	qhandle_t hShader;
	int sliderWidth, sliderHeight, buttonWidth, buttonHeight;
	float min, max, value;

	min = item->cvarRange->min;
	max = item->cvarRange->max;
	value = item->vmCvar.value;

	// draw the background
	if ( colorBar ) {
		sliderWidth = 128;
		sliderHeight = 8;
		buttonWidth = 16;
		buttonHeight = 12;
		hShader = uiAssets.fxBasePic;
	} else {
		sliderWidth = 96;
		sliderHeight = 16;
		buttonWidth = 12;
		buttonHeight = 20;
		hShader = uiAssets.sliderBar;

		// set color of slider bar and button
		trap_R_SetColor( drawcolor );
	}

	if ( hShader ) {
		CG_DrawPic( x, y, sliderWidth, sliderHeight, hShader );
	} else {
		UI_BuiltinSliderBar( x, y, sliderWidth, sliderHeight, drawcolor );
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
	if ( item->numPairs ) {
		// ZTM: TODO: Make color bar smoother while dragging it?
		frac = ( item->cvarPair / (float)item->numPairs );
	} else {
		frac = ( sliderValue - min ) / ( max - min );
	}

	if ( colorBar ) {
		//float xOffset = 128.0f / (NUM_COLOR_EFFECTS + 1);
		hShader = uiAssets.fxPic[item->cvarPair];
		if ( !hShader ) {
			vec4_t picColor;

			hShader = uiAssets.fxPic[NUM_COLOR_EFFECTS-1]; // white
			if ( !hShader )
				hShader = cgs.media.whiteShader;

			CG_PlayerColorFromIndex( atoi( item->cvarPairs[item->cvarPair].value ), picColor );
			picColor[3] = 1;
			trap_R_SetColor( picColor );
		}
		//CG_DrawPic( item->generic.x + item->curvalue * xOffset + xOffset * 0.5f, item->generic.y + PROP_HEIGHT + 6, 16, 12, colorShader );

		// FIXME
		buttonX = x + 8 + ( sliderWidth - 16 ) * frac - 2;
	} else {
#ifndef MISSIONPACK
		// team arena colorizes the button, q3 does not
		trap_R_SetColor( NULL );
#endif

		buttonX = x + 8 + ( sliderWidth - 16 ) * frac - 2;
	}

	if ( hShader ) {
		CG_DrawPic( buttonX, y - 2, buttonWidth, buttonHeight, hShader );
	} else {
		UI_BuiltinCircle( buttonX, y - 2, buttonWidth, buttonHeight, drawcolor );
	}

	trap_R_SetColor( NULL );

	if ( colorBar ) {
		return;
	}

	// draw value
#ifdef Q3UIFONTS
	if ( style & UI_GIANTFONT ) {
		UI_DrawProportionalString( x + sliderWidth + 8, y, va("%g", value), style, drawcolor );
	} else
#endif
	{
		CG_DrawString( x + sliderWidth + 8, y, va("%g", value), UI_DROPSHADOW|style, drawcolor );
	}
}

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

	UI_DrawSlider( item, x, item->captionPos.y, style, drawcolor, qfalse );
}

qboolean UI_ItemIsSlider( currentMenuItem_t *item ) {
	return ( item->cvarRange != NULL );
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

	UI_DrawSlider( item, x, item->captionPos.y, style, drawcolor, qtrue );
}


/*
============================================================================
Radio button
============================================================================
*/
// this is called from Generic_Draw
void UI_DrawRadioButton( currentMenuItem_t *item, float *x ) {
	qhandle_t hShader;
	int valuePair0, valuePair1, offPair, picY;

	valuePair0 = atoi( item->cvarPairs[ 0 ].value );
	valuePair1 = atoi( item->cvarPairs[ 1 ].value );

	if ( valuePair0 == 0 && valuePair1 == 1 ) {
		offPair = 0;
	} else /* if ( valuePair0 == 1 && valuePair1 == 0 ) */ {
		// reversed case
		offPair = 1;
	}

	hShader = ( item->cvarPair == offPair ) ? uiAssets.radioButtonOff : uiAssets.radioButtonOn;

	// center ratio button on box and move down 2 pixels at 16 pt size
	picY = item->captionPos.y + item->captionPos.height / 2 - 16 / 2
				+ 2.0f * SMALLCHAR_HEIGHT / 16.0f;

	CG_DrawPic( *x, picY, 16, 16, hShader );
	*x += 16;
}

qboolean UI_ItemIsRadioButton( currentMenuItem_t *item ) {
	int valuePair0, valuePair1, offPair;

	if ( !uiAssets.radioButtonOff || !uiAssets.radioButtonOn ) {
		return qfalse;
	}

	if ( !item->cvarPairs || item->numPairs != 2
			|| item->cvarPairs[ 0 ].type != CVT_INT
			|| item->cvarPairs[ 1 ].type != CVT_INT ) {
		return qfalse;
	}

	valuePair0 = atoi( item->cvarPairs[ 0 ].value );
	valuePair1 = atoi( item->cvarPairs[ 1 ].value );

	if ( valuePair0 == 0 && valuePair1 == 1 ) {
		offPair = 0;
	} else if ( valuePair0 == 1 && valuePair1 == 0 ) {
		// reversed case
		offPair = 1;
	} else {
		return qfalse;
	}

	if ( Q_stricmp( item->cvarPairs[ offPair ].string, "off" ) || Q_stricmp( item->cvarPairs[ !offPair ].string, "on" ) ) {
		return qfalse;
	}

	return qtrue;
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
	{ Generic_Init, Generic_Draw, NULL },			// UIW_RADIO
	{ ListBox_Init, ListBox_Draw, NULL },			// UIW_LISTBOX
};

