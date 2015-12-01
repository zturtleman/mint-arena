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
currentMenu_t currentMenu;
cvarValuePair_t cp_resolution[MAX_RESOLUTIONS];

void	UI_Init( qboolean inGameLoad, int maxSplitView ) {
	int i;

	(void)inGameLoad;
	(void)maxSplitView;

	Com_Memset( &uis, 0, sizeof ( uis ) );

	UI_LoadAssets();
	UI_GetResolutions();

#ifdef Q3UIFONTS
	if ( !CG_InitTrueTypeFont( "fonts/font1_prop", PROP_HEIGHT, 0, &uis.fontProp ) ) {
		UI_InitPropFont( &uis.fontProp, qfalse );
	}
	if ( !CG_InitTrueTypeFont( "fonts/font1_prop_glo", PROP_HEIGHT, 0, &uis.fontPropGlow ) ) {
		UI_InitPropFont( &uis.fontPropGlow, qtrue );
	}
	if ( !CG_InitTrueTypeFont( "fonts/font2_prop", PROPB_HEIGHT, 0, &uis.fontPropB ) ) {
		UI_InitBannerFont( &uis.fontPropB );
	}
#endif

	uis.itemActionSound = trap_S_RegisterSound( ITEM_ACTION_SOUND, qfalse );
	uis.itemFocusSound = trap_S_RegisterSound( ITEM_FOCUS_SOUND, qfalse );
	uis.itemWarnSound = trap_S_RegisterSound( ITEM_WARN_SOUND, qfalse );
	uis.menuPopSound = trap_S_RegisterSound( MENU_POP_SOUND, qfalse );

	uis.cursorShader = trap_R_RegisterShaderNoMip( CURSOR_SHADER );
	for ( i = 0; i < MAX_SPLITVIEW; i++ ) {
		uis.cursors[i].shader = uis.cursorShader;
	}

	// show player 1's cursor by default
	uis.cursors[0].show = qtrue;

	Com_Memset( &currentMenu, 0, sizeof ( currentMenu ) );
}

void	UI_Shutdown( void ) {

}

void	UI_SetActiveMenu( uiMenuCommand_t menu ) {
	if ( menu == UIMENU_NONE ) {
		Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
		UI_SetMenu( &currentMenu, M_NONE );
	}
	else if ( menu == UIMENU_MAIN || menu == UIMENU_INGAME ) {
		trap_Mouse_SetState( 0, MOUSE_CGAME );
		Key_SetCatcher( KEYCATCH_UI );
		trap_Cvar_SetValue( "cl_paused", 1 );

		 if ( menu == UIMENU_MAIN ) {
			char errorMessage[2];

			trap_Cvar_VariableStringBuffer( "com_errorMessage", errorMessage, sizeof ( errorMessage ) );

			if ( errorMessage[0] != '\0' ) {
				UI_SetMenu( &currentMenu, M_ERROR );
			} else {
				UI_SetMenu( &currentMenu, M_MAIN );
			}
		} else if ( menu == UIMENU_INGAME ) {
			UI_SetMenu( &currentMenu, M_INGAME );
		}
	} else if ( menu == UIMENU_POSTGAME ) {
		// postgame doesn't pause
		trap_Mouse_SetState( 0, MOUSE_CGAME );
		Key_SetCatcher( KEYCATCH_UI );

		UI_SetMenu( &currentMenu, M_POSTGAME );
	}
}

void	UI_KeyEvent( int key, qboolean down ) {

	if ( key & K_CHAR_FLAG )
		return;

	if ( !currentMenu.menu )
		return;

	if ( currentMenu.mouseClickDown == key && !down ) {
		currentMenu.mouseClickDown = 0;
		UI_MenuMouseAction( &currentMenu, currentMenu.mouseItem, uis.cursors[0].x, uis.cursors[0].y, MACTION_RELEASE );
		UI_MenuCursorPoint( &currentMenu, uis.cursors[0].x, uis.cursors[0].y );
		return;
	}

	if ( !down )
		return;

	switch ( key ) {
		case K_F11:
			// Q3A/TA toggle debug drawing mode
			break;

		case K_F12:
			trap_Cmd_ExecuteText( EXEC_APPEND, "screenshot\n" );
			break;

		case K_JOY_BACK:
		case K_2JOY_BACK:
		case K_3JOY_BACK:
		case K_4JOY_BACK:
		case K_MOUSE4: // this is typically 'back'
		case K_ESCAPE:
			if ( !( ui_menus[currentMenu.menu].menuFlags & MF_NOESCAPE ) ) {
				UI_PopMenu( &currentMenu );
			}
			break;

		case K_JOY_DPAD_UP:
		case K_JOY_LEFTSTICK_UP:
		case K_2JOY_DPAD_UP:
		case K_2JOY_LEFTSTICK_UP:
		case K_3JOY_DPAD_UP:
		case K_3JOY_LEFTSTICK_UP:
		case K_4JOY_DPAD_UP:
		case K_4JOY_LEFTSTICK_UP:
		case K_KP_UPARROW:
		case K_UPARROW:
			UI_MenuAdjustCursor( &currentMenu, -1 );
			break;

		case K_TAB:
		case K_JOY_DPAD_DOWN:
		case K_JOY_LEFTSTICK_DOWN:
		case K_2JOY_DPAD_DOWN:
		case K_2JOY_LEFTSTICK_DOWN:
		case K_3JOY_DPAD_DOWN:
		case K_3JOY_LEFTSTICK_DOWN:
		case K_4JOY_DPAD_DOWN:
		case K_4JOY_LEFTSTICK_DOWN:
		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			UI_MenuAdjustCursor( &currentMenu, 1 );
			break;

		case K_JOY_DPAD_LEFT:
		case K_JOY_LEFTSTICK_LEFT:
		case K_2JOY_DPAD_LEFT:
		case K_2JOY_LEFTSTICK_LEFT:
		case K_3JOY_DPAD_LEFT:
		case K_3JOY_LEFTSTICK_LEFT:
		case K_4JOY_DPAD_LEFT:
		case K_4JOY_LEFTSTICK_LEFT:
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
			UI_MenuAdjustCursor( &currentMenu, 2 );
			break;

		case K_JOY_DPAD_RIGHT:
		case K_JOY_LEFTSTICK_RIGHT:
		case K_2JOY_DPAD_RIGHT:
		case K_2JOY_LEFTSTICK_RIGHT:
		case K_3JOY_DPAD_RIGHT:
		case K_3JOY_LEFTSTICK_RIGHT:
		case K_4JOY_DPAD_RIGHT:
		case K_4JOY_LEFTSTICK_RIGHT:
		case K_KP_RIGHTARROW:
		case K_RIGHTARROW:
			UI_MenuAdjustCursor( &currentMenu, 4 );
			break;

		case K_JOY_A:
		case K_2JOY_A:
		case K_3JOY_A:
		case K_4JOY_A:
		case K_AUX1:
		case K_AUX2:
		case K_AUX3:
		case K_AUX4:
		case K_AUX5:
		case K_AUX6:
		case K_AUX7:
		case K_AUX8:
		case K_AUX9:
		case K_AUX10:
		case K_AUX11:
		case K_AUX12:
		case K_AUX13:
		case K_AUX14:
		case K_AUX15:
		case K_AUX16:
		case K_KP_ENTER:
		case K_ENTER:
			UI_MenuAction( &currentMenu, currentMenu.selectedItem, 1 );
			break;

		case K_MOUSE1: // left click
		case K_MOUSE3: // middle click
			// ignore if in the middle of a mouse item operation
			if ( currentMenu.mouseClickDown ) {
				break;
			}

			UI_MenuCursorPoint( &currentMenu, uis.cursors[0].x, uis.cursors[0].y );
			if ( currentMenu.mouseItem != -1 ) {
				if ( UI_MenuMouseAction( &currentMenu, currentMenu.mouseItem, uis.cursors[0].x, uis.cursors[0].y, MACTION_PRESS ) ) {
					currentMenu.mouseClickDown = key;
				} else {
					UI_MenuAction( &currentMenu, currentMenu.mouseItem, 1 );
				}
			}
			break;

		case K_MOUSE2: // right click
			// ignore if in the middle of a mouse item operation
			if ( currentMenu.mouseClickDown ) {
				break;
			}

			UI_MenuCursorPoint( &currentMenu, uis.cursors[0].x, uis.cursors[0].y );
			if ( currentMenu.mouseItem != -1 ) {
				UI_MenuAction( &currentMenu, currentMenu.mouseItem, -1 );
			}
			break;

		default:
			break;
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

	if ( currentMenu.mouseClickDown ) {
		UI_MenuMouseAction( &currentMenu, currentMenu.mouseItem, uis.cursors[localClientNum].x, uis.cursors[localClientNum].y, MACTION_DRAG );
	} else {
		UI_MenuCursorPoint( &currentMenu, uis.cursors[localClientNum].x, uis.cursors[localClientNum].y );
	}
}

void	UI_GetCursorPos( int localClientNum, int *x, int *y ) {
	if ( x ) *x = uis.cursors[localClientNum].x;
	if ( y ) *y = uis.cursors[localClientNum].y;
}

void	UI_SetCursorPos( int localClientNum, int x, int y ) {
	if ( uis.cursors[localClientNum].x == x && uis.cursors[localClientNum].y == y ) {
		// ignore duplicate events
		return;
	}

	uis.cursors[localClientNum].x = x;
	uis.cursors[localClientNum].y = y;

	UI_MouseEvent( localClientNum, 0, 0 );
}

qboolean UI_IsFullscreen( void ) {
	return ( Key_GetCatcher() & KEYCATCH_UI ) && !cg.connected;
}

void	UI_Refresh( int time ) {
	int i;

	if ( !( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	#ifdef MAIN_MUSIC
	if ( !cg.connected && !uis.startedMusic ) {
		trap_S_StopBackgroundTrack();
		trap_S_StartBackgroundTrack( MAIN_MUSIC, MAIN_MUSIC, 1.0f, 1.0f );
		uis.startedMusic = qtrue;
	}
	#endif

	if ( UI_IsFullscreen() ) {
		CG_ClearViewport();
	}

	UI_DrawCurrentMenu( &currentMenu );

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

void UI_WindowResized( void ) {
	UI_GetResolutions();
}

void	UI_DrawConnectScreen( qboolean overlay ) {
	if ( !overlay ) {
		CG_ClearViewport();
		UI_DrawConnectBackground();
	}

	// ZTM: TODO: draw status
}

#ifndef Q3UIFONTS
// used by cg_info.c
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color ) {
	CG_DrawString( x, y, str, style, color );
}
#endif

#if defined MISSIONPACK && defined MISSIONPACK_HUD
// missionpack hud requires this
void UI_Load( void ) {

}
#endif

/*
=================
UI_GetResolutions

Based on ioq3 GraphicsOptions_GetResolutions

Build list of available resolutions and save current resolution index
=================
*/
void UI_GetResolutions( void ) {
	static char resbuf[ MAX_STRING_CHARS ];
	static char cmdBuf[ MAX_RESOLUTIONS ][ 64 ];
	static const char *builtinResolutions[ ] =
	{
		"320x240",
		"400x300",
		"512x384",
		"640x480",
		"800x600",
		"960x720",
		"1024x768",
		"1152x864",
		"1280x1024",
		"1600x1200",
		"2048x1536",
		"856x480",
		NULL
	};
	static char displayRes[64];
	static char customRes[64];
	unsigned int i = 0;
	unsigned int builtin;
	int currentMode;
	int customWidth;
	int customHeight;

	currentMode = trap_Cvar_VariableIntegerValue( "r_mode" );

	customWidth = trap_Cvar_VariableIntegerValue( "r_customWidth" );
	customHeight = trap_Cvar_VariableIntegerValue( "r_customHeight" );

	// set to invalid pair, filled in when mode is found in list
	uis.currentResPair = -1;

	// Add display resolution video mode
	Com_sprintf( displayRes, sizeof (displayRes), "Auto (%dx%d)", cgs.glconfig.displayWidth, cgs.glconfig.displayHeight );
	cp_resolution[i].type = CVT_CMD;
	cp_resolution[i].value = "r_mode -2;";
	cp_resolution[i].string = displayRes;
	if ( currentMode == -2 ) {
		uis.currentResPair = i;
	}
	i++;

	// ZTM: FIXME?: This cvar was added in ioq3, mint-arena can assume it will always be present?
	Q_strncpyz(resbuf, CG_Cvar_VariableString("r_availableModes"), sizeof(resbuf));
	if (*resbuf)
	{
		char *s = resbuf;

		while ( s && i < MAX_RESOLUTIONS-1 ) {
			cp_resolution[i].string = s;

			s = strchr(s, ' ');
			if( s )
				*s++ = '\0';

			// find built in mode
			for( builtin = 0; builtinResolutions[ builtin ]; builtin++ )
			{
				if( !Q_stricmp( builtinResolutions[ builtin ], cp_resolution[i].string ) )
					break;
			}

			if ( builtinResolutions[ builtin ] ) {
				Com_sprintf( cmdBuf[i], sizeof (cmdBuf[i]), "r_mode %d;", builtin );

				cp_resolution[i].type = CVT_CMD;
				cp_resolution[i].value = cmdBuf[i];

				if ( currentMode == builtin ) {
					uis.currentResPair = i;
				}
			} else {
				int width = atoi( cp_resolution[i].string );
				int height = atoi( strchr(cp_resolution[i].string, 'x')+1 );

				Com_sprintf( cmdBuf[i], sizeof (cmdBuf[i]), "r_mode -1; r_customwidth %d; r_customheight %d;", width, height );

				cp_resolution[i].type = CVT_CMD;
				cp_resolution[i].value = cmdBuf[i];

				if ( currentMode != -2 && width == cgs.glconfig.vidWidth && height == cgs.glconfig.vidHeight ) {
					uis.currentResPair = i;
				}
			}

			i++;
		}
	} else {
		// use built in modes
		for ( builtin = 0; builtinResolutions[builtin]; builtin++ ) {
			cp_resolution[i].string = builtinResolutions[i];

			Com_sprintf( cmdBuf[i], sizeof (cmdBuf[i]), "r_mode %d;", builtin );

			cp_resolution[i].type = CVT_CMD;
			cp_resolution[i].value = cmdBuf[i];

			if ( currentMode == builtin ) {
				uis.currentResPair = i;
			}

			i++;
		}
	}

	// Add custom mode option
	Com_sprintf( cmdBuf[i], sizeof (cmdBuf[i]), "r_mode -1; r_customwidth %d; r_customheight %d;", customWidth, customHeight );
	Com_sprintf( customRes, sizeof (customRes), "Custom (%dx%d)", customWidth, customHeight );
	cp_resolution[i].type = CVT_CMD;
	cp_resolution[i].value = cmdBuf[i];
	cp_resolution[i].string = customRes;
	if ( uis.currentResPair < 0 ) {
		uis.currentResPair = i;
	}
	i++;

	cp_resolution[i].type = CVT_NONE;
	cp_resolution[i].value = NULL;
	cp_resolution[i].string = NULL;

	//Com_Printf("DEBUG: Num modes: %d, current mode pair %d (%s)\n", i, uis.currentResPair, cp_resolution[uis.currentResPair].string );
}

