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
// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"

char	*cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
	"*death1.wav",
	"*death2.wav",
	"*death3.wav",
	"*jump1.wav",
	"*pain25_1.wav",
	"*pain50_1.wav",
	"*pain75_1.wav",
	"*pain100_1.wav",
	"*falling1.wav",
	"*gasp.wav",
	"*drown.wav",
	"*fall1.wav",
	"*taunt.wav"
};


/*
================
CG_CustomSound

================
*/
sfxHandle_t	CG_CustomSound( int playerNum, const char *soundName ) {
	playerInfo_t *pi;
	int			i;

	if ( soundName[0] != '*' ) {
		return trap_S_RegisterSound( soundName, qfalse );
	}

	if ( playerNum < 0 || playerNum >= MAX_CLIENTS ) {
		playerNum = 0;
	}
	pi = &cgs.playerinfo[ playerNum ];

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i] ; i++ ) {
		if ( !strcmp( soundName, cg_customSoundNames[i] ) ) {
			return pi->sounds[i];
		}
	}

	CG_Error( "Unknown custom sound: %s", soundName );
	return 0;
}



/*
=============================================================================

PLAYER INFO

=============================================================================
*/

/*
======================
CG_ParseAnimationFile

Read a configuration file containing animation counts and rates
models/players/visor/animation.cfg, etc
======================
*/
static qboolean	CG_ParseAnimationFile( const char *filename, playerInfo_t *pi ) {
	char		*text_p, *prev;
	int			len;
	int			i;
	char		*token;
	float		fps;
	int			skip;
	char		text[20000];
	fileHandle_t	f;
	animation_t *animations;

	animations = pi->animations;

	// load the file
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		CG_Printf( "File %s too long\n", filename );
		trap_FS_FCloseFile( f );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	text_p = text;
	skip = 0;	// quite the compiler warning

	pi->footsteps = FOOTSTEP_NORMAL;
	VectorClear( pi->headOffset );
	pi->gender = GENDER_MALE;
	pi->fixedlegs = qfalse;
	pi->fixedtorso = qfalse;

	// read optional parameters
	while ( 1 ) {
		prev = text_p;	// so we can unget
		token = COM_Parse( &text_p );
		if ( !token ) {
			break;
		}
		if ( !Q_stricmp( token, "footsteps" ) ) {
			token = COM_Parse( &text_p );
			if ( !token ) {
				break;
			}
			if ( !Q_stricmp( token, "default" ) || !Q_stricmp( token, "normal" ) ) {
				pi->footsteps = FOOTSTEP_NORMAL;
			} else if ( !Q_stricmp( token, "boot" ) ) {
				pi->footsteps = FOOTSTEP_BOOT;
			} else if ( !Q_stricmp( token, "flesh" ) ) {
				pi->footsteps = FOOTSTEP_FLESH;
			} else if ( !Q_stricmp( token, "mech" ) ) {
				pi->footsteps = FOOTSTEP_MECH;
			} else if ( !Q_stricmp( token, "energy" ) ) {
				pi->footsteps = FOOTSTEP_ENERGY;
			} else {
				CG_Printf( "Bad footsteps parm in %s: %s\n", filename, token );
			}
			continue;
		} else if ( !Q_stricmp( token, "headoffset" ) ) {
			for ( i = 0 ; i < 3 ; i++ ) {
				token = COM_Parse( &text_p );
				if ( !token ) {
					break;
				}
				pi->headOffset[i] = atof( token );
			}
			continue;
		} else if ( !Q_stricmp( token, "sex" ) ) {
			token = COM_Parse( &text_p );
			if ( !token ) {
				break;
			}
			if ( token[0] == 'f' || token[0] == 'F' ) {
				pi->gender = GENDER_FEMALE;
			} else if ( token[0] == 'n' || token[0] == 'N' ) {
				pi->gender = GENDER_NEUTER;
			} else {
				pi->gender = GENDER_MALE;
			}
			continue;
		} else if ( !Q_stricmp( token, "fixedlegs" ) ) {
			pi->fixedlegs = qtrue;
			continue;
		} else if ( !Q_stricmp( token, "fixedtorso" ) ) {
			pi->fixedtorso = qtrue;
			continue;
		}

		// if it is a number, start parsing animations
		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}
		Com_Printf( "unknown token '%s' in %s\n", token, filename );
	}

	// read information for each frame
	for ( i = 0 ; i < MAX_ANIMATIONS ; i++ ) {

		token = COM_Parse( &text_p );
		if ( !*token ) {
			if( i >= TORSO_GETFLAG && i <= TORSO_NEGATIVE ) {
				animations[i].firstFrame = animations[TORSO_GESTURE].firstFrame;
				animations[i].frameLerp = animations[TORSO_GESTURE].frameLerp;
				animations[i].initialLerp = animations[TORSO_GESTURE].initialLerp;
				animations[i].loopFrames = animations[TORSO_GESTURE].loopFrames;
				animations[i].numFrames = animations[TORSO_GESTURE].numFrames;
				animations[i].reversed = qfalse;
				animations[i].flipflop = qfalse;
				continue;
			}
			break;
		}
		animations[i].firstFrame = atoi( token );
		// leg only frames are adjusted to not count the upper body only frames
		if ( i == LEGS_WALKCR ) {
			skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
		}
		if ( i >= LEGS_WALKCR && i<TORSO_GETFLAG) {
			animations[i].firstFrame -= skip;
		}

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		animations[i].numFrames = atoi( token );

		animations[i].reversed = qfalse;
		animations[i].flipflop = qfalse;
		// if numFrames is negative the animation is reversed
		if (animations[i].numFrames < 0) {
			animations[i].numFrames = -animations[i].numFrames;
			animations[i].reversed = qtrue;
		}

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		animations[i].loopFrames = atoi( token );

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		fps = atof( token );
		if ( fps == 0 ) {
			fps = 1;
		}
		animations[i].frameLerp = 1000 / fps;
		animations[i].initialLerp = 1000 / fps;
	}

	if ( i != MAX_ANIMATIONS ) {
		CG_Printf( "Error parsing animation file: %s\n", filename );
		return qfalse;
	}

	// crouch backward animation
	memcpy(&animations[LEGS_BACKCR], &animations[LEGS_WALKCR], sizeof(animation_t));
	animations[LEGS_BACKCR].reversed = qtrue;
	// walk backward animation
	memcpy(&animations[LEGS_BACKWALK], &animations[LEGS_WALK], sizeof(animation_t));
	animations[LEGS_BACKWALK].reversed = qtrue;
	// flag moving fast
	animations[FLAG_RUN].firstFrame = 0;
	animations[FLAG_RUN].numFrames = 16;
	animations[FLAG_RUN].loopFrames = 16;
	animations[FLAG_RUN].frameLerp = 1000 / 15;
	animations[FLAG_RUN].initialLerp = 1000 / 15;
	animations[FLAG_RUN].reversed = qfalse;
	// flag not moving or moving slowly
	animations[FLAG_STAND].firstFrame = 16;
	animations[FLAG_STAND].numFrames = 5;
	animations[FLAG_STAND].loopFrames = 0;
	animations[FLAG_STAND].frameLerp = 1000 / 20;
	animations[FLAG_STAND].initialLerp = 1000 / 20;
	animations[FLAG_STAND].reversed = qfalse;
	// flag speeding up
	animations[FLAG_STAND2RUN].firstFrame = 16;
	animations[FLAG_STAND2RUN].numFrames = 5;
	animations[FLAG_STAND2RUN].loopFrames = 1;
	animations[FLAG_STAND2RUN].frameLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].initialLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].reversed = qtrue;
	//
	// new anims changes
	//
//	animations[TORSO_GETFLAG].flipflop = qtrue;
//	animations[TORSO_GUARDBASE].flipflop = qtrue;
//	animations[TORSO_PATROL].flipflop = qtrue;
//	animations[TORSO_AFFIRMATIVE].flipflop = qtrue;
//	animations[TORSO_NEGATIVE].flipflop = qtrue;
	//
	return qtrue;
}

/*
==========================
CG_FileExists
==========================
*/
static qboolean	CG_FileExists(const char *filename) {
	int len;

	len = trap_FS_FOpenFile( filename, NULL, FS_READ );
	if (len>0) {
		return qtrue;
	}
	return qfalse;
}

/*
==========================
CG_FindPlayerModelFile
==========================
*/
static qboolean	CG_FindPlayerModelFile( char *filename, int length, playerInfo_t *pi, const char *teamName, const char *modelName, const char *skinName, const char *base, const char *ext ) {
	char *team;
	int i;

	if ( cgs.gametype >= GT_TEAM ) {
		switch ( pi->team ) {
			case TEAM_BLUE: {
				team = "blue";
				break;
			}
			default: {
				team = "red";
				break;
			}
		}
	}
	else {
		team = "default";
	}

	for ( i = 0; i < 2; i++ ) {
		if ( i == 0 && teamName && *teamName ) {
			//								"models/players/james/stroggs/lower_lily_red.skin"
			Com_sprintf( filename, length, "models/players/%s/%s%s_%s_%s.%s", modelName, teamName, base, skinName, team, ext );
		}
		else {
			//								"models/players/james/lower_lily_red.skin"
			Com_sprintf( filename, length, "models/players/%s/%s_%s_%s.%s", modelName, base, skinName, team, ext );
		}
		if ( CG_FileExists( filename ) ) {
			return qtrue;
		}
		if ( cgs.gametype >= GT_TEAM ) {
			if ( i == 0 && teamName && *teamName ) {
				//								"models/players/james/stroggs/lower_red.skin"
				Com_sprintf( filename, length, "models/players/%s/%s%s_%s.%s", modelName, teamName, base, team, ext );
			}
			else {
				//								"models/players/james/lower_red.skin"
				Com_sprintf( filename, length, "models/players/%s/%s_%s.%s", modelName, base, team, ext );
			}
		}
		else {
			if ( i == 0 && teamName && *teamName ) {
				//								"models/players/james/stroggs/lower_lily.skin"
				Com_sprintf( filename, length, "models/players/%s/%s%s_%s.%s", modelName, teamName, base, skinName, ext );
			}
			else {
				//								"models/players/james/lower_lily.skin"
				Com_sprintf( filename, length, "models/players/%s/%s_%s.%s", modelName, base, skinName, ext );
			}
		}
		if ( CG_FileExists( filename ) ) {
			return qtrue;
		}
		if ( !teamName || !*teamName ) {
			break;
		}
	}

	return qfalse;
}

/*
==========================
CG_FindPlayerHeadFile
==========================
*/
static qboolean	CG_FindPlayerHeadFile( char *filename, int length, playerInfo_t *pi, const char *teamName, const char *headModelName, const char *headSkinName, const char *base, const char *ext ) {
	char *team, *headsFolder;
	int i;

	if ( cgs.gametype >= GT_TEAM ) {
		switch ( pi->team ) {
			case TEAM_BLUE: {
				team = "blue";
				break;
			}
			default: {
				team = "red";
				break;
			}
		}
	}
	else {
		team = "default";
	}

	if ( headModelName[0] == '*' ) {
		headsFolder = "heads/";
		headModelName++;
	}
	else {
		headsFolder = "";
	}
	while(1) {
		for ( i = 0; i < 2; i++ ) {
			if ( i == 0 && teamName && *teamName ) {
				Com_sprintf( filename, length, "models/players/%s%s/%s/%s%s_%s.%s", headsFolder, headModelName, headSkinName, teamName, base, team, ext );
			}
			else {
				Com_sprintf( filename, length, "models/players/%s%s/%s/%s_%s.%s", headsFolder, headModelName, headSkinName, base, team, ext );
			}

			if (Q_stricmpn(ext, "$image", 6) == 0) {
				COM_StripExtension(filename, filename, length);
				if (trap_R_RegisterShaderNoMip(filename)) {
					return qtrue;
				}
			} else if ( CG_FileExists( filename ) ) {
				return qtrue;
			}

			if ( cgs.gametype >= GT_TEAM ) {
				if ( i == 0 &&  teamName && *teamName ) {
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, team, ext );
				}
				else {
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, team, ext );
				}
			}
			else {
				if ( i == 0 && teamName && *teamName ) {
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, headSkinName, ext );
				}
				else {
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, headSkinName, ext );
				}
			}

			if (Q_stricmpn(ext, "$image", 6) == 0) {
				COM_StripExtension(filename, filename, length);
				if (trap_R_RegisterShaderNoMip(filename)) {
					return qtrue;
				}
			} else if ( CG_FileExists( filename ) ) {
				return qtrue;
			}

			if ( !teamName || !*teamName ) {
				break;
			}
		}
		// if tried the heads folder first
		if ( headsFolder[0] ) {
			break;
		}
		headsFolder = "heads/";
	}

	return qfalse;
}

/*
==========================
CG_AddSkinToFrame
==========================
*/
qhandle_t CG_AddSkinToFrame( const cgSkin_t *skin ) {
	if ( !skin || !skin->numSurfaces ) {
		return 0;
	}

	return trap_R_AddSkinToFrame( skin->numSurfaces, skin->surfaces );
}

/*
==========================
CG_RegisterSkin
==========================
*/
qboolean CG_RegisterSkin( const char *name, cgSkin_t *skin, qboolean append ) {
	char		*text_p;
	int			len;
	char		*token;
	char		text[20000];
	fileHandle_t	f;
	char		surfName[MAX_QPATH];
	char		shaderName[MAX_QPATH];
	qhandle_t	hShader;

	if ( !name || !name[0] ) {
		Com_DPrintf( "Empty name passed to RE_RegisterSkin\n" );
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_DPrintf( "Skin name exceeds MAX_QPATH\n" );
		return 0;
	}

	if ( !COM_CompareExtension( name, ".skin" ) ) {
		Com_DPrintf( "WARNING: CG_RegisterSkin ignoring '%s', must have \".skin\" extension\n", name );
		return 0;
	}

	if ( !append ) {
		skin->numSurfaces = 0;
	}

	// load the file
	len = trap_FS_FOpenFile( name, &f, FS_READ );
	if ( len <= 0 ) {
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		CG_Printf( "File %s too long\n", name );
		trap_FS_FCloseFile( f );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	text_p = text;

	while ( text_p && *text_p ) {
		// get surface name
		token = COM_ParseExt2( &text_p, qtrue, ',' );
		Q_strncpyz( surfName, token, sizeof( surfName ) );

		if ( !token[0] ) {
			break;
		}

		if ( *text_p == ',' ) {
			text_p++;
		}

		if ( !Q_stricmpn( token, "tag_", 4 ) ) {
			SkipRestOfLine( &text_p );
			continue;
		}

		// skip RTCW/ET skin settings
		if ( !Q_stricmpn( token, "md3_", 4 ) || !Q_stricmp( token, "playerscale" ) ) {
			SkipRestOfLine( &text_p );
			continue;
		}

		// parse the shader name
		token = COM_ParseExt2( &text_p, qfalse, ',' );
		Q_strncpyz( shaderName, token, sizeof( shaderName ) );

		if ( skin->numSurfaces >= MAX_CG_SKIN_SURFACES ) {
			Com_Printf( "WARNING: Ignoring surfaces in '%s', the max is %d surfaces!\n", name, MAX_CG_SKIN_SURFACES );
			break;
		}

		hShader = trap_R_RegisterShaderEx( shaderName, LIGHTMAP_NONE, qtrue );

		// for compatibility with quake3 skins, don't render missing shaders listed in skins
		if ( !hShader ) {
			hShader = cgs.media.nodrawShader;
		}

		skin->surfaces[skin->numSurfaces] = trap_R_AllocSkinSurface( surfName, hShader );
		skin->numSurfaces++;
	}

	// failed to load surfaces
	if ( !skin->numSurfaces ) {
		return qfalse;
	}

	return qtrue;
}

/*
==========================
CG_RegisterPlayerSkin
==========================
*/
static qboolean	CG_RegisterPlayerSkin( playerInfo_t *pi, const char *teamName, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName ) {
	char filename[MAX_QPATH];
	qboolean legsSkin, torsoSkin, headSkin;

	legsSkin = torsoSkin = headSkin = qfalse;

	/*
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/%slower_%s.skin", modelName, teamName, skinName );
	pi->legsSkin = trap_R_RegisterSkin( filename );
	if (!pi->legsSkin) {
		Com_Printf( "Leg skin load failure: %s\n", filename );
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/%supper_%s.skin", modelName, teamName, skinName );
	pi->torsoSkin = trap_R_RegisterSkin( filename );
	if (!pi->torsoSkin) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}
	*/
	if ( CG_FindPlayerModelFile( filename, sizeof(filename), pi, teamName, modelName, skinName, "lower", "skin" ) ) {
		legsSkin = CG_RegisterSkin( filename, &pi->modelSkin, qfalse );
	}
	if (!legsSkin) {
		Com_Printf( "Leg skin load failure: %s\n", filename );
	}

	if ( CG_FindPlayerModelFile( filename, sizeof(filename), pi, teamName, modelName, skinName, "upper", "skin" ) ) {
		torsoSkin = CG_RegisterSkin( filename, &pi->modelSkin, qtrue );
	}
	if (!torsoSkin) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}

	if ( CG_FindPlayerHeadFile( filename, sizeof(filename), pi, teamName, headModelName, headSkinName, "head", "skin" ) ) {
		headSkin = CG_RegisterSkin( filename, &pi->modelSkin, qtrue );
	}
	if (!headSkin) {
		Com_Printf( "Head skin load failure: %s\n", filename );
	}

	// if any skins failed to load
	if ( !legsSkin || !torsoSkin || !headSkin ) {
		return qfalse;
	}
	return qtrue;
}

/*
==========================
CG_RegisterPlayerModelname
==========================
*/
static qboolean CG_RegisterPlayerModelname( playerInfo_t *pi, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName, const char *teamName ) {
	char	filename[MAX_QPATH];
	const char		*headName;
	char newTeamName[MAX_QPATH];

	if ( headModelName[0] == '\0' ) {
		headName = modelName;
	}
	else {
		headName = headModelName;
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower.md3", modelName );
	pi->legsModel = trap_R_RegisterModel( filename );
	if ( !pi->legsModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper.md3", modelName );
	pi->torsoModel = trap_R_RegisterModel( filename );
	if ( !pi->torsoModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	if( headName[0] == '*' ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/%s/%s.md3", &headModelName[1], &headModelName[1] );
	}
	else {
		Com_sprintf( filename, sizeof( filename ), "models/players/%s/head.md3", headName );
	}
	pi->headModel = trap_R_RegisterModel( filename );
	// if the head model could not be found and we didn't load from the heads folder try to load from there
	if ( !pi->headModel && headName[0] != '*' ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/%s/%s.md3", headModelName, headModelName );
		pi->headModel = trap_R_RegisterModel( filename );
	}
	if ( !pi->headModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	// if any skins failed to load, return failure
	if ( !CG_RegisterPlayerSkin( pi, teamName, modelName, skinName, headName, headSkinName ) ) {
		if ( teamName && *teamName) {
			Com_Printf( "Failed to load skin file: %s : %s : %s, %s : %s\n", teamName, modelName, skinName, headName, headSkinName );
			if( pi->team == TEAM_BLUE ) {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_BLUETEAM_NAME);
			}
			else {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_REDTEAM_NAME);
			}
			if ( !CG_RegisterPlayerSkin( pi, newTeamName, modelName, skinName, headName, headSkinName ) ) {
				Com_Printf( "Failed to load skin file: %s : %s : %s, %s : %s\n", newTeamName, modelName, skinName, headName, headSkinName );
				return qfalse;
			}
		} else {
			Com_Printf( "Failed to load skin file: %s : %s, %s : %s\n", modelName, skinName, headName, headSkinName );
			return qfalse;
		}
	}

	// load the animations
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/animation.cfg", modelName );
	if ( !CG_ParseAnimationFile( filename, pi ) ) {
		Com_Printf( "Failed to load animation file %s\n", filename );
		return qfalse;
	}

	if ( CG_FindPlayerHeadFile( filename, sizeof(filename), pi, teamName, headName, headSkinName, "icon", "$image" ) ) {
		pi->modelIcon = trap_R_RegisterShaderNoMip( filename );
	}

	if ( !pi->modelIcon ) {
		return qfalse;
	}

	return qtrue;
}

/*
====================
CG_PlayerColorFromString
====================
*/
static void CG_PlayerColorFromString( const char *v, vec3_t color ) {
	int val;

	val = atoi( v );

	CG_PlayerColorFromIndex( val, color );
}

/*
====================
CG_PlayerColorFromIndex
====================
*/
void CG_PlayerColorFromIndex( int val, vec3_t color ) {

	switch (val)
	{
		case 1: // blue
		case 2: // green
		case 3: // cyan
		case 4: // red
		case 5: // magenta
		case 6: // yellow
		case 7: // white
			VectorClear( color );

			if ( val & 1 ) {
				color[2] = 1.0f;
			}
			if ( val & 2 ) {
				color[1] = 1.0f;
			}
			if ( val & 4 ) {
				color[0] = 1.0f;
			}
			break;

		case 8: // orange
			VectorSet( color, 1, 0.5f, 0 );
			break;
		case 9: // lime
			VectorSet( color, 0.5f, 1, 0 );
			break;
		case 10: // vivid green
			VectorSet( color, 0, 1, 0.5f );
			break;
		case 11: // light blue
			VectorSet( color, 0, 0.5f, 1 );
			break;
		case 12: // purple
			VectorSet( color, 0.5f, 0, 1 );
			break;
		case 13: // pink
			VectorSet( color, 1, 0, 0.5f );
			break;

		default: // fall back to white
			VectorSet( color, 1, 1, 1 );
			break;
	}
}

/*
===================
CG_LoadPlayerInfo

Load it now, taking the disk hits.
This will usually be deferred to a safe time
===================
*/
static void CG_LoadPlayerInfo( int playerNum, playerInfo_t *pi ) {
	const char	*dir, *fallback;
	int			i, modelloaded;
	const char	*s;
	char		teamname[MAX_QPATH];

	teamname[0] = 0;
#ifdef MISSIONPACK
	if( cgs.gametype >= GT_TEAM) {
		if( pi->team == TEAM_BLUE ) {
			Q_strncpyz(teamname, cg_blueTeamName.string, sizeof(teamname) );
		} else {
			Q_strncpyz(teamname, cg_redTeamName.string, sizeof(teamname) );
		}
	}
	if( teamname[0] ) {
		strcat( teamname, "/" );
	}
#endif
	modelloaded = qtrue;
	if ( !CG_RegisterPlayerModelname( pi, pi->modelName, pi->skinName, pi->headModelName, pi->headSkinName, teamname ) ) {
		if ( cg_buildScript.integer ) {
			CG_Error( "CG_RegisterPlayerModelname( %s, %s, %s, %s %s ) failed", pi->modelName, pi->skinName, pi->headModelName, pi->headSkinName, teamname );
		}

		// fall back to default team name
		if( cgs.gametype >= GT_TEAM) {
			// keep skin name
			if( pi->team == TEAM_BLUE ) {
				Q_strncpyz(teamname, DEFAULT_BLUETEAM_NAME, sizeof(teamname) );
			} else {
				Q_strncpyz(teamname, DEFAULT_REDTEAM_NAME, sizeof(teamname) );
			}
			if ( !CG_RegisterPlayerModelname( pi, DEFAULT_TEAM_MODEL, pi->skinName, DEFAULT_TEAM_HEAD, pi->skinName, teamname ) ) {
				CG_Error( "DEFAULT_TEAM_MODEL / skin (%s/%s) failed to register", DEFAULT_TEAM_MODEL, pi->skinName );
			}
		} else {
			if ( !CG_RegisterPlayerModelname( pi, DEFAULT_MODEL, "default", DEFAULT_HEAD, "default", teamname ) ) {
				CG_Error( "DEFAULT_MODEL (%s) failed to register", DEFAULT_MODEL );
			}
		}
		modelloaded = qfalse;
	}

	pi->newAnims = qfalse;
	if ( pi->torsoModel ) {
		orientation_t tag;
		// if the torso model has the "tag_flag"
		if ( trap_R_LerpTag( &tag, pi->torsoModel, 0, 0, 1, "tag_flag" ) ) {
			pi->newAnims = qtrue;
		}
	}

	// sounds
	dir = pi->modelName;
	if (cgs.gametype >= GT_TEAM) {
		fallback = (pi->gender == GENDER_FEMALE) ? DEFAULT_TEAM_MODEL_FEMALE : DEFAULT_TEAM_MODEL_MALE;
	} else {
		fallback = (pi->gender == GENDER_FEMALE) ? DEFAULT_MODEL_FEMALE : DEFAULT_MODEL_MALE;
	}

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS ; i++ ) {
		s = cg_customSoundNames[i];
		if ( !s ) {
			break;
		}
		pi->sounds[i] = 0;
		// if the model didn't load use the sounds of the default model
		if (modelloaded) {
			pi->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", dir, s + 1), qfalse );
		}
		if ( !pi->sounds[i] ) {
			pi->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", fallback, s + 1), qfalse );
		}
	}

	pi->deferred = qfalse;

	// reset any existing players and bodies, because they might be in bad
	// frames for this new model
	for ( i = 0 ; i < MAX_GENTITIES ; i++ ) {
		if ( cg_entities[i].currentState.playerNum == playerNum
			&& cg_entities[i].currentState.eType == ET_PLAYER ) {
			CG_ResetPlayerEntity( &cg_entities[i] );
		}
	}
}

/*
======================
CG_CopyPlayerInfoModel
======================
*/
static void CG_CopyPlayerInfoModel( playerInfo_t *from, playerInfo_t *to ) {
	VectorCopy( from->headOffset, to->headOffset );
	to->footsteps = from->footsteps;
	to->gender = from->gender;

	to->legsModel = from->legsModel;
	to->torsoModel = from->torsoModel;
	to->headModel = from->headModel;
	to->modelSkin = from->modelSkin;
	to->modelIcon = from->modelIcon;

	to->newAnims = from->newAnims;

	memcpy( to->animations, from->animations, sizeof( to->animations ) );
	memcpy( to->sounds, from->sounds, sizeof( to->sounds ) );
}

/*
======================
CG_ScanForExistingPlayerInfo
======================
*/
static qboolean CG_ScanForExistingPlayerInfo( playerInfo_t *pi ) {
	int		i;
	playerInfo_t	*match;

	for ( i = 0 ; i < cgs.maxplayers ; i++ ) {
		match = &cgs.playerinfo[ i ];
		if ( !match->infoValid ) {
			continue;
		}
		if ( match->deferred ) {
			continue;
		}
		if ( !Q_stricmp( pi->modelName, match->modelName )
			&& !Q_stricmp( pi->skinName, match->skinName )
			&& !Q_stricmp( pi->headModelName, match->headModelName )
			&& !Q_stricmp( pi->headSkinName, match->headSkinName ) 
#ifdef MISSIONPACK
			&& !Q_stricmp( pi->blueTeam, match->blueTeam ) 
			&& !Q_stricmp( pi->redTeam, match->redTeam )
#endif
			&& (cgs.gametype < GT_TEAM || pi->team == match->team) ) {
			// this playerinfo is identical, so use its handles

			pi->deferred = qfalse;

			CG_CopyPlayerInfoModel( match, pi );

			return qtrue;
		}
	}

	// nothing matches, so defer the load
	return qfalse;
}

/*
======================
CG_SetDeferredPlayerInfo

We aren't going to load it now, so grab some other
plsyrt's info to use until we have some spare time.
======================
*/
static void CG_SetDeferredPlayerInfo( int playerNum, playerInfo_t *pi ) {
	int		i;
	playerInfo_t	*match;

	// if someone else is already the same models and skins we
	// can just load the player info
	for ( i = 0 ; i < cgs.maxplayers ; i++ ) {
		match = &cgs.playerinfo[ i ];
		if ( !match->infoValid || match->deferred ) {
			continue;
		}
		if ( Q_stricmp( pi->skinName, match->skinName ) ||
			 Q_stricmp( pi->modelName, match->modelName ) ||
//			 Q_stricmp( pi->headModelName, match->headModelName ) ||
//			 Q_stricmp( pi->headSkinName, match->headSkinName ) ||
			 (cgs.gametype >= GT_TEAM && pi->team != match->team) ) {
			continue;
		}
		// just load the real info cause it uses the same models and skins
		CG_LoadPlayerInfo( playerNum, pi );
		return;
	}

	// if we are in teamplay, only grab a model if the skin is correct
	if ( cgs.gametype >= GT_TEAM ) {
		for ( i = 0 ; i < cgs.maxplayers ; i++ ) {
			match = &cgs.playerinfo[ i ];
			if ( !match->infoValid || match->deferred ) {
				continue;
			}
			if ( Q_stricmp( pi->skinName, match->skinName ) ||
				(cgs.gametype >= GT_TEAM && pi->team != match->team) ) {
				continue;
			}
			pi->deferred = qtrue;
			CG_CopyPlayerInfoModel( match, pi );
			return;
		}
		// load the full model, because we don't ever want to show
		// an improper team skin.  This will cause a hitch for the first
		// player, when the second enters.  Combat shouldn't be going on
		// yet, so it shouldn't matter
		CG_LoadPlayerInfo( playerNum, pi );
		return;
	}

	// find the first valid playerinfo and grab its stuff
	for ( i = 0 ; i < cgs.maxplayers ; i++ ) {
		match = &cgs.playerinfo[ i ];
		if ( !match->infoValid ) {
			continue;
		}

		pi->deferred = qtrue;
		CG_CopyPlayerInfoModel( match, pi );
		return;
	}

	// we should never get here...
	CG_Printf( "CG_SetDeferredPlayerInfo: no valid players!\n" );

	CG_LoadPlayerInfo( playerNum, pi );
}


/*
======================
CG_NewPlayerInfo
======================
*/
void CG_NewPlayerInfo( int playerNum ) {
	playerInfo_t *pi;
	playerInfo_t newInfo;
	const char	*configstring;
	const char	*v;
	char		*slash;

	pi = &cgs.playerinfo[playerNum];

	configstring = CG_ConfigString( playerNum + CS_PLAYERS );
	if ( !configstring[0] ) {
		memset( pi, 0, sizeof( *pi ) );
		return;		// player just left
	}

	// build into a temp buffer so the defer checks can use
	// the old value
	memset( &newInfo, 0, sizeof( newInfo ) );

	// isolate the player's name
	v = Info_ValueForKey(configstring, "n");
	Q_strncpyz( newInfo.name, v, sizeof( newInfo.name ) );

	// colors
	v = Info_ValueForKey( configstring, "c1" );
	CG_PlayerColorFromString( v, newInfo.color1 );

	newInfo.c1RGBA[0] = 255 * newInfo.color1[0];
	newInfo.c1RGBA[1] = 255 * newInfo.color1[1];
	newInfo.c1RGBA[2] = 255 * newInfo.color1[2];
	newInfo.c1RGBA[3] = 255;

	v = Info_ValueForKey( configstring, "c2" );
	CG_PlayerColorFromString( v, newInfo.color2 );

	newInfo.c2RGBA[0] = 255 * newInfo.color2[0];
	newInfo.c2RGBA[1] = 255 * newInfo.color2[1];
	newInfo.c2RGBA[2] = 255 * newInfo.color2[2];
	newInfo.c2RGBA[3] = 255;

	// bot skill
	v = Info_ValueForKey( configstring, "skill" );
	newInfo.botSkill = atoi( v );

	// handicap
	v = Info_ValueForKey( configstring, "hc" );
	newInfo.handicap = atoi( v );

	// wins
	v = Info_ValueForKey( configstring, "w" );
	newInfo.wins = atoi( v );

	// losses
	v = Info_ValueForKey( configstring, "l" );
	newInfo.losses = atoi( v );

	// team
	v = Info_ValueForKey( configstring, "t" );
	newInfo.team = atoi( v );

	// team task
	v = Info_ValueForKey( configstring, "tt" );
	newInfo.teamTask = atoi(v);

	// team leader
	v = Info_ValueForKey( configstring, "tl" );
	newInfo.teamLeader = atoi(v);

#ifdef MISSIONPACK
	Q_strncpyz(newInfo.redTeam, cg_redTeamName.string, MAX_TEAMNAME);

	Q_strncpyz(newInfo.blueTeam, cg_blueTeamName.string, MAX_TEAMNAME);
#endif

	// model
	v = Info_ValueForKey( configstring, "model" );
	if ( cg_forceModel.integer ) {
		// forcemodel makes everyone use a single model
		// to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if( cgs.gametype >= GT_TEAM ) {
			Q_strncpyz( newInfo.modelName, DEFAULT_TEAM_MODEL, sizeof( newInfo.modelName ) );
			Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );
		} else {
			trap_Cvar_VariableStringBuffer( "model", modelStr, sizeof( modelStr ) );
			if ( ( skin = strchr( modelStr, '/' ) ) == NULL) {
				skin = "default";
			} else {
				*skin++ = 0;
			}

			Q_strncpyz( newInfo.skinName, skin, sizeof( newInfo.skinName ) );
			Q_strncpyz( newInfo.modelName, modelStr, sizeof( newInfo.modelName ) );
		}

		if ( cgs.gametype >= GT_TEAM ) {
			// keep skin name
			slash = strchr( v, '/' );
			if ( slash ) {
				Q_strncpyz( newInfo.skinName, slash + 1, sizeof( newInfo.skinName ) );
			}
		}
	} else {
		Q_strncpyz( newInfo.modelName, v, sizeof( newInfo.modelName ) );

		slash = strchr( newInfo.modelName, '/' );
		if ( !slash ) {
			// modelName didn not include a skin name
			Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );
		} else {
			Q_strncpyz( newInfo.skinName, slash + 1, sizeof( newInfo.skinName ) );
			// truncate modelName
			*slash = 0;
		}
	}

	// head model
	v = Info_ValueForKey( configstring, "hmodel" );
	if ( cg_forceModel.integer ) {
		// forcemodel makes everyone use a single model
		// to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if( cgs.gametype >= GT_TEAM ) {
			Q_strncpyz( newInfo.headModelName, DEFAULT_TEAM_HEAD, sizeof( newInfo.headModelName ) );
			Q_strncpyz( newInfo.headSkinName, "default", sizeof( newInfo.headSkinName ) );
		} else {
			trap_Cvar_VariableStringBuffer( "headmodel", modelStr, sizeof( modelStr ) );
			if ( ( skin = strchr( modelStr, '/' ) ) == NULL) {
				skin = "default";
			} else {
				*skin++ = 0;
			}

			Q_strncpyz( newInfo.headSkinName, skin, sizeof( newInfo.headSkinName ) );
			Q_strncpyz( newInfo.headModelName, modelStr, sizeof( newInfo.headModelName ) );
		}

		if ( cgs.gametype >= GT_TEAM ) {
			// keep skin name
			slash = strchr( v, '/' );
			if ( slash ) {
				Q_strncpyz( newInfo.headSkinName, slash + 1, sizeof( newInfo.headSkinName ) );
			}
		}
	} else {
		Q_strncpyz( newInfo.headModelName, v, sizeof( newInfo.headModelName ) );

		slash = strchr( newInfo.headModelName, '/' );
		if ( !slash ) {
			// modelName didn not include a skin name
			Q_strncpyz( newInfo.headSkinName, "default", sizeof( newInfo.headSkinName ) );
		} else {
			Q_strncpyz( newInfo.headSkinName, slash + 1, sizeof( newInfo.headSkinName ) );
			// truncate modelName
			*slash = 0;
		}
	}

	// scan for an existing playerinfo that matches this modelname
	// so we can avoid loading checks if possible
	if ( !CG_ScanForExistingPlayerInfo( &newInfo ) ) {
		qboolean	forceDefer;

		forceDefer = trap_MemoryRemaining() < 4000000;

		// if we are defering loads, just have it pick the first valid
		if ( forceDefer || (cg_deferPlayers.integer && !cg_buildScript.integer && !cg.loading ) ) {
			// keep whatever they had if it won't violate team skins
			CG_SetDeferredPlayerInfo( playerNum, &newInfo );
			// if we are low on memory, leave them with this model
			if ( forceDefer ) {
				CG_Printf( "Memory is low. Using deferred model.\n" );
				newInfo.deferred = qfalse;
			}
		} else {
			CG_LoadPlayerInfo( playerNum, &newInfo );
		}
	}

	// replace whatever was there with the new one
	newInfo.infoValid = qtrue;
	*pi = newInfo;
}



/*
======================
CG_LoadDeferredPlayers

Called each frame when a player is dead
and the scoreboard is up
so deferred players can be loaded
======================
*/
void CG_LoadDeferredPlayers( void ) {
	int		i;
	playerInfo_t	*pi;

	// scan for a deferred player to load
	for ( i = 0, pi = cgs.playerinfo ; i < cgs.maxplayers ; i++, pi++ ) {
		if ( pi->infoValid && pi->deferred ) {
			// if we are low on memory, leave it deferred
			if ( trap_MemoryRemaining() < 4000000 ) {
				CG_Printf( "Memory is low. Using deferred model.\n" );
				pi->deferred = qfalse;
				continue;
			}
			CG_LoadPlayerInfo( i, pi );
//			break;
		}
	}
}

/*
=============================================================================

PLAYER ANIMATION

=============================================================================
*/


/*
===============
CG_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetLerpFrameAnimation( playerInfo_t *pi, lerpFrame_t *lf, int newAnimation ) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS ) {
		CG_Error( "Bad animation number: %i", newAnimation );
	}

	anim = &pi->animations[ newAnimation ];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	if ( cg_debugAnim.integer ) {
		CG_Printf( "Anim: %i\n", newAnimation );
	}
}

/*
===============
CG_RunLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunLerpFrame( playerInfo_t *pi, lerpFrame_t *lf, int newAnimation, float speedScale ) {
	int			f, numFrames;
	animation_t	*anim;

	// debugging tool to get no animations
	if ( cg_animSpeed.integer == 0 ) {
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if ( newAnimation != lf->animationNumber || !lf->animation ) {
		CG_SetLerpFrameAnimation( pi, lf, newAnimation );
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( cg.time >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( !anim->frameLerp ) {
			return;		// shouldn't happen
		}
		if ( cg.time < lf->animationTime ) {
			lf->frameTime = lf->animationTime;		// initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
		f *= speedScale;		// adjust for haste, etc

		numFrames = anim->numFrames;
		if (anim->flipflop) {
			numFrames *= 2;
		}
		if ( f >= numFrames ) {
			f -= numFrames;
			if ( anim->loopFrames ) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}
		if ( anim->reversed ) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else if (anim->flipflop && f>=anim->numFrames) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
		}
		else {
			lf->frame = anim->firstFrame + f;
		}
		if ( cg.time > lf->frameTime ) {
			lf->frameTime = cg.time;
			if ( cg_debugAnim.integer ) {
				CG_Printf( "Clamp lf->frameTime\n");
			}
		}
	}

	if ( lf->frameTime > cg.time + 200 ) {
		lf->frameTime = cg.time;
	}

	if ( lf->oldFrameTime > cg.time ) {
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}


/*
===============
CG_ClearLerpFrame
===============
*/
static void CG_ClearLerpFrame( playerInfo_t *pi, lerpFrame_t *lf, int animationNumber ) {
	lf->frameTime = lf->oldFrameTime = cg.time;
	CG_SetLerpFrameAnimation( pi, lf, animationNumber );
	lf->oldFrame = lf->frame = lf->animation->firstFrame;
}


/*
===============
CG_PlayerAnimation
===============
*/
static void CG_PlayerAnimation( centity_t *cent, int *legsOld, int *legs, float *legsBackLerp,
						int *torsoOld, int *torso, float *torsoBackLerp ) {
	playerInfo_t	*pi;
	int				playerNum;
	float			speedScale;

	playerNum = cent->currentState.playerNum;

	if ( cg_noPlayerAnims.integer ) {
		*legsOld = *legs = *torsoOld = *torso = 0;
		return;
	}

	if ( cent->currentState.powerups & ( 1 << PW_HASTE ) ) {
		speedScale = 1.5;
	} else {
		speedScale = 1;
	}

	pi = &cgs.playerinfo[ playerNum ];

	// do the shuffle turn frames locally
	if ( cent->pe.legs.yawing && ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_IDLE ) {
		CG_RunLerpFrame( pi, &cent->pe.legs, LEGS_TURN, speedScale );
	} else {
		CG_RunLerpFrame( pi, &cent->pe.legs, cent->currentState.legsAnim, speedScale );
	}

	*legsOld = cent->pe.legs.oldFrame;
	*legs = cent->pe.legs.frame;
	*legsBackLerp = cent->pe.legs.backlerp;

	CG_RunLerpFrame( pi, &cent->pe.torso, cent->currentState.torsoAnim, speedScale );

	*torsoOld = cent->pe.torso.oldFrame;
	*torso = cent->pe.torso.frame;
	*torsoBackLerp = cent->pe.torso.backlerp;
}

/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

/*
==================
CG_SwingAngles
==================
*/
static void CG_SwingAngles( float destination, float swingTolerance, float clampTolerance,
					float speed, float *angle, qboolean *swinging ) {
	float	swing;
	float	move;
	float	scale;

	if ( !*swinging ) {
		// see if a swing should be started
		swing = AngleSubtract( *angle, destination );
		if ( swing > swingTolerance || swing < -swingTolerance ) {
			*swinging = qtrue;
		}
	}

	if ( !*swinging ) {
		return;
	}
	
	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract( destination, *angle );
	scale = fabs( swing );
	if ( scale < swingTolerance * 0.5 ) {
		scale = 0.5;
	} else if ( scale < swingTolerance ) {
		scale = 1.0;
	} else {
		scale = 2.0;
	}

	// swing towards the destination angle
	if ( swing >= 0 ) {
		move = cg.frametime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	} else if ( swing < 0 ) {
		move = cg.frametime * scale * -speed;
		if ( move <= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	}

	// clamp to no more than tolerance
	swing = AngleSubtract( destination, *angle );
	if ( swing > clampTolerance ) {
		*angle = AngleMod( destination - (clampTolerance - 1) );
	} else if ( swing < -clampTolerance ) {
		*angle = AngleMod( destination + (clampTolerance - 1) );
	}
}

/*
=================
CG_AddPainTwitch
=================
*/
static void CG_AddPainTwitch( centity_t *cent, vec3_t torsoAngles ) {
	int		t;
	float	f;

	t = cg.time - cent->pe.painTime;
	if ( t >= PAIN_TWITCH_TIME ) {
		return;
	}

	f = 1.0 - (float)t / PAIN_TWITCH_TIME;

	if ( cent->pe.painDirection ) {
		torsoAngles[ROLL] += 20 * f;
	} else {
		torsoAngles[ROLL] -= 20 * f;
	}
}


/*
===============
CG_PlayerAngles

Handles seperate torso motion

  legs pivot based on direction of movement

  head always looks exactly at cent->lerpAngles

  if motion < 20 degrees, show in head only
  if < 45 degrees, also show in torso
===============
*/
static void CG_PlayerAngles( centity_t *cent, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] ) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	static	int	movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t		velocity;
	float		speed;
	int			dir, playerNum;
	playerInfo_t	*pi;

	VectorCopy( cent->lerpAngles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_IDLE 
		|| ((cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND 
		&& (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND2)) {
		// if not standing still, always point all in the same direction
		cent->pe.torso.yawing = qtrue;	// always center
		cent->pe.torso.pitching = qtrue;	// always center
		cent->pe.legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	if ( cent->currentState.eFlags & EF_DEAD ) {
		// don't let dead bodies twitch
		dir = 0;
	} else {
		dir = cent->currentState.angles2[YAW];
		if ( dir < 0 || dir > 7 ) {
			CG_Error( "Bad player movement angle" );
		}
	}
	legsAngles[YAW] = headAngles[YAW] + movementOffsets[ dir ];
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[ dir ];

	// torso
	CG_SwingAngles( torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing );
	CG_SwingAngles( legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing );

	torsoAngles[YAW] = cent->pe.torso.yawAngle;
	legsAngles[YAW] = cent->pe.legs.yawAngle;


	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = (-360 + headAngles[PITCH]) * 0.75f;
	} else {
		dest = headAngles[PITCH] * 0.75f;
	}
	CG_SwingAngles( dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching );
	torsoAngles[PITCH] = cent->pe.torso.pitchAngle;

	//
	playerNum = cent->currentState.playerNum;
	if ( playerNum >= 0 && playerNum < MAX_CLIENTS ) {
		pi = &cgs.playerinfo[ playerNum ];
		if ( pi->fixedtorso ) {
			torsoAngles[PITCH] = 0.0f;
		}
	}

	// --------- roll -------------


	// lean towards the direction of travel
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	speed = VectorNormalize( velocity );
	if ( speed ) {
		vec3_t	axis[3];
		float	side;

		speed *= 0.05f;

		AnglesToAxis( legsAngles, axis );
		side = speed * DotProduct( velocity, axis[1] );
		legsAngles[ROLL] -= side;

		side = speed * DotProduct( velocity, axis[0] );
		legsAngles[PITCH] += side;
	}

	//
	playerNum = cent->currentState.playerNum;
	if ( playerNum >= 0 && playerNum < MAX_CLIENTS ) {
		pi = &cgs.playerinfo[ playerNum ];
		if ( pi->fixedlegs ) {
			legsAngles[YAW] = torsoAngles[YAW];
			legsAngles[PITCH] = 0.0f;
			legsAngles[ROLL] = 0.0f;
		}
	}

	// pain twitch
	CG_AddPainTwitch( cent, torsoAngles );

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
	AnglesToAxis( headAngles, head );
}


//==========================================================================

/*
===============
CG_HasteTrail
===============
*/
static void CG_HasteTrail( centity_t *cent ) {
	localEntity_t	*smoke;
	vec3_t			origin;
	int				anim;

	if ( cent->trailTime > cg.time ) {
		return;
	}
	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
	if ( anim != LEGS_RUN && anim != LEGS_BACK ) {
		return;
	}

	cent->trailTime += 100;
	if ( cent->trailTime < cg.time ) {
		cent->trailTime = cg.time;
	}

	VectorCopy( cent->lerpOrigin, origin );
	origin[2] -= 16;

	smoke = CG_SmokePuff( origin, vec3_origin, 
				  8, 
				  1, 1, 1, 1,
				  500, 
				  cg.time,
				  0,
				  0,
				  cgs.media.hastePuffShader );

	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}

/*
===============
CG_BreathPuff
===============
*/
static void CG_BreathPuff( int playerNum, qboolean firstPerson, vec3_t origin, vec3_t *axis ) {
	int				i;
	int				contents;
	localEntity_t	*le;
	localEntity_t	*puffs[ 3 + 5 ] = { 0 };
	int				numPuffs = 0;

	contents = CG_PointContents( origin, 0 );

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		numPuffs = CG_SpawnBubbles( puffs, origin, 2, (int)( 3 + random() * 5 ) );
	} else {
#ifdef MISSIONPACK
		if ( cg_enableBreath.integer ) {
			vec3_t up;

			VectorSet( up, 0, 0, 8 );

			puffs[0] = CG_SmokePuff( origin, up, 16, 1, 1, 1, 0.66f, 1500, cg.time, cg.time + 400, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
			numPuffs = 1;
		}
#endif
	}

	// if first person entity, only draw for specific player in first person
	// else draw for everyone, except the specific player in third person
	for ( i = 0; i < numPuffs; ++i ) {
		le = puffs[i];

		if ( firstPerson ) {
			le->defaultViewFlags = LEVF_NO_DRAW;
			le->playerEffects[0].viewFlags = LEVF_THIRD_PERSON_NO_DRAW;
		} else {
			le->defaultViewFlags = 0;
			le->playerEffects[0].viewFlags = LEVF_FIRST_PERSON_NO_DRAW;
		}

		le->playerEffects[0].playerNum = playerNum;
		le->numPlayerEffects = 1;
	}
}

/*
===============
CG_AddBreathPuffs
===============
*/
static void CG_AddBreathPuffs( centity_t *cent, refEntity_t *head ) {
	playerInfo_t *pi;
	vec3_t origin;

	if ( cent->currentState.number >= MAX_CLIENTS ) {
		return;
	}
	pi = &cgs.playerinfo[ cent->currentState.number ];

	// if it's a local player, only add the particles when they're rendering them self
	// FIXME?: Causes particles to add a frame late for local players before them (i.e., add for player 4, player 1-3 don't see until next frame)
	// FIXME: Does not add particles for all players if only a single viewport
	if ( CG_LocalPlayerState( cent->currentState.number ) && cent->currentState.number != cg.cur_ps->playerNum ) {
		return;
	}

	if ( cent->currentState.eFlags & EF_DEAD ) {
		return;
	}
	if ( pi->breathPuffTime > cg.time ) {
		return;
	}

	// Add first person effects
	if ( cent->currentState.number == cg.cur_ps->playerNum /* && !cg.cur_lc->renderingThirdPerson */ ) {
		VectorMA( cg.refdef.vieworg, 20, cg.refdef.viewaxis[0], origin );
		VectorMA( origin, -4, cg.refdef.viewaxis[2], origin );

		CG_BreathPuff( cent->currentState.number, qtrue, origin, cg.refdef.viewaxis );
	}

	// Add third person effects for mirrors / other splitscreen players
	VectorMA( head->origin, 8, head->axis[0], origin );
	VectorMA( origin, -4, head->axis[2], origin );

	CG_BreathPuff( cent->currentState.number, qfalse, origin, head->axis );

	pi->breathPuffTime = cg.time + 2000;
}

#ifdef MISSIONPACK
/*
===============
CG_DustTrail
===============
*/
static void CG_DustTrail( centity_t *cent ) {
	int				anim;
	vec3_t end, vel;
	trace_t tr;

	if (!cg_enableDust.integer)
		return;

	if ( cent->dustTrailTime > cg.time ) {
		return;
	}

	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
	if ( anim != LEGS_LANDB && anim != LEGS_LAND ) {
		return;
	}

	cent->dustTrailTime += 40;
	if ( cent->dustTrailTime < cg.time ) {
		cent->dustTrailTime = cg.time;
	}

	VectorCopy(cent->currentState.pos.trBase, end);
	end[2] -= 64;
	CG_Trace( &tr, cent->currentState.pos.trBase, NULL, NULL, end, cent->currentState.number, MASK_PLAYERSOLID );

	if ( !(tr.surfaceFlags & SURF_DUST) )
		return;

	VectorCopy( cent->currentState.pos.trBase, end );
	end[2] -= 16;

	VectorSet(vel, 0, 0, -30);
	CG_SmokePuff( end, vel,
				  24,
				  .8f, .8f, 0.7f, 0.33f,
				  500,
				  cg.time,
				  0,
				  0,
				  cgs.media.dustPuffShader );
}

#endif

/*
===============
CG_TrailItem
===============
*/
static void CG_TrailItem( centity_t *cent, qhandle_t hModel ) {
	refEntity_t		ent;
	vec3_t			angles;
	vec3_t			axis[3];

	VectorCopy( cent->lerpAngles, angles );
	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AnglesToAxis( angles, axis );

	memset( &ent, 0, sizeof( ent ) );
	VectorMA( cent->lerpOrigin, -16, axis[0], ent.origin );
	ent.origin[2] += 16;
	angles[YAW] += 90;
	AnglesToAxis( angles, ent.axis );

	if (cent->currentState.playerNum == cg.cur_lc->predictedPlayerState.playerNum
		&& cg_thirdPerson[cg.cur_localPlayerNum].integer)
	{
		// flag blocks view in third person, so only draw in mirrors
		ent.renderfx |= RF_ONLY_MIRROR;
	}

	ent.hModel = hModel;
	CG_AddRefEntityWithMinLight( &ent );
}


/*
===============
CG_PlayerFlag
===============
*/
static void CG_PlayerFlag( centity_t *cent, const cgSkin_t *skin, refEntity_t *torso ) {
	playerInfo_t	*pi;
	refEntity_t	pole;
	refEntity_t	flag;
	vec3_t		angles, dir;
	int			legsAnim, flagAnim, updateangles;
	float		angle, d;

	// show the flag pole model
	memset( &pole, 0, sizeof(pole) );
	pole.hModel = cgs.media.flagPoleModel;
	VectorCopy( torso->lightingOrigin, pole.lightingOrigin );
	pole.shadowPlane = torso->shadowPlane;
	pole.renderfx = torso->renderfx;
	CG_PositionEntityOnTag( &pole, torso, torso->hModel, "tag_flag" );
	CG_AddRefEntityWithMinLight( &pole );

	// show the flag model
	memset( &flag, 0, sizeof(flag) );
	flag.hModel = cgs.media.flagFlapModel;
	flag.customSkin = CG_AddSkinToFrame( skin );
	VectorCopy( torso->lightingOrigin, flag.lightingOrigin );
	flag.shadowPlane = torso->shadowPlane;
	flag.renderfx = torso->renderfx;

	VectorClear(angles);

	updateangles = qfalse;
	legsAnim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if( legsAnim == LEGS_IDLE || legsAnim == LEGS_IDLECR ) {
		flagAnim = FLAG_STAND;
	} else if ( legsAnim == LEGS_WALK || legsAnim == LEGS_WALKCR ) {
		flagAnim = FLAG_STAND;
		updateangles = qtrue;
	} else {
		flagAnim = FLAG_RUN;
		updateangles = qtrue;
	}

	if ( updateangles ) {

		VectorCopy( cent->currentState.pos.trDelta, dir );
		// add gravity
		dir[2] += 100;
		VectorNormalize( dir );
		d = DotProduct(pole.axis[2], dir);
		// if there is enough movement orthogonal to the flag pole
		if (fabs(d) < 0.9) {
			//
			d = DotProduct(pole.axis[0], dir);
			if (d > 1.0f) {
				d = 1.0f;
			}
			else if (d < -1.0f) {
				d = -1.0f;
			}
			angle = Q_acos(d);

			d = DotProduct(pole.axis[1], dir);
			if (d < 0) {
				angles[YAW] = 360 - angle * 180 / M_PI;
			}
			else {
				angles[YAW] = angle * 180 / M_PI;
			}
			if (angles[YAW] < 0)
				angles[YAW] += 360;
			if (angles[YAW] > 360)
				angles[YAW] -= 360;

			//vectoangles( cent->currentState.pos.trDelta, tmpangles );
			//angles[YAW] = tmpangles[YAW] + 45 - cent->pe.torso.yawAngle;
			// change the yaw angle
			CG_SwingAngles( angles[YAW], 25, 90, 0.15f, &cent->pe.flag.yawAngle, &cent->pe.flag.yawing );
		}

		/*
		d = DotProduct(pole.axis[2], dir);
		angle = Q_acos(d);

		d = DotProduct(pole.axis[1], dir);
		if (d < 0) {
			angle = 360 - angle * 180 / M_PI;
		}
		else {
			angle = angle * 180 / M_PI;
		}
		if (angle > 340 && angle < 20) {
			flagAnim = FLAG_RUNUP;
		}
		if (angle > 160 && angle < 200) {
			flagAnim = FLAG_RUNDOWN;
		}
		*/
	}

	// set the yaw angle
	angles[YAW] = cent->pe.flag.yawAngle;
	// lerp the flag animation frames
	pi = &cgs.playerinfo[ cent->currentState.playerNum ];
	CG_RunLerpFrame( pi, &cent->pe.flag, flagAnim, 1 );
	flag.oldframe = cent->pe.flag.oldFrame;
	flag.frame = cent->pe.flag.frame;
	flag.backlerp = cent->pe.flag.backlerp;

	AnglesToAxis( angles, flag.axis );
	CG_PositionRotatedEntityOnTag( &flag, &pole, pole.hModel, "tag_flag" );

	CG_AddRefEntityWithMinLight( &flag );
}


#ifdef MISSIONPACK
/*
===============
CG_PlayerTokens
===============
*/
static void CG_PlayerTokens( centity_t *cent, int renderfx ) {
	int			tokens, i, j;
	float		angle;
	refEntity_t	ent;
	vec3_t		dir, origin;
	skulltrail_t *trail;
	if ( cent->currentState.number >= MAX_CLIENTS ) {
		return;
	}
	trail = &cg.skulltrails[cent->currentState.number];
	tokens = cent->currentState.tokens;
	if ( !tokens ) {
		trail->numpositions = 0;
		return;
	}

	if ( tokens > MAX_SKULLTRAIL ) {
		tokens = MAX_SKULLTRAIL;
	}

	// add skulls if there are more than last time
	for (i = 0; i < tokens - trail->numpositions; i++) {
		for (j = trail->numpositions; j > 0; j--) {
			VectorCopy(trail->positions[j-1], trail->positions[j]);
		}
		VectorCopy(cent->lerpOrigin, trail->positions[0]);
	}
	trail->numpositions = tokens;

	// move all the skulls along the trail
	VectorCopy(cent->lerpOrigin, origin);
	for (i = 0; i < trail->numpositions; i++) {
		VectorSubtract(trail->positions[i], origin, dir);
		if (VectorNormalize(dir) > 30) {
			VectorMA(origin, 30, dir, trail->positions[i]);
		}
		VectorCopy(trail->positions[i], origin);
	}

	memset( &ent, 0, sizeof( ent ) );
	if( cgs.playerinfo[ cent->currentState.playerNum ].team == TEAM_BLUE ) {
		ent.hModel = cgs.media.redCubeModel;
	} else {
		ent.hModel = cgs.media.blueCubeModel;
	}
	ent.renderfx = renderfx;

	VectorCopy(cent->lerpOrigin, origin);
	for (i = 0; i < trail->numpositions; i++) {
		VectorSubtract(origin, trail->positions[i], ent.axis[0]);
		ent.axis[0][2] = 0;
		VectorNormalize(ent.axis[0]);
		VectorSet(ent.axis[2], 0, 0, 1);
		CrossProduct(ent.axis[0], ent.axis[2], ent.axis[1]);

		VectorCopy(trail->positions[i], ent.origin);
		angle = (((cg.time + 500 * MAX_SKULLTRAIL - 500 * i) / 16) & 255) * (M_PI * 2) / 255;
		ent.origin[2] += sin(angle) * 10;
		CG_AddRefEntityWithMinLight( &ent );
		VectorCopy(trail->positions[i], origin);
	}
}
#endif


/*
===============
CG_PlayerPowerups
===============
*/
static void CG_PlayerPowerups( centity_t *cent, refEntity_t *torso ) {
	int		powerups;
	playerInfo_t	*pi;

	powerups = cent->currentState.powerups;
	if ( !powerups ) {
		return;
	}

	// quad gives a dlight
	if ( powerups & ( 1 << PW_QUAD ) ) {
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0f, 0.2f, 0.2f, 1 );
	}

	// flight plays a looped sound
	if ( powerups & ( 1 << PW_FLIGHT ) ) {
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.flightSound );
	}

	pi = &cgs.playerinfo[ cent->currentState.playerNum ];
	// redflag
	if ( powerups & ( 1 << PW_REDFLAG ) ) {
		if (pi->newAnims) {
			CG_PlayerFlag( cent, &cgs.media.redFlagFlapSkin, torso );
		}
		else {
			CG_TrailItem( cent, cgs.media.redFlagModel );
		}
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0f, 1.0f, 0.2f, 0.2f );
	}

	// blueflag
	if ( powerups & ( 1 << PW_BLUEFLAG ) ) {
		if (pi->newAnims){
			CG_PlayerFlag( cent, &cgs.media.blueFlagFlapSkin, torso );
		}
		else {
			CG_TrailItem( cent, cgs.media.blueFlagModel );
		}
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0f, 0.2f, 0.2f, 1.0f );
	}

	// neutralflag
	if ( powerups & ( 1 << PW_NEUTRALFLAG ) ) {
		if (pi->newAnims) {
			CG_PlayerFlag( cent, &cgs.media.neutralFlagFlapSkin, torso );
		}
		else {
			CG_TrailItem( cent, cgs.media.neutralFlagModel );
		}
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0f, 1.0f, 1.0f, 1.0f );
	}

	// haste leaves smoke trails
	if ( powerups & ( 1 << PW_HASTE ) ) {
		CG_HasteTrail( cent );
	}
}


/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
static void CG_PlayerFloatSprite( vec3_t origin, int rf, qhandle_t shader ) {
	refEntity_t		ent;

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( origin, ent.origin );
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;
	trap_R_AddRefEntityToScene( &ent );
}



/*
===============
CG_PlayerSprites

Float sprites over the player's head
===============
*/
static void CG_PlayerSprites( centity_t *cent, const refEntity_t *parent ) {
	int		friendFlags, awardFlags, thirdPersonFlags;
	int		team;
	vec3_t	origin;

	VectorCopy( parent->origin, origin );
	origin[2] += 42;

	if ( cent->currentState.number == cg.cur_ps->playerNum ) {
		// current player's team sprite should only be shown in mirrors
		friendFlags = RF_ONLY_MIRROR;

		if ( !cg.cur_lc->renderingThirdPerson ) {
			thirdPersonFlags = RF_ONLY_MIRROR;
		} else {
			thirdPersonFlags = 0;
		}

		// if first person or drawing awards on HUD, only show your award sprites in mirrors
		if ( !cg.cur_lc->renderingThirdPerson || cg_draw2D.integer ) {
			awardFlags = RF_ONLY_MIRROR;
		} else {
			awardFlags = 0;
		}
	} else {
		friendFlags = awardFlags = thirdPersonFlags = 0;
	}

	if ( cent->currentState.eFlags & EF_CONNECTION ) {
		CG_PlayerFloatSprite( origin, thirdPersonFlags, cgs.media.connectionShader );
		return;
	}

	if ( cent->currentState.eFlags & EF_TALK ) {
		CG_PlayerFloatSprite( origin, thirdPersonFlags, cgs.media.balloonShader );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_IMPRESSIVE ) {
		CG_PlayerFloatSprite( origin, awardFlags, cgs.media.medalImpressive );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_EXCELLENT ) {
		CG_PlayerFloatSprite( origin, awardFlags, cgs.media.medalExcellent );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_GAUNTLET ) {
		CG_PlayerFloatSprite( origin, awardFlags, cgs.media.medalGauntlet );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_DEFEND ) {
		CG_PlayerFloatSprite( origin, awardFlags, cgs.media.medalDefend );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_ASSIST ) {
		CG_PlayerFloatSprite( origin, awardFlags, cgs.media.medalAssist );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_CAP ) {
		CG_PlayerFloatSprite( origin, awardFlags, cgs.media.medalCapture );
		return;
	}

	team = cgs.playerinfo[ cent->currentState.playerNum ].team;
	if ( !(cent->currentState.eFlags & EF_DEAD) && 
		cg.cur_ps->persistant[PERS_TEAM] == team &&
		cgs.gametype >= GT_TEAM) {
		if (cg_drawFriend.integer) {
			CG_PlayerFloatSprite( origin, friendFlags, cgs.media.friendShader );
		}
		return;
	}
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
#define	SHADOW_DISTANCE		128
static qboolean CG_PlayerShadow( centity_t *cent, vec3_t start, float alphaMult, float *shadowPlane ) {
	vec3_t		end, mins = {-8, -8, 0}, maxs = {8, 8, 2};
	trace_t		trace;
	float		alpha;

	*shadowPlane = 0;

	if ( cg_shadows.integer == 0 ) {
		return qfalse;
	}

	// no shadows when invisible
	if ( cent->currentState.powerups & ( 1 << PW_INVIS ) ) {
		return qfalse;
	}

	// send a trace down from the player to the ground
	VectorCopy( start, end );
	end[2] -= SHADOW_DISTANCE;

	CG_Trace( &trace, start, mins, maxs, end, cent->currentState.number, MASK_SOLID );

	// no shadow if too high
	if ( trace.fraction == 1.0 || trace.startsolid || trace.allsolid ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if ( cg_shadows.integer != 1 ) {	// no mark for stencil or projection shadows
		return qtrue;
	}

	// fade the shadow out with height
	alpha = ( 1.0 - trace.fraction ) * alphaMult;

	// hack / FPE - bogus planes?
	//assert( DotProduct( trace.plane.normal, trace.plane.normal ) != 0.0f ) 

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal, 
		cent->pe.legs.yawAngle, alpha,alpha,alpha,1, qfalse, 24, qtrue );

	return qtrue;
}


/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
static void CG_PlayerSplash( centity_t *cent ) {
	vec3_t		start, end;
	trace_t		trace;
	int			contents;
	polyVert_t	verts[4];

	if ( !cg_shadows.integer ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, end );
	end[2] -= 24;

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = CG_PointContents( end, 0 );
	if ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, start );
	start[2] += 32;

	// if the head isn't out of liquid, don't make a mark
	contents = CG_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	// trace down to find the surface
	trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) );

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// create a mark polygon
	VectorCopy( trace.endpos, verts[0].xyz );
	verts[0].xyz[0] -= 32;
	verts[0].xyz[1] -= 32;
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[1].xyz );
	verts[1].xyz[0] -= 32;
	verts[1].xyz[1] += 32;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[2].xyz );
	verts[2].xyz[0] += 32;
	verts[2].xyz[1] += 32;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[3].xyz );
	verts[3].xyz[0] += 32;
	verts[3].xyz[1] -= 32;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts, 0 );
}



/*
===============
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups
Also called by CG_Missile for quad rockets, but nobody can tell...
===============
*/
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state ) {

	if ( state->powerups & ( 1 << PW_INVIS ) ) {
		ent->customShader = cgs.media.invisShader;
		CG_AddRefEntityWithMinLight( ent );
	} else {
		/*
		if ( state->eFlags & EF_KAMIKAZE ) {
			if ( state->team == TEAM_BLUE )
				ent->customShader = cgs.media.blueKamikazeShader;
			else
				ent->customShader = cgs.media.redKamikazeShader;
			CG_AddRefEntityWithMinLight( ent );
		}
		else {*/
			CG_AddRefEntityWithMinLight( ent );
		//}

		if ( state->powerups & ( 1 << PW_QUAD ) )
		{
			if ( state->team == TEAM_RED )
				ent->customShader = cgs.media.redQuadShader;
			else
				ent->customShader = cgs.media.quadShader;
			CG_AddRefEntityWithMinLight( ent );
		}
		if ( state->powerups & ( 1 << PW_REGEN ) ) {
			if ( ( ( cg.time / 100 ) % 10 ) == 1 ) {
				ent->customShader = cgs.media.regenShader;
				CG_AddRefEntityWithMinLight( ent );
			}
		}
		if ( state->powerups & ( 1 << PW_BATTLESUIT ) ) {
			ent->customShader = cgs.media.battleSuitShader;
			CG_AddRefEntityWithMinLight( ent );
		}
	}
}

/*
=================
CG_LightVerts
=================
*/
int CG_LightVerts( vec3_t normal, int numVerts, polyVert_t *verts )
{
	int				i, j;
	float			incoming;
	vec3_t			ambientLight;
	vec3_t			lightDir;
	vec3_t			directedLight;

	trap_R_LightForPoint( verts[0].xyz, ambientLight, directedLight, lightDir );

	for (i = 0; i < numVerts; i++) {
		incoming = DotProduct (normal, lightDir);
		if ( incoming <= 0 ) {
			verts[i].modulate[0] = ambientLight[0];
			verts[i].modulate[1] = ambientLight[1];
			verts[i].modulate[2] = ambientLight[2];
			verts[i].modulate[3] = 255;
			continue;
		} 
		j = ( ambientLight[0] + incoming * directedLight[0] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[0] = j;

		j = ( ambientLight[1] + incoming * directedLight[1] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[1] = j;

		j = ( ambientLight[2] + incoming * directedLight[2] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[2] = j;

		verts[i].modulate[3] = 255;
	}
	return qtrue;
}

/*
===============
CG_Corpse
===============
*/
#define BODY_SINK_DIST 15
void CG_Corpse( centity_t *cent, int playerNum, float *bodySinkOffset, float *shadowAlpha ) {
	float offset;

	// After sitting around for five seconds, fall into the ground and dissapear.
	if ( cg.time - cent->currentState.pos.trTime > BODY_SINK_DELAY ) {
		if ( cg_smoothBodySink.integer ) {
			float sinkFrac;

			sinkFrac = ( cg.time - cent->currentState.pos.trTime - BODY_SINK_DELAY ) / (float)BODY_SINK_TIME;
			offset = sinkFrac * BODY_SINK_DIST;

			*shadowAlpha = 1.0f - sinkFrac;
		} else {
			// snap time to move in ( BODY_SINK_TIME / BODY_SINK_DIST ) msec jumps
			offset = ( cg.time - cent->currentState.pos.trTime - BODY_SINK_DELAY ) / (int)( (float)BODY_SINK_TIME / BODY_SINK_DIST );
			*shadowAlpha = 1;
		}
	} else {
		offset = 0;
		*shadowAlpha = 1;
	}

	*bodySinkOffset = offset;
}

/*
===============
CG_Player
===============
*/
void CG_Player( centity_t *cent ) {
	playerInfo_t	*pi;
	refEntity_t		legs;
	refEntity_t		torso;
	refEntity_t		head;
	int				playerNum;
	int				renderfx;
	qboolean		shadow;
	float			shadowPlane;
	refEntity_t		shadowRef;
	vec3_t			shadowOrigin;
	float			shadowAlpha;
	float			bodySinkOffset;
#ifdef MISSIONPACK
	refEntity_t		skull;
	refEntity_t		powerup;
	int				t;
	float			c;
	float			angle;
	vec3_t			dir, angles;
#endif

	// the player number is stored in playerNum.  It can't be derived
	// from the entity number, because a single player may have
	// multiple corpses on the level using the same playerinfo
	playerNum = cent->currentState.playerNum;
	if ( playerNum < 0 || playerNum >= MAX_CLIENTS ) {
		CG_Error( "Bad playerNum on player entity");
	}
	pi = &cgs.playerinfo[ playerNum ];

	// it is possible to see corpses from disconnected players that may
	// not have valid playerinfo
	if ( !pi->infoValid ) {
		return;
	}

	if ( cg_blood.integer && cg_gibs.integer && ( cent->currentState.eFlags & EF_GIBBED ) ) {
		return;
	}

	// get the player model information
	renderfx = 0;
	if ( cent->currentState.number == cg.cur_ps->playerNum) {
		CG_StepOffset( cent->lerpOrigin );

		if (!cg.cur_lc->renderingThirdPerson) {
			renderfx = RF_ONLY_MIRROR;
		} else {
			if (cg_cameraMode.integer) {
				return;
			}
		}
	}


	memset( &legs, 0, sizeof(legs) );
	memset( &torso, 0, sizeof(torso) );
	memset( &head, 0, sizeof(head) );

	// get the rotation information
	CG_PlayerAngles( cent, legs.axis, torso.axis, head.axis );
	
	// get the animation state (after rotation, to allow feet shuffle)
	CG_PlayerAnimation( cent, &legs.oldframe, &legs.frame, &legs.backlerp,
		 &torso.oldframe, &torso.frame, &torso.backlerp );

	if ( cent->currentState.number != playerNum && ( cent->currentState.contents & CONTENTS_CORPSE ) ) {
		CG_Corpse( cent, playerNum, &bodySinkOffset, &shadowAlpha );
	} else {
		bodySinkOffset = 0;
		shadowAlpha = 1;
	}

	// cast shadow from torso origin
	memcpy(&shadowRef, &torso, sizeof(shadowRef));
	VectorCopy(cent->lerpOrigin, legs.origin);

	// ZTM: NOTE: Make sure to set legs.frameModel / legs.oldframeModel before this
	//            call, if you're going to use them!
	if (CG_PositionRotatedEntityOnTag(&shadowRef, &legs, pi->legsModel, "tag_torso")) {
		VectorCopy(shadowRef.origin, shadowOrigin);
	} else {
		VectorCopy(cent->lerpOrigin, shadowOrigin);
	}

	// add the shadow
	shadow = CG_PlayerShadow( cent, shadowOrigin, shadowAlpha, &shadowPlane );

	// have corpse sink after shadow, so shadow doesn't disappear when origin goes into ground
	cent->lerpOrigin[2] -= bodySinkOffset;

	// add a water splash if partially in and out of water
	CG_PlayerSplash( cent );

	if ( cg_shadows.integer == 3 && shadow ) {
		renderfx |= RF_SHADOW_PLANE;
	}
	renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all
#ifdef MISSIONPACK
	if( cgs.gametype == GT_HARVESTER ) {
		CG_PlayerTokens( cent, renderfx );
	}
#endif
	//
	// add the legs
	//
	legs.hModel = pi->legsModel;
	legs.customSkin = CG_AddSkinToFrame( &pi->modelSkin );

	VectorCopy( cent->lerpOrigin, legs.origin );

	VectorCopy( cent->lerpOrigin, legs.lightingOrigin );
	legs.shadowPlane = shadowPlane;
	legs.renderfx = renderfx;
	VectorCopy (legs.origin, legs.oldorigin);	// don't positionally lerp at all

	CG_AddRefEntityWithPowerups( &legs, &cent->currentState );

	// if the model failed, allow the default nullmodel to be displayed
	if (!legs.hModel) {
		return;
	}

	//
	// add the torso
	//
	torso.hModel = pi->torsoModel;
	if (!torso.hModel) {
		return;
	}

	torso.customSkin = legs.customSkin;

	VectorCopy( cent->lerpOrigin, torso.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &torso, &legs, pi->legsModel, "tag_torso");

	torso.shadowPlane = shadowPlane;
	torso.renderfx = renderfx;

	CG_AddRefEntityWithPowerups( &torso, &cent->currentState );

	// add the talk baloon or disconnect icon
	CG_PlayerSprites( cent, &torso );

#ifdef MISSIONPACK
	if ( cent->currentState.eFlags & EF_KAMIKAZE ) {

		memset( &skull, 0, sizeof(skull) );

		VectorCopy( cent->lerpOrigin, skull.lightingOrigin );
		skull.shadowPlane = shadowPlane;
		skull.renderfx = renderfx;

		if ( cent->currentState.eFlags & EF_DEAD ) {
			// one skull bobbing above the dead body
			angle = ((cg.time / 7) & 255) * (M_PI * 2) / 255;
			if (angle > M_PI * 2)
				angle -= (float)M_PI * 2;
			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[2] = 15 + sin(angle) * 8;
			VectorAdd(torso.origin, dir, skull.origin);
			
			dir[2] = 0;
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;
			CG_AddRefEntityWithMinLight( &skull );
			skull.hModel = cgs.media.kamikazeHeadTrail;
			CG_AddRefEntityWithMinLight( &skull );
		}
		else {
			// three skulls spinning around the player
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[0] = cos(angle) * 20;
			dir[1] = sin(angle) * 20;
			dir[2] = cos(angle) * 20;
			VectorAdd(torso.origin, dir, skull.origin);

			angles[0] = sin(angle) * 30;
			angles[1] = (angle * 180 / M_PI) + 90;
			if (angles[1] > 360)
				angles[1] -= 360;
			angles[2] = 0;
			AnglesToAxis( angles, skull.axis );

			/*
			dir[2] = 0;
			VectorInverse(dir);
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			*/

			skull.hModel = cgs.media.kamikazeHeadModel;
			CG_AddRefEntityWithMinLight( &skull );
			// flip the trail because this skull is spinning in the other direction
			VectorInverse(skull.axis[1]);
			skull.hModel = cgs.media.kamikazeHeadTrail;
			CG_AddRefEntityWithMinLight( &skull );

			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255 + M_PI;
			if (angle > M_PI * 2)
				angle -= (float)M_PI * 2;
			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = cos(angle) * 20;
			VectorAdd(torso.origin, dir, skull.origin);

			angles[0] = cos(angle - 0.5 * M_PI) * 30;
			angles[1] = 360 - (angle * 180 / M_PI);
			if (angles[1] > 360)
				angles[1] -= 360;
			angles[2] = 0;
			AnglesToAxis( angles, skull.axis );

			/*
			dir[2] = 0;
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			*/

			skull.hModel = cgs.media.kamikazeHeadModel;
			CG_AddRefEntityWithMinLight( &skull );
			skull.hModel = cgs.media.kamikazeHeadTrail;
			CG_AddRefEntityWithMinLight( &skull );

			angle = ((cg.time / 3) & 255) * (M_PI * 2) / 255 + 0.5 * M_PI;
			if (angle > M_PI * 2)
				angle -= (float)M_PI * 2;
			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = 0;
			VectorAdd(torso.origin, dir, skull.origin);
			
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;
			CG_AddRefEntityWithMinLight( &skull );
			skull.hModel = cgs.media.kamikazeHeadTrail;
			CG_AddRefEntityWithMinLight( &skull );
		}
	}

	if ( cent->currentState.powerups & ( 1 << PW_GUARD ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.guardPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		CG_AddRefEntityWithMinLight( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_SCOUT ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.scoutPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		CG_AddRefEntityWithMinLight( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_DOUBLER ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.doublerPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		CG_AddRefEntityWithMinLight( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_AMMOREGEN ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.ammoRegenPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		CG_AddRefEntityWithMinLight( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_INVULNERABILITY ) ) {
		if ( !pi->invulnerabilityStartTime ) {
			pi->invulnerabilityStartTime = cg.time;
		}
		pi->invulnerabilityStopTime = cg.time;
	}
	else {
		pi->invulnerabilityStartTime = 0;
	}
	if ( (cent->currentState.powerups & ( 1 << PW_INVULNERABILITY ) ) ||
		cg.time - pi->invulnerabilityStopTime < 250 ) {

		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.invulnerabilityPowerupModel;
		powerup.customSkin = 0;
		// always draw
		powerup.renderfx &= ~RF_ONLY_MIRROR;
		VectorCopy(cent->lerpOrigin, powerup.origin);

		if ( cg.time - pi->invulnerabilityStartTime < 250 ) {
			c = (float) (cg.time - pi->invulnerabilityStartTime) / 250;
		}
		else if (cg.time - pi->invulnerabilityStopTime < 250 ) {
			c = (float) (250 - (cg.time - pi->invulnerabilityStopTime)) / 250;
		}
		else {
			c = 1;
		}
		VectorSet( powerup.axis[0], c, 0, 0 );
		VectorSet( powerup.axis[1], 0, c, 0 );
		VectorSet( powerup.axis[2], 0, 0, c );
		CG_AddRefEntityWithMinLight( &powerup );
	}

	t = cg.time - pi->medkitUsageTime;
	if ( pi->medkitUsageTime && t < 500 ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.medkitUsageModel;
		powerup.customSkin = 0;
		// always draw
		powerup.renderfx &= ~RF_ONLY_MIRROR;
		VectorClear(angles);
		AnglesToAxis(angles, powerup.axis);
		VectorCopy(cent->lerpOrigin, powerup.origin);
		powerup.origin[2] += -24 + (float) t * 80 / 500;
		if ( t > 400 ) {
			c = (float) (t - 1000) * 0xff / 100;
			powerup.shaderRGBA[0] = 0xff - c;
			powerup.shaderRGBA[1] = 0xff - c;
			powerup.shaderRGBA[2] = 0xff - c;
			powerup.shaderRGBA[3] = 0xff - c;
		}
		else {
			powerup.shaderRGBA[0] = 0xff;
			powerup.shaderRGBA[1] = 0xff;
			powerup.shaderRGBA[2] = 0xff;
			powerup.shaderRGBA[3] = 0xff;
		}
		CG_AddRefEntityWithMinLight( &powerup );
	}
#endif // MISSIONPACK

	//
	// add the head
	//
	head.hModel = pi->headModel;
	if (!head.hModel) {
		return;
	}
	head.customSkin = legs.customSkin;

	VectorCopy( cent->lerpOrigin, head.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &head, &torso, pi->torsoModel, "tag_head");

	head.shadowPlane = shadowPlane;
	head.renderfx = renderfx;

	CG_AddRefEntityWithPowerups( &head, &cent->currentState );

	CG_AddBreathPuffs( cent, &head );

#ifdef MISSIONPACK
	CG_DustTrail(cent);
#endif

	//
	// add the gun / barrel / flash
	//
	CG_AddPlayerWeapon( &torso, NULL, cent, pi->team );

	// add powerups floating behind the player
	CG_PlayerPowerups( cent, &torso );
}


//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
	cent->errorTime = -99999;		// guarantee no error decay added

	CG_ClearLerpFrame( &cgs.playerinfo[ cent->currentState.playerNum ], &cent->pe.legs, cent->currentState.legsAnim );
	CG_ClearLerpFrame( &cgs.playerinfo[ cent->currentState.playerNum ], &cent->pe.torso, cent->currentState.torsoAnim );

	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	VectorCopy( cent->lerpOrigin, cent->rawOrigin );
	VectorCopy( cent->lerpAngles, cent->rawAngles );

	memset( &cent->pe.legs, 0, sizeof( cent->pe.legs ) );
	cent->pe.legs.yawAngle = cent->rawAngles[YAW];
	cent->pe.legs.yawing = qfalse;
	cent->pe.legs.pitchAngle = 0;
	cent->pe.legs.pitching = qfalse;

	memset( &cent->pe.torso, 0, sizeof( cent->pe.torso ) );
	cent->pe.torso.yawAngle = cent->rawAngles[YAW];
	cent->pe.torso.yawing = qfalse;
	cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
	cent->pe.torso.pitching = qfalse;

	if ( cg_debugPosition.integer ) {
		CG_Printf("%i ResetPlayerEntity yaw=%f\n", cent->currentState.number, cent->pe.torso.yawAngle );
	}
}

