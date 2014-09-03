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

vec4_t color_header           = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_bigtext          = {0.80f, 0.00f, 0.00f, 1.00f};
vec4_t color_selected         = {1.00f, 0.00f, 0.00f, 1.00f};

#define MAIN_MENU_VERTICAL_SPACING		34
void UI_DrawMainMenuBackground( int *headerBottom );

void UI_DrawCurrentMenu( currentMenu_t *current ) {
	int i, x, y, lineHeight;
	int headerBottom = 10 + BIGCHAR_HEIGHT;
	menuitem_t *item;

	if ( !current->menu ) {
		return;
	}

	// TODO: draw background
	if ( !cg.connected ) {
		if ( current->menu->menuType == MENUTYPE_MAIN ) {
			UI_DrawMainMenuBackground( &headerBottom );
		} else {
			// draw menubacknologo
		}
	}

	if ( current->menu->menuType != MENUTYPE_MAIN ) {
		CG_DrawBigStringColor( ( SCREEN_WIDTH - CG_DrawStrlen( current->menu->header ) * BIGCHAR_WIDTH ) / 2, 10, current->menu->header, color_header );
	}

	if ( 0 && current->menu->menuType == MENUTYPE_MAIN ) {
		// Q3 main menu spacing. Does it need to be different?
		lineHeight = MAIN_MENU_VERTICAL_SPACING;
	} else {
		lineHeight = BIGCHAR_HEIGHT * 2;
	}

	for ( i = 0, item = current->menu->items; i < current->menu->numItems; i++, item++ ) {
		if ( i == 0 && item->y == 0 )
			y = headerBottom + (SCREEN_HEIGHT - headerBottom - current->menu->numItems * lineHeight) / 2;
		else if ( item->y != 0 )
			y = item->y;
		else
			y += lineHeight;

		if ( item->caption == NULL )
			continue;

		if ( item->flags & MIF_CENTER )
			x = (SCREEN_WIDTH - CG_DrawStrlen( item->caption ) * BIGCHAR_WIDTH) / 2;
		else
			x = 50;

		CG_DrawBigStringColor( x, y, item->caption, current->selectedItem == i ? color_selected : color_bigtext );
	}

}

void UI_DrawMainMenuBackground( int *headerBottom ) {
#ifdef MISSIONPACK
	*headerBottom = 0;
#else
	refdef_t		refdef;
	refEntity_t		ent;
	vec3_t			origin;
	vec3_t			angles;
	float			adjust;
	float			x, y, w, h;

	// draw menuback

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

	*headerBottom = 120;

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
	ent.hModel = uis.bannerModel;
	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent.origin, ent.oldorigin );

	trap_R_AddRefEntityToScene( &ent );

	trap_R_RenderScene( &refdef );
#endif

	CG_DrawSmallStringColor( ( SCREEN_WIDTH - CG_DrawStrlen( MENU_COPYRIGHT ) * SMALLCHAR_WIDTH ) / 2, SCREEN_HEIGHT - SMALLCHAR_HEIGHT - 2, MENU_COPYRIGHT, color_bigtext );
}



