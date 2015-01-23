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
vec4_t color_copyright        = {1.00f, 0.00f, 0.00f, 1.00f};	// bright red
#ifdef MISSIONPACK
vec4_t color_smalltext        = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_smallSelected    = {1.00f, 0.75f, 0.00f, 1.00f};
vec4_t color_bigtext          = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_bigSelected      = {1.00f, 0.75f, 0.00f, 1.00f};
#else
//vec4_t text_color_disabled  = {0.50f, 0.50f, 0.50f, 1.00f};	// light gray
vec4_t color_smalltext        = {1.00f, 0.43f, 0.00f, 1.00f};	// light orange // text_color_normal
vec4_t color_smallSelected    = {1.00f, 1.00f, 0.00f, 1.00f};	// bright yellow // text_color_highlight
vec4_t color_bigtext          = {0.80f, 0.00f, 0.00f, 1.00f};	// ZTM: FIXME: is this suppose to be bright red?
vec4_t color_bigSelected      = {1.00f, 0.00f, 0.00f, 1.00f};	// bright red // text_big_color
#endif

typedef struct {
	qhandle_t menuBackground;
	qhandle_t menuBackgroundNoLogo;

#ifdef MISSIONPACK
	qhandle_t menuBackgroundB;
	qhandle_t menuBackgroundC;
	qhandle_t menuBackgroundD;
	qhandle_t menuBackgroundE;
	qhandle_t levelShotDetail;
#else
	qhandle_t bannerModel;

	qhandle_t frameLeft;
	qhandle_t frameLeftFilled; // player model select menu
	qhandle_t frameRight;
#endif

	qhandle_t sliderBar;
	qhandle_t sliderButton;
	qhandle_t sliderButtonSelected;

	qhandle_t dialogSmallBackground;
	qhandle_t dialogLargeBackground;

} uiAssets_t;

uiAssets_t uiAssets;

void UI_LoadAssets( void ) {
	Com_Memset( &uiAssets, 0, sizeof ( uiAssets ) );

#ifdef MISSIONPACK
	uiAssets.menuBackground = trap_R_RegisterShaderNoMip( "menuback_a" );
	uiAssets.menuBackgroundB = trap_R_RegisterShaderNoMip( "menuback_b" );
	uiAssets.menuBackgroundC = trap_R_RegisterShaderNoMip( "menuback_c" );
	uiAssets.menuBackgroundD = trap_R_RegisterShaderNoMip( "menuback_d" );
	uiAssets.menuBackgroundE = trap_R_RegisterShaderNoMip( "menuback_e" );
	uiAssets.levelShotDetail = trap_R_RegisterShaderNoMip( "levelshotdetail" );
	uiAssets.menuBackgroundNoLogo = uiAssets.menuBackground;
	uiAssets.sliderBar = trap_R_RegisterShaderNoMip( "ui/assets/slider2" );
	uiAssets.sliderButton = trap_R_RegisterShaderNoMip( "ui/assets/sliderbutt_1" );
	uiAssets.sliderButtonSelected = uiAssets.sliderButton;
#else
	uiAssets.bannerModel = trap_R_RegisterModel( "models/mapobjects/banner/banner5.md3" );
	uiAssets.menuBackground = trap_R_RegisterShaderNoMip( "menuback" );
	uiAssets.menuBackgroundNoLogo = trap_R_RegisterShaderNoMip( "menubacknologo" );

	uiAssets.frameLeft = trap_R_RegisterShaderNoMip( "menu/art/frame2_l" );
	uiAssets.frameLeftFilled = trap_R_RegisterShaderNoMip( "menu/art/frame1_l" );
	uiAssets.frameRight = trap_R_RegisterShaderNoMip( "menu/art/frame1_r" );
	uiAssets.sliderBar = trap_R_RegisterShaderNoMip( "menu/art/slider2" );
	uiAssets.sliderButton = trap_R_RegisterShaderNoMip( "menu/art/sliderbutt_0" );
	uiAssets.sliderButtonSelected = trap_R_RegisterShaderNoMip( "menu/art/sliderbutt_1" );
#endif

	uiAssets.dialogSmallBackground = trap_R_RegisterShaderNoMip( "menu/art/cut_frame" );
	uiAssets.dialogLargeBackground = trap_R_RegisterShaderNoMip( "menu/art/addbotframe" );
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

void UI_DrawSlider( float x, float y, float min, float max, float value, int style, float *drawcolor ) {
	float frac, sliderValue;
	qhandle_t hShader;

	trap_R_SetColor( drawcolor );

	// draw the background
	CG_DrawPic( x, y, 96, 16, uiAssets.sliderBar );

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

	CG_DrawPic( x + 8 + ( 96 - 16 ) * frac - 2, y - 2, 12, 20, hShader );

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

void UI_DrawCurrentMenu( currentMenu_t *current ) {
	int i;
	qboolean drawFramePics = qtrue;
	vec4_t drawcolor;
	int style, panelNum;
	menudef_t	*menuInfo;
	currentMenuItem_t *item;

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

	for ( i = 0, item = current->items; i < current->numItems; i++, item++ ) {
		style = 0;

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

			if ( item->cvarRange && item->cvarRange->numPairs > 0 ) {
				string = item->cvarRange->pairs[ item->cvarPair ].string;
			} else {
				string = item->vmCvar.string;
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

// TODO: Fix font heights in UI_BuildCurrentMenu with Q3UIFONTS defined
#define MAX_MENU_HEADERS	8
// this is used for drawing and logic. it's closely related to UI_DrawCurrentMenu.
void UI_BuildCurrentMenu( currentMenu_t *current ) {
	int i, horizontalGap = BIGCHAR_WIDTH, verticalGap = 2;
	int	numHeaders = 0, totalWidth[MAX_MENU_HEADERS] = {0}, totalHeight = 0, totalPanelHeight = 0;
	int panelItemX, panelItemY, itemX, itemY, curX, curY;
	int headerBottom = -1;
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
		current->header.captionPos.y = 10;

		headerBottom = current->header.captionPos.y + current->header.captionPos.height;
	} else {
		Com_Memset( &current->header, 0, sizeof ( current->header ) );
		headerBottom = 0;
	}

#ifndef MISSIONPACK
	if ( !cg.connected && ( menuInfo->menuFlags & MF_MAINMENU ) ) {
		// Q3 banner model
		headerBottom = 120;
	}
#endif

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
		item->caption = itemInfo->caption;
		item->captionPos.y = itemInfo->y;
		item->captionPos.x = 0;

		if ( item->flags & MIF_PANEL ) {
			panelNum++;
		} else {
			// if not part of current panel, don't show it
			if ( panelNum > 0 && panelNum != current->panel )
			{
				continue;
			}
		}

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

		if ( UI_ItemIsSlider( item ) ) {
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
			item->captionPos.x = SCREEN_WIDTH - item->captionPos.width - 10;
			item->captionPos.y = SCREEN_HEIGHT - item->captionPos.height - 10;

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

		totalWidth[numHeaders] += item->captionPos.width;
		if ( i != current->numItems - 1 && !(current->items[i+1].flags & MIF_HEADER) ) {
			totalWidth[numHeaders] += horizontalGap;
		}

		if ( item->flags & MIF_PANEL ) {
			totalPanelHeight += item->captionPos.height + verticalGap;
		} else {
			totalHeight += item->captionPos.height + verticalGap;
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
				curY += item->captionPos.height + verticalGap;
		} else if ( horizontalMenu && curY == -1 ) {
			curY = headerBottom;
		} else if ( curY == -1 ) {
			// center Y
			curY = headerBottom + (SCREEN_HEIGHT - headerBottom - sectionHeight) / 2;
		}

		if ( item->flags & MIF_PANEL ) {
			// right align
			curX = 216 - item->captionPos.width;
		} if ( !panelNum && item->cvarName && centerX ) {
			// right align HACK for M_GAME_OPTIONS
			curX = SCREEN_WIDTH / 2 + totalWidth[0] / menuInfo->numItems / 6 - item->captionPos.width;
		} else if ( horizontalMenu && curX == -1 ) {
			// center line
			curX = (SCREEN_WIDTH - totalWidth[numHeaders]) / 2;
		} else if ( centerX ) {
			// center item
			curX = (SCREEN_WIDTH - item->captionPos.width) / 2;

			// ZTM: TODO: properly deal with changing the center x?
			if ( panelNum ) {
				curX = 216 + 12;
			}
		}

		// uses absolute position because y might already be set to absolute
		item->captionPos.x = curX;
		item->captionPos.y = curY;

		// clickable position is relative
		item->clickPos.x += curX;
		item->clickPos.y += curY;

		// move draw point for next item
		if ( ( horizontalMenu && !numHeaders ) || ( numHeaders && !( item->flags & MIF_HEADER ) ) ) {
			curX += item->captionPos.width + horizontalGap;
		} else {
			curY += item->captionPos.height + verticalGap;
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
		item->flags = MIF_BIGTEXT|MIF_POPMENU;
		item->action = NULL;
		item->menuid = M_NONE;
		item->cvarName = NULL;
		item->cvarRange = NULL;

#ifdef MISSIONPACK
		if ( current->numStacked == 1 )
			item->caption = "Exit to Main Menu";
		else
#endif
		item->caption = "Back";

		// note: assumes MIF_BIGTEXT
#ifdef Q3UIFONTS
		item->captionPos.width = UI_ProportionalStringWidth( item->caption );
		item->captionPos.height = PROP_HEIGHT;
#else
		item->captionPos.width = CG_DrawStrlen( item->caption, UI_GIANTFONT );
		item->captionPos.height = GIANTCHAR_HEIGHT;
#endif

#if 1 // Fixed place in lower left
		item->captionPos.y = SCREEN_HEIGHT - item->captionPos.height - 10;
		item->captionPos.x = 10;
#else // fake next of menu item (though it's not vertically centered by the above numItems code)
		if ( i > 0 ) {
			y += current->items[i-1].captionPos.height + verticalGap;
		}
		item->captionPos.y = y;
		item->captionPos.x = (SCREEN_WIDTH - item->captionPos.width) / 2;
#endif

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

