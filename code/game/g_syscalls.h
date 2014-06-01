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

#ifndef __G_SYSCALLS_H__
#define __G_SYSCALLS_H__

//
// system traps
// These functions are how game communicates with the server system
//

// Additional shared traps in bg_public.h

void	trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
void	trap_SetNetFields( int entityStateSize, int entityNetworkSize, vmNetField_t *entityStateFields, int numEntityStateFields,
						   int playerStateSize, int playerNetworkSize, vmNetField_t *playerStateFields, int numPlayerStateFields );
void	trap_DropClient( int playerNum, const char *reason );
void	trap_SendServerCommandEx( int connectionNum, int localPlayerNum, const char *text );
void	trap_SetConfigstring( int num, const char *string );
void	trap_GetConfigstring( int num, char *buffer, int bufferSize );
void	trap_SetConfigstringRestrictions( int num, const clientList_t *clientList );
void	trap_GetUserinfo( int num, char *buffer, int bufferSize );
void	trap_SetUserinfo( int num, const char *buffer );
void	trap_GetServerinfo( char *buffer, int bufferSize );
void	trap_GetBrushBounds( int modelindex, vec3_t mins, vec3_t maxs );
void	trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void	trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void	trap_ClipToEntities( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void	trap_ClipToEntitiesCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
int		trap_PointContents( const vec3_t point, int passEntityNum );
qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 );
qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
void	trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
qboolean trap_AreasConnected( int area1, int area2 );
void	trap_LinkEntity( gentity_t *ent );
void	trap_UnlinkEntity( gentity_t *ent );
int		trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
int		trap_BotAllocateClient( void );
void	trap_BotFreeClient( int clientNum );
void	trap_GetUsercmd( int playerNum, usercmd_t *cmd );
qboolean	trap_GetEntityToken( char *buffer, int bufferSize );
void	trap_ClientCommand(int playerNum, const char *command);

int		trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points);
void	trap_DebugPolygonDelete(int id);

qhandle_t trap_R_RegisterModel( const char *name );
int trap_R_ModelBounds( clipHandle_t handle, vec3_t mins, vec3_t maxs, int startFrame, int endFrame, float frac );
int trap_R_LerpTag( orientation_t *tag, clipHandle_t handle, int startFrame, int endFrame,
					   float frac, const char *tagName );
int trap_R_LerpTagFrameModel( orientation_t *tag, clipHandle_t mod, clipHandle_t frameModel, int startFrame, clipHandle_t endFrameModel, int endFrame,
					   float frac, const char *tagName );

int		trap_BotLibSetup( void );
int		trap_BotLibShutdown( void );
int		trap_BotLibVarSet(const char *var_name, char *value);
int		trap_BotLibVarGet(const char *var_name, char *value, int size);
int		trap_BotLibStartFrame(float time);
int		trap_BotLibLoadMap(const char *mapname);
int		trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);
int		trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);

int		trap_BotGetSnapshotEntity( int playerNum, int sequence );
int		trap_BotGetServerCommand(int playerNum, char *message, int size);
void	trap_BotUserCommand(int playerNum, usercmd_t *ucmd);

int		trap_AAS_BBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas);
int		trap_AAS_AreaInfo( int areanum, void /* struct aas_areainfo_s */ *info );

int		trap_AAS_Loaded(void);
int		trap_AAS_Initialized(void);
void	trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs);
float	trap_AAS_Time(void);

int		trap_AAS_PointAreaNum(vec3_t point);
int		trap_AAS_PointReachabilityAreaIndex(vec3_t point);
void	trap_AAS_TraceClientBBox(void /* aas_trace_t */ *trace, vec3_t start, vec3_t end, int presencetype, int passent, int contentmask);
int		trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas);

int		trap_AAS_PointContents(vec3_t point);
int		trap_AAS_NextBSPEntity(int ent);
int		trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size);
int		trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v);
int		trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value);
int		trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value);

int		trap_AAS_AreaReachability(int areanum);
int		trap_AAS_BestReachableArea(vec3_t origin, vec3_t mins, vec3_t maxs, vec3_t goalorigin);
int		trap_AAS_BestReachableFromJumpPadArea(vec3_t origin, vec3_t mins, vec3_t maxs);
int		trap_AAS_NextModelReachability(int num, int modelnum);
float	trap_AAS_AreaGroundFaceArea(int areanum);
int		trap_AAS_AreaCrouch(int areanum);
int		trap_AAS_AreaSwim(int areanum);
int		trap_AAS_AreaLiquid(int areanum);
int		trap_AAS_AreaLava(int areanum);
int		trap_AAS_AreaSlime(int areanum);
int		trap_AAS_AreaGrounded(int areanum);
int		trap_AAS_AreaLadder(int areanum);
int		trap_AAS_AreaJumpPad(int areanum);
int		trap_AAS_AreaDoNotEnter(int areanum);

int		trap_AAS_TravelFlagForType( int traveltype );
int		trap_AAS_AreaContentsTravelFlags( int areanum );
int		trap_AAS_NextAreaReachability( int areanum, int reachnum );
int		trap_AAS_ReachabilityFromNum( int num, void /*struct aas_reachability_s*/ *reach );
int		trap_AAS_RandomGoalArea( int areanum, int travelflags, int contentmask, int *goalareanum, vec3_t goalorigin );
int		trap_AAS_EnableRoutingArea( int areanum, int enable );
unsigned short int trap_AAS_AreaTravelTime(int areanum, vec3_t start, vec3_t end);
int		trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags);
int		trap_AAS_PredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin,
							int goalareanum, int travelflags, int maxareas, int maxtime,
							int stopevent, int stopcontents, int stoptfl, int stopareanum);

int		trap_AAS_AlternativeRouteGoals(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags,
										void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,
										int type);


int		trap_AAS_PredictClientMovement(void /* aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize, int contentmask);
int		trap_AAS_OnGround(vec3_t origin, int presencetype, int passent, int contentmask);
int		trap_AAS_Swimming(vec3_t origin);
void	trap_AAS_JumpReachRunStart(void /* struct aas_reachability_s */ *reach, vec3_t runstart, int contentmask);
int		trap_AAS_AgainstLadder(vec3_t origin);
int		trap_AAS_HorizontalVelocityForJump(float zvel, vec3_t start, vec3_t end, float *velocity);
int		trap_AAS_DropToFloor(vec3_t origin, vec3_t mins, vec3_t maxs, int passent, int contentmask);

#endif

