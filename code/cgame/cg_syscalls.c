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

#include "../qcommon/q_shared.h"
#include "../renderercommon/tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"
#include "cg_syscalls.h"

// ZTM: TODO: move usage of this out of this file?
qboolean CG_AddCustomSurface( const refEntity_t *re );

#ifndef Q3_VM
static intptr_t (QDECL *syscall)( intptr_t arg, ... ) = (intptr_t (QDECL *)( intptr_t, ...))-1;

Q_EXPORT void dllEntry( intptr_t (QDECL  *syscallptr)( intptr_t arg,... ) ) {
	syscall = syscallptr;
}
#endif

int PASSFLOAT( float x ) {
	floatint_t fi;
	fi.f = x;
	return fi.i;
}

void	trap_Print( const char *fmt ) {
	syscall( CG_PRINT, fmt );
}

void	trap_Error(const char *fmt) {
	syscall(CG_ERROR, fmt);
#ifndef Q3_VM
	// shut up GCC warning about returning functions, because we know better
	exit(1);
#endif
}

int		trap_Milliseconds( void ) {
	return syscall( CG_MILLISECONDS ); 
}

void	trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags ) {
	syscall( CG_CVAR_REGISTER, vmCvar, varName, defaultValue, flags );
}

void	trap_Cvar_Update( vmCvar_t *vmCvar ) {
	syscall( CG_CVAR_UPDATE, vmCvar );
}

void	trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall( CG_CVAR_SET, var_name, value );
}

void	trap_Cvar_SetValue( const char *var_name, float value ) {
	syscall( CG_CVAR_SET_VALUE, var_name, PASSFLOAT( value ) );
}

void	trap_Cvar_Reset( const char *name ) {
	syscall( CG_CVAR_RESET, name );
}

float	trap_Cvar_VariableValue( const char *var_name ) {
	floatint_t fi;
	fi.i = syscall( CG_CVAR_VARIABLE_VALUE, var_name );
	return fi.f;
}

int		trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return syscall( CG_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( CG_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( CG_CVAR_LATCHED_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_DefaultVariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( CG_CVAR_DEFAULT_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

void	trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) {
	syscall( CG_CVAR_INFO_STRING_BUFFER, bit, buffer, bufsize );
}

void	trap_Cvar_CheckRange( const char *var_name, float min, float max, qboolean integral ) {
	syscall( CG_CVAR_CHECK_RANGE, var_name, PASSFLOAT(min), PASSFLOAT(max), integral );
}


int		trap_Argc( void ) {
	return syscall( CG_ARGC );
}

void	trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( CG_ARGV, n, buffer, bufferLength );
}

void	trap_Args( char *buffer, int bufferLength ) {
	syscall( CG_ARGS, buffer, bufferLength );
}

void	trap_LiteralArgs( char *buffer, int bufferLength ) {
	syscall( CG_LITERAL_ARGS, buffer, bufferLength );
}

int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall( CG_FS_FOPENFILE, qpath, f, mode );
}

int		trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	return syscall( CG_FS_READ, buffer, len, f );
}

int		trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return syscall( CG_FS_WRITE, buffer, len, f );
}

int		trap_FS_Seek( fileHandle_t f, long offset, int origin ) {
	return syscall( CG_FS_SEEK, f, offset, origin );
}

int		trap_FS_Tell( fileHandle_t f ) {
	return syscall( CG_FS_TELL, f );
}

void	trap_FS_FCloseFile( fileHandle_t f ) {
	syscall( CG_FS_FCLOSEFILE, f );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall( CG_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

int trap_FS_Delete( const char *path ) {
	return syscall( CG_FS_DELETE, path );
}

int trap_FS_Rename( const char *from, const char *to ) {
	return syscall( CG_FS_RENAME, from, to );
}

void	trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	syscall( CG_CMD_EXECUTETEXT, exec_when, text );
}

void	trap_AddCommand( const char *cmdName ) {
	syscall( CG_ADDCOMMAND, cmdName );
}

void	trap_RemoveCommand( const char *cmdName ) {
	syscall( CG_REMOVECOMMAND, cmdName );
}

void	trap_SendClientCommand( const char *s ) {
	syscall( CG_SENDCLIENTCOMMAND, s );
}

void	trap_UpdateScreen( void ) {
	syscall( CG_UPDATESCREEN );
}

void	trap_CM_LoadMap( const char *mapname ) {
	syscall( CG_CM_LOADMAP, mapname );
}

int		trap_CM_NumInlineModels( void ) {
	return syscall( CG_CM_NUMINLINEMODELS );
}

clipHandle_t trap_CM_InlineModel( int index ) {
	return syscall( CG_CM_INLINEMODEL, index );
}

clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int contents ) {
	return syscall( CG_CM_TEMPBOXMODEL, mins, maxs, contents );
}

clipHandle_t trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs, int contents ) {
	return syscall( CG_CM_TEMPCAPSULEMODEL, mins, maxs, contents );
}

int		trap_CM_PointContents( const vec3_t p, clipHandle_t model ) {
	return syscall( CG_CM_POINTCONTENTS, p, model );
}

int		trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles ) {
	return syscall( CG_CM_TRANSFORMEDPOINTCONTENTS, p, model, origin, angles );
}

void	trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	syscall( CG_CM_BOXTRACE, results, start, end, mins, maxs, model, brushmask );
}

void	trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	syscall( CG_CM_CAPSULETRACE, results, start, end, mins, maxs, model, brushmask );
}

void	trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const vec3_t origin, const vec3_t angles ) {
	syscall( CG_CM_TRANSFORMEDBOXTRACE, results, start, end, mins, maxs, model, brushmask, origin, angles );
}

void	trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const vec3_t origin, const vec3_t angles ) {
	syscall( CG_CM_TRANSFORMEDCAPSULETRACE, results, start, end, mins, maxs, model, brushmask, origin, angles );
}

void trap_CM_BiSphereTrace( trace_t *results, const vec3_t start,
             const vec3_t end, float startRad, float endRad,
             clipHandle_t model, int mask ) {
  syscall( CG_CM_BISPHERETRACE, results, start, end,
      PASSFLOAT( startRad ), PASSFLOAT( endRad ), model, mask );
}

void trap_CM_TransformedBiSphereTrace( trace_t *results, const vec3_t start,
             const vec3_t end, float startRad, float endRad,
             clipHandle_t model, int mask,
             const vec3_t origin ) {
  syscall( CG_CM_TRANSFORMEDBISPHERETRACE, results, start, end, PASSFLOAT( startRad ),
      PASSFLOAT( endRad ), model, mask, origin );
}

int		trap_CM_MarkFragments( int numPoints, const vec3_t *points, 
				const vec3_t projection,
				int maxPoints, vec3_t pointBuffer,
				int maxFragments, markFragment_t *fragmentBuffer ) {
	return syscall( CG_CM_MARKFRAGMENTS, numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer );
}

void	trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx ) {
	syscall( CG_S_STARTSOUND, origin, entityNum, entchannel, sfx );
}

void	trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	syscall( CG_S_STARTLOCALSOUND, sfx, channelNum );
}

void	trap_S_StopLoopingSound( int entityNum ) {
	syscall( CG_S_STOPLOOPINGSOUND, entityNum );
}

void	trap_S_ClearLoopingSounds( qboolean killall ) {
	syscall( CG_S_CLEARLOOPINGSOUNDS, killall );
}

void	trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx ) {
	syscall( CG_S_ADDLOOPINGSOUND, entityNum, origin, velocity, sfx );
}

void	trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx ) {
	syscall( CG_S_ADDREALLOOPINGSOUND, entityNum, origin, velocity, sfx );
}

void	trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin ) {
	syscall( CG_S_UPDATEENTITYPOSITION, entityNum, origin );
}

void	trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater, qboolean firstPerson ) {
	syscall( CG_S_RESPATIALIZE, entityNum, origin, axis, inwater, firstPerson );
}

sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed ) {
	return syscall( CG_S_REGISTERSOUND, sample, compressed );
}

int trap_S_SoundDuration( sfxHandle_t handle ) {
	return syscall( CG_S_SOUNDDURATION, handle );
}

void	trap_S_StartBackgroundTrack( const char *intro, const char *loop, float volume, float loopVolume ) {
	syscall( CG_S_STARTBACKGROUNDTRACK, intro, loop, PASSFLOAT( volume ), PASSFLOAT( loopVolume ) );
}

void	trap_S_StopBackgroundTrack( void ) {
	syscall( CG_S_STOPBACKGROUNDTRACK );
}

void trap_S_StartStreamingSound( int stream, int entityNum, const char *filename, float volume ) {
	syscall( CG_S_STARTSTREAMINGSOUND, stream, entityNum, filename, PASSFLOAT( volume ) );
}

void trap_S_StopStreamingSound( int stream ) {
	syscall( CG_S_STOPSTREAMINGSOUND, stream );
}

void trap_S_QueueStreamingSound( int stream, const char *filename, float volume ) {
	syscall( CG_S_QUEUESTREAMINGSOUND, stream, filename, PASSFLOAT( volume ) );
}

int trap_S_GetStreamPlayCount( int stream ) {
	return syscall( CG_S_GETSTREAMPLAYCOUNT, stream );
}

void trap_S_SetStreamVolume( int stream, float volume ) {
	syscall( CG_S_SETSTREAMVOLUME, stream, PASSFLOAT( volume ) );
}

void trap_S_StopAllSounds( void ) {
	syscall( CG_S_STOPALLSOUNDS );
}

void	trap_R_LoadWorldMap( const char *mapname ) {
	syscall( CG_R_LOADWORLDMAP, mapname );
}

qboolean trap_GetEntityToken( int *parseOffset, char *buffer, int bufferSize ) {
	return syscall( CG_GET_ENTITY_TOKEN, parseOffset, buffer, bufferSize );
}

qhandle_t trap_R_RegisterModel( const char *name ) {
	return syscall( CG_R_REGISTERMODEL, name );
}

qhandle_t trap_R_RegisterShaderEx( const char *name, int lightmapIndex, qboolean mipRawImage ) {
	return syscall( CG_R_REGISTERSHADEREX, name, lightmapIndex, mipRawImage );
}

qhandle_t trap_R_RegisterShader( const char *name ) {
	return syscall( CG_R_REGISTERSHADER, name );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return syscall( CG_R_REGISTERSHADERNOMIP, name );
}

void trap_R_RegisterFont(const char *fontName, int pointSize, float borderWidth, qboolean forceAutoHint, fontInfo_t *font) {
	syscall( CG_R_REGISTERFONT, fontName, pointSize, PASSFLOAT( borderWidth ), forceAutoHint, font, sizeof ( fontInfo_t ) );
}

qhandle_t	trap_R_AllocSkinSurface( const char *surface, qhandle_t hShader ) {
	return syscall( CG_R_ALLOCSKINSURFACE, surface, hShader );
}

qhandle_t	trap_R_AddSkinToFrame( int numSurfaces, const qhandle_t *surfaces ) {
	return syscall( CG_R_ADDSKINTOFRAME, numSurfaces, surfaces );
}

void	trap_R_ClearScene( void ) {
	syscall( CG_R_CLEARSCENE );
}

void	trap_R_AddPolyRefEntityToScene( const refEntity_t *re, int numVerts, const polyVert_t *verts, int numPolys ) {
	syscall( CG_R_ADDPOLYREFENTITYTOSCENE, re, sizeof ( refEntity_t ), numVerts, verts, numPolys );
}

void	trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	if ( CG_AddCustomSurface( re ) ) {
		return;
	}

	syscall( CG_R_ADDREFENTITYTOSCENE, re, sizeof ( refEntity_t ) );
}

void	trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int bmodelNum, int sortLevel ) {
	syscall( CG_R_ADDPOLYTOSCENE, hShader, numVerts, verts, bmodelNum, sortLevel );
}

void	trap_R_AddPolysToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys, int bmodelNum, int sortLevel ) {
	syscall( CG_R_ADDPOLYSTOSCENE, hShader, numVerts, verts, numPolys, bmodelNum, sortLevel );
}

void    trap_R_AddPolyBufferToScene( polyBuffer_t* pPolyBuffer ) {
	syscall( CG_R_ADDPOLYBUFFERTOSCENE, pPolyBuffer );
}

void	trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader ) {
	syscall( CG_R_ADDLIGHTTOSCENE, org, PASSFLOAT( radius ), PASSFLOAT( intensity ), PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), hShader );
}

void	trap_R_AddAdditiveLightToScene( const vec3_t org, float radius,float intensity, float r, float g, float b ) {
	syscall( CG_R_ADDADDITIVELIGHTTOSCENE, org, PASSFLOAT( radius ), PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
}

void	trap_R_AddVertexLightToScene( const vec3_t org, float radius,float intensity, float r, float g, float b ) {
	syscall( CG_R_ADDVERTEXLIGHTTOSCENE, org, PASSFLOAT( radius ), PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
}

void	trap_R_AddJuniorLightToScene( const vec3_t org, float radius,float intensity, float r, float g, float b ) {
	syscall( CG_R_ADDJUNIORLIGHTTOSCENE, org, PASSFLOAT( radius ), PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
}

void	trap_R_AddDirectedLightToScene( const vec3_t normal, float intensity, float r, float g, float b ) {
	syscall( CG_R_ADDDIRECTEDLIGHTTOSCENE, normal, PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
}

void	trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible, qhandle_t hShader ) {
	syscall( CG_R_ADDCORONATOSCENE, org, PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), PASSFLOAT( scale ), id, visible, hShader );
}

int		trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir ) {
	return syscall( CG_R_LIGHTFORPOINT, point, ambientLight, directedLight, lightDir );
}

void	trap_R_RenderScene( const refdef_t *fd ) {
	syscall( CG_R_RENDERSCENE, fd, sizeof ( refdef_t ) );
}

void	trap_R_SetColor( const float *rgba ) {
	syscall( CG_R_SETCOLOR, rgba );
}

void  trap_R_SetClipRegion( const float *region ) {
	syscall( CG_R_SETCLIPREGION, region );
}

void	trap_R_DrawStretchPic( float x, float y, float w, float h, 
							   float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	syscall( CG_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), hShader );
}

void	trap_R_DrawRotatedPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, float angle ) {
	syscall( CG_R_DRAWROTATEDPIC, PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader, PASSFLOAT( angle ) );
}

void	trap_R_DrawStretchPicGradient(  float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader,
										const float *gradientColor ) {
	syscall( CG_R_DRAWSTRETCHPIC_GRADIENT, PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader, gradientColor );
}

void	trap_R_Add2dPolys( polyVert_t *verts, int numverts, qhandle_t hShader ) {
	syscall( CG_R_DRAW2DPOLYS, verts, numverts, hShader );
}

int		trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs, int startFrame, int endFrame, float frac ) {
	return syscall( CG_R_MODELBOUNDS, model, mins, maxs, startFrame, endFrame, PASSFLOAT(frac) );
}

int		trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
					   float frac, const char *tagName ) {
	return syscall( CG_R_LERPTAG, tag, mod, startFrame, endFrame, PASSFLOAT(frac), tagName );
}

int		trap_R_LerpTagFrameModel( orientation_t *tag, clipHandle_t mod,
					   clipHandle_t frameModel, int startFrame,
					   clipHandle_t endFrameModel, int endFrame,
					   float frac, const char *tagName,
					   int *tagIndex )
{
	return syscall( CG_R_LERPTAG_FRAMEMODEL, tag, mod, frameModel, startFrame, endFrameModel, endFrame, PASSFLOAT(frac), tagName, tagIndex );
}

int		trap_R_LerpTagTorso( orientation_t *tag, clipHandle_t mod,
					   clipHandle_t frameModel, int startFrame,
					   clipHandle_t endFrameModel, int endFrame,
					   float frac, const char *tagName,
					   int *tagIndex, const vec3_t *torsoAxis,
					   qhandle_t torsoFrameModel, int torsoFrame,
					   qhandle_t oldTorsoFrameModel, int oldTorsoFrame,
					   float torsoFrac )
{
	return syscall( CG_R_LERPTAG_TORSO, tag, mod, frameModel, startFrame, endFrameModel, endFrame, PASSFLOAT(frac), tagName, tagIndex,
										torsoAxis, torsoFrameModel, torsoFrame, oldTorsoFrameModel, oldTorsoFrame, PASSFLOAT(torsoFrac) );
}

void	trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	syscall( CG_R_REMAP_SHADER, oldShader, newShader, timeOffset );
}

qboolean trap_R_inPVS( const vec3_t p1, const vec3_t p2 ) {
	return syscall( CG_R_INPVS, p1, p2 );
}

void trap_R_GetGlobalFog( fogType_t *type, vec3_t color, float *depthForOpaque, float *density, float *farClip ) {
	syscall( CG_R_GET_GLOBAL_FOG, type, color, depthForOpaque, density, farClip );
}

void trap_R_GetViewFog( const vec3_t origin, fogType_t *type, vec3_t color, float *depthForOpaque, float *density, float *farClip, qboolean inwater ) {
	syscall( CG_R_GET_VIEW_FOG, origin, type, color, depthForOpaque, density, farClip, inwater );
}

void		trap_R_SetSurfaceShader( int surfaceNum, const char *name ) {
	syscall( CG_R_SET_SURFACE_SHADER, surfaceNum, name );
}

qhandle_t	trap_R_GetSurfaceShader( int surfaceNum, int lightmapIndex ) {
	return syscall( CG_R_GET_SURFACE_SHADER, surfaceNum, lightmapIndex );
}

qhandle_t	trap_R_GetShaderFromModel( qhandle_t hModel, int surfnum, int lightmapIndex ) {
	return syscall( CG_R_GET_SHADER_FROM_MODEL, hModel, surfnum, lightmapIndex );
}

void		trap_R_GetShaderName( qhandle_t hShader, char *buffer, int bufferSize ) {
	syscall( CG_R_GET_SHADER_NAME, hShader, buffer, bufferSize );
}

void		trap_GetClipboardData( char *buf, int bufsize ) {
	syscall( CG_GETCLIPBOARDDATA, buf, bufsize );
}

void		trap_GetGlconfig( glconfig_t *glconfig ) {
	syscall( CG_GETGLCONFIG, glconfig, sizeof ( glconfig_t )  );
}

int trap_GetVoipTime( int playerNum ) {
	return syscall( CG_GET_VOIP_TIME, playerNum );
}

float trap_GetVoipPower( int playerNum ) {
	floatint_t fi;
	fi.i = syscall( CG_GET_VOIP_POWER, playerNum );
	return fi.f;
}

float trap_GetVoipGain( int playerNum ) {
	floatint_t fi;
	fi.i = syscall( CG_GET_VOIP_GAIN, playerNum );
	return fi.f;
}

qboolean trap_GetVoipMute( int playerNum ) {
	return syscall( CG_GET_VOIP_MUTE_PLAYER, playerNum );
}

qboolean trap_GetVoipMuteAll( void ) {
	return syscall(	CG_GET_VOIP_MUTE_ALL );
}

void		trap_Cmd_AutoComplete( const char *in, char *out, int outSize ) {
	syscall( CG_CMD_AUTOCOMPLETE, in, out, outSize );
}

void		trap_SV_Shutdown( const char *msg ) {
	syscall( CG_SV_SHUTDOWN, msg );
}

void		trap_GetGameState( gameState_t *gamestate ) {
	syscall( CG_GETGAMESTATE, gamestate, sizeof ( gameState_t ) );
}

void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	syscall( CG_GETCURRENTSNAPSHOTNUMBER, snapshotNumber, serverTime );
}

qboolean	trap_GetSnapshot( int snapshotNumber, vmSnapshot_t *snapshot, void *playerStates, void *entities, int maxEntities ) {
	return syscall( CG_GETSNAPSHOT, snapshotNumber, snapshot, sizeof ( vmSnapshot_t ), playerStates, entities, maxEntities );
}

qboolean	trap_GetServerCommand( int serverCommandNumber ) {
	return syscall( CG_GETSERVERCOMMAND, serverCommandNumber );
}

int			trap_GetCurrentCmdNumber( void ) {
	return syscall( CG_GETCURRENTCMDNUMBER );
}

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd, int localPlayerNum ) {
	return syscall( CG_GETUSERCMD, cmdNumber, ucmd, localPlayerNum );
}

void		trap_SetNetFields( int entityStateSize, int entityNetworkSize, vmNetField_t *entityStateFields, int numEntityStateFields,
						int playerStateSize, int playerNetworkSize, vmNetField_t *playerStateFields, int numPlayerStateFields ) {
	syscall( CG_SET_NET_FIELDS, entityStateSize, entityNetworkSize, entityStateFields, numEntityStateFields,
								playerStateSize, playerNetworkSize, playerStateFields, numPlayerStateFields );
}

int			trap_GetDemoState( void ) {
  return syscall( CG_GETDEMOSTATE );
}

int			trap_GetDemoPos( void ) {
  return syscall( CG_GETDEMOPOS );
}

void		trap_GetDemoName( char *buffer, int size ) {
  syscall( CG_GETDEMONAME, buffer, size );
}

int			trap_GetDemoLength( void ) {
  return syscall( CG_GETDEMOLENGTH );
}

int			trap_GetDemoFileInfo( const char *demoName, int *protocol, int *length, char *startTime, char *endTime, int *runTime ) {
  return syscall( CG_GETDEMOFILEINFO, demoName, protocol, length, startTime, endTime, runTime );
}

void trap_GetClientState( uiClientState_t *state ) {
	syscall( CG_GETCLIENTSTATE, state, sizeof ( uiClientState_t ) );
}

int trap_GetConfigString( int index, char* buff, int buffsize ) {
	return syscall( CG_GETCONFIGSTRING, index, buff, buffsize );
}

void trap_SetMapTitle( const char *name ) {
	syscall( CG_SETMAPTITLE, name );
}

void trap_SetViewAngles( int localPlayerNum, const vec3_t angles ) {
	syscall( CG_SETVIEWANGLES, localPlayerNum, angles );
}

void trap_GetViewAngles( int localPlayerNum, const vec3_t angles ) {
	syscall( CG_GETVIEWANGLES, localPlayerNum, angles );
}

int trap_MemoryRemaining( void ) {
	return syscall( CG_MEMORY_REMAINING );
}

qboolean trap_Key_IsDown( int keynum ) {
	return syscall( CG_KEY_ISDOWN, keynum );
}

void trap_Key_ClearStates( void ) {
	syscall( CG_KEY_CLEARSTATES );
}

int trap_Key_GetKey( const char *binding, int startKey ) {
	return syscall( CG_KEY_GETKEY, binding, startKey );
}

void trap_Key_SetRepeat( qboolean repeat ) {
	syscall( CG_KEY_SETREPEAT, repeat );
}

void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
  syscall( CG_KEY_KEYNUMTOSTRINGBUF, keynum, buf, buflen );
}

void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
  syscall( CG_KEY_GETBINDINGBUF, keynum, buf, buflen );
}

void trap_Key_SetBinding( int keynum, const char *binding ) {
  syscall( CG_KEY_SETBINDING, keynum, binding );
}

void trap_Key_SetOverstrikeMode( qboolean state ) {
  syscall( CG_KEY_SETOVERSTRIKEMODE, state );
}

qboolean trap_Key_GetOverstrikeMode( void ) {
  return syscall( CG_KEY_GETOVERSTRIKEMODE );
}

qboolean trap_Key_GetCapsLockMode( void ) {
	return syscall( CG_KEY_GETCAPSLOCKMODE );
}

qboolean trap_Key_GetNumLockMode( void ) {
	return syscall( CG_KEY_GETNUMLOCKMODE );
}

int trap_Mouse_GetState( int localPlayerNum ) {
	return syscall( CG_MOUSE_GETSTATE, localPlayerNum );
}

void trap_Mouse_SetState( int localPlayerNum, int state ) {
	syscall( CG_MOUSE_SETSTATE, localPlayerNum, state );
}

int trap_SetKeyForJoyEvent( int localPlayerNum, const joyevent_t *joyevent, int keynum ) {
	return syscall( CG_SET_KEY_FOR_JOY_EVENT, localPlayerNum, joyevent, keynum );
}

int trap_GetKeyForJoyEvent( int localPlayerNum, const joyevent_t *joyevent ) {
	return syscall( CG_GET_KEY_FOR_JOY_EVENT, localPlayerNum, joyevent );
}

int trap_GetJoyEventForKey( int localPlayerNum, int keynum, int startIndex, joyevent_t *joyevent ) {
	return syscall( CG_GET_JOY_EVENT_FOR_KEY, localPlayerNum, keynum, startIndex, joyevent );
}

void trap_JoyEventToString( const joyevent_t *joyevent, char *buf, int size ) {
	syscall( CG_JOY_EVENT_TO_STRING, joyevent, buf, size );
}

int trap_LAN_GetPingQueueCount( void ) {
	return syscall( CG_LAN_GETPINGQUEUECOUNT );
}

void trap_LAN_ClearPing( int n ) {
	syscall( CG_LAN_CLEARPING, n );
}

void trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	syscall( CG_LAN_GETPING, n, buf, buflen, pingtime );
}

void trap_LAN_GetPingInfo( int n, char *buf, int buflen ) {
	syscall( CG_LAN_GETPINGINFO, n, buf, buflen );
}

int	trap_LAN_GetServerCount( int source ) {
	return syscall( CG_LAN_GETSERVERCOUNT, source );
}

void trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	syscall( CG_LAN_GETSERVERADDRESSSTRING, source, n, buf, buflen );
}

void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	syscall( CG_LAN_GETSERVERINFO, source, n, buf, buflen );
}

int trap_LAN_GetServerPing( int source, int n ) {
	return syscall( CG_LAN_GETSERVERPING, source, n );
}

int trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return syscall( CG_LAN_SERVERSTATUS, serverAddress, serverStatus, maxLen );
}

void trap_LAN_SaveCachedServers( void ) {
	syscall( CG_LAN_SAVECACHEDSERVERS );
}

void trap_LAN_LoadCachedServers( void ) {
	syscall( CG_LAN_LOADCACHEDSERVERS );
}

void trap_LAN_ResetPings(int n) {
	syscall( CG_LAN_RESETPINGS, n );
}

void trap_LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	syscall( CG_LAN_MARKSERVERVISIBLE, source, n, visible );
}

int trap_LAN_ServerIsVisible( int source, int n) {
	return syscall( CG_LAN_SERVERISVISIBLE, source, n );
}

qboolean trap_LAN_UpdateVisiblePings( int source ) {
	return syscall( CG_LAN_UPDATEVISIBLEPINGS, source );
}

int trap_LAN_AddServer(int source, const char *name, const char *addr) {
	return syscall( CG_LAN_ADDSERVER, source, name, addr );
}

void trap_LAN_RemoveServer(int source, const char *addr) {
	syscall( CG_LAN_REMOVESERVER, source, addr );
}

int trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	return syscall( CG_LAN_COMPARESERVERS, source, sortKey, sortDir, s1, s2 );
}

qboolean trap_LAN_ServerIsInFavoriteList( int source, int n  ) {
	return syscall( CG_LAN_SERVERISINFAVORITELIST, source, n );
}

int trap_PC_AddGlobalDefine( const char *define ) {
	return syscall( CG_PC_ADD_GLOBAL_DEFINE, define );
}

int trap_PC_RemoveGlobalDefine( const char *define ) {
	return syscall( CG_PC_REMOVE_GLOBAL_DEFINE, define );
}

void trap_PC_RemoveAllGlobalDefines( void ) {
	syscall( CG_PC_REMOVE_ALL_GLOBAL_DEFINES );
}

int trap_PC_LoadSource( const char *filename, const char *basepath ) {
	return syscall( CG_PC_LOAD_SOURCE, filename, basepath );
}

int trap_PC_FreeSource( int handle ) {
	return syscall( CG_PC_FREE_SOURCE, handle );
}

int trap_PC_AddDefine( int handle, const char *define ) {
	return syscall( CG_PC_ADD_DEFINE, handle, define );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall( CG_PC_READ_TOKEN, handle, pc_token );
}

void trap_PC_UnreadToken( int handle ) {
	syscall( CG_PC_UNREAD_TOKEN, handle );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall( CG_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}

void *trap_HeapMalloc( int size ) {
	return (void *)syscall( CG_HEAP_MALLOC, size );
}

int trap_HeapAvailable( void ) {
	return syscall( CG_HEAP_AVAILABLE );
}

void trap_HeapFree( void *data ) {
	syscall( CG_HEAP_FREE, data );
}

void	trap_Field_CompleteFilename( const char *dir, const char *ext, qboolean stripExt, qboolean allowNonPureFilesOnDisk ) {
	syscall( CG_FIELD_COMPLETEFILENAME, dir, ext, stripExt, allowNonPureFilesOnDisk );
}

void	trap_Field_CompleteCommand( const char *cmd, qboolean doCommands, qboolean doCvars ) {
	syscall( CG_FIELD_COMPLETECOMMAND, cmd, doCommands, doCvars );
}

void	trap_Field_CompleteList( const char *list ) {
	syscall( CG_FIELD_COMPLETELIST, list );
}

int trap_RealTime(qtime_t *qtime) {
	return syscall( CG_REAL_TIME, qtime );
}

void trap_SnapVector( float *v ) {
	syscall( CG_SNAPVECTOR, v );
}

int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits) {
  return syscall(CG_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits);
}

e_status trap_CIN_StopCinematic(int handle) {
  return syscall(CG_CIN_STOPCINEMATIC, handle);
}

e_status trap_CIN_RunCinematic (int handle) {
  return syscall(CG_CIN_RUNCINEMATIC, handle);
}

void trap_CIN_DrawCinematic (int handle) {
  syscall(CG_CIN_DRAWCINEMATIC, handle);
}

void trap_CIN_SetExtents (int handle, int x, int y, int w, int h) {
  syscall(CG_CIN_SETEXTENTS, handle, x, y, w, h);
}

/*
qboolean trap_loadCamera( const char *name ) {
	return syscall( CG_LOADCAMERA, name );
}

void trap_startCamera(int time) {
	syscall(CG_STARTCAMERA, time);
}

qboolean trap_getCameraInfo( int time, vec3_t *origin, vec3_t *angles) {
	return syscall( CG_GETCAMERAINFO, time, origin, angles );
}
*/

