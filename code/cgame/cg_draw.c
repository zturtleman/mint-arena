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
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

#ifdef MISSIONPACK_HUD
#include "../ui/ui_shared.h"

// used for scoreboard
extern displayContextDef_t cgDC;
menuDef_t *menuScoreboard = NULL;
#else
int drawTeamOverlayModificationCount = -1;
#endif

int sortedTeamPlayers[TEAM_NUM_TEAMS][TEAM_MAXOVERLAY];
int	numSortedTeamPlayers[TEAM_NUM_TEAMS];
int sortedTeamPlayersTime[TEAM_NUM_TEAMS];

char systemChat[256];
char teamChat1[256];
char teamChat2[256];

/*
==============
CG_DrawField

Draws large numbers for status bar and powerups
==============
*/
#ifndef MISSIONPACK_HUD
static void CG_DrawField (int x, int y, int style, int width, int value, float *color) {
	char	num[16];

	if ( width < 1 ) {
		return;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf (num, sizeof(num), "%i", value);

	CG_DrawString( x + 2 + CHAR_WIDTH * width, y, num, UI_RIGHT|UI_GRADIENT|UI_NUMBERFONT|UI_NOSCALE|style, color );
}
#endif // MISSIONPACK_HUD

/*
================
CG_Draw3DModelEx

================
*/
void CG_Draw3DModelEx( float x, float y, float w, float h, qhandle_t model, cgSkin_t *skin, vec3_t origin, vec3_t angles, const byte *rgba ) {
	refdef_t		refdef;
	refEntity_t		ent;

	if ( !cg_draw3dIcons.integer || !cg_drawIcons.integer ) {
		return;
	}

	CG_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
	ent.customSkin = CG_AddSkinToFrame( skin );
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows

	if ( rgba ) {
		Byte4Copy( rgba, ent.shaderRGBA );
	}

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();
	CG_AddRefEntityWithMinLight( &ent );
	trap_R_RenderScene( &refdef );
}

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, cgSkin_t *skin, vec3_t origin, vec3_t angles ) {
	CG_Draw3DModelEx( x, y, w, h, model, skin, origin, angles, NULL );
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead( float x, float y, float w, float h, int playerNum, vec3_t headAngles ) {
	clipHandle_t	cm;
	playerInfo_t	*pi;
	float			len;
	vec3_t			origin;
	vec3_t			mins, maxs;

	pi = &cgs.playerinfo[ playerNum ];

	if ( cg_draw3dIcons.integer ) {
		cm = pi->headModel;
		if ( !cm ) {
			return;
		}

		// offset the origin y and z to center the head
		trap_R_ModelBounds( cm, mins, maxs, 0, 0, 0 );

		origin[2] = -0.5 * ( mins[2] + maxs[2] );
		origin[1] = 0.5 * ( mins[1] + maxs[1] );

		// calculate distance so the head nearly fills the box
		// assume heads are taller than wide
		len = 0.7 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		// allow per-model tweaking
		VectorAdd( origin, pi->headOffset, origin );

		CG_Draw3DModelEx( x, y, w, h, pi->headModel, &pi->modelSkin, origin, headAngles, pi->c1RGBA );
	} else if ( cg_drawIcons.integer ) {
		CG_DrawPic( x, y, w, h, pi->modelIcon );
	}

	// if they are deferred, draw a cross out
	if ( pi->deferred ) {
		CG_DrawPic( x, y, w, h, cgs.media.deferShader );
	}
}

/*
================
CG_DrawFlagModel

Used for both the status bar and the scoreboard
================
*/
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D ) {
	qhandle_t		cm;
	float			len;
	vec3_t			origin, angles;
	vec3_t			mins, maxs;
	qhandle_t		handle;

	if ( !force2D && cg_draw3dIcons.integer ) {

		VectorClear( angles );

		cm = cgs.media.redFlagModel;

		// offset the origin y and z to center the flag
		trap_R_ModelBounds( cm, mins, maxs, 0, 0, 0 );

		origin[2] = -0.5 * ( mins[2] + maxs[2] );
		origin[1] = 0.5 * ( mins[1] + maxs[1] );

		// calculate distance so the flag nearly fills the box
		// assume heads are taller than wide
		len = 0.5 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		angles[YAW] = 60 * sin( cg.time / 2000.0 );;

		if( team == TEAM_RED ) {
			handle = cgs.media.redFlagModel;
		} else if( team == TEAM_BLUE ) {
			handle = cgs.media.blueFlagModel;
		} else if( team == TEAM_FREE ) {
			handle = cgs.media.neutralFlagModel;
		} else {
			return;
		}
		CG_Draw3DModel( x, y, w, h, handle, NULL, origin, angles );
	} else if ( cg_drawIcons.integer ) {
		gitem_t *item;

		if( team == TEAM_RED ) {
			item = BG_FindItemForPowerup( PW_REDFLAG );
		} else if( team == TEAM_BLUE ) {
			item = BG_FindItemForPowerup( PW_BLUEFLAG );
		} else if( team == TEAM_FREE ) {
			item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
		} else {
			return;
		}
		if (item) {
		  CG_DrawPic( x, y, w, h, cg_items[ BG_ItemNumForItem( item ) ].icon );
		}
	}
}

/*
================
CG_DrawStatusBarHead

================
*/
#ifndef MISSIONPACK_HUD

static void CG_DrawStatusBarHead( float x ) {
	vec3_t		angles;
	float		size, stretch;
	float		frac;

	VectorClear( angles );

	if ( cg.cur_lc->damageTime && cg.time - cg.cur_lc->damageTime < DAMAGE_TIME ) {
		frac = (float)(cg.time - cg.cur_lc->damageTime ) / DAMAGE_TIME;
		size = ICON_SIZE * 1.25 * ( 1.5 - frac * 0.5 );

		stretch = size - ICON_SIZE * 1.25;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.cur_lc->damageX * stretch * 0.5;

		cg.cur_lc->headStartYaw = 180 + cg.cur_lc->damageX * 45;

		cg.cur_lc->headEndYaw = 180 + 20 * cos( crandom()*M_PI );
		cg.cur_lc->headEndPitch = 5 * cos( crandom()*M_PI );

		cg.cur_lc->headStartTime = cg.time;
		cg.cur_lc->headEndTime = cg.time + 100 + random() * 2000;
	} else {
		if ( cg.time >= cg.cur_lc->headEndTime ) {
			// select a new head angle
			cg.cur_lc->headStartYaw = cg.cur_lc->headEndYaw;
			cg.cur_lc->headStartPitch = cg.cur_lc->headEndPitch;
			cg.cur_lc->headStartTime = cg.cur_lc->headEndTime;
			cg.cur_lc->headEndTime = cg.time + 100 + random() * 2000;

			cg.cur_lc->headEndYaw = 180 + 20 * cos( crandom()*M_PI );
			cg.cur_lc->headEndPitch = 5 * cos( crandom()*M_PI );
		}

		size = ICON_SIZE * 1.25;
	}

	// if the server was frozen for a while we may have a bad head start time
	if ( cg.cur_lc->headStartTime > cg.time ) {
		cg.cur_lc->headStartTime = cg.time;
	}

	frac = ( cg.time - cg.cur_lc->headStartTime ) / (float)( cg.cur_lc->headEndTime - cg.cur_lc->headStartTime );
	frac = frac * frac * ( 3 - 2 * frac );
	angles[YAW] = cg.cur_lc->headStartYaw + ( cg.cur_lc->headEndYaw - cg.cur_lc->headStartYaw ) * frac;
	angles[PITCH] = cg.cur_lc->headStartPitch + ( cg.cur_lc->headEndPitch - cg.cur_lc->headStartPitch ) * frac;

	CG_DrawHead( x, 480 - size, size, size, 
				cg.cur_ps->playerNum, angles );
}
#endif // MISSIONPACK_HUD

/*
================
CG_DrawStatusBarFlag

================
*/
#ifndef MISSIONPACK_HUD
static void CG_DrawStatusBarFlag( float x, int team ) {
	CG_DrawFlagModel( x, 480 - ICON_SIZE, ICON_SIZE, ICON_SIZE, team, qfalse );
}
#endif // MISSIONPACK_HUD

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team )
{
	vec4_t		hcolor;

	hcolor[3] = alpha;
	if ( team == TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	} else if ( team == TEAM_BLUE ) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	} else {
		return;
	}
	trap_R_SetColor( hcolor );
	CG_SetScreenPlacement(PLACE_STRETCH, CG_GetScreenVerticalPlacement());
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	CG_PopScreenPlacement();
	trap_R_SetColor( NULL );
}

/*
================
CG_DrawStatusBar

================
*/
#ifndef MISSIONPACK_HUD
static void CG_DrawStatusBar( void ) {
	int			color;
	centity_t	*cent;
	playerState_t	*ps;
	int			value;
	vec3_t		angles;
	vec3_t		origin;

	static float colors[4][4] = { 
//		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
		{ 1.0f, 0.69f, 0.0f, 1.0f },    // normal
		{ 1.0f, 0.2f, 0.2f, 1.0f },     // low health
		{ 0.5f, 0.5f, 0.5f, 1.0f },     // weapon firing
		{ 1.0f, 1.0f, 1.0f, 1.0f } };   // health > 100

	if ( cg_drawStatus.integer == 0 ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_BOTTOM);

	ps = cg.cur_ps;
	cent = &cg_entities[ps->playerNum];

	// draw the team background
	CG_DrawTeamBackground( 0, 420, 640, 60, 0.33f, ps->persistant[PERS_TEAM] );

	VectorClear( angles );

	// draw any 3D icons first, so the changes back to 2D are minimized
	if ( cent->currentState.weapon && cg_weapons[ cent->currentState.weapon ].ammoModel ) {
		origin[0] = 70;
		origin[1] = 0;
		origin[2] = 0;
		angles[YAW] = 90 + 20 * sin( cg.time / 1000.0 );
		CG_Draw3DModel( CHAR_WIDTH*3 + TEXT_ICON_SPACE, SCREEN_HEIGHT - ICON_SIZE, ICON_SIZE, ICON_SIZE,
					   cg_weapons[ cent->currentState.weapon ].ammoModel, NULL, origin, angles );
	}

	CG_DrawStatusBarHead( 185 + CHAR_WIDTH*3 + TEXT_ICON_SPACE );

	if( cg.cur_lc->predictedPlayerState.powerups[PW_REDFLAG] ) {
		CG_DrawStatusBarFlag( 185 + CHAR_WIDTH*3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_RED );
	} else if( cg.cur_lc->predictedPlayerState.powerups[PW_BLUEFLAG] ) {
		CG_DrawStatusBarFlag( 185 + CHAR_WIDTH*3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_BLUE );
	} else if( cg.cur_lc->predictedPlayerState.powerups[PW_NEUTRALFLAG] ) {
		CG_DrawStatusBarFlag( 185 + CHAR_WIDTH*3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_FREE );
	}

	if ( ps->stats[ STAT_ARMOR ] ) {
		origin[0] = 90;
		origin[1] = 0;
		origin[2] = -10;
		angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;
		CG_Draw3DModel( 370 + CHAR_WIDTH*3 + TEXT_ICON_SPACE, SCREEN_HEIGHT - ICON_SIZE, ICON_SIZE, ICON_SIZE,
					   cgs.media.armorModel, NULL, origin, angles );
	}
	//
	// ammo
	//
	if ( cent->currentState.weapon ) {
		value = ps->ammo[cent->currentState.weapon];
		if ( value > -1 ) {
			if ( cg.cur_lc->predictedPlayerState.weaponstate == WEAPON_FIRING
				&& cg.cur_lc->predictedPlayerState.weaponTime > 100 ) {
				// draw as dark grey when reloading
				color = 2;	// dark grey
			} else {
				color = 0;	// green
			}

			CG_DrawField (0, SCREEN_HEIGHT, UI_VA_BOTTOM, 3, value, colors[color] );

			// if we didn't draw a 3D icon, draw a 2D icon for ammo
			if ( !cg_draw3dIcons.integer && cg_drawIcons.integer ) {
				qhandle_t	icon;

				icon = cg_weapons[ cg.cur_lc->predictedPlayerState.weapon ].ammoIcon;
				if ( icon ) {
					CG_DrawPic( CHAR_WIDTH*3 + TEXT_ICON_SPACE, SCREEN_HEIGHT - ICON_SIZE, ICON_SIZE, ICON_SIZE, icon );
				}
			}
		}
	}

	//
	// health
	//
	value = ps->stats[STAT_HEALTH];
	if ( value > 100 ) {
		color = 3; // white
	} else if (value > 25) {
		color = 0; // green
	} else if (value > 0) {
		color = (cg.time >> 8) & 1; // flash
	} else {
		color = 1; // red
	}

	// stretch the health up when taking damage
	CG_DrawField ( 185, SCREEN_HEIGHT, UI_VA_BOTTOM, 3, value, colors[color] );


	//
	// armor
	//
	value = ps->stats[STAT_ARMOR];
	if (value > 0 ) {
		CG_DrawField (370, SCREEN_HEIGHT, UI_VA_BOTTOM, 3, value, colors[0]);

		// if we didn't draw a 3D icon, draw a 2D icon for armor
		if ( !cg_draw3dIcons.integer && cg_drawIcons.integer ) {
			CG_DrawPic( 370 + CHAR_WIDTH*3 + TEXT_ICON_SPACE, SCREEN_HEIGHT - ICON_SIZE, ICON_SIZE, ICON_SIZE, cgs.media.armorIcon );
		}

	}
}
#endif // MISSIONPACK_HUD

/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
================
CG_DrawAttacker

================
*/
static float CG_DrawAttacker( float y ) {
	int			t;
	float		size;
	vec3_t		angles;
	vec4_t		color;
	const char	*info;
	const char	*name;
	int			playerNum;

	if ( cg.cur_lc->predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return y;
	}

	if ( !cg.cur_lc->attackerTime ) {
		return y;
	}

	playerNum = cg.cur_lc->predictedPlayerState.persistant[PERS_ATTACKER];
	if ( playerNum < 0 || playerNum >= MAX_CLIENTS || playerNum == cg.cur_ps->playerNum ) {
		return y;
	}

	if ( !cgs.playerinfo[playerNum].infoValid ) {
		cg.cur_lc->attackerTime = 0;
		return y;
	}

	t = cg.time - cg.cur_lc->attackerTime;
	if ( t > ATTACKER_HEAD_TIME ) {
		cg.cur_lc->attackerTime = 0;
		return y;
	}

	size = ICON_SIZE * 1.25;

	angles[PITCH] = 0;
	angles[YAW] = 180;
	angles[ROLL] = 0;
	CG_DrawHead( 640 - size, y, size, size, playerNum, angles );

	info = CG_ConfigString( CS_PLAYERS + playerNum );
	name = Info_ValueForKey(  info, "n" );
	y += size;
	color[0] = color[1] = color[2] = 1;
	color[3] = 0.5f;
	CG_DrawString( 635, y + 2, name, UI_RIGHT|UI_DROPSHADOW|UI_BIGFONT, color );

	return y + 2 + CG_DrawStringLineHeight( UI_BIGFONT );
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) {
	char		*s;

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, 
		cg.latestSnapshotNum, cgs.serverCommandSequence );

	CG_DrawString( 635, y + 2, s, UI_RIGHT|UI_DROPSHADOW|UI_BIGFONT, NULL );

	return y + 2 + CG_DrawStringLineHeight( UI_BIGFONT );
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS( float y ) {
	char		*s;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	if (cg.viewport != 0) {
		return y;
	}

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / (float)total;

		s = va( "%ifps", fps );
		CG_DrawString( 635, y + 2, s, UI_RIGHT|UI_DROPSHADOW|UI_BIGFONT, NULL );
	}

	return y + 2 + CG_DrawStringLineHeight( UI_BIGFONT );
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer( float y ) {
	char		*s;
	int			hours, mins, seconds;
	int			msec;

	msec = cg.time - cgs.levelStartTime;

	seconds	= ( msec / 1000 ) % 60;
	mins	= ( msec / ( 1000*60 ) ) % 60;
	hours	= ( msec / ( 1000*60*60 ) );

	if ( hours > 0 ) {
		s = va( "%i:%s%i:%s%i", hours, mins < 10 ? "0" : "", mins, seconds < 10 ? "0" : "", seconds );
	} else {
		s = va( "%i:%s%i", mins, seconds < 10 ? "0" : "", seconds );
	}

	CG_DrawString( 635, y + 2, s, UI_RIGHT|UI_DROPSHADOW|UI_BIGFONT, NULL );

	return y + 2 + CG_DrawStringLineHeight( UI_BIGFONT );
}


/*
=================
CG_DrawTeamOverlay
=================
*/

// ZTM: TODO: Use CG_DrawStringLineHeight for team overlay
static float CG_DrawTeamOverlay( float y, qboolean right, qboolean upper ) {
	int x, w, h, xx;
	int i, j, len;
	const char *p;
	vec4_t		hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	playerInfo_t *pi;
	gitem_t	*item;
	int ret_y, count;
	int team;

	if ( !cg_drawTeamOverlay.integer ) {
		return y;
	}

	team = cg.cur_ps->persistant[PERS_TEAM];

	if ( team != TEAM_RED && team != TEAM_BLUE ) {
		return y; // Not on any team
	}

	if (cg.time - sortedTeamPlayersTime[team] > 5000) {
		// Info is too out of date.
		return y;
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers[team] > 8) ? 8 : numSortedTeamPlayers[team];
	for (i = 0; i < count; i++) {
		pi = cgs.playerinfo + sortedTeamPlayers[team][i];
		if ( pi->infoValid && pi->team == team) {
			plyrs++;
			len = CG_DrawStrlen( pi->name, UI_TINYFONT );
			if (len > pwidth)
				pwidth = len;
		}
	}

	if (!plyrs)
		return y;

	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH*TINYCHAR_WIDTH)
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH*TINYCHAR_WIDTH;

	// max location name width
	lwidth = 0;
	for (i = 1; i < MAX_LOCATIONS; i++) {
		p = CG_ConfigString(CS_LOCATIONS + i);
		if (p && *p) {
			len = CG_DrawStrlen( p, UI_TINYFONT );
			if (len > lwidth)
				lwidth = len;
		}
	}

	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH*TINYCHAR_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH*TINYCHAR_WIDTH;

	w = pwidth + lwidth + (4 + 7) * TINYCHAR_WIDTH;

	if ( right )
		x = 640 - w;
	else
		x = 0;

	h = plyrs * TINYCHAR_HEIGHT;

	if ( upper ) {
		ret_y = y + h;
	} else {
		y -= h;
		ret_y = y;
	}

	if ( team == TEAM_RED ) {
		hcolor[0] = 1.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 0.0f;
		hcolor[3] = 0.33f;
	} else { // if ( team == TEAM_BLUE )
		hcolor[0] = 0.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = 0.33f;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );

	for (i = 0; i < count; i++) {
		pi = cgs.playerinfo + sortedTeamPlayers[team][i];
		if ( pi->infoValid && pi->team == team) {

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt( xx, y, pi->name, UI_TINYFONT, NULL, 0, TEAM_OVERLAY_MAXNAME_WIDTH, 0 );

			if (lwidth) {
				p = CG_ConfigString(CS_LOCATIONS + pi->location);
				if (!p || !*p)
					p = "unknown";
				xx = x + TINYCHAR_WIDTH * 2 + pwidth;
				CG_DrawStringExt( xx, y, p, UI_TINYFONT, NULL, 0, TEAM_OVERLAY_MAXLOCATION_WIDTH, 0 );
			}

			CG_GetColorForHealth( pi->health, pi->armor, hcolor );

			Com_sprintf (st, sizeof(st), "%3i %3i", pi->health,	pi->armor);

			xx = x + TINYCHAR_WIDTH * 3 + pwidth + lwidth;

			CG_DrawString( xx, y, st, UI_TINYFONT, hcolor );

			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			if ( cg_weapons[pi->curWeapon].weaponIcon ) {
				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
					cg_weapons[pi->curWeapon].weaponIcon );
			} else {
				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
					cgs.media.deferShader );
			}

			// Draw powerup icons
			if (right) {
				xx = x;
			} else {
				xx = x + w - TINYCHAR_WIDTH;
			}
			for (j = 0; j <= PW_NUM_POWERUPS; j++) {
				if (pi->powerups & (1 << j)) {

					item = BG_FindItemForPowerup( j );

					if (item) {
						CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
						trap_R_RegisterShader( item->icon ) );
						if (right) {
							xx -= TINYCHAR_WIDTH;
						} else {
							xx += TINYCHAR_WIDTH;
						}
					}
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	return ret_y;
//#endif
}


/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight(stereoFrame_t stereoFrame)
{
	float	y;

	y = 0;

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_TOP);

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1 ) {
		y = CG_DrawTeamOverlay( y, qtrue, qtrue );
	} 
	if ( cg_drawSnapshot.integer ) {
		y = CG_DrawSnapshot( y );
	}
	if (cg_drawFPS.integer && (stereoFrame == STEREO_CENTER || stereoFrame == STEREO_RIGHT)) {
		y = CG_DrawFPS( y );
	}
	if ( cg_drawTimer.integer ) {
		y = CG_DrawTimer( y );
	}
	if ( cg_drawAttacker.integer ) {
		CG_DrawAttacker( y );
	}

}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

/*
=================
CG_DrawScores

Draw the small two score display
=================
*/
#ifndef MISSIONPACK_HUD
static float CG_DrawScores( float y ) {
	const char	*s;
	int			s1, s2, score;
	int			x, w;
	int			v;
	vec4_t		color;
	float		y1;
	gitem_t		*item;
	int			scoreHeight;

	if ( !cg_drawScores.integer ) {
		return y;
	}

	s1 = cgs.scores1;
	s2 = cgs.scores2;

	scoreHeight = CG_DrawStringLineHeight( UI_BIGFONT ) + 6;
	y -= scoreHeight;

	y1 = y;

	// draw from the right side to left
	if ( cgs.gametype >= GT_TEAM ) {
		x = 640;
		color[0] = 0.0f;
		color[1] = 0.0f;
		color[2] = 1.0f;
		color[3] = 0.33f;
		s = va( "%2i", s2 );
		w = CG_DrawStrlen( s, UI_BIGFONT ) + 8;
		x -= w;
		CG_FillRect( x, y-4,  w, scoreHeight, color );
		if ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_BLUE ) {
			CG_DrawPic( x, y-4, w, scoreHeight, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 4, y, s, 1.0F);

		if ( cgs.gametype == GT_CTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( PW_BLUEFLAG );

			if (item) {
				y1 = y - scoreHeight;
				if( cgs.blueflag >= 0 && cgs.blueflag <= 2 ) {
					CG_DrawPic( x, y1-4, w, scoreHeight, cgs.media.blueFlagShader[cgs.blueflag] );
				}
			}
		}

#ifdef MISSIONPACK
		if ( cgs.gametype == GT_1FCTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( PW_NEUTRALFLAG );

			if (item) {
				y1 = y - scoreHeight;
				if( cgs.flagStatus >= 0 && cgs.flagStatus <= 4 ) {
					vec4_t color = {1, 1, 1, 1};
					int index = 0;
					if (cgs.flagStatus == FLAG_TAKEN_RED) {
						color[1] = color[2] = 0;
						index = 1;
					} else if (cgs.flagStatus == FLAG_TAKEN_BLUE) {
						color[0] = color[1] = 0;
						index = 1;
					} else if (cgs.flagStatus == FLAG_DROPPED) {
						index = 2;
					}
					trap_R_SetColor(color);
					CG_DrawPic( x, y1-4, w, scoreHeight, cgs.media.flagShaders[index] );
					trap_R_SetColor(NULL);
				}
			}
		}
#endif

		color[0] = 1.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		color[3] = 0.33f;
		s = va( "%2i", s1 );
		w = CG_DrawStrlen( s, UI_BIGFONT ) + 8;
		x -= w;
		CG_FillRect( x, y-4,  w, scoreHeight, color );
		if ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_RED ) {
			CG_DrawPic( x, y-4, w, scoreHeight, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 4, y, s, 1.0F);

		if ( cgs.gametype == GT_CTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( PW_REDFLAG );

			if (item) {
				y1 = y - scoreHeight;
				if( cgs.redflag >= 0 && cgs.redflag <= 2 ) {
					CG_DrawPic( x, y1-4, w, scoreHeight, cgs.media.redFlagShader[cgs.redflag] );
				}
			}
		}

		if ( cgs.gametype >= GT_CTF ) {
			v = cgs.capturelimit;
		} else {
			v = cgs.fraglimit;
		}
		if ( v ) {
			s = va( "%2i", v );
			w = CG_DrawStrlen( s, UI_BIGFONT ) + 8;
			x -= w;
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

	} else {
		qboolean	spectator;

		x = 640;
		score = cg.cur_ps->persistant[PERS_SCORE];
		spectator = ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_SPECTATOR );

		// always show your score in the second box if not in first place
		if ( s1 != score ) {
			s2 = score;
		}
		if ( s2 != SCORE_NOT_PRESENT ) {
			s = va( "%2i", s2 );
			w = CG_DrawStrlen( s, UI_BIGFONT ) + 8;
			x -= w;
			if ( !spectator && score == s2 && score != s1 ) {
				color[0] = 1.0f;
				color[1] = 0.0f;
				color[2] = 0.0f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, scoreHeight, color );
				CG_DrawPic( x, y-4, w, scoreHeight, cgs.media.selectShader );
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, scoreHeight, color );
			}	
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

		// first place
		if ( s1 != SCORE_NOT_PRESENT ) {
			s = va( "%2i", s1 );
			w = CG_DrawStrlen( s, UI_BIGFONT ) + 8;
			x -= w;
			if ( !spectator && score == s1 ) {
				color[0] = 0.0f;
				color[1] = 0.0f;
				color[2] = 1.0f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, scoreHeight, color );
				CG_DrawPic( x, y-4, w, scoreHeight, cgs.media.selectShader );
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, scoreHeight, color );
			}	
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

		if ( cgs.fraglimit ) {
			s = va( "%2i", cgs.fraglimit );
			w = CG_DrawStrlen( s, UI_BIGFONT ) + 8;
			x -= w;
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

	}

	return y1 - 8;
}
#endif // MISSIONPACK_HUD

/*
================
CG_DrawPowerups
================
*/
#ifndef MISSIONPACK_HUD
static float CG_DrawPowerups( float y ) {
	int		sorted[MAX_POWERUPS];
	int		sortedTime[MAX_POWERUPS];
	int		i, j, k;
	int		active;
	playerState_t	*ps;
	int		t;
	gitem_t	*item;
	int		x;
	int		color;
	float	size;
	float	f;
	static float colors[2][4] = { 
    { 0.2f, 1.0f, 0.2f, 1.0f } , 
    { 1.0f, 0.2f, 0.2f, 1.0f } 
  };

	ps = cg.cur_ps;

	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		return y;
	}

	// sort the list by time remaining
	active = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( !ps->powerups[ i ] ) {
			continue;
		}

#ifdef MISSIONPACK
		if (i == PW_SCOUT
			|| i == PW_GUARD
			|| i == PW_DOUBLER
			|| i == PW_AMMOREGEN)
		{
			continue;
		}
#endif

		t = ps->powerups[ i ] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if ( t < 0 || t > 999000) {
			continue;
		}

		// insert into the list
		for ( j = 0 ; j < active ; j++ ) {
			if ( sortedTime[j] >= t ) {
				for ( k = active - 1 ; k >= j ; k-- ) {
					sorted[k+1] = sorted[k];
					sortedTime[k+1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	// draw the icons and timers
	x = 640 - ICON_SIZE - CHAR_WIDTH * 2;
	for ( i = 0 ; i < active ; i++ ) {
		item = BG_FindItemForPowerup( sorted[i] );

    if (item) {

		  color = 1;

		  y -= ICON_SIZE;

		  CG_DrawField( x, y + ICON_SIZE/2, UI_VA_CENTER, 2, sortedTime[ i ] / 1000, colors[color] );

		  t = ps->powerups[ sorted[i] ];
		  if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
		  } else {
			  vec4_t	modulate;

			  f = (float)( t - cg.time ) / POWERUP_BLINK_TIME;
			  f -= (int)f;
			  modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
			  trap_R_SetColor( modulate );
		  }

		  if ( cg.cur_lc->powerupActive == sorted[i] && 
			  cg.time - cg.cur_lc->powerupTime < PULSE_TIME ) {
			  f = 1.0 - ( ( (float)cg.time - cg.cur_lc->powerupTime ) / PULSE_TIME );
			  size = ICON_SIZE * ( 1.0 + ( PULSE_SCALE - 1.0 ) * f );
		  } else {
			  size = ICON_SIZE;
		  }

		  CG_DrawPic( 640 - size, y + ICON_SIZE / 2 - size / 2, 
			  size, size, trap_R_RegisterShader( item->icon ) );

			trap_R_SetColor( NULL );
		}
	}

	return y;
}
#endif // MISSIONPACK_HUD

/*
=====================
CG_DrawLowerRight

=====================
*/
#ifndef MISSIONPACK_HUD
static void CG_DrawLowerRight( void ) {
	float	y;

	y = 480 - ICON_SIZE;

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_BOTTOM);

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 2 ) {
		y = CG_DrawTeamOverlay( y, qtrue, qfalse );
	} 

	y = CG_DrawScores( y );
	CG_DrawPowerups( y );
}
#endif // MISSIONPACK_HUD

/*
===================
CG_DrawPickupItem
===================
*/
#ifndef MISSIONPACK_HUD
static int CG_DrawPickupItem( int y ) {
	int		value;
	float	*fadeColor;

	if ( cg.cur_ps->stats[STAT_HEALTH] <= 0 ) {
		return y;
	}

	y -= ICON_SIZE;

	value = cg.cur_lc->itemPickup;
	if ( value ) {
		fadeColor = CG_FadeColor( cg.cur_lc->itemPickupTime, 3000 );
		if ( fadeColor ) {
			CG_RegisterItemVisuals( value );
			trap_R_SetColor( fadeColor );
			CG_DrawPic( 8, y, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
			trap_R_SetColor( NULL );
			CG_DrawString( ICON_SIZE + 16, y + (ICON_SIZE/2), BG_ItemForItemNum( value )->pickup_name, UI_VA_CENTER|UI_DROPSHADOW|UI_BIGFONT, fadeColor );
		}
	}
	
	return y;
}
#endif // MISSIONPACK_HUD

/*
=====================
CG_DrawLowerLeft

=====================
*/
#ifndef MISSIONPACK_HUD
static void CG_DrawLowerLeft( void ) {
	float	y;

	y = 480 - ICON_SIZE;

	CG_SetScreenPlacement(PLACE_LEFT, PLACE_BOTTOM);

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 3 ) {
		y = CG_DrawTeamOverlay( y, qfalse, qfalse );
	} 


	CG_DrawPickupItem( y );
}
#endif // MISSIONPACK_HUD


//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
#ifndef MISSIONPACK_HUD
static void CG_DrawTeamInfo( void ) {
	int h;
	int i;
	vec4_t		hcolor;
	int		chatHeight;
	int		lineHeight;

#define CHATLOC_Y 420 // bottom end
#define CHATLOC_X 0

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
		chatHeight = cg_teamChatHeight.integer;
	else
		chatHeight = TEAMCHAT_HEIGHT;
	if (chatHeight <= 0)
		return; // disabled

	CG_SetScreenPlacement( PLACE_LEFT, PLACE_BOTTOM );

	lineHeight = CG_DrawStringLineHeight( UI_TINYFONT );

	if (cgs.teamLastChatPos != cgs.teamChatPos) {
		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer) {
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * lineHeight;

		if ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_RED ) {
			hcolor[0] = 1.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		} else if ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_BLUE ) {
			hcolor[0] = 0.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 1.0f;
			hcolor[3] = 0.33f;
		} else {
			hcolor[0] = 0.0f;
			hcolor[1] = 1.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}

		trap_R_SetColor( hcolor );
		CG_DrawPic( CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar );
		trap_R_SetColor( NULL );

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--) {
			CG_DrawString( CHATLOC_X + TINYCHAR_WIDTH, 
				CHATLOC_Y - (cgs.teamChatPos - i)*lineHeight, 
				cgs.teamChatMsgs[i % chatHeight],
				UI_TINYFONT, hcolor );
		}
	}
}
#endif // MISSIONPACK_HUD

/*
===================
CG_DrawHoldableItem
===================
*/
#ifndef MISSIONPACK_HUD
static void CG_DrawHoldableItem( void ) { 
	int		value;

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_CENTER);

	value = cg.cur_ps->stats[STAT_HOLDABLE_ITEM];
	if ( value ) {
		CG_RegisterItemVisuals( value );
		CG_DrawPic( 640-ICON_SIZE, (SCREEN_HEIGHT-ICON_SIZE)/2, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
	}

}
#endif // MISSIONPACK_HUD

#ifdef MISSIONPACK
/*
===================
CG_DrawPersistantPowerup
===================
*/
#ifndef MISSIONPACK_HUD
static void CG_DrawPersistantPowerup( void ) { 
	int		value;

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_CENTER);

	value = cg.cur_ps->stats[STAT_PERSISTANT_POWERUP];
	if ( value ) {
		CG_RegisterItemVisuals( value );
		CG_DrawPic( 640-ICON_SIZE, (SCREEN_HEIGHT-ICON_SIZE)/2 - ICON_SIZE, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
	}
}
#endif // MISSIONPACK_HUD
#endif // MISSIONPACK


/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward( void ) { 
	float	*color;
	int		i, count;
	float	x, y;
	char	buf[32];

	if ( !cg_drawRewards.integer ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	color = CG_FadeColor( cg.cur_lc->rewardTime, REWARD_TIME );
	if ( !color ) {
		if (cg.cur_lc->rewardStack > 0) {
			for(i = 0; i < cg.cur_lc->rewardStack; i++) {
				cg.cur_lc->rewardSound[i] = cg.cur_lc->rewardSound[i+1];
				cg.cur_lc->rewardShader[i] = cg.cur_lc->rewardShader[i+1];
				cg.cur_lc->rewardCount[i] = cg.cur_lc->rewardCount[i+1];
			}
			cg.cur_lc->rewardTime = cg.time;
			cg.cur_lc->rewardStack--;
			color = CG_FadeColor( cg.cur_lc->rewardTime, REWARD_TIME );
			if ( cg.cur_lc->rewardSound[0] ) {
				trap_S_StartLocalSound( cg.cur_lc->rewardSound[0], CHAN_ANNOUNCER );
			}
		} else {
			return;
		}
	}

	trap_R_SetColor( color );

	/*
	count = cg.cur_lc->rewardCount[0]/10;				// number of big rewards to draw

	if (count) {
		y = 4;
		x = 320 - count * ICON_SIZE;
		for ( i = 0 ; i < count ; i++ ) {
			CG_DrawPic( x, y, (ICON_SIZE*2)-4, (ICON_SIZE*2)-4, cg.cur_lc->rewardShader[0] );
			x += (ICON_SIZE*2);
		}
	}

	count = cg.cur_lc->rewardCount[0] - count*10;		// number of small rewards to draw
	*/

	if ( cg.cur_lc->rewardCount[0] >= 10 ) {
		y = 56;
		x = 320 - ICON_SIZE/2 + 2;
		CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.cur_lc->rewardShader[0] );
		Com_sprintf(buf, sizeof(buf), "%d", cg.cur_lc->rewardCount[0]);
		CG_DrawString( SCREEN_WIDTH / 2, y+ICON_SIZE, buf, UI_CENTER|UI_DROPSHADOW|UI_SMALLFONT, color );
	}
	else {

		count = cg.cur_lc->rewardCount[0];

		y = 56;
		x = 320 - count * ICON_SIZE/2 + 2;
		for ( i = 0 ; i < count ; i++ ) {
			CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.cur_lc->rewardShader[0] );
			x += ICON_SIZE;
		}
	}
	trap_R_SetColor( NULL );
}


/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
} lagometer_t;

lagometer_t		lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void ) {
	float		x, y;
	int			cmdNum;
	usercmd_t	cmd;
	const char		*s;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &cmd, cg.cur_localPlayerNum );
	if ( cmd.serverTime <= cg.cur_ps->commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	// also add text in center of screen
	s = "Connection Interrupted";
	CG_DrawString( SCREEN_WIDTH / 2, 100, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_BOTTOM);

	x = 640 - 48;
	y = 480 - 48;

	CG_DrawPic( x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga" ) );
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( void ) {
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	if ( !cg_lagometer.integer || cgs.localServer ) {
		CG_DrawDisconnect();
		return;
	}

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_BOTTOM);

	//
	// draw the graph
	//
#ifdef MISSIONPACK_HUD
	x = 640 - 48;
	y = 480 - 144;
#else
	x = 640 - 48;
	y = 480 - 48;
#endif

	CG_DrawPic( x, y, 48, 48, cgs.media.lagometerShader );

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
			}
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	trap_R_SetColor( NULL );

	if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
		CG_DrawBigString( x, y, "snc", 1.0 );
	}

	CG_DrawDisconnect();
}



/*
===============================================================================

VOIP METER

===============================================================================
*/

/*
==============
CG_DrawVoipMeter
==============
*/
void CG_DrawVoipMeter( void ) {
	char	buffer[16];
	char	string[256];
	int		limit, i;
	int		voipTime;
	float	voipPower;

	if ( !cg_voipShowMeter.integer ) {
		return; // player doesn't want to show meter at all.
	}

	voipTime = trap_GetVoipTime( cg.cur_lc->playerNum );

	// check if voip was used in the last 1/4 second
	if ( !voipTime || voipTime < cg.time - 250 ) {
		return;
	}

	CG_SetScreenPlacement( PLACE_CENTER, PLACE_TOP );

	voipPower = trap_GetVoipPower( cg.cur_lc->playerNum );

	limit = (int) (voipPower * 10.0f);
	if (limit > 10)
		limit = 10;

	for (i = 0; i < limit; i++)
		buffer[i] = '*';
	while (i < 10)
		buffer[i++] = ' ';
	buffer[i] = '\0';

	Com_sprintf( string, sizeof ( string ), "VoIP: [%s]", buffer );
	CG_DrawString( SCREEN_WIDTH / 2, 10, string, UI_CENTER|UI_DROPSHADOW|UI_TINYFONT, NULL );
}


/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the viewport
for a few moments
==============
*/
void CG_CenterPrint( int localPlayerNum, const char *str, int y, float charScale ) {
	localPlayer_t *player;
	char *s;

	player = &cg.localPlayers[localPlayerNum];

	if ( cg.numViewports != 1 ) {
		charScale *= cg_splitviewTextScale.value;
	} else {
		charScale *= cg_hudTextScale.value;
	}

	Q_strncpyz( player->centerPrint, str, sizeof(player->centerPrint) );

	player->centerPrintTime = cg.time;
	player->centerPrintY = y;
	player->centerPrintCharScale = charScale;

	// count the number of lines for centering
	player->centerPrintLines = 1;
	s = player->centerPrint;
	while( *s ) {
		if (*s == '\n')
			player->centerPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void ) {
	char	*start;
	int		l;
	int		y;
	int		charHeight;
	float	*color;
	float	scale;

	if ( !cg.cur_lc->centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.cur_lc->centerPrintTime, 1000 * cg_centertime.value );
	if ( !color ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	start = cg.cur_lc->centerPrint;

	scale = cg.cur_lc->centerPrintCharScale;
	charHeight = GIANTCHAR_HEIGHT;

	if ( scale <= 0 ) {
		scale = charHeight / 48.0f;
	} else {
		charHeight = 48 * scale;
	}

	y = cg.cur_lc->centerPrintY - cg.cur_lc->centerPrintLines * charHeight / 2;

	while ( 1 ) {
		char linebuffer[1024];

		for ( l = 0; l < 50; l++ ) {
			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		CG_DrawStringExt( SCREEN_WIDTH / 2, y, linebuffer, UI_CENTER|UI_DROPSHADOW|UI_GIANTFONT|UI_NOSCALE, color, scale, 0, 0 );
		y += charHeight + 6;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}
}


/*
==============
CG_GlobalCenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_GlobalCenterPrint( const char *str, int y, float charScale ) {
	char	*s;

	if ( cg.numViewports != 1 ) {
		charScale *= cg_splitviewTextScale.value;
	} else {
		charScale *= cg_hudTextScale.value;
	}

	Q_strncpyz( cg.centerPrint, str, sizeof(cg.centerPrint) );

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharScale = charScale;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while( *s ) {
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawGlobalCenterString
===================
*/
static void CG_DrawGlobalCenterString( void ) {
	char	*start;
	int		l;
	int		y;
	int		charHeight;
	float	*color;
	float	scale;

	if ( !cg.centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
	if ( !color ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	start = cg.centerPrint;

	scale = cg.centerPrintCharScale;
	charHeight = GIANTCHAR_HEIGHT;

	if ( scale <= 0 ) {
		scale = charHeight / 48.0f;
	} else {
		charHeight = 48 * scale;
	}

	y = cg.centerPrintY - cg.centerPrintLines * charHeight / 2;

	while ( 1 ) {
		char linebuffer[1024];

		for ( l = 0; l < 50; l++ ) {
			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		CG_DrawStringExt( SCREEN_WIDTH / 2, y, linebuffer, UI_CENTER|UI_DROPSHADOW|UI_GIANTFONT|UI_NOSCALE, color, scale, 0, 0 );
		y += charHeight + 6;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}
}


/*
================================================================================

CROSSHAIR

================================================================================
*/


/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair(void)
{
	float		w, h;
	qhandle_t	hShader;
	float		f;
	float		x, y;
	int			ca;

	if ( !cg_drawCrosshair.integer ) {
		return;
	}

	if ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	if ( cg.cur_lc->renderingThirdPerson ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	// set color based on health
	if ( cg_crosshairHealth.integer ) {
		vec4_t		hcolor;

		CG_ColorForHealth( hcolor );
		trap_R_SetColor( hcolor );
	}

	if (cg.numViewports > 1) {
		// In splitscreen make crosshair normal [non-splitscreen] size, so it is easier to see.
		w = h = cg_crosshairSize.value * 2.0f;
	} else {
		w = h = cg_crosshairSize.value;
	}

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.cur_lc->itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
		h *= ( 1 + f );
	}

	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;

	ca = cg_drawCrosshair.integer;
	if (ca < 0) {
		ca = 0;
	}
	hShader = cgs.media.crosshairShader[ ca % NUM_CROSSHAIRS ];

	CG_DrawPic( ((SCREEN_WIDTH-w)*0.5f)+x, ((SCREEN_HEIGHT-h)*0.5f)+y, w, h, hShader );

	trap_R_SetColor( NULL );
}

/*
=================
CG_DrawCrosshair3D
=================
*/
static void CG_DrawCrosshair3D(void)
{
	float		w;
	qhandle_t	hShader;
	float		f;
	int			ca;

	vec4_t hcolor;
	trace_t trace;
	vec3_t endpos;
	float stereoSep, zProj, maxdist, xmax;
	char rendererinfos[128];
	refEntity_t ent;

	if ( !cg_drawCrosshair.integer ) {
		return;
	}

	if ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	if ( cg.cur_lc->renderingThirdPerson ) {
		return;
	}

	// set color based on health
	if ( cg_crosshairHealth.integer ) {
		CG_ColorForHealth( hcolor );
	} else {
		hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1;
	}

	w = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.cur_lc->itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
	}

	ca = cg_drawCrosshair.integer;
	if (ca < 0) {
		ca = 0;
	}
	hShader = cgs.media.crosshairShader[ ca % NUM_CROSSHAIRS ];

	// Use a different method rendering the crosshair so players don't see two of them when
	// focusing their eyes at distant objects with high stereo separation
	// We are going to trace to the next shootable object and place the crosshair in front of it.

	// first get all the important renderer information
	trap_Cvar_VariableStringBuffer("r_zProj", rendererinfos, sizeof(rendererinfos));
	zProj = atof(rendererinfos);
	trap_Cvar_VariableStringBuffer("r_stereoSeparation", rendererinfos, sizeof(rendererinfos));
	stereoSep = zProj / atof(rendererinfos);
	
	xmax = zProj * tan(cg.refdef.fov_x * M_PI / 360.0f);
	
	// let the trace run through until a change in stereo separation of the crosshair becomes less than one pixel.
	maxdist = cgs.glconfig.vidWidth * stereoSep * zProj / (2 * xmax);
	VectorMA(cg.refdef.vieworg, maxdist, cg.refdef.viewaxis[0], endpos);
	CG_Trace(&trace, cg.refdef.vieworg, NULL, NULL, endpos, 0, MASK_SHOT);
	
	memset(&ent, 0, sizeof(ent));
	ent.reType = RT_SPRITE;
	ent.renderfx = RF_DEPTHHACK | RF_CROSSHAIR;
	ent.shaderRGBA[0] = 255 * hcolor[0];
	ent.shaderRGBA[1] = 255 * hcolor[1];
	ent.shaderRGBA[2] = 255 * hcolor[2];
	ent.shaderRGBA[3] = 255 * hcolor[3];
	
	VectorCopy(trace.endpos, ent.origin);
	
	// scale the crosshair so it appears the same size for all distances
	ent.radius = w / 640 * xmax * trace.fraction * maxdist / zProj;
	ent.customShader = hShader;

	trap_R_AddRefEntityToScene(&ent);
}


/*
=================
CG_DrawThirdPersonCrosshair
=================
*/
static void CG_DrawThirdPersonCrosshair(void)
{
	float		w;
	qhandle_t	hShader;
	float		f;
	int			ca;

	vec4_t hcolor;
	trace_t trace;
	vec3_t endpos, distToScreen;
	float maxdist, xmax;
	refEntity_t ent;

	if ( !cg_drawCrosshair.integer ) {
		return;
	}

	if ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	if ( !cg.cur_lc->renderingThirdPerson || !cg_thirdPerson[cg.cur_localPlayerNum].integer ) {
		return;
	}

	// set color based on health
	if ( cg_crosshairHealth.integer ) {
		CG_ColorForHealth( hcolor );
	} else {
		hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1;
	}

	w = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.cur_lc->itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
	}

	ca = cg_drawCrosshair.integer;
	if (ca < 0) {
		ca = 0;
	}
	hShader = cgs.media.crosshairShader[ ca % NUM_CROSSHAIRS ];

	xmax = tan(cg.refdef.fov_x * M_PI / 360.0f);
	// We are going to trace to the next shootable object and place the crosshair in front of it.
	maxdist = cgs.glconfig.vidWidth / (2 * xmax);
	VectorMA(cg.firstPersonViewOrg, maxdist, cg.firstPersonViewAxis[0], endpos);
	CG_Trace(&trace, cg.firstPersonViewOrg, NULL, NULL, endpos, 0, MASK_SHOT);

	memset(&ent, 0, sizeof(ent));
	ent.reType = RT_SPRITE;
	ent.renderfx = RF_DEPTHHACK | RF_CROSSHAIR;
	ent.shaderRGBA[0] = 255 * hcolor[0];
	ent.shaderRGBA[1] = 255 * hcolor[1];
	ent.shaderRGBA[2] = 255 * hcolor[2];
	ent.shaderRGBA[3] = 255 * hcolor[3];

	VectorCopy(trace.endpos, ent.origin);
	VectorSubtract( cg.refdef.vieworg, ent.origin, distToScreen );

	// scale the crosshair so it appears the same size for all distances
	ent.radius = w / 640 * xmax * VectorLength( distToScreen );
	ent.customShader = hShader;

	trap_R_AddRefEntityToScene(&ent);
}



/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity( void ) {
	trace_t		trace;
	vec3_t		start, end;
	int			content;

	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, 131072, cg.refdef.viewaxis[0], end );

	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, 
		cg.cur_ps->playerNum, CONTENTS_SOLID|CONTENTS_BODY );
	if ( trace.entityNum >= MAX_CLIENTS ) {
		return;
	}

	// if the player is in fog, don't show it
	content = CG_PointContents( trace.endpos, 0 );
	if ( content & CONTENTS_FOG ) {
		return;
	}

	// if the player is invisible, don't show it
	if ( cg_entities[ trace.entityNum ].currentState.powerups & ( 1 << PW_INVIS ) ) {
		return;
	}

	// update the fade timer
	cg.cur_lc->crosshairPlayerNum = trace.entityNum;
	cg.cur_lc->crosshairPlayerTime = cg.time;
}


/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames( void ) {
	float		*color;
	char		*name;
	int			crosshairSize, lineHeight, y;

	if ( !cg_drawCrosshair.integer ) {
		return;
	}
	if ( !cg_drawCrosshairNames.integer ) {
		return;
	}
	if ( cg.cur_lc->renderingThirdPerson ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	color = CG_FadeColor( cg.cur_lc->crosshairPlayerTime, 1000 );
	if ( !color ) {
		return;
	}

	if (cg.numViewports > 1) {
		// In splitscreen make crosshair normal [non-splitscreen] size, so it is easier to see.
		crosshairSize = cg_crosshairSize.value * 2.0f;
	} else {
		crosshairSize = cg_crosshairSize.value;
	}

	// ZTM: TODO: Check how much different this Y is from original 170
	// space for two lines above crosshair
	lineHeight = CG_DrawStringLineHeight( UI_BIGFONT );
	y = ( SCREEN_HEIGHT - crosshairSize ) / 2 - lineHeight * 2;

	name = cgs.playerinfo[ cg.cur_lc->crosshairPlayerNum ].name;
	color[3] *= 0.5f;
	CG_DrawStringExt( SCREEN_WIDTH / 2, 170, name, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, color, 0, 0, 1 );

	if ( cg_voipShowCrosshairMeter.integer )
	{
		float voipPower = trap_GetVoipPower( cg.cur_lc->crosshairPlayerNum );
		int voipTime = trap_GetVoipTime( cg.cur_lc->crosshairPlayerNum );

		if ( voipPower > 0 && voipTime > 0 && voipTime >= cg.time - 250 ) {
			int limit, i;
			char string[256];
			char buffer[16];

			limit = (int) (voipPower * 10.0f);
			if (limit > 10)
				limit = 10;

			for (i = 0; i < limit; i++)
				buffer[i] = '*';
			while (i < 10)
				buffer[i++] = ' ';
			buffer[i] = '\0';

			Com_sprintf( string, sizeof ( string ), "VoIP: [%s]", buffer );
			CG_DrawStringExt( SCREEN_WIDTH / 2, y + lineHeight, string, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, color, 0, 0, 1 );
		}
	}
}

/*
=====================
CG_DrawShaderInfo
=====================
*/
static void CG_DrawShaderInfo( void ) {
	char		name[MAX_QPATH];
	float		x, y, width, height;
	vec4_t		color = { 1, 1, 1, 1 };
	qhandle_t	hShader;
	trace_t		trace, trace2;
	vec3_t		start, end;

	if ( !cg_drawShaderInfo.integer ) {
		return;
	}

	width = 128;
	height = 128;

	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, 131072, cg.refdef.viewaxis[0], end );

	// Get shader behind clip volume:
	// Do a trace that hits all surfaces (including solid and non-solid),
	// and a trace that only hits solid. Use which ever is closer and use
	// solid if 'any contents' trace didn't get a textured surface (such as clip)
	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, cg.cur_ps->playerNum, ~0 );
	CG_Trace( &trace2, start, vec3_origin, vec3_origin, end, cg.cur_ps->playerNum, CONTENTS_SOLID );

	// trace2 is valid and closer or trace is invalid
	if ( trace2.surfaceNum > 0 && ( trace2.fraction < trace.fraction || trace.surfaceNum <= 0 ) ) {
		Com_Memcpy( &trace, &trace2, sizeof ( trace_t ) );
	}

	if ( trace.surfaceNum <= 0 ) {
		return;
	}

	hShader = trap_R_GetSurfaceShader( trace.surfaceNum, LIGHTMAP_2D );

	trap_R_GetShaderName( hShader, name, sizeof ( name ) );

	CG_SetScreenPlacement(PLACE_LEFT, PLACE_BOTTOM);

	color[3] *= 0.5f;
	CG_DrawString( 0, SCREEN_HEIGHT - height, name, UI_VA_BOTTOM|UI_DROPSHADOW|UI_BIGFONT, color );

	x = 0;
	y = SCREEN_HEIGHT - height;

	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) {
	int lineHeight, y;

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_BOTTOM);

	lineHeight = CG_DrawStringLineHeight( UI_BIGFONT );
	y = SCREEN_HEIGHT - lineHeight * 2;

	CG_DrawString(320, y, "Spectator", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	y += lineHeight;

	if ( cgs.gametype == GT_TOURNAMENT ) {
		CG_DrawString(320, y, "Waiting to play", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	}
	else if ( cgs.gametype >= GT_TEAM ) {
		CG_DrawString(320, y, "Press ESC and use the JOIN menu to play", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void) {
	char	*s;
	int		sec;
	char	yesKeys[64];
	char	noKeys[64];
	int		lineHeight;

	if ( !cgs.voteTime ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_LEFT, PLACE_TOP);

	// play a talk beep whenever it is modified
	if ( cgs.voteModified ) {
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}

	CG_KeysStringForBinding( Com_LocalPlayerCvarName( cg.cur_localPlayerNum, "vote yes" ), yesKeys, sizeof (yesKeys) );
	CG_KeysStringForBinding( Com_LocalPlayerCvarName( cg.cur_localPlayerNum, "vote no" ), noKeys, sizeof (noKeys) );

	lineHeight = CG_DrawStringLineHeight( UI_SMALLFONT );

	s = va( "Vote (%i): %s", sec, cgs.voteString );
	CG_DrawSmallString( 2, 58, s, 1.0F );
	s = va( "Yes (%s): %i, No (%s): %i", yesKeys, cgs.voteYes, noKeys, cgs.voteNo );
	CG_DrawSmallString( 2, 58 + lineHeight, s, 1.0F );
#ifdef MISSIONPACK_HUD
	s = "or press ESC then click Vote";
	CG_DrawSmallString( 2, 58 + lineHeight * 2, s, 1.0F );
#endif
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void) {
	char	*s;
	int		sec, cs_offset;

	if ( cgs.playerinfo[cg.cur_ps->playerNum].team == TEAM_RED )
		cs_offset = 0;
	else if ( cgs.playerinfo[cg.cur_ps->playerNum].team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !cgs.teamVoteTime[cs_offset] ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_LEFT, PLACE_TOP);

	// play a talk beep whenever it is modified
	if ( cgs.teamVoteModified[cs_offset] ) {
		cgs.teamVoteModified[cs_offset] = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.teamVoteTime[cs_offset] ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
	s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
							cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset] );
	CG_DrawSmallString( 0, 90, s, 1.0F );
}

/*
=============
CG_AnyScoreboardShowing
=============
*/
qboolean CG_AnyScoreboardShowing( void ) {
	int i;

	for ( i = 0; i < CG_MaxSplitView(); i++ ) {
		if ( cg.localPlayers[i].playerNum != -1 && cg.localPlayers[i].scoreBoardShowing ) {
			return qtrue;
		}
	}

	return qfalse;
}

static qboolean CG_DrawScoreboard( void ) {
#ifdef MISSIONPACK_HUD
	static qboolean firstTime[MAX_SPLITVIEW] = {qtrue, qtrue, qtrue, qtrue};

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	if (menuScoreboard) {
		menuScoreboard->window.flags &= ~WINDOW_FORCED;
	}
	if (cg_paused.integer) {
		if (cg.cur_lc) {
			firstTime[cg.cur_localPlayerNum] = qtrue;
		}
		return qfalse;
	}

	// should never happen in Team Arena
	if (cgs.gametype == GT_SINGLE_PLAYER && cg.cur_lc && cg.cur_lc->predictedPlayerState.pm_type == PM_INTERMISSION ) {
		firstTime[cg.cur_localPlayerNum] = qtrue;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if ( cg.warmup && cg.cur_lc && !cg.cur_lc->showScores ) {
		return qfalse;
	}

	if ( !cg.cur_lc || cg.cur_lc->showScores || cg.cur_lc->predictedPlayerState.pm_type == PM_DEAD ||
		 cg.cur_lc->predictedPlayerState.pm_type == PM_INTERMISSION ) {
	} else {
		if ( !CG_FadeColor( cg.cur_lc->scoreFadeTime, FADE_TIME ) ) {
			// next time scoreboard comes up, don't print killer
			cg.cur_lc->killerName[0] = 0;
			firstTime[cg.cur_localPlayerNum] = qtrue;
			return qfalse;
		}
	}

	if (menuScoreboard == NULL) {
		if ( cgs.gametype >= GT_TEAM ) {
			menuScoreboard = Menus_FindByName("teamscore_menu");
		} else {
			menuScoreboard = Menus_FindByName("score_menu");
		}
	}

	if (menuScoreboard) {
		int selectedScore;

		if (cg.cur_lc) {
			if ( firstTime[cg.cur_localPlayerNum] ) {
				int i;

				firstTime[cg.cur_localPlayerNum] = qfalse;

				// reset selected score to self
				for (i = 0; i < cg.numScores; i++) {
					if (cg.cur_ps->playerNum == cg.scores[i].playerNum) {
						cg.cur_lc->selectedScore = i;
					}
				}

				// Update time now to prevent spectator list from jumping.
				cg.spectatorTime = trap_Milliseconds();
			}

			selectedScore = cg.cur_lc->selectedScore;
		} else {
			if ( cg.intermissionSelectedScore >= cg.numScores ) {
				cg.intermissionSelectedScore = cg.numScores - 1;
			}

			selectedScore = cg.intermissionSelectedScore;
		}

		if ( cgs.gametype >= GT_TEAM ) {
			int i, red, blue;

			red = blue = 0;

			for (i = 0; i < cg.numScores; i++) {
				if ( i == selectedScore ) {
					break;
				}
				if (cg.scores[i].team == TEAM_RED) {
					red++;
				} else if (cg.scores[i].team == TEAM_BLUE) {
					blue++;
				}
			}

			// only draw selection for the selected team (which may be spectator)
			if (cg.scores[selectedScore].team != TEAM_RED) {
				red = -1;
			}
			if (cg.scores[selectedScore].team != TEAM_BLUE) {
				blue = -1;
			}
			Menu_SetFeederSelection(menuScoreboard, FEEDER_REDTEAM_LIST, red, NULL);
			Menu_SetFeederSelection(menuScoreboard, FEEDER_BLUETEAM_LIST, blue, NULL);
		} else {
			Menu_SetFeederSelection(menuScoreboard, FEEDER_SCOREBOARD, selectedScore, NULL);
		}

		Menu_Paint(menuScoreboard, qtrue);
	}

	return qtrue;
#else
	return CG_DrawOldScoreboard();
#endif
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) {
//	int key;
#ifdef MISSIONPACK_HUD
	//if (cg_singlePlayer.integer) {
	//	CG_DrawCenterString();
	//	return;
	//}
#else
	if ( cgs.gametype == GT_SINGLE_PLAYER ) {
		CG_DrawCenterString();
		return;
	}
#endif
	cg.cur_lc->scoreFadeTime = cg.time;
	cg.cur_lc->scoreBoardShowing = CG_DrawScoreboard();
}

/*
=================
CG_DrawBotInfo

Draw info for bot that player is following.
=================
*/
static qboolean CG_DrawBotInfo( int y ) {
	const char	*info, *str, *leader, *carrying, *action, *node;
	int lineHeight;

	if ( !(cg.cur_ps->pm_flags & PMF_FOLLOW) ) {
		return qfalse;
	}

	info = CG_ConfigString( CS_BOTINFO + cg.cur_ps->playerNum );

	if (!*info) {
		return qfalse;
	}

	lineHeight = CG_DrawStringLineHeight( UI_BIGFONT );

	node = Info_ValueForKey(info, "n");

	if ( *node ) {
		str = va("AI Node: %s", node);
		CG_DrawString( SCREEN_WIDTH / 2, y, str, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );

		y += lineHeight;
	}

	action = Info_ValueForKey(info, "a");

	if ( *action ) {
		CG_DrawString( SCREEN_WIDTH / 2, y, action, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );

		y += lineHeight;
	}

	leader = Info_ValueForKey(info, "l");

	if ( *leader ) {
		str = "bot is leader";
		CG_DrawString( SCREEN_WIDTH / 2, y, str, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );

		y += lineHeight;
	}

	carrying = Info_ValueForKey(info, "c");

	if ( *carrying ) {
		str = va("bot carrying: %s", carrying);
		CG_DrawString( SCREEN_WIDTH / 2, y, str, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );
	}

	return qtrue;
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow( void ) {
	const char	*name;
	int y;

	if ( !(cg.cur_ps->pm_flags & PMF_FOLLOW) ) {
		return qfalse;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_TOP);

	y = 24;

	CG_DrawString( SCREEN_WIDTH / 2, y, "following", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );
	y += CG_DrawStringLineHeight( UI_BIGFONT );

	name = cgs.playerinfo[ cg.cur_ps->playerNum ].name;

	CG_DrawString( SCREEN_WIDTH / 2, y, name, UI_CENTER|UI_DROPSHADOW|UI_GIANTFONT, NULL );
	y += CG_DrawStringLineHeight( UI_GIANTFONT );

	CG_DrawBotInfo( y );

	return qtrue;
}



/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning( void ) {
	const char	*s;

	if ( cg_drawAmmoWarning.integer == 0 ) {
		return;
	}

	if ( !cg.cur_lc->lowAmmoWarning ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_TOP);

	if ( cg.cur_lc->lowAmmoWarning == 2 ) {
		s = "OUT OF AMMO";
	} else {
		s = "LOW AMMO WARNING";
	}
	CG_DrawString( SCREEN_WIDTH / 2, 64, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );
}


#ifdef MISSIONPACK
/*
=================
CG_DrawProxWarning
=================
*/
static void CG_DrawProxWarning( void ) {
	char s [32];
  int proxTick;
	int lineHeight;

	if( !(cg.cur_ps->eFlags & EF_TICKING ) ) {
    cg.cur_lc->proxTime = 0;
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_TOP);

  if (cg.cur_lc->proxTime == 0) {
    cg.cur_lc->proxTime = cg.time;
  }

  proxTick = 10 - ((cg.time - cg.cur_lc->proxTime) / 1000);

  if (proxTick > 0 && proxTick <= 5) {
    Com_sprintf(s, sizeof(s), "INTERNAL COMBUSTION IN: %i", proxTick);
  } else {
    Com_sprintf(s, sizeof(s), "YOU HAVE BEEN MINED");
  }

	// put proxy warning below where the out of ammo location
	lineHeight = CG_DrawStringLineHeight( UI_BIGFONT );

	CG_DrawString( SCREEN_WIDTH / 2, 64 + lineHeight, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, g_color_table[ColorIndex(COLOR_RED)] );
}
#endif


/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) {
	int			sec;
	int			i;
	float		scale;
	playerInfo_t	*ci1, *ci2;
	const char	*s;

	sec = cg.warmup;
	if ( !sec ) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_TOP);

	if ( sec < 0 ) {
		s = "Waiting for players";		
		CG_DrawString( SCREEN_WIDTH / 2, 24, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );
		cg.warmupCount = 0;
		return;
	}

	if (cgs.gametype == GT_TOURNAMENT) {
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for ( i = 0 ; i < cgs.maxplayers ; i++ ) {
			if ( cgs.playerinfo[i].infoValid && cgs.playerinfo[i].team == TEAM_FREE ) {
				if ( !ci1 ) {
					ci1 = &cgs.playerinfo[i];
				} else {
					ci2 = &cgs.playerinfo[i];
				}
			}
		}

		if ( ci1 && ci2 ) {
			s = va( "%s vs %s", ci1->name, ci2->name );
			CG_DrawStringExt( SCREEN_WIDTH / 2, 25, s, UI_CENTER|UI_DROPSHADOW|UI_GIANTFONT, NULL, 32 / 48.0f, 0, 0 );
		}
	} else {
		s = cgs.gametypeName;
		CG_DrawStringExt( SCREEN_WIDTH / 2, 25, s, UI_CENTER|UI_DROPSHADOW|UI_GIANTFONT, NULL, 32 / 48.0f, 0, 0 );
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		cg.warmup = 0;
		sec = 0;
	}
	s = va( "Starts in: %i", sec + 1 );
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;
		switch ( sec ) {
		case 0:
			trap_S_StartLocalSound( cgs.media.count1Sound, CHAN_ANNOUNCER );
			break;
		case 1:
			trap_S_StartLocalSound( cgs.media.count2Sound, CHAN_ANNOUNCER );
			break;
		case 2:
			trap_S_StartLocalSound( cgs.media.count3Sound, CHAN_ANNOUNCER );
			break;
		default:
			break;
		}
	}

	switch ( cg.warmupCount ) {
	case 0:
		scale = 28 / 48.0f;
		break;
	case 1:
		scale = 24 / 48.0f;
		break;
	case 2:
		scale = 20 / 48.0f;
		break;
	default:
		scale = 16 / 48.0f;
		break;
	}

	CG_DrawStringExt( SCREEN_WIDTH / 2, 70, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT|UI_NOSCALE, NULL, scale, 0, 0 );
}


/*
=====================
CG_DrawSmallWrappedText

Draw multiline text
=====================
*/
void CG_DrawSmallWrappedText(int x, int y, const char *textPtr) {
	const char *p, *start;
	char buff[1024];
	int lineHeight;

	if (!textPtr || *textPtr == '\0') {
		return;
	}

	lineHeight = CG_DrawStringLineHeight( UI_SMALLFONT );

	start = textPtr;
	p = strchr(textPtr, '\n');
	while (p && *p) {
		strncpy(buff, start, p-start+1);
		buff[p-start] = '\0';
		CG_DrawSmallString(x, y, buff, 1.0f );
		y += lineHeight;
		start += p - start + 1;
		p = strchr(p+1, '\n');
	}
	CG_DrawSmallString(x, y, start, 1.0f );
}

/*
=====================
CG_DrawNotify

Draw console notify area.
=====================
*/
void CG_DrawNotify( void ) {
	int x;

#ifdef MISSIONPACK_HUD
	// voice head is being shown
	if ( !cg.cur_lc->showScores && cg.cur_ps->stats[STAT_HEALTH] > 0 &&
		cg.cur_lc->voiceTime && cg.cur_lc->voiceTime >= cg.time && cg.cur_lc->playerNum != cg.cur_lc->currentVoicePlayerNum )
		x = 72;
	else
#endif
		x = 0;

	CG_SetScreenPlacement(PLACE_LEFT, PLACE_TOP);
	CG_DrawSmallWrappedText(x, 2, cg.cur_lc->consoleText);
}

//==================================================================================
#ifdef MISSIONPACK_HUD
/* 
=================
CG_DrawTimedMenus
=================
*/
void CG_DrawTimedMenus( void ) {
	if ( cg.cur_lc->voiceTime && cg.cur_lc->voiceTime >= cg.time && cg.cur_lc->playerNum != cg.cur_lc->currentVoicePlayerNum ) {
		Menus_OpenByName("voiceMenu");
	} else {
		Menus_CloseByName("voiceMenu");
	}
}
#endif
/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D(stereoFrame_t stereoFrame)
{
#ifdef MISSIONPACK
	if (cg.cur_lc->orderPending && cg.time > cg.cur_lc->orderTime) {
		CG_CheckOrderPending( cg.cur_localPlayerNum );
	}
#endif
	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) {
		return;
	}

	if ( cg.singleCamera ) {
		return;
	}

	if ( cg_draw2D.integer == 0 ) {
		return;
	}

	if ( cg.cur_ps->pm_type == PM_INTERMISSION ) {
		CG_DrawIntermission();
		return;
	}

/*
	if (cg.cameraMode) {
		return;
	}
*/
	if ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		CG_DrawSpectator();

		if(stereoFrame == STEREO_CENTER)
			CG_DrawCrosshair();

		CG_DrawCrosshairNames();
	} else {
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if ( !cg.cur_lc->showScores && cg.cur_ps->stats[STAT_HEALTH] > 0 ) {

#ifdef MISSIONPACK_HUD
			if ( cg_drawStatus.integer ) {
				CG_SetScreenPlacement(PLACE_CENTER, PLACE_BOTTOM);

				CG_DrawTimedMenus();
				Menu_PaintAll();
			}
#else
			CG_DrawStatusBar();
#endif
      
			CG_DrawAmmoWarning();

#ifdef MISSIONPACK
			CG_DrawProxWarning();
#endif      
			if(stereoFrame == STEREO_CENTER)
				CG_DrawCrosshair();
			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();

#ifndef MISSIONPACK_HUD
			CG_DrawHoldableItem();
#ifdef MISSIONPACK
			CG_DrawPersistantPowerup();
#endif
#endif
			CG_DrawReward();
		}
    
		if ( cgs.gametype >= GT_TEAM ) {
#ifndef MISSIONPACK_HUD
			CG_DrawTeamInfo();
#endif
		}
	}

	CG_DrawVote();
	CG_DrawTeamVote();

	CG_DrawLagometer();

	CG_DrawVoipMeter();

#ifdef MISSIONPACK
	if (!cg_paused.integer) {
		CG_DrawUpperRight(stereoFrame);
	}
#else
	CG_DrawUpperRight(stereoFrame);
#endif

#ifndef MISSIONPACK_HUD
	CG_DrawLowerRight();
	CG_DrawLowerLeft();
#endif

	CG_DrawShaderInfo();

	CG_DrawFollow();

	// don't draw center string if scoreboard is up
	cg.cur_lc->scoreBoardShowing = CG_DrawScoreboard();
	if (!cg.cur_lc->scoreBoardShowing) {
		CG_DrawCenterString();
	}
}


/*
=====================
CG_FogView
=====================
*/
void CG_FogView( void ) {
	qboolean inwater;

	inwater = ( cg.refdef.rdflags & RDF_UNDERWATER );

	trap_R_GetViewFog( cg.refdef.vieworg, &cg.refdef.fogType, cg.refdef.fogColor, &cg.refdef.fogDepthForOpaque, &cg.refdef.fogDensity, inwater );

	if ( cg.refdef.fogType == FT_NONE && ( cg.refdef.fogColor[0] || cg.refdef.fogColor[1] || cg.refdef.fogColor[2] ) ) {
		// use global fog with custom color
		cg.refdef.fogType = cgs.globalFogType;
		cg.refdef.fogDepthForOpaque = cgs.globalFogDepthForOpaque;
		cg.refdef.fogDensity = cgs.globalFogDensity;
	} else if ( cg.refdef.fogType == FT_NONE ) {
		// no view fog, use global fog
		cg.refdef.fogType = cgs.globalFogType;
		cg.refdef.fogDepthForOpaque = cgs.globalFogDepthForOpaque;
		cg.refdef.fogDensity = cgs.globalFogDensity;

		VectorCopy( cgs.globalFogColor, cg.refdef.fogColor );
	}
}

/*
=====================
CG_DrawMiscGamemodels
=====================
*/
void CG_DrawMiscGamemodels( void ) {
	int i, j;
	refEntity_t ent;
	int drawn = 0;

	memset( &ent, 0, sizeof( ent ) );

	ent.reType = RT_MODEL;
	ent.nonNormalizedAxes = qtrue;

	// ydnar: static gamemodels don't project shadows
	ent.renderfx = RF_NOSHADOW;

	for ( i = 0; i < cg.numMiscGameModels; i++ ) {
		if ( cgs.miscGameModels[i].radius ) {
			if ( CG_CullPointAndRadius( cgs.miscGameModels[i].org, cgs.miscGameModels[i].radius ) ) {
				continue;
			}
		}

		if ( !trap_R_inPVS( cg.refdef.vieworg, cgs.miscGameModels[i].org ) ) {
			continue;
		}

		VectorCopy( cgs.miscGameModels[i].org, ent.origin );
		VectorCopy( cgs.miscGameModels[i].org, ent.oldorigin );
		VectorCopy( cgs.miscGameModels[i].org, ent.lightingOrigin );

/*		{
			vec3_t v;
			vec3_t vu = { 0.f, 0.f, 1.f };
			vec3_t vl = { 0.f, 1.f, 0.f };
			vec3_t vf = { 1.f, 0.f, 0.f };

			VectorCopy( cgs.miscGameModels[i].org, v );
			VectorMA( v, cgs.miscGameModels[i].radius, vu, v );
			CG_RailTrail2( NULL, cgs.miscGameModels[i].org, v );

			VectorCopy( cgs.miscGameModels[i].org, v );
			VectorMA( v, cgs.miscGameModels[i].radius, vf, v );
			CG_RailTrail2( NULL, cgs.miscGameModels[i].org, v );

			VectorCopy( cgs.miscGameModels[i].org, v );
			VectorMA( v, cgs.miscGameModels[i].radius, vl, v );
			CG_RailTrail2( NULL, cgs.miscGameModels[i].org, v );

			VectorCopy( cgs.miscGameModels[i].org, v );
			VectorMA( v, -cgs.miscGameModels[i].radius, vu, v );
			CG_RailTrail2( NULL, cgs.miscGameModels[i].org, v );

			VectorCopy( cgs.miscGameModels[i].org, v );
			VectorMA( v, -cgs.miscGameModels[i].radius, vf, v );
			CG_RailTrail2( NULL, cgs.miscGameModels[i].org, v );

			VectorCopy( cgs.miscGameModels[i].org, v );
			VectorMA( v, -cgs.miscGameModels[i].radius, vl, v );
			CG_RailTrail2( NULL, cgs.miscGameModels[i].org, v );
		}*/

		for ( j = 0; j < 3; j++ ) {
			VectorCopy( cgs.miscGameModels[i].axes[j], ent.axis[j] );
		}
		ent.hModel = cgs.miscGameModels[i].model;

		ent.customSkin = CG_AddSkinToFrame( &cgs.miscGameModels[i].skin );
		trap_R_AddRefEntityToScene( &ent );

		drawn++;
	}
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the viewport
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) {
	// optionally draw the info screen instead
	if ( !cg.snap ) {
		CG_DrawInformation();
		return;
	}

	// optionally draw the tournement scoreboard instead
	if ( cg.cur_ps->persistant[PERS_TEAM] == TEAM_SPECTATOR &&
		( cg.cur_ps->pm_flags & PMF_SCOREBOARD ) ) {
		CG_DrawTourneyScoreboard();
		return;
	}

	// clear around the rendered view if sized down
	CG_TileClear();

	if(stereoView != STEREO_CENTER)
		CG_DrawCrosshair3D();

	CG_DrawThirdPersonCrosshair();

	CG_PB_RenderPolyBuffers();

	CG_DrawMiscGamemodels();

	CG_FogView();

	cg.refdef.skyAlpha = cg.skyAlpha;

	// draw 3D view
	trap_R_RenderScene( &cg.refdef );

	// draw status bar and other floating elements
 	CG_Draw2D(stereoView);

	CG_DrawNotify();
}

/*
=================
CG_DrawDemoRecording
=================
*/
void CG_DrawDemoRecording( void ) {
	char	string[1024];
	char	demoName[MAX_QPATH];
	int		pos;

	if ( trap_GetDemoState() != DS_RECORDING ) {
		return;
	}
#ifdef MISSIONPACK
	if ( cg_recordSPDemo.integer ) {
		return;
	}
#endif

	trap_GetDemoName( demoName, sizeof( demoName ) );
	pos = trap_GetDemoPos();
	Com_sprintf( string, sizeof (string), "RECORDING %s: %ik", demoName, pos / 1024 );

	CG_SetScreenPlacement( PLACE_CENTER, PLACE_TOP );

	CG_DrawString( SCREEN_WIDTH / 2, 20, string, UI_CENTER|UI_DROPSHADOW|UI_TINYFONT, NULL );
}

/*
=================
CG_DrawMessageMode
=================
*/
void CG_DrawMessageMode( void ) {
	if ( !( Key_GetCatcher() & KEYCATCH_MESSAGE ) ) {
		return;
	}

	// draw the chat line
	CG_DrawString( 8, 232, cg.messagePrompt, UI_DROPSHADOW|UI_BIGFONT, NULL );

	MField_Draw( &cg.messageField, 8 + CG_DrawStrlen( cg.messagePrompt, UI_BIGFONT ), 232,
			UI_DROPSHADOW|UI_BIGFONT, NULL, qtrue );
}

/*
=====================
CG_DrawScreen2D

Perform drawing that fills the screen, drawing over all viewports
=====================
*/
void CG_DrawScreen2D( stereoFrame_t stereoView ) {
	if ( !cg.snap ) {
		return;
	}

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) {
		return;
	}

	if ( cg_draw2D.integer == 0 ) {
		return;
	}

	CG_DrawWarmup();

	if ( cg.singleCamera ) {
		CG_DrawScoreboard();
	} else {
		CG_DrawGlobalCenterString();
	}

	CG_DrawDemoRecording();
	CG_DrawMessageMode();
}



