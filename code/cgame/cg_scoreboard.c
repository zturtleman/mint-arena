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
			if ( cgs.gametype == GT_TOURNAMENT )
				CG_DrawSmallStringColor( iconx, y - SMALLCHAR_HEIGHT/2, string, color );
			else
				CG_DrawSmallStringColor( iconx, y, string, color );
		}

		// draw the wins / losses
		if ( cgs.gametype == GT_TOURNAMENT ) {
			Com_sprintf( string, sizeof( string ), "%i/%i", pi->wins, pi->losses );
			if( pi->handicap < 100 && !pi->botSkill ) {
				CG_DrawSmallStringColor( iconx, y + SMALLCHAR_HEIGHT/2, string, color );
			}
			else {
				CG_DrawSmallStringColor( iconx, y, string, color );
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
	if ( pi->teamTask != TEAMTASK_NONE ) {
		if ( pi->teamTask == TEAMTASK_OFFENSE ) {
			CG_DrawPic( headx + 48, y, 16, 16, cgs.media.assaultShader );
		}
		else if ( pi->teamTask == TEAMTASK_DEFENSE ) {
			CG_DrawPic( headx + 48, y, 16, 16, cgs.media.defendShader );
		}
	}
#endif
	// draw the score line
	if ( score->ping == -1 ) {
		Com_sprintf(string, sizeof(string),
			" connecting    %s", pi->name);
	} else if ( pi->team == TEAM_SPECTATOR ) {
		Com_sprintf(string, sizeof(string),
			" SPECT %3i %4i %s", score->ping, score->time, pi->name);
	} else {
		Com_sprintf(string, sizeof(string),
			"%5i %4i %4i %s", score->score, score->ping, score->time, pi->name);
	}

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

	CG_DrawBigString( SB_SCORELINE_X + (SB_RATING_WIDTH / 2), y, string, fade );

	// add the "ready" marker for intermission exiting
	if ( Com_ClientListContains( &cg.readyPlayers, score->playerNum ) ) {
		CG_DrawBigStringColor( iconx, y, "READY", color );
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
	int		x, y, w, i, n1, n2;
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
		fade = *fadeColor;
	}


	// fragged by ... line
	if ( cg.cur_lc && cg.cur_lc->killerName[0] ) {
		s = va("Fragged by %s", cg.cur_lc->killerName );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		x = ( SCREEN_WIDTH - w ) / 2;
		y = 40;
		CG_DrawBigString( x, y, s, fade );
	}

	// current rank
	if ( cgs.gametype < GT_TEAM) {
		if (cg.cur_ps && cg.cur_ps->persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
			s = va("%s place with %i",
				CG_PlaceString( cg.cur_ps->persistant[PERS_RANK] + 1 ),
				cg.cur_ps->persistant[PERS_SCORE] );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
			x = ( SCREEN_WIDTH - w ) / 2;
			y = 60;
			CG_DrawBigString( x, y, s, fade );
		}
	} else {
		if ( cg.teamScores[0] == cg.teamScores[1] ) {
			s = va("Teams are tied at %i", cg.teamScores[0] );
		} else if ( cg.teamScores[0] >= cg.teamScores[1] ) {
			s = va("Red leads %i to %i",cg.teamScores[0], cg.teamScores[1] );
		} else {
			s = va("Blue leads %i to %i",cg.teamScores[1], cg.teamScores[0] );
		}

		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		x = ( SCREEN_WIDTH - w ) / 2;
		y = 60;
		CG_DrawBigString( x, y, s, fade );
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

//================================================================================

/*
================
CG_CenterGiantLine
================
*/
static void CG_CenterGiantLine( float y, const char *string ) {
	float		x;
	vec4_t		color;

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

	x = 0.5 * ( 640 - GIANT_WIDTH * CG_DrawStrlen( string ) );

	CG_DrawStringExt( x, y, string, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
}

/*
=================
CG_DrawTourneyScoreboard

Draw the oversize scoreboard for tournements
=================
*/
void CG_DrawOldTourneyScoreboard( void ) {
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

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

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
		CG_DrawStringExt( 8, y, "Red Team", color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
		s = va("%i", cg.teamScores[0] );
		CG_DrawStringExt( 632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
		
		y += 64;

		CG_DrawStringExt( 8, y, "Blue Team", color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
		s = va("%i", cg.teamScores[1] );
		CG_DrawStringExt( 632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
	} else {
		//
		// free for all scoreboard
		//
		for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
			pi = &cgs.playerinfo[i];
			if ( !pi->infoValid ) {
				continue;
			}
			if ( pi->team != TEAM_FREE ) {
				continue;
			}

			CG_DrawStringExt( 8, y, pi->name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
			s = va("%i", pi->score );
			CG_DrawStringExt( 632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
			y += 64;
		}
	}


}

#endif // !MISSIONPACK_HUD
