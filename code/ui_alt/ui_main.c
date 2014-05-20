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

uiStatic_t uis;

void	UI_Init( qboolean inGameLoad, int maxSplitView ) {
	int i;

	(void)inGameLoad;
	(void)maxSplitView;

	Com_Memset( &uis, 0, sizeof ( uis ) );

	uis.cursorShader = trap_R_RegisterShaderNoMip( CURSOR_SHADER );
	for ( i = 0; i < MAX_SPLITVIEW; i++ ) {
		uis.cursors[i].shader = uis.cursorShader;
	}

	// show player 1's cursor by default
	uis.cursors[0].show = qtrue;
}

void	UI_Shutdown( void ) {

}

void	UI_SetActiveMenu( uiMenuCommand_t menu ) {
	if ( menu != UIMENU_NONE ) {
		trap_Mouse_SetState( 0, MOUSE_CGAME );
		trap_Key_SetCatcher( KEYCATCH_UI );
		trap_Cvar_SetValue( "cl_paused", 1 );
	}
}

void	UI_KeyEvent( int key, qboolean down ) {
	if ( !down )
		return;

	if ( key & K_CHAR_FLAG )
		return;

	if ( key == K_ESCAPE && cg.connected ) {
		trap_Mouse_SetState( 0, MOUSE_CLIENT );
		trap_Key_SetCatcher( 0 );
		trap_Cvar_SetValue( "cl_paused", 0 );
	}
}

void	UI_MouseEvent( int localClientNum, int dx, int dy ) {
	float ax, ay, aw, ah;
	float xbias, ybias;

	ax = 0;
	ay = 0;
	aw = 1;
	ah = 1;
	CG_SetScreenPlacement( PLACE_CENTER, PLACE_CENTER );
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	xbias = ax/aw;
	ybias = ay/ah;

	// update mouse screen position
	uis.cursors[localClientNum].x = Com_Clamp( -xbias, SCREEN_WIDTH+xbias, uis.cursors[localClientNum].x + dx );
	uis.cursors[localClientNum].y = Com_Clamp( -ybias, SCREEN_HEIGHT+ybias, uis.cursors[localClientNum].y + dy );

	uis.cursors[localClientNum].show = qtrue;
}

void	UI_GetCursorPos( int localClientNum, int *x, int *y ) {
	if ( x ) *x = uis.cursors[localClientNum].x;
	if ( y ) *y = uis.cursors[localClientNum].y;
}

void	UI_SetCursorPos( int localClientNum, int x, int y ) {
	uis.cursors[localClientNum].x = x;
	uis.cursors[localClientNum].y = y;

	UI_MouseEvent( localClientNum, 0, 0 );
}

qboolean UI_IsFullscreen( void ) {
	return ( trap_Key_GetCatcher() &  KEYCATCH_UI );
}

void	UI_Refresh( int time ) {
	int i;

	if ( ! ( trap_Key_GetCatcher() &  KEYCATCH_UI ) ) {
		return;
	}

	if ( UI_IsFullscreen() ) {
		CG_ClearViewport();
	}

	// Draw cursors
	for ( i = 0; i < MAX_SPLITVIEW; i++ ) {
		if ( !uis.cursors[i].show ) {
			continue;
		}

		CG_DrawPic( uis.cursors[i].x-16, uis.cursors[i].y-16, 32, 32, uis.cursors[i].shader );
	}
}

qboolean UI_WantsBindKeys( void ) {
	return qfalse;
}

void	UI_DrawConnectScreen( qboolean overlay ) {
	if ( !overlay ) {
		CG_ClearViewport();
	}
}

// used by cg_info.c
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color ) {
	CG_DrawBigStringColor( x, y, str, color );
}

#if defined MISSIONPACK && defined MISSIONPACK_HUD
// missionpack hud requires this
void UI_Load( void ) {

}
#endif

