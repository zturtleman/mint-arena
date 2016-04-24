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
#include "g_local.h"

#ifndef Q3_VM
static intptr_t (QDECL *syscall)( intptr_t arg, ... ) = (intptr_t (QDECL *)( intptr_t, ...))-1;

Q_EXPORT void dllEntry( intptr_t (QDECL *syscallptr)( intptr_t arg,... ) ) {
	syscall = syscallptr;
}
#endif

int PASSFLOAT( float x ) {
	floatint_t fi;
	fi.f = x;
	return fi.i;
}

void	trap_Print( const char *text ) {
	syscall( G_PRINT, text );
}

void trap_Error( const char *text )
{
	syscall( G_ERROR, text );
#ifndef Q3_VM
	// shut up GCC warning about returning functions, because we know better
	exit(1);
#endif
}

int		trap_Milliseconds( void ) {
	return syscall( G_MILLISECONDS ); 
}
int		trap_Argc( void ) {
	return syscall( G_ARGC );
}

void	trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( G_ARGV, n, buffer, bufferLength );
}

void	trap_Args( char *buffer, int bufferLength ) {
	syscall( G_ARGS, buffer, bufferLength );
}

void	trap_LiteralArgs( char *buffer, int bufferLength ) {
	syscall( G_LITERAL_ARGS, buffer, bufferLength );
}

int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall( G_FS_FOPEN_FILE, qpath, f, mode );
}

int		trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	return syscall( G_FS_READ, buffer, len, f );
}

int		trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return syscall( G_FS_WRITE, buffer, len, f );
}

int trap_FS_Seek( fileHandle_t f, long offset, int origin ) {
	return syscall( G_FS_SEEK, f, offset, origin );
}

int trap_FS_Tell( fileHandle_t f ) {
	return syscall( G_FS_TELL, f );
}

void	trap_FS_FCloseFile( fileHandle_t f ) {
	syscall( G_FS_FCLOSE_FILE, f );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall( G_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

int trap_FS_Delete( const char *path ) {
	return syscall( G_FS_DELETE, path );
}

int trap_FS_Rename( const char *from, const char *to ) {
	return syscall( G_FS_RENAME, from, to );
}

void	trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	syscall( G_CMD_EXECUTETEXT, exec_when, text );
}

void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	syscall( G_CVAR_REGISTER, cvar, var_name, value, flags );
}

void	trap_Cvar_Update( vmCvar_t *cvar ) {
	syscall( G_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall( G_CVAR_SET, var_name, value );
}

void trap_Cvar_SetValue( const char *var_name, float value ) {
	syscall( G_CVAR_SET_VALUE, var_name, PASSFLOAT( value ) );
}

float trap_Cvar_VariableValue( const char *var_name ) {
	floatint_t fi;
	fi.i = syscall( G_CVAR_VARIABLE_VALUE, var_name );
	return fi.f;
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return syscall( G_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( G_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( G_CVAR_LATCHED_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) {
	syscall( G_CVAR_INFO_STRING_BUFFER, bit, buffer, bufsize );
}

void	trap_Cvar_CheckRange( const char *var_name, float min, float max, qboolean integral ) {
	syscall( G_CVAR_CHECK_RANGE, var_name, PASSFLOAT(min), PASSFLOAT(max), integral );
}


void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						 playerState_t *players, int sizeofGamePlayer ) {
	syscall( G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, players, sizeofGamePlayer );
}

void trap_SetNetFields( int entityStateSize, int entityNetworkSize, vmNetField_t *entityStateFields, int numEntityStateFields,
						int playerStateSize, int playerNetworkSize, vmNetField_t *playerStateFields, int numPlayerStateFields ) {
	syscall( G_SET_NET_FIELDS,  entityStateSize, entityNetworkSize, entityStateFields, numEntityStateFields,
								playerStateSize, playerNetworkSize, playerStateFields, numPlayerStateFields );
}

void trap_DropPlayer( int playerNum, const char *reason ) {
	syscall( G_DROP_PLAYER, playerNum, reason );
}

void trap_SendServerCommandEx( int clientNum, int localPlayerNum, const char *text ) {
	syscall( G_SEND_SERVER_COMMAND, clientNum, localPlayerNum, text );
}

void trap_SetConfigstring( int num, const char *string ) {
	syscall( G_SET_CONFIGSTRING, num, string );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	syscall( G_GET_CONFIGSTRING, num, buffer, bufferSize );
}

void trap_SetConfigstringRestrictions( int num, const clientList_t *clientList ) {
	syscall( G_SET_CONFIGSTRING_RESTRICTIONS, num, clientList );
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	syscall( G_GET_USERINFO, num, buffer, bufferSize );
}

void trap_SetUserinfo( int num, const char *buffer ) {
	syscall( G_SET_USERINFO, num, buffer );
}

void trap_GetServerinfo( char *buffer, int bufferSize ) {
	syscall( G_GET_SERVERINFO, buffer, bufferSize );
}

void trap_GetBrushBounds( int modelindex, vec3_t mins, vec3_t maxs ) {
	syscall( G_GET_BRUSH_BOUNDS, modelindex, mins, maxs );
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_TRACE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_TRACECAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_ClipToEntities( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_CLIPTOENTITIES, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_ClipToEntitiesCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_CLIPTOENTITIESCAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

int trap_PointContents( const vec3_t point, int passEntityNum ) {
	return syscall( G_POINT_CONTENTS, point, passEntityNum );
}


qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 ) {
	return syscall( G_IN_PVS, p1, p2 );
}

qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	return syscall( G_IN_PVS_IGNORE_PORTALS, p1, p2 );
}

void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	syscall( G_ADJUST_AREA_PORTAL_STATE, ent, open );
}

qboolean trap_AreasConnected( int area1, int area2 ) {
	return syscall( G_AREAS_CONNECTED, area1, area2 );
}

void trap_LinkEntity( gentity_t *ent ) {
	syscall( G_LINKENTITY, ent );
}

void trap_UnlinkEntity( gentity_t *ent ) {
	syscall( G_UNLINKENTITY, ent );
}

int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) {
	return syscall( G_ENTITIES_IN_BOX, mins, maxs, list, maxcount );
}

qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall( G_ENTITY_CONTACT, mins, maxs, ent );
}

qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall( G_ENTITY_CONTACTCAPSULE, mins, maxs, ent );
}

int trap_BotAllocateClient( void ) {
	return syscall( G_BOT_ALLOCATE_CLIENT );
}

void trap_BotFreeClient( int playerNum ) {
	syscall( G_BOT_FREE_CLIENT, playerNum );
}

void trap_GetUsercmd( int playerNum, usercmd_t *cmd ) {
	syscall( G_GET_USERCMD, playerNum, cmd );
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return syscall( G_GET_ENTITY_TOKEN, buffer, bufferSize );
}

int trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points) {
	return syscall( G_DEBUG_POLYGON_CREATE, color, numPoints, points );
}

void trap_DebugPolygonDelete(int id) {
	syscall( G_DEBUG_POLYGON_DELETE, id );
}

int trap_RealTime( qtime_t *qtime ) {
	return syscall( G_REAL_TIME, qtime );
}

void trap_SnapVector( float *v ) {
	syscall( G_SNAPVECTOR, v );
}

void trap_AddCommand( const char *cmdName ) {
	syscall( G_ADDCOMMAND, cmdName );
}

void trap_RemoveCommand( const char *cmdName ) {
	syscall( G_REMOVECOMMAND, cmdName );
}

qhandle_t trap_R_RegisterModel( const char *name ) {
	return syscall( G_R_REGISTERMODEL, name );
}

int trap_R_LerpTag( orientation_t *tag, clipHandle_t handle, int startFrame, int endFrame,
					   float frac, const char *tagName ) {
	return syscall( G_R_LERPTAG, tag, handle, startFrame, endFrame, PASSFLOAT(frac), tagName );
}

int		trap_R_LerpTagFrameModel( orientation_t *tag, clipHandle_t mod,
					   clipHandle_t frameModel, int startFrame,
					   clipHandle_t endFrameModel, int endFrame,
					   float frac, const char *tagName,
					   int *tagIndex )
{
	return syscall( G_R_LERPTAG_FRAMEMODEL, tag, mod, frameModel, startFrame, endFrameModel, endFrame, PASSFLOAT(frac), tagName, tagIndex );
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
	return syscall( G_R_LERPTAG_TORSO, tag, mod, frameModel, startFrame, endFrameModel, endFrame, PASSFLOAT(frac), tagName, tagIndex,
										torsoAxis, torsoFrameModel, torsoFrame, oldTorsoFrameModel, oldTorsoFrame, PASSFLOAT(torsoFrac) );
}

int trap_R_ModelBounds( clipHandle_t handle, vec3_t mins, vec3_t maxs, int startFrame, int endFrame, float frac ) {
	return syscall( G_R_MODELBOUNDS, handle, mins, maxs, startFrame, endFrame, PASSFLOAT(frac) );
}

void trap_ClientCommand(int playerNum, const char *command) {
	syscall( G_CLIENT_COMMAND, playerNum, command );
}

#include "../botlib/botlib.h"
extern vmCvar_t bot_developer;

botlib_export_t *botlib_export;
qboolean bot_enable = qtrue;

/*
==================
BotImport_Print
==================
*/
static __attribute__ ((format (printf, 2, 3))) void QDECL BotImport_Print(int type, char *fmt, ...)
{
	char str[2048];
	va_list ap;

	va_start(ap, fmt);
	Q_vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	switch(type) {
		case PRT_DEVELOPER: {
			if (bot_developer.integer) {
				Com_Printf("%s", str);
			}
			break;
		}
		case PRT_MESSAGE: {
			Com_Printf("%s", str);
			break;
		}
		case PRT_WARNING: {
			Com_Printf(S_COLOR_YELLOW "Warning: %s", str);
			break;
		}
		case PRT_ERROR: {
			Com_Printf(S_COLOR_RED "Error: %s", str);
			break;
		}
		case PRT_FATAL: {
			Com_Printf(S_COLOR_RED "Fatal: %s", str);
			break;
		}
		case PRT_EXIT: {
			Com_Error(ERR_DROP, S_COLOR_RED "Exit: %s", str);
			break;
		}
		default: {
			Com_Printf("unknown print type\n");
			break;
		}
	}
}

/*
==================
BotImport_Trace
==================
*/
static void BotImport_Trace(bsp_trace_t *bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passent, int contentmask) {
	trap_Trace(bsptrace, start, mins, maxs, end, passent, contentmask);
}

/*
==================
BotImport_EntityTrace
==================
*/
static void BotImport_EntityTrace(bsp_trace_t *bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int entnum, int contentmask) {
	trap_ClipToEntities(bsptrace, start, mins, maxs, end, entnum, contentmask);
}


/*
==================
BotImport_PointContents
==================
*/
static int BotImport_PointContents(vec3_t point) {
	return trap_PointContents(point, -1);
}

/*
==================
BotImport_inPVS
==================
*/
static int BotImport_inPVS(vec3_t p1, vec3_t p2) {
	return trap_InPVS(p1, p2);
}

/*
==================
BotImport_BSPModelMinsMaxsOrigin
==================
*/
static void BotImport_BSPModelMinsMaxsOrigin(int modelnum, vec3_t angles, vec3_t outmins, vec3_t outmaxs, vec3_t origin) {
#if 0 // ZTM: FIXME: BOTLIBPORT
	clipHandle_t h;
	vec3_t mins, maxs;
	float max;
	int	i;

	h = CM_InlineModel(modelnum);
	CM_ModelBounds(h, mins, maxs);
	//if the model is rotated
	if ((angles[0] || angles[1] || angles[2])) {
		// expand for rotation

		max = RadiusFromBounds(mins, maxs);
		for (i = 0; i < 3; i++) {
			mins[i] = -max;
			maxs[i] = max;
		}
	}
	if (outmins) VectorCopy(mins, outmins);
	if (outmaxs) VectorCopy(maxs, outmaxs);
	if (origin) VectorClear(origin);
#endif
}

/*
==================
BotImport_GetMemory
==================
*/
static void *BotImport_GetMemory(int size) {
#if 1
	return trap_Alloc( size, NULL );
#else
	void *ptr;

	ptr = Z_TagMalloc( size, TAG_BOTLIB );
	return ptr;
#endif
}

/*
==================
BotImport_FreeMemory
==================
*/
static void BotImport_FreeMemory(void *ptr) {
#if 0
	Z_Free(ptr);
#endif
}

/*
=================
BotImport_HunkAlloc
=================
*/
static void *BotImport_HunkAlloc( int size ) {
#if 1
	return trap_Alloc( size, NULL );
#else
	if( Hunk_CheckMark() ) {
		Com_Error( ERR_DROP, "SV_Bot_HunkAlloc: Alloc with marks already set" );
	}
	return Hunk_Alloc( size, h_high );
#endif
}

/*
==================
BotImport_DebugPolygonCreate
==================
*/
int BotImport_DebugPolygonCreate(int color, int numPoints, vec3_t *points) {
#if 1
	return 0;
#else
	bot_debugpoly_t *poly;
	int i;

	if (!debugpolygons)
		return 0;

	for (i = 1; i < bot_maxdebugpolys; i++) 	{
		if (!debugpolygons[i].inuse)
			break;
	}
	if (i >= bot_maxdebugpolys)
		return 0;
	poly = &debugpolygons[i];
	poly->inuse = qtrue;
	poly->color = color;
	poly->numPoints = numPoints;
	Com_Memcpy(poly->points, points, numPoints * sizeof(vec3_t));
	//
	return i;
#endif
}

/*
==================
BotImport_DebugPolygonShow
==================
*/
static void BotImport_DebugPolygonShow(int id, int color, int numPoints, vec3_t *points) {
#if 0
	bot_debugpoly_t *poly;

	if (!debugpolygons) return;
	poly = &debugpolygons[id];
	poly->inuse = qtrue;
	poly->color = color;
	poly->numPoints = numPoints;
	Com_Memcpy(poly->points, points, numPoints * sizeof(vec3_t));
#endif
}

/*
==================
BotImport_DebugPolygonDelete
==================
*/
void BotImport_DebugPolygonDelete(int id)
{
#if 0
	if (!debugpolygons) return;
	debugpolygons[id].inuse = qfalse;
#endif
}

/*
==================
BotImport_DebugLineCreate
==================
*/
static int BotImport_DebugLineCreate(void) {
	vec3_t points[1];
	return BotImport_DebugPolygonCreate(0, 0, points);
}

/*
==================
BotImport_DebugLineDelete
==================
*/
static void BotImport_DebugLineDelete(int line) {
#if 0
	BotImport_DebugPolygonDelete(line);
#endif
}

/*
==================
BotImport_DebugLineShow
==================
*/
static void BotImport_DebugLineShow(int line, vec3_t start, vec3_t end, int color) {
#if 0
	vec3_t points[4], dir, cross, up = {0, 0, 1};
	float dot;

	VectorCopy(start, points[0]);
	VectorCopy(start, points[1]);
	//points[1][2] -= 2;
	VectorCopy(end, points[2]);
	//points[2][2] -= 2;
	VectorCopy(end, points[3]);


	VectorSubtract(end, start, dir);
	VectorNormalize(dir);
	dot = DotProduct(dir, up);
	if (dot > 0.99 || dot < -0.99) VectorSet(cross, 1, 0, 0);
	else CrossProduct(dir, up, cross);

	VectorNormalize(cross);

	VectorMA(points[0], 2, cross, points[0]);
	VectorMA(points[1], -2, cross, points[1]);
	VectorMA(points[2], -2, cross, points[2]);
	VectorMA(points[3], 2, cross, points[3]);

	BotImport_DebugPolygonShow(line, color, 4, points);
#endif
}

int Z_AvailableMemory( void ) {
	return 0;
}

/*
==================
SV_BotInitBotLib
==================
*/
void SV_BotInitBotLib(void) {
	botlib_import_t	botlib_import;

/*
	if (debugpolygons) Z_Free(debugpolygons);
	bot_maxdebugpolys = Cvar_VariableIntegerValue("bot_maxdebugpolys");
	debugpolygons = Z_Malloc(sizeof(bot_debugpoly_t) * bot_maxdebugpolys);
*/

	botlib_import.Print = BotImport_Print;
	botlib_import.Trace = BotImport_Trace;
	botlib_import.EntityTrace = BotImport_EntityTrace;
	botlib_import.PointContents = BotImport_PointContents;
	botlib_import.inPVS = BotImport_inPVS;
	botlib_import.GetEntityToken = trap_GetEntityToken;
	botlib_import.BSPModelMinsMaxsOrigin = BotImport_BSPModelMinsMaxsOrigin;
	botlib_import.BotClientCommand = trap_ClientCommand; // SV_ForceClientCommand;

	//memory management
	botlib_import.GetMemory = BotImport_GetMemory;
	botlib_import.FreeMemory = BotImport_FreeMemory;
	botlib_import.AvailableMemory = Z_AvailableMemory;
	botlib_import.HunkAlloc = BotImport_HunkAlloc;

	// file system access
	botlib_import.FS_FOpenFile = trap_FS_FOpenFile;
	botlib_import.FS_Read = trap_FS_Read;
	botlib_import.FS_Write = trap_FS_Write;
	botlib_import.FS_FCloseFile = trap_FS_FCloseFile;
	botlib_import.FS_Seek = trap_FS_Seek;

	//debug lines
	botlib_import.DebugLineCreate = BotImport_DebugLineCreate;
	botlib_import.DebugLineDelete = BotImport_DebugLineDelete;
	botlib_import.DebugLineShow = BotImport_DebugLineShow;

	//debug polygons
	botlib_import.DebugPolygonCreate = BotImport_DebugPolygonCreate;
	botlib_import.DebugPolygonDelete = BotImport_DebugPolygonDelete;

	botlib_export = (botlib_export_t *)GetBotLibAPI( BOTLIB_API_VERSION, &botlib_import );
#ifndef Q3_VM
	assert(botlib_export); 	// somehow we end up with a zero import.
#endif
}

/*
===============
SV_BotLibSetup
===============
*/
int SV_BotLibSetup( void ) {
	char basedir[MAX_STRING_CHARS];
	char homedir[MAX_STRING_CHARS];
	char gamedir[MAX_STRING_CHARS];

	if (!bot_enable) {
		return 0;
	}

	if ( !botlib_export ) {
		Com_Printf( S_COLOR_RED "Error: SV_BotLibSetup without SV_BotInitBotLib\n" );
		return -1;
	}

	trap_Cvar_VariableStringBuffer( "fs_basepath", basedir, sizeof( basedir ) );
	trap_Cvar_VariableStringBuffer( "fs_homepath", homedir, sizeof( homedir ) );
	trap_Cvar_VariableStringBuffer( "fs_game", gamedir, sizeof( gamedir ) );

	botlib_export->BotLibVarSet( "basedir", basedir );
	botlib_export->BotLibVarSet( "homedir", homedir );
	botlib_export->BotLibVarSet( "gamedir", gamedir );

	return botlib_export->BotLibSetup();
}

/*
===============
SV_ShutdownBotLib

Called when either the entire server is being killed, or
it is changing to a different game directory.
===============
*/
int SV_BotLibShutdown( void ) {

	if ( !botlib_export ) {
		return -1;
	}

	return botlib_export->BotLibShutdown();
}

// BotLib traps start here
int trap_BotLibSetup( void ) {
	return SV_BotLibSetup();
}

int trap_BotLibShutdown( void ) {
	return SV_BotLibShutdown();
}

int trap_BotLibVarSet(const char *var_name, char *value) {
	return botlib_export->BotLibVarSet( var_name, value );
}

int trap_BotLibVarGet(const char *var_name, char *value, int size) {
	return botlib_export->BotLibVarGet( var_name, value, size );
}

int trap_BotLibStartFrame(float time) {
	return botlib_export->BotLibStartFrame( time );
}

int trap_BotLibLoadMap(const char *mapname) {
	return botlib_export->BotLibLoadMap( mapname );
}

int trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue) {
	return botlib_export->BotLibUpdateEntity( ent, bue );
}

int trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3) {
	return botlib_export->Test( parm0, parm1, parm2, parm3 );
}

int trap_BotGetSnapshotEntity( int playerNum, int sequence ) {
	return syscall( BOTLIB_GET_SNAPSHOT_ENTITY, playerNum, sequence );
}

int trap_BotGetServerCommand(int playerNum, char *message, int size) {
	return syscall( BOTLIB_GET_CONSOLE_MESSAGE, playerNum, message, size );
}

void trap_BotUserCommand(int playerNum, usercmd_t *ucmd) {
	syscall( BOTLIB_USER_COMMAND, playerNum, ucmd );
}

int trap_AAS_Loaded(void) {
	return botlib_export->aas.AAS_Loaded();
}

int trap_AAS_Initialized(void) {
	return botlib_export->aas.AAS_Initialized();
}

void trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs) {
	botlib_export->aas.AAS_PresenceTypeBoundingBox( presencetype, mins, maxs );
}

float trap_AAS_Time(void) {
	return botlib_export->aas.AAS_Time();
}

int trap_AAS_PointAreaNum(vec3_t point) {
	return botlib_export->aas.AAS_PointAreaNum( point );
}

int trap_AAS_PointReachabilityAreaIndex(vec3_t point) {
	return botlib_export->aas.AAS_PointReachabilityAreaIndex( point );
}

void trap_AAS_TracePlayerBBox(void /* aas_trace_t */ *trace, vec3_t start, vec3_t end, int presencetype, int passent, int contentmask) {
	botlib_export->aas.AAS_TracePlayerBBox( trace, start, end, presencetype, passent, contentmask );
}

int trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas) {
	return botlib_export->aas.AAS_TraceAreas( start, end, areas, points, maxareas );
}

int trap_AAS_BBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas) {
	return botlib_export->aas.AAS_BBoxAreas( absmins, absmaxs, areas, maxareas );
}

int trap_AAS_AreaInfo( int areanum, void /* struct aas_areainfo_s */ *info ) {
	return botlib_export->aas.AAS_AreaInfo( areanum, info );
}

int trap_AAS_PointContents(vec3_t point) {
	return botlib_export->aas.AAS_PointContents( point );
}

int trap_AAS_NextBSPEntity(int ent) {
	return botlib_export->aas.AAS_NextBSPEntity( ent );
}

int trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size) {
	return botlib_export->aas.AAS_ValueForBSPEpairKey( ent, key, value, size );
}

int trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v) {
	return botlib_export->aas.AAS_VectorForBSPEpairKey( ent, key, v );
}

int trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value) {
	return botlib_export->aas.AAS_FloatForBSPEpairKey( ent, key, value );
}

int trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value) {
	return botlib_export->aas.AAS_IntForBSPEpairKey( ent, key, value );
}



int trap_AAS_AreaReachability(int areanum) {
	return botlib_export->aas.AAS_AreaReachability( areanum );
}

int trap_AAS_BestReachableArea(vec3_t origin, vec3_t mins, vec3_t maxs, vec3_t goalorigin) {
	return botlib_export->aas.AAS_BestReachableArea( origin, mins, maxs, goalorigin );
}

int trap_AAS_BestReachableFromJumpPadArea(vec3_t origin, vec3_t mins, vec3_t maxs) {
	return botlib_export->aas.AAS_BestReachableFromJumpPadArea( origin, mins, maxs );
}

int trap_AAS_NextModelReachability(int num, int modelnum) {
	return botlib_export->aas.AAS_NextModelReachability( num, modelnum );
}

float trap_AAS_AreaGroundFaceArea(int areanum) {
	return botlib_export->aas.AAS_AreaGroundFaceArea( areanum );
}

int trap_AAS_AreaCrouch(int areanum) {
	return botlib_export->aas.AAS_AreaCrouch( areanum );
}

int trap_AAS_AreaSwim(int areanum) {
	return botlib_export->aas.AAS_AreaSwim( areanum );
}

int trap_AAS_AreaLiquid(int areanum) {
	return botlib_export->aas.AAS_AreaLiquid( areanum );
}

int trap_AAS_AreaLava(int areanum) {
	return botlib_export->aas.AAS_AreaLava( areanum );
}

int trap_AAS_AreaSlime(int areanum) {
	return botlib_export->aas.AAS_AreaSlime( areanum );
}

int trap_AAS_AreaGrounded(int areanum) {
	return botlib_export->aas.AAS_AreaGrounded( areanum );
}

int trap_AAS_AreaLadder(int areanum) {
	return botlib_export->aas.AAS_AreaLadder( areanum );
}

int trap_AAS_AreaJumpPad(int areanum) {
	return botlib_export->aas.AAS_AreaJumpPad( areanum );
}

int trap_AAS_AreaDoNotEnter(int areanum) {
	return botlib_export->aas.AAS_AreaDoNotEnter( areanum );
}


int trap_AAS_TravelFlagForType( int traveltype ) {
	return botlib_export->aas.AAS_TravelFlagForType( traveltype );
}

int trap_AAS_AreaContentsTravelFlags( int areanum ) {
	return botlib_export->aas.AAS_AreaContentsTravelFlags( areanum );
}

int trap_AAS_NextAreaReachability( int areanum, int reachnum ) {
	return botlib_export->aas.AAS_NextAreaReachability( areanum, reachnum );
}

int trap_AAS_ReachabilityFromNum( int num, void /*struct aas_reachability_s*/ *reach ) {
	botlib_export->aas.AAS_ReachabilityFromNum( num, reach );
	return 0;
}

int trap_AAS_RandomGoalArea( int areanum, int travelflags, int contentmask, int *goalareanum, vec3_t goalorigin ) {
	return botlib_export->aas.AAS_RandomGoalArea( areanum, travelflags, contentmask, goalareanum, goalorigin );
}

int trap_AAS_EnableRoutingArea( int areanum, int enable ) {
	return botlib_export->aas.AAS_EnableRoutingArea( areanum, enable );
}

unsigned short int trap_AAS_AreaTravelTime(int areanum, vec3_t start, vec3_t end) {
	return botlib_export->aas.AAS_AreaTravelTime( areanum, start, end );
}

int trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags) {
	return botlib_export->aas.AAS_AreaTravelTimeToGoalArea( areanum, origin, goalareanum, travelflags );
}

int trap_AAS_PredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin,
							int goalareanum, int travelflags, int maxareas, int maxtime,
							int stopevent, int stopcontents, int stoptfl, int stopareanum) {
	return botlib_export->aas.AAS_PredictRoute( route, areanum, origin, goalareanum, travelflags, maxareas, maxtime, stopevent, stopcontents, stoptfl, stopareanum );
}

int trap_AAS_AlternativeRouteGoals(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags,
										void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,
										int type) {
	return botlib_export->aas.AAS_AlternativeRouteGoals( start, startareanum, goal, goalareanum, travelflags, altroutegoals, maxaltroutegoals, type );
}


int trap_AAS_PredictPlayerMovement(void /* struct aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize, int contentmask) {
	return botlib_export->aas.AAS_PredictPlayerMovement( move, entnum, origin, presencetype, onground, velocity, cmdmove, cmdframes, maxframes, frametime, stopevent, stopareanum, visualize, contentmask );
}

int trap_AAS_OnGround(vec3_t origin, int presencetype, int passent, int contentmask) {
	return botlib_export->aas.AAS_OnGround( origin, presencetype, passent, contentmask );
}

int trap_AAS_Swimming(vec3_t origin) {
	return botlib_export->aas.AAS_Swimming( origin );
}

void trap_AAS_JumpReachRunStart(void /* struct aas_reachability_s */ *reach, vec3_t runstart, int contentmask) {
	botlib_export->aas.AAS_JumpReachRunStart( reach, runstart, contentmask );
}

int trap_AAS_AgainstLadder(vec3_t origin) {
	return botlib_export->aas.AAS_AgainstLadder( origin );
}

int trap_AAS_HorizontalVelocityForJump(float zvel, vec3_t start, vec3_t end, float *velocity) {
	return botlib_export->aas.AAS_HorizontalVelocityForJump( zvel, start, end, velocity );
}
int trap_AAS_DropToFloor(vec3_t origin, vec3_t mins, vec3_t maxs, int passent, int contentmask) {
	return botlib_export->aas.AAS_DropToFloor( origin, mins, maxs, passent, contentmask );
}


int trap_PC_AddGlobalDefine(char *string) {
	return syscall( G_PC_ADD_GLOBAL_DEFINE, string );
}

void trap_PC_RemoveAllGlobalDefines( void ) {
	syscall( G_PC_REMOVE_ALL_GLOBAL_DEFINES );
}

int trap_PC_LoadSource( const char *filename, const char *basepath ) {
	return syscall( G_PC_LOAD_SOURCE, filename, basepath );
}

int trap_PC_FreeSource( int handle ) {
	return syscall( G_PC_FREE_SOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall( G_PC_READ_TOKEN, handle, pc_token );
}

void trap_PC_UnreadToken( int handle ) {
	syscall( G_PC_UNREAD_TOKEN, handle );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall( G_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}

void *trap_Alloc( int size, const char *tag ) {
	return (void *)syscall( G_ALLOC, size, tag );
}
