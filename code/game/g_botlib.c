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
// g_botlib.c

#include "g_local.h"
#include "../botlib/botlib.h"
#include "ai_goal.h"
#include "ai_main.h"

botlib_export_t *botlib_export;

/*
==================
BotImport_BSPModelMinsMaxsOrigin
==================
*/
static void BotImport_BSPModelMinsMaxsOrigin( int modelnum, vec3_t angles, vec3_t outmins, vec3_t outmaxs, vec3_t origin ) {
	vec3_t mins, maxs;
	float max;
	int	i;

	trap_GetBrushBounds( modelnum, mins, maxs );

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
}

/*
==================
BotImport_GetMemory
==================
*/
static void *BotImport_GetMemory( int size ) {
	return trap_HeapMalloc( size );
}

/*
=================
BotImport_AvailableMemory
=================
*/
static int BotImport_AvailableMemory( void ) {
	return trap_HeapAvailable();
}

/*
==================
BotImport_FreeMemory
==================
*/
static void BotImport_FreeMemory( void *ptr ) {
	trap_HeapFree( ptr );
}

/*
=================
BotImport_HunkAlloc
=================
*/
static void *BotImport_HunkAlloc( int size ) {
	return trap_HeapMalloc( size );
}

/*
==================
G_BotInitBotLib
==================
*/
void G_BotInitBotLib(void) {
	botlib_import_t	botlib_import;

	botlib_import.MilliSeconds = trap_Milliseconds;
	botlib_import.Print = BotAI_Print;
	botlib_import.Trace = BotAI_Trace;
	botlib_import.EntityTrace = BotAI_EntityTrace;
	botlib_import.PointContents = BotAI_PointContents;
	botlib_import.inPVS = BotAI_InPVS;
	botlib_import.GetEntityToken = trap_GetEntityToken;
	botlib_import.BSPModelMinsMaxsOrigin = BotImport_BSPModelMinsMaxsOrigin;
	botlib_import.BotClientCommand = trap_ClientCommand;

	//memory management
	botlib_import.GetMemory = BotImport_GetMemory;
	botlib_import.FreeMemory = BotImport_FreeMemory;
	botlib_import.AvailableMemory = BotImport_AvailableMemory;
	botlib_import.HunkAlloc = BotImport_HunkAlloc;

	// file system access
	botlib_import.FS_FOpenFile = trap_FS_FOpenFile;
	botlib_import.FS_Read = trap_FS_Read;
	botlib_import.FS_Write = trap_FS_Write;
	botlib_import.FS_FCloseFile = trap_FS_FCloseFile;
	botlib_import.FS_Seek = trap_FS_Seek;

	//debug lines
	botlib_import.DebugLineCreate = BotAI_DebugLineCreate;
	botlib_import.DebugLineDelete = BotAI_DebugLineDelete;
	botlib_import.DebugLineShow = BotAI_DebugLineShow;

	//debug polygons
	botlib_import.DebugPolygonCreate = trap_DebugPolygonCreate;
	botlib_import.DebugPolygonDelete = trap_DebugPolygonDelete;

	botlib_export = (botlib_export_t *)GetBotLibAPI( BOTLIB_API_VERSION, &botlib_import );
	assert(botlib_export); 	// somehow we end up with a zero import.
}

/*
===============
G_BotLibSetup
===============
*/
static int G_BotLibSetup( void ) {
	char basedir[MAX_STRING_CHARS];
	char homedir[MAX_STRING_CHARS];
	char gamedir[MAX_STRING_CHARS];

	if ( !trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		return 0;
	}

	if ( !botlib_export ) {
		Com_Printf( S_COLOR_RED "Error: G_BotLibSetup without G_BotInitBotLib\n" );
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
G_BotLibShutdown

Called when map is restarted or changed
===============
*/
static int G_BotLibShutdown( void ) {

	if ( !botlib_export ) {
		return -1;
	}

	return botlib_export->BotLibShutdown();
}

// BotLib traps start here
int trap_BotLibSetup( void ) {
	return G_BotLibSetup();
}

int trap_BotLibShutdown( void ) {
	return G_BotLibShutdown();
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


void trap_AAS_ClearShownDebugLines( void ) {
	botlib_export->aas.AAS_ClearShownDebugLines();
}
void trap_AAS_ClearShownPolygons( void ) {
	botlib_export->aas.AAS_ClearShownPolygons();
}
void trap_AAS_DebugLine( vec3_t start, vec3_t end, int color ) {
	botlib_export->aas.AAS_DebugLine( start, end, color );
}
void trap_AAS_PermanentLine( vec3_t start, vec3_t end, int color ) {
	botlib_export->aas.AAS_PermanentLine( start, end, color );
}
void trap_AAS_DrawPermanentCross( vec3_t origin, float size, int color ) {
	botlib_export->aas.AAS_DrawPermanentCross( origin, size, color );
}
void trap_AAS_DrawPlaneCross( vec3_t point, vec3_t normal, float dist, int type, int color ) {
	botlib_export->aas.AAS_DrawPlaneCross( point, normal, dist, type, color );
}
void trap_AAS_ShowBoundingBox( vec3_t origin, vec3_t mins, vec3_t maxs ) {
	botlib_export->aas.AAS_ShowBoundingBox( origin, mins, maxs );
}
void trap_AAS_ShowFace( int facenum ) {
	botlib_export->aas.AAS_ShowFace( facenum );
}
void trap_AAS_ShowArea( int areanum, int groundfacesonly ) {
	botlib_export->aas.AAS_ShowArea( areanum, groundfacesonly );
}
void trap_AAS_ShowAreaPolygons( int areanum, int color, int groundfacesonly ) {
	botlib_export->aas.AAS_ShowAreaPolygons( areanum, color, groundfacesonly );
}
void trap_AAS_DrawCross( vec3_t origin, float size, int color ) {
	botlib_export->aas.AAS_DrawCross( origin, size, color );
}
void trap_AAS_PrintTravelType( int traveltype ) {
	botlib_export->aas.AAS_PrintTravelType( traveltype );
}
void trap_AAS_DrawArrow( vec3_t start, vec3_t end, int linecolor, int arrowcolor ) {
	botlib_export->aas.AAS_DrawArrow( start, end, linecolor, arrowcolor );
}
void trap_AAS_ShowReachability( void /*struct aas_reachability_s*/ *reach, int contentmask ) {
	botlib_export->aas.AAS_ShowReachability( reach, contentmask );
}
void trap_AAS_ShowReachableAreas( int areanum, int contentmask ) {
	botlib_export->aas.AAS_ShowReachableAreas( areanum, contentmask );
}
void trap_AAS_FloodAreas( vec3_t origin ) {
	botlib_export->aas.AAS_FloodAreas( origin );
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
