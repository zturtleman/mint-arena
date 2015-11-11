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
// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"

#ifndef MISSIONPACK_HUD

#define	SCOREBOARD_X		(0)

#define SB_HEADER			86
#define SB_TOP				(SB_HEADER+32)

// Where the status bar starts, so we don't overwrite it
#define SB_STATUSBAR		420

#define SB_NORMAL_HEIGHT	40
#define SB_INTER_HEIGHT		16 // interleaved height

#define SB_MAXPLAYERS_NORMAL  ((SB_STATUSBAR - SB_TOP) / SB_NORMAL_HEIGHT)
#define SB_MAXPLAYERS_INTER   ((SB_STATUSBAR - SB_TOP) / SB_INTER_HEIGHT - 1)

// Used when interleaved



#define SB_LEFT_BOTICON_X	(SCOREBOARD_X+0)
#define SB_LEFT_HEAD_X		(SCOREBOARD_X+32)
#define SB_RIGHT_BOTICON_X	(SCOREBOARD_X+64)
#define SB_RIGHT_HEAD_X		(SCOREBOARD_X+96)
// Normal
#define SB_BOTICON_X		(SCOREBOARD_X+32)
#define SB_HEAD_X			(SCOREBOARD_X+64)

#define SB_SCORELINE_X		112

#define SB_RATING_WIDTH	    (6 * BIGCHAR_WIDTH) // width 6
#define SB_SCORE_X			(SB_SCORELINE_X + BIGCHAR_WIDTH) // width 6
#define SB_RATING_X			(SB_SCORELINE_X + 6 * BIGCHAR_WIDTH) // width 6
#define SB_PING_X			(SB_SCORELINE_X + 12 * BIGCHAR_WIDTH + 8) // width 5
#define SB_TIME_X			(SB_SCORELINE_X + 17 * BIGCHAR_WIDTH + 8) // width 5
#define SB_NAME_X			(SB_SCORELINE_X + 22 * BIGCHAR_WIDTH) // width 15

// The new and improved score board
//
// In cases where the number of players is high, the score board heads are interleaved
// here's the layout

//
//	0   32   80  112  144   240  320  400   <-- pixel position
//  bot head bot head score ping time name
//  
//  wins/losses are drawn on bot icon now

static qboolean localPlayer; // true if local player has been displayed


							 /*
=================
CG_DrawPlayerScore
=================
*/
static void CG_DrawPlayerScore( int y, score_t *score, float *color, float fade, qboolean largeFormat ) {
	char	string[1024];
	vec3_t	headAngles;
	playerInfo_t	*pi;
	int iconx, headx;
	playerState_t *ps;

	if ( score->playerNum < 0 || score->playerNum >= cgs.maxplayers ) {
		Com_Printf( "Bad score->playerNum: %i\n", score->playerNum );
		return;
	}
	
	pi = &cgs.playerinfo[score->playerNum];

	iconx = SB_BOTICON_X + (SB_RATING_WIDTH / 2);
	headx = SB_HEAD_X + (SB_RATING_WIDTH / 2);

	// draw the handicap or bot skill marker (unless player has flag)
	if ( pi->powerups & ( 1 << PW_NEUTRALFLAG ) ) {
		if( largeFormat ) {
			CG_DrawFlagModel( iconx, y - ( 32 - BIGCHAR_HEIGHT ) / 2, 32, 32, TEAM_FREE, qfalse );
		}
		else {
			CG_DrawFlagModel( iconx, y, 16, 16, TEAM_FREE, qfalse );
		}
	} else if ( pi->powerups & ( 1 << PW_REDFLAG ) ) {
		if( largeFormat ) {
			CG_DrawFlagModel( iconx, y - ( 32 - BIGCHAR_HEIGHT ) / 2, 32, 32, TEAM_RED, qfalse );
		}
		else {
			CG_DrawFlagModel( iconx, y, 16, 16, TEAM_RED, qfalse );
		}
	} else if ( pi->powerups & ( 1 << PW_BLUEFLAG ) ) {
		if( largeFormat ) {
			CG_DrawFlagModel( iconx, y - ( 32 - BIGCHAR_HEIGHT ) / 2, 32, 32, TEAM_BLUE, qfalse );
		}
		else {
			CG_DrawFlagModel( iconx, y, 16, 16, TEAM_BLUE, qfalse );
		}
	} else {
		if ( pi->botSkill > 0 && pi->botSkill <= 5 ) {
			if ( cg_drawIcons.integer ) {
				if( largeFormat ) {
					CG_DrawPic( iconx, y - ( 32 - BIGCHAR_HEIGHT ) / 2, 32, 32, cgs.media.botSkillShaders[ pi->botSkill - 1 ] );
				}
				else {
					CG_DrawPic( iconx, y, 16, 16, cgs.media.botSkillShaders[ pi->botSkill - 1 ] );
				}
			}
		} else if ( pi->handicap < 100 ) {
			Com_sprintf( string, sizeof( string ), "%i", pi->handicap );
			if ( cgs.gametype == GT_TOURNAMENT ) {
				CG_DrawString( iconx, y - SMALLCHAR_HEIGHT/2, string, UI_SMALLFONT|UI_NOSCALE, color );
			}
			else {
				CG_DrawString( iconx, y, string, UI_SMALLFONT|UI_NOSCALE, color );
			}
		}

		// draw the wins / losses
		if ( cgs.gametype == GT_TOURNAMENT ) {
			Com_sprintf( string, sizeof( string ), "%i/%i", pi->wins, pi->losses );
			if( pi->handicap < 100 && !pi->botSkill ) {
				CG_DrawString( iconx, y + SMALLCHAR_HEIGHT/2, string, UI_SMALLFONT|UI_NOSCALE, color );
			}
			else {
				CG_DrawString( iconx, y, string, UI_SMALLFONT|UI_NOSCALE, color );
			}
		}
	}

	// draw the face
	VectorClear( headAngles );
	headAngles[YAW] = 180;
	if( largeFormat ) {
		CG_DrawHead( headx, y - ( ICON_SIZE - BIGCHAR_HEIGHT ) / 2, ICON_SIZE, ICON_SIZE, 
			score->playerNum, headAngles );
	}
	else {
		CG_DrawHead( headx, y, 16, 16, score->playerNum, headAngles );
	}

#ifdef MISSIONPACK
	// draw the team task
	switch ( pi->teamTask ) {
		case TEAMTASK_OFFENSE:
			CG_DrawPic( headx + 48, y, 16, 16, cgs.media.assaultShader );
			break;
		case TEAMTASK_DEFENSE:
			CG_DrawPic( headx + 48, y, 16, 16, cgs.media.defendShader );
			break;
		case TEAMTASK_PATROL:
			CG_DrawPic( headx + 48, y, 16, 16, cgs.media.patrolShader );
			break;
		case TEAMTASK_FOLLOW:
			CG_DrawPic( headx + 48, y, 16, 16, cgs.media.followShader );
			break;
		case TEAMTASK_CAMP:
			CG_DrawPic( headx + 48, y, 16, 16, cgs.media.campShader );
			break;
		case TEAMTASK_RETRIEVE:
			CG_DrawPic( headx + 48, y, 16, 16, cgs.media.retrieveShader );
			break;
		case TEAMTASK_ESCORT:
			CG_DrawPic( headx + 48, y, 16, 16, cgs.media.escortShader );
			break;
		default:
			break;
	}
#endif

	if (cg.cur_ps) {
		if (score->playerNum == cg.cur_ps->playerNum) {
			ps = cg.cur_ps;
		} else {
			ps = NULL;
		}
	} else {
		ps = CG_LocalPlayerState(score->playerNum);
	}

	// highlight your position
	if ( ps ) {
		float	hcolor[4];
		int		rank;

		localPlayer = qtrue;

		if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR || cgs.gametype >= GT_TEAM ) {
			rank = -1;
		} else {
			rank = ps->persistant[PERS_RANK] & ~RANK_TIED_FLAG;
		}
		if ( rank == 0 ) {
			hcolor[0] = 0;
			hcolor[1] = 0;
			hcolor[2] = 0.7f;
		} else if ( rank == 1 ) {
			hcolor[0] = 0.7f;
			hcolor[1] = 0;
			hcolor[2] = 0;
		} else if ( rank == 2 ) {
			hcolor[0] = 0.7f;
			hcolor[1] = 0.7f;
			hcolor[2] = 0;
		} else {
			hcolor[0] = 0.7f;
			hcolor[1] = 0.7f;
			hcolor[2] = 0.7f;
		}

		hcolor[3] = fade * 0.7;
		CG_FillRect( SB_SCORELINE_X + BIGCHAR_WIDTH + (SB_RATING_WIDTH / 2), y, 
			640 - SB_SCORELINE_X - BIGCHAR_WIDTH - (SB_RATING_WIDTH / 2), BIGCHAR_HEIGHT+1, hcolor );
	}

	// draw the score line
	if ( score->ping == -1 ) {
		Com_sprintf(string, sizeof(string), "connecting");
	} else if ( pi->team == TEAM_SPECTATOR ) {
		Com_sprintf(string, sizeof(string), "SPECT");
	} else {
		Com_sprintf(string, sizeof(string), "%5i", score->score);
	}
	CG_DrawString( SB_SCORE_X + (SB_RATING_WIDTH / 2) + 4*BIGCHAR_WIDTH, y, string, UI_RIGHT|UI_DROPSHADOW|UI_BIGFONT|UI_NOSCALE, color );

	if ( score->ping != -1 ) {
		Com_sprintf(string, sizeof(string), "%4i", score->ping);
		CG_DrawString( SB_PING_X - (SB_RATING_WIDTH / 2) + 4*BIGCHAR_WIDTH, y, string, UI_RIGHT|UI_DROPSHADOW|UI_BIGFONT|UI_NOSCALE, color );

		Com_sprintf(string, sizeof(string), "%4i", score->time);
		CG_DrawString( SB_TIME_X - (SB_RATING_WIDTH / 2) + 4*BIGCHAR_WIDTH, y, string, UI_RIGHT|UI_DROPSHADOW|UI_BIGFONT|UI_NOSCALE, color );
	}

	CG_DrawString( SB_NAME_X - (SB_RATING_WIDTH / 2), y, pi->name, UI_LEFT|UI_DROPSHADOW|UI_BIGFONT|UI_NOSCALE, color );

	// add the "ready" marker for intermission exiting
	if ( Com_ClientListContains( &cg.readyPlayers, score->playerNum ) ) {
		CG_DrawString( iconx, y, "READY", UI_LEFT|UI_DROPSHADOW|UI_BIGFONT|UI_NOSCALE, color );
	}
}

/*
=================
CG_TeamScoreboard
=================
*/
static int CG_TeamScoreboard( int y, team_t team, float fade, int maxPlayers, int lineHeight ) {
	int		i;
	score_t	*score;
	float	color[4];
	int		count;
	playerInfo_t	*pi;

	color[0] = color[1] = color[2] = 1.0;
	color[3] = fade;

	count = 0;
	for ( i = 0 ; i < cg.numScores && count < maxPlayers ; i++ ) {
		score = &cg.scores[i];
		pi = &cgs.playerinfo[ score->playerNum ];

		if ( team != pi->team ) {
			continue;
		}

		CG_DrawPlayerScore( y + lineHeight * count, score, color, fade, lineHeight == SB_NORMAL_HEIGHT );

		count++;
	}

	return count;
}

/*
=================
CG_DrawScoreboard

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawOldScoreboard( void ) {
	int		y, i, n1, n2;
	float	fade;
	float	*fadeColor;
	char	*s;
	int maxPlayers;
	int lineHeight;
	int topBorderSize, bottomBorderSize;

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	// don't draw amuthing if the menu or console is up
	if ( cg_paused.integer ) {
		return qfalse;
	}

	if ( cgs.gametype == GT_SINGLE_PLAYER && cg.cur_lc && cg.cur_lc->predictedPlayerState.pm_type == PM_INTERMISSION ) {
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if ( cg.warmup && cg.cur_lc && !cg.cur_lc->showScores ) {
		return qfalse;
	}

	if ( !cg.cur_lc || cg.cur_lc->showScores || cg.cur_lc->predictedPlayerState.pm_type == PM_DEAD ||
		 cg.cur_lc->predictedPlayerState.pm_type == PM_INTERMISSION ) {
		fade = 1.0;
		fadeColor = colorWhite;
	} else {
		fadeColor = CG_FadeColor( cg.cur_lc->scoreFadeTime, FADE_TIME );
		
		if ( !fadeColor ) {
			// next time scoreboard comes up, don't print killer
			cg.cur_lc->killerName[0] = 0;
			return qfalse;
		}
		// ZTM: FIXME?: to actually fade, should be fade=fadeColor[3] and later CG_DrawString should use fadeColor
		fade = *fadeColor;
	}


	// fragged by ... line
	if ( cg.cur_lc && cg.cur_lc->killerName[0] ) {
		s = va("Fragged by %s", cg.cur_lc->killerName );
		y = SB_HEADER - 6 - CG_DrawStringLineHeight( UI_BIGFONT ) * 2;
		CG_DrawString( SCREEN_WIDTH / 2, y, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );
	}

	// current rank
	if ( cgs.gametype < GT_TEAM) {
		if (cg.cur_ps && cg.cur_ps->persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
			s = va("%s place with %i",
				CG_PlaceString( cg.cur_ps->persistant[PERS_RANK] + 1 ),
				cg.cur_ps->persistant[PERS_SCORE] );
			y = SB_HEADER - 6 - CG_DrawStringLineHeight( UI_BIGFONT );
			CG_DrawString( SCREEN_WIDTH / 2, y, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );
		}
	} else {
		if ( cg.teamScores[0] == cg.teamScores[1] ) {
			s = va("Teams are tied at %i", cg.teamScores[0] );
		} else if ( cg.teamScores[0] >= cg.teamScores[1] ) {
			s = va("Red leads %i to %i",cg.teamScores[0], cg.teamScores[1] );
		} else {
			s = va("Blue leads %i to %i",cg.teamScores[1], cg.teamScores[0] );
		}

		y = SB_HEADER - 6 - CG_DrawStringLineHeight( UI_BIGFONT );
		CG_DrawString( SCREEN_WIDTH / 2, y, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL );
	}

	// scoreboard
	y = SB_HEADER;

	CG_DrawPic( SB_SCORE_X + (SB_RATING_WIDTH / 2), y, 64, 32, cgs.media.scoreboardScore );
	CG_DrawPic( SB_PING_X - (SB_RATING_WIDTH / 2), y, 64, 32, cgs.media.scoreboardPing );
	CG_DrawPic( SB_TIME_X - (SB_RATING_WIDTH / 2), y, 64, 32, cgs.media.scoreboardTime );
	CG_DrawPic( SB_NAME_X - (SB_RATING_WIDTH / 2), y, 64, 32, cgs.media.scoreboardName );

	y = SB_TOP;

	// If there are more than SB_MAXPLAYERS_NORMAL, use the interleaved scores
	if ( cg.numScores > SB_MAXPLAYERS_NORMAL ) {
		maxPlayers = SB_MAXPLAYERS_INTER;
		lineHeight = SB_INTER_HEIGHT;
		topBorderSize = 8;
		bottomBorderSize = 16;
	} else {
		maxPlayers = SB_MAXPLAYERS_NORMAL;
		lineHeight = SB_NORMAL_HEIGHT;
		topBorderSize = 16;
		bottomBorderSize = 16;
	}

	localPlayer = qfalse;

	if ( cgs.gametype >= GT_TEAM ) {
		//
		// teamplay scoreboard
		//
		y += lineHeight/2;

		if ( cg.teamScores[0] >= cg.teamScores[1] ) {
			n1 = CG_TeamScoreboard( y, TEAM_RED, fade, maxPlayers, lineHeight );
			CG_DrawTeamBackground( 0, y - topBorderSize, 640, n1 * lineHeight + bottomBorderSize, 0.33f, TEAM_RED );
			y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
			maxPlayers -= n1;
			n2 = CG_TeamScoreboard( y, TEAM_BLUE, fade, maxPlayers, lineHeight );
			CG_DrawTeamBackground( 0, y - topBorderSize, 640, n2 * lineHeight + bottomBorderSize, 0.33f, TEAM_BLUE );
			y += (n2 * lineHeight) + BIGCHAR_HEIGHT;
			maxPlayers -= n2;
		} else {
			n1 = CG_TeamScoreboard( y, TEAM_BLUE, fade, maxPlayers, lineHeight );
			CG_DrawTeamBackground( 0, y - topBorderSize, 640, n1 * lineHeight + bottomBorderSize, 0.33f, TEAM_BLUE );
			y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
			maxPlayers -= n1;
			n2 = CG_TeamScoreboard( y, TEAM_RED, fade, maxPlayers, lineHeight );
			CG_DrawTeamBackground( 0, y - topBorderSize, 640, n2 * lineHeight + bottomBorderSize, 0.33f, TEAM_RED );
			y += (n2 * lineHeight) + BIGCHAR_HEIGHT;
			maxPlayers -= n2;
		}
		n1 = CG_TeamScoreboard( y, TEAM_SPECTATOR, fade, maxPlayers, lineHeight );
		y += (n1 * lineHeight) + BIGCHAR_HEIGHT;

	} else {
		//
		// free for all scoreboard
		//
		n1 = CG_TeamScoreboard( y, TEAM_FREE, fade, maxPlayers, lineHeight );
		y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
		n2 = CG_TeamScoreboard( y, TEAM_SPECTATOR, fade, maxPlayers - n1, lineHeight );
		y += (n2 * lineHeight) + BIGCHAR_HEIGHT;
	}

	if (cg.cur_ps && !localPlayer) {
		// draw local player at the bottom
		for ( i = 0 ; i < cg.numScores ; i++ ) {
			if ( cg.scores[i].playerNum == cg.cur_ps->playerNum ) {
				CG_DrawPlayerScore( y, &cg.scores[i], fadeColor, fade, lineHeight == SB_NORMAL_HEIGHT );
				break;
			}
		}
	}

	return qtrue;
}

#endif // !MISSIONPACK_HUD

//================================================================================

static void CG_CenterGiantLine( float y, const char *string ) {
	CG_DrawString( 320, y, string, UI_CENTER|UI_DROPSHADOW|UI_GIANTFONT|UI_NOSCALE, NULL );
}

/*
=================
CG_DrawTourneyScoreboard

Draw the oversize scoreboard for tournements
=================
*/
void CG_DrawTourneyScoreboard( void ) {
	const char		*s;
	vec4_t			color;
	int				min, tens, ones;
	playerInfo_t	*pi;
	int				y;
	int				i;

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	// request more scores regularly
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );
	}

	// draw the dialog background
	color[0] = color[1] = color[2] = 0;
	color[3] = 1;
	CG_SetScreenPlacement(PLACE_STRETCH, PLACE_STRETCH);
	CG_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color );
	CG_PopScreenPlacement();

	// print the mesage of the day
	s = CG_ConfigString( CS_MOTD );
	if ( !s[0] ) {
		s = "Scoreboard";
	}

	// print optional title
	CG_CenterGiantLine( 8, s );

	// print server time
	ones = cg.time / 1000;
	min = ones / 60;
	ones %= 60;
	tens = ones / 10;
	ones %= 10;
	s = va("%i:%i%i", min, tens, ones );

	CG_CenterGiantLine( 64, s );


	// print the two scores

	y = 160;
	if ( cgs.gametype >= GT_TEAM ) {
		//
		// teamplay scoreboard
		//
		CG_DrawString( 8, y, "Red Team", UI_LEFT|UI_DROPSHADOW|UI_GIANTFONT|UI_NOSCALE, NULL );
		s = va("%i", cg.teamScores[0] );
		CG_DrawString( 632, y, s, UI_RIGHT|UI_DROPSHADOW|UI_GIANTFONT|UI_NOSCALE, NULL );
		
		y += GIANTCHAR_HEIGHT + 16;

		CG_DrawString( 8, y, "Blue Team", UI_LEFT|UI_DROPSHADOW|UI_GIANTFONT|UI_NOSCALE, NULL );
		s = va("%i", cg.teamScores[1] );
		CG_DrawString( 632, y, s, UI_RIGHT|UI_DROPSHADOW|UI_GIANTFONT|UI_NOSCALE, NULL );
	} else if ( cgs.gametype == GT_TOURNAMENT ) {
		//
		// tournament scoreboard
		//
		for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
			pi = &cgs.playerinfo[i];
			if ( !pi->infoValid ) {
				continue;
			}
			if ( pi->team != TEAM_FREE ) {
				continue;
			}

			CG_DrawString( 8, y, pi->name, UI_LEFT|UI_DROPSHADOW|UI_GIANTFONT|UI_NOSCALE, NULL );
			s = va("%i", pi->score );
			CG_DrawString( 632, y, s, UI_RIGHT|UI_DROPSHADOW|UI_GIANTFONT|UI_NOSCALE, NULL );
			y += GIANTCHAR_HEIGHT + 16;
		}
	} else {
		//
		// free for all scoreboard (players sorted by score)
		//
		int style, gap;

		style = UI_GIANTFONT;
		gap = GIANTCHAR_HEIGHT + 16;

		// use smaller font if not all players fit
		if ( cg.numScores > ( SCREEN_HEIGHT - y ) / gap ) {
			style = UI_BIGFONT;
			gap = BIGCHAR_HEIGHT + 4;
		}

		for ( i = 0 ; i < cg.numScores; i++ ) {
			pi = &cgs.playerinfo[ cg.scores[i].playerNum ];
			if ( !pi->infoValid ) {
				continue;
			}
			if ( pi->team != TEAM_FREE ) {
				continue;
			}

			CG_DrawString( 8, y, pi->name, UI_LEFT|UI_DROPSHADOW|UI_NOSCALE|style, NULL );
			s = va("%i", pi->score );
			CG_DrawString( 632, y, s, UI_RIGHT|UI_DROPSHADOW|UI_NOSCALE|style, NULL );
			y += gap;

			if ( y >= SCREEN_HEIGHT ) {
				break;
			}
		}
	}


}

