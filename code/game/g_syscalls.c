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

void trap_Cvar_DefaultVariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( G_CVAR_DEFAULT_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
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

qboolean trap_GetEntityToken( int *parseOffset, char *buffer, int bufferSize ) {
	return syscall( G_GET_ENTITY_TOKEN, parseOffset, buffer, bufferSize );
}

int trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points) {
	return syscall( G_DEBUG_POLYGON_CREATE, color, numPoints, points );
}

void trap_DebugPolygonShow(int id, int color, int numPoints, vec3_t *points) {
	syscall( G_DEBUG_POLYGON_SHOW, id, color, numPoints, points );
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


int trap_BotGetSnapshotEntity( int playerNum, int sequence ) {
	return syscall( G_BOT_GET_SNAPSHOT_ENTITY, playerNum, sequence );
}

int trap_BotGetServerCommand(int playerNum, char *command, int size) {
	return syscall( G_BOT_GET_SERVER_COMMAND, playerNum, command, size );
}

void trap_BotUserCommand(int playerNum, usercmd_t *ucmd) {
	syscall( G_BOT_USER_COMMAND, playerNum, ucmd );
}


int trap_PC_AddGlobalDefine( const char *define ) {
	return syscall( G_PC_ADD_GLOBAL_DEFINE, define );
}

int trap_PC_RemoveGlobalDefine( const char *define ) {
	return syscall( G_PC_REMOVE_GLOBAL_DEFINE, define );
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

int trap_PC_AddDefine( int handle, const char *define ) {
	return syscall( G_PC_ADD_DEFINE, handle, define );
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

void *trap_HeapMalloc( int size ) {
	return (void *)syscall( G_HEAP_MALLOC, size );
}

int trap_HeapAvailable( void ) {
	return syscall( G_HEAP_AVAILABLE );
}

void trap_HeapFree( void *data ) {
	syscall( G_HEAP_FREE, data );
}

void trap_Field_CompleteFilename( const char *dir, const char *ext, qboolean stripExt, qboolean allowNonPureFilesOnDisk ) {
	syscall( G_FIELD_COMPLETEFILENAME, dir, ext, stripExt, allowNonPureFilesOnDisk );
}

void trap_Field_CompleteCommand( const char *cmd, qboolean doCommands, qboolean doCvars ) {
	syscall( G_FIELD_COMPLETECOMMAND, cmd, doCommands, doCvars );
}

void	trap_Field_CompleteList( const char *list ) {
	syscall( G_FIELD_COMPLETELIST, list );
}
