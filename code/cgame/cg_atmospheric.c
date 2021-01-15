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

/*
**
**	cg_atmospheric.c
**
**	Add atmospheric effects (e.g. rain, snow etc.) to view.
**
**	Current supported effects are rain and snow.
**
*/

#include "cg_local.h"

#define ATM_NEW // Use PolyBuffer

#define MAX_ATMOSPHERIC_HEIGHT          MAX_MAP_SIZE    // maximum world height
#define MIN_ATMOSPHERIC_HEIGHT          -MAX_MAP_SIZE   // minimum world height

//int getgroundtime, getskytime, rendertime, checkvisibletime, generatetime;
//int n_getgroundtime, n_getskytime, n_rendertime, n_checkvisibletime, n_generatetime;

#define MAX_ATMOSPHERIC_PARTICLES       4000    // maximum # of particles
#define MAX_ATMOSPHERIC_DISTANCE        1000    // maximum distance from refdef origin that particles are visible
#define MAX_ATMOSPHERIC_EFFECTSHADERS   6       // maximum different effectshaders for an atmospheric effect
#define ATMOSPHERIC_DROPDELAY           1000
#define ATMOSPHERIC_CUTHEIGHT           800

#define ATMOSPHERIC_RAIN_SPEED      ( 1.1f * DEFAULT_GRAVITY )
#define ATMOSPHERIC_RAIN_HEIGHT     150

#define ATMOSPHERIC_SNOW_SPEED      ( 0.1f * DEFAULT_GRAVITY )
#define ATMOSPHERIC_SNOW_HEIGHT     3

typedef enum {
	ATM_NONE,
	ATM_RAIN,
	ATM_SNOW
} atmFXType_t;

#ifndef ATM_NEW
/*
** Atmospheric Particles PolyPool
*/
static polyVert_t atmPolyPool[MAX_ATMOSPHERIC_PARTICLES * 3];
static int numParticlesInFrame;
static qhandle_t atmPolyShader;

static void CG_ClearPolyPool( void ) {
	numParticlesInFrame = 0;
	atmPolyShader = 0;
}

static void CG_RenderPolyPool( void ) {
	if ( numParticlesInFrame ) {
		trap_R_AddPolysToScene( atmPolyShader, 3, atmPolyPool, numParticlesInFrame, 0, 0 );
		CG_ClearPolyPool();
	}
}
#endif // ATM_NEW

static void CG_AddPolyToPool( qhandle_t shader, const polyVert_t *verts ) {
#ifndef ATM_NEW
	if ( atmPolyShader && atmPolyShader != shader ) {
		CG_RenderPolyPool();
	}

	if ( numParticlesInFrame == MAX_ATMOSPHERIC_PARTICLES ) {
		CG_RenderPolyPool();
	}

	atmPolyShader = shader;
	memcpy( &atmPolyPool[numParticlesInFrame * 3], verts, 3 * sizeof( polyVert_t ) );
	numParticlesInFrame++;
#else
	int firstIndex;
	int firstVertex;
	int i;

	polyBuffer_t* pPolyBuffer = CG_PB_FindFreePolyBuffer( shader, 3, 3 );
	if ( !pPolyBuffer ) {
		return;
	}

	firstIndex = pPolyBuffer->numIndicies;
	firstVertex = pPolyBuffer->numVerts;

	for ( i = 0; i < 3; i++ ) {
		VectorCopy( verts[i].xyz, pPolyBuffer->xyz[firstVertex + i] );

		pPolyBuffer->st[firstVertex + i][0] = verts[i].st[0];
		pPolyBuffer->st[firstVertex + i][1] = verts[i].st[1];
		pPolyBuffer->color[firstVertex + i][0] = verts[i].modulate[0];
		pPolyBuffer->color[firstVertex + i][1] = verts[i].modulate[1];
		pPolyBuffer->color[firstVertex + i][2] = verts[i].modulate[2];
		pPolyBuffer->color[firstVertex + i][3] = verts[i].modulate[3];

		pPolyBuffer->indicies[firstIndex + i] = firstVertex + i;

	}

	pPolyBuffer->numIndicies += 3;
	pPolyBuffer->numVerts += 3;
#endif // ATM_NEW
}

/*
**	CG_AtmosphericKludge
*/

static qboolean kludgeChecked, kludgeResult;
qboolean CG_AtmosphericKludge(void) {
	// Activate rain for specified kludge maps that don't
	// have it specified for them.

	if ( kludgeChecked ) {
		return( kludgeResult );
	}
	kludgeChecked = qtrue;
	kludgeResult = qfalse;

	/*if( !Q_stricmp( cgs.mapname, "maps/trainyard.bsp" ) )
	{
		//CG_EffectParse( "T=RAIN,B=5 10,C=0.5 2,G=0.5 2,BV=30 100,GV=20 80,W=1 2,D=1000 1000" );
		CG_EffectParse( "T=RAIN,B=5 10,C=0.5,G=0.5 2,BV=50 50,GV=200 200,W=1 2,D=1000" );
		return( kludgeResult = qtrue );
	}*/
/*	if( !Q_stricmp( cgs.mapname, "maps/mp_railgun.bsp" ) )
	{
		//CG_EffectParse( "T=RAIN,B=5 10,C=0.5 2,G=0.5 2,BV=30 100,GV=20 80,W=1 2,D=1000 1000" );
//		CG_EffectParse( "T=SNOW,B=5 10,C=0.5,G=0.3 2,BV=50 50,GV=30 80,W=1 2,D=5000" );

		// snow storm, quite horizontally
		//CG_EffectParse( "T=SNOW,B=20 30,C=0.8,G=0.5 8,BV=100 100,GV=70 150,W=3 5,D=5000" );

		// mild snow storm, quite vertically - likely go for this
		//CG_EffectParse( "T=SNOW,B=5 10,C=0.5,G=0.3 2,BV=20 30,GV=25 40,W=3 5,D=5000" );
		CG_EffectParse( "T=SNOW,B=5 10,C=0.5,G=0.3 2,BV=20 30,GV=25 40,W=3 5,D=2000" );

		// cpu-cheap press event effect
		//CG_EffectParse( "T=SNOW,B=5 10,C=0.5,G=0.3 2,BV=20 30,GV=25 40,W=3 5,D=500" );
//		CG_EffectParse( "T=SNOW,B=5 10,C=0.5,G=0.3 2,BV=20 30,GV=25 40,W=3 5,D=750" );
		return( kludgeResult = qtrue );
	}*/

	/*if( !Q_stricmp( cgs.mapname, "maps/mp_goliath.bsp" ) ) {
		//CG_EffectParse( "T=SNOW,B=5 7,C=0.2,G=0.1 5,BV=15 25,GV=25 40,W=3 5,D=400" );
		CG_EffectParse( "T=SNOW,B=5 7,C=0.2,G=0.1 5,BV=15 25,GV=25 40,W=3 5,H=512,D=2000" );
		return( kludgeResult = qtrue );
	}*/
	/*if( !Q_stricmp( cgs.rawmapname, "sp_bruck_test006" ) ) {
		//T=SNOW,B=5 10,C=0.5,G=0.3 2,BV=20 30,GV=25 40,W=3 5,H=608,D=2000
		CG_EffectParse( "T=SNOW,B=5 10,C=0.5,G=0.3 2,BV=20 30,GV=25 40,W=3 5,H=512,D=2000 4000" );
		//CG_EffectParse( "T=SNOW,B=5 7,C=0.2,G=0.1 5,BV=15 25,GV=25 40,W=3 5,H=512,D=2000" );
		return( kludgeResult = qtrue );
	}*/

	return( kludgeResult = qfalse );
}

typedef enum {
	ACT_NOT,
	ACT_FALLING
} active_t;

typedef struct cg_atmosphericParticle_s {
	vec3_t pos, delta, deltaNormalized, colour;
	float height, weight;
	active_t active;
	int nextDropTime;
	qhandle_t *effectshader;
} cg_atmosphericParticle_t;

typedef struct cg_atmosphericEffect_s {
	cg_atmosphericParticle_t particles[MAX_ATMOSPHERIC_PARTICLES];
	qhandle_t effectshaders[MAX_ATMOSPHERIC_EFFECTSHADERS];
	int lastRainTime, numDrops;
	int gustStartTime, gustEndTime;
	int baseStartTime, baseEndTime;
	int gustMinTime, gustMaxTime;
	int changeMinTime, changeMaxTime;
	int baseMinTime, baseMaxTime;
	float baseWeight, gustWeight;
	int baseDrops, gustDrops;
	int baseHeightOffset;
	int numEffectShaders;
	vec3_t baseVec, gustVec;

	vec3_t viewDir;

	qboolean ( *ParticleCheckVisible )( struct cg_atmosphericEffect_s *atmFx, cg_atmosphericParticle_t *particle );
	qboolean ( *ParticleGenerate )( struct cg_atmosphericEffect_s *atmFx, cg_atmosphericParticle_t *particle, vec3_t currvec, float currweight );
	void ( *ParticleRender )( struct cg_atmosphericEffect_s *atmFx, cg_atmosphericParticle_t *particle );

	int dropsActive, oldDropsActive;
	int dropsRendered, dropsCreated, dropsSkipped;
} cg_atmosphericEffect_t;

static cg_atmosphericEffect_t cg_atmFx[MAX_SPLITVIEW];


static qboolean CG_SetParticleActive( cg_atmosphericParticle_t *particle, active_t active ) {
	particle->active = active;
	return active ? qtrue : qfalse;
}


/*
**	Raindrop management functions
*/

static qboolean CG_RainParticleGenerate( struct cg_atmosphericEffect_s *atmFx, cg_atmosphericParticle_t *particle, vec3_t currvec, float currweight ) {
	// Attempt to 'spot' a raindrop somewhere below a sky texture.

	float angle, distance;
	float groundHeight, skyHeight;
//	int msec = trap_Milliseconds();

//	n_generatetime++;

	angle = random() * 2 * M_PI;
	distance = 20 + MAX_ATMOSPHERIC_DISTANCE * random();

	particle->pos[0] = cg.refdef.vieworg[0] + sin( angle ) * distance;
	particle->pos[1] = cg.refdef.vieworg[1] + cos( angle ) * distance;

	// ydnar: choose a spawn point randomly between sky and ground
	skyHeight = BG_GetSkyHeightAtPoint( particle->pos );
	if ( skyHeight == MAX_ATMOSPHERIC_HEIGHT ) {
		return qfalse;
	}
	groundHeight = BG_GetSkyGroundHeightAtPoint( particle->pos );
	if ( groundHeight >= skyHeight ) {
		return qfalse;
	}
	particle->pos[2] = groundHeight + random() * ( skyHeight - groundHeight );

	// make sure it doesn't fall from too far cause it then will go over our heads ('lower the ceiling')
	if ( atmFx->baseHeightOffset > 0 ) {
		if ( particle->pos[2] - cg.refdef.vieworg[2] > atmFx->baseHeightOffset ) {
			particle->pos[2] = cg.refdef.vieworg[2] + atmFx->baseHeightOffset;

			if ( particle->pos[2] < groundHeight ) {
				return qfalse;
			}
		}
	}

	// ydnar: rain goes in bursts
	{
		float maxActiveDrops;

		// every 10 seconds allow max raindrops
		maxActiveDrops = 0.50 * atmFx->numDrops + 0.001 * atmFx->numDrops * ( 10000 - ( cg.time % 10000 ) );
		if ( atmFx->oldDropsActive > maxActiveDrops ) {
			return qfalse;
		}
	}

	CG_SetParticleActive( particle, ACT_FALLING );
	particle->colour[0] = 0.6 + 0.2 * random() * 0xFF;
	particle->colour[1] = 0.6 + 0.2 * random() * 0xFF;
	particle->colour[2] = 0.6 + 0.2 * random() * 0xFF;
	VectorCopy( currvec, particle->delta );
	particle->delta[2] += crandom() * 100;
	VectorCopy( particle->delta, particle->deltaNormalized );
	VectorNormalizeFast( particle->deltaNormalized );
	particle->height = ATMOSPHERIC_RAIN_HEIGHT + crandom() * 100;
	particle->weight = currweight;

	if ( atmFx->numEffectShaders > 1 ) {
		particle->effectshader = &atmFx->effectshaders[ rand()%atmFx->numEffectShaders ];
	} else {
		particle->effectshader = &atmFx->effectshaders[0];
	}

//	generatetime += trap_Milliseconds() - msec;
	return( qtrue );
}

static qboolean CG_RainParticleCheckVisible( struct cg_atmosphericEffect_s *atmFx, cg_atmosphericParticle_t *particle ) {
	// Check the raindrop is visible and still going, wrapping if necessary.

	float moved;
	vec2_t distance;
//	int msec = trap_Milliseconds();

	if ( !particle || particle->active == ACT_NOT ) {
//		checkvisibletime += trap_Milliseconds() - msec;
		return( qfalse );
	}

	moved = ( cg.time - atmFx->lastRainTime ) * 0.001;  // Units moved since last frame
	VectorMA( particle->pos, moved, particle->delta, particle->pos );
	if ( particle->pos[2] + particle->height < BG_GetSkyGroundHeightAtPoint( particle->pos ) ) {
//		checkvisibletime += trap_Milliseconds() - msec;
		return CG_SetParticleActive( particle, ACT_NOT );
	}

	distance[0] = particle->pos[0] - cg.refdef.vieworg[0];
	distance[1] = particle->pos[1] - cg.refdef.vieworg[1];
	if ( ( distance[0] * distance[0] + distance[1] * distance[1] ) > Square( MAX_ATMOSPHERIC_DISTANCE ) ) {
		// ydnar: just nuke this particle, let it respawn
		return CG_SetParticleActive( particle, ACT_NOT );

		/*
		// Attempt to respot the particle at our other side
		particle->pos[0] -= 1.85f * distance[0];
		particle->pos[1] -= 1.85f * distance[1];

		// Valid spot?
		pointHeight = BG_GetSkyHeightAtPoint( particle->pos );
		if( pointHeight == MAX_ATMOSPHERIC_HEIGHT ) {
//			checkvisibletime += trap_Milliseconds() - msec;
			return CG_SetParticleActive( particle, ACT_NOT );
		}

		pointHeight = BG_GetSkyGroundHeightAtPoint( particle->pos );
		if( pointHeight == MAX_ATMOSPHERIC_HEIGHT || pointHeight >= particle->pos[2] ) {
//			checkvisibletime += trap_Milliseconds() - msec;
			return CG_SetParticleActive( particle, ACT_NOT );
		}
		*/
	}

//	checkvisibletime += trap_Milliseconds() - msec;
	return( qtrue );
}

static void CG_RainParticleRender( struct cg_atmosphericEffect_s *atmFx, cg_atmosphericParticle_t *particle ) {
	// Draw a raindrop

	vec3_t forward, right;
	polyVert_t verts[3];
	vec2_t line;
	float len, dist;
	vec3_t start, finish;
	float groundHeight;
//	int			msec = trap_Milliseconds();

//	n_rendertime++;

	if ( particle->active == ACT_NOT ) {
//		rendertime += trap_Milliseconds() - msec;
		return;
	}

	if ( CG_CullPoint( particle->pos ) ) {
		return;
	}

	VectorCopy( particle->pos, start );

	dist = DistanceSquared( particle->pos, cg.refdef.vieworg );

	// Make sure it doesn't clip through surfaces
	groundHeight = BG_GetSkyGroundHeightAtPoint( start );
	len = particle->height;
	if ( start[2] <= groundHeight ) {
		// Stop snow going through surfaces.
		len = particle->height - groundHeight + start[2];
		VectorMA( start, len - particle->height, particle->deltaNormalized, start );
	}

	if ( len <= 0 ) {
//		rendertime += trap_Milliseconds() - msec;
		return;
	}

	// fade nearby rain particles
	if ( dist < Square( 128.f ) ) {
		dist = .25f + .75f * ( dist / Square( 128.f ) );
	} else {
		dist = 1.0f;
	}

	VectorCopy( particle->deltaNormalized, forward );
	VectorMA( start, -len, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	// dist = 1.0;

	VectorCopy( finish, verts[0].xyz );
	verts[0].st[0] = 0.5f;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = particle->colour[0];
	verts[0].modulate[1] = particle->colour[1];
	verts[0].modulate[2] = particle->colour[2];
	verts[0].modulate[3] = 100 * dist;

	VectorMA( start, -particle->weight, right, verts[1].xyz );
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = particle->colour[0];
	verts[1].modulate[1] = particle->colour[1];
	verts[2].modulate[2] = particle->colour[2];
	verts[1].modulate[3] = 200 * dist;

	VectorMA( start, particle->weight, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = particle->colour[0];
	verts[2].modulate[1] = particle->colour[1];
	verts[2].modulate[2] = particle->colour[2];
	verts[2].modulate[3] = 200 * dist;

	CG_AddPolyToPool( *particle->effectshader, verts );

//	rendertime += trap_Milliseconds() - msec;
}

/*
**	Snow management functions
*/

static qboolean CG_SnowParticleGenerate( struct cg_atmosphericEffect_s *atmFx, cg_atmosphericParticle_t *particle, vec3_t currvec, float currweight ) {
	// Attempt to 'spot' a snowflake somewhere below a sky texture.

	float angle, distance;
	float groundHeight, skyHeight;
//	int msec = trap_Milliseconds();

//	n_generatetime++;

	angle = random() * 2 * M_PI;
	distance = 20 + MAX_ATMOSPHERIC_DISTANCE * random();

	particle->pos[0] = cg.refdef.vieworg[0] + sin( angle ) * distance;
	particle->pos[1] = cg.refdef.vieworg[1] + cos( angle ) * distance;

	// ydnar: choose a spawn point randomly between sky and ground
	skyHeight = BG_GetSkyHeightAtPoint( particle->pos );
	if ( skyHeight == MAX_ATMOSPHERIC_HEIGHT ) {
		return qfalse;
	}
	groundHeight = BG_GetSkyGroundHeightAtPoint( particle->pos );
	if ( groundHeight >= skyHeight ) {
		return qfalse;
	}
	particle->pos[2] = groundHeight + random() * ( skyHeight - groundHeight );

	// make sure it doesn't fall from too far cause it then will go over our heads ('lower the ceiling')
	if ( atmFx->baseHeightOffset > 0 ) {
		if ( particle->pos[2] - cg.refdef.vieworg[2] > atmFx->baseHeightOffset ) {
			particle->pos[2] = cg.refdef.vieworg[2] + atmFx->baseHeightOffset;
			if ( particle->pos[2] < groundHeight ) {
				return qfalse;
			}
		}
	}

	CG_SetParticleActive( particle, ACT_FALLING );
	VectorCopy( currvec, particle->delta );
	particle->delta[2] += crandom() * 25;
	VectorCopy( particle->delta, particle->deltaNormalized );
	VectorNormalizeFast( particle->deltaNormalized );
	particle->height = ATMOSPHERIC_SNOW_HEIGHT + random() * 2;
	particle->weight = particle->height * 0.5f;

	if ( atmFx->numEffectShaders > 1 ) {
		particle->effectshader = &atmFx->effectshaders[ rand()%atmFx->numEffectShaders ];
	} else {
		particle->effectshader = &atmFx->effectshaders[0];
	}

//	generatetime += trap_Milliseconds() - msec;
	return( qtrue );
}

static qboolean CG_SnowParticleCheckVisible( struct cg_atmosphericEffect_s *atmFx, cg_atmosphericParticle_t *particle ) {
	// Check the snowflake is visible and still going, wrapping if necessary.

	float moved;
	vec2_t distance;
//	int msec = trap_Milliseconds();

//	n_checkvisibletime++;

	if ( !particle || particle->active == ACT_NOT ) {
//		checkvisibletime += trap_Milliseconds() - msec;
		return( qfalse );
	}

	moved = ( cg.time - atmFx->lastRainTime ) * 0.001;  // Units moved since last frame
	VectorMA( particle->pos, moved, particle->delta, particle->pos );
	if ( particle->pos[2] < BG_GetSkyGroundHeightAtPoint( particle->pos ) ) {
//		checkvisibletime += trap_Milliseconds() - msec;
		return CG_SetParticleActive( particle, ACT_NOT );
	}

	distance[0] = particle->pos[0] - cg.refdef.vieworg[0];
	distance[1] = particle->pos[1] - cg.refdef.vieworg[1];
	if ( ( distance[0] * distance[0] + distance[1] * distance[1] ) > Square( MAX_ATMOSPHERIC_DISTANCE ) ) {
		// ydnar: just nuke this particle, let it respawn
		return CG_SetParticleActive( particle, ACT_NOT );

		/*
		// Attempt to respot the particle at our other side
		particle->pos[0] -= 1.85f * distance[0];
		particle->pos[1] -= 1.85f * distance[1];

		// ydnar: place particle in random position between ground and sky
		groundHeight = BG_GetSkyGroundHeightAtPoint( particle->pos );
		skyHeight = BG_GetSkyHeightAtPoint( particle->pos );
		if( skyHeight == MAX_ATMOSPHERIC_HEIGHT )
			return CG_SetParticleActive( particle, ACT_NOT );
		particle->pos[ 2 ] = groundHeight + random() * (skyHeight - groundHeight);

		// ydnar: valid spot?
		if( particle->pos[ 2 ] <= groundHeight || particle->pos[ 2 ] >= skyHeight )
			return CG_SetParticleActive( particle, ACT_NOT );
		*/
	}

//	checkvisibletime += trap_Milliseconds() - msec;
	return( qtrue );
}

static void CG_SnowParticleRender( struct cg_atmosphericEffect_s *atmFx, cg_atmosphericParticle_t *particle ) {
	// Draw a snowflake

	vec3_t forward, right;
	polyVert_t verts[3];
	vec2_t line;
	float len, sinTumbling, cosTumbling, particleWidth, dist;
	vec3_t start, finish;
	float groundHeight;
//	int			msec = trap_Milliseconds();

//	n_rendertime++;

	if ( particle->active == ACT_NOT ) {
//		rendertime += trap_Milliseconds() - msec;
		return;
	}

	if ( CG_CullPoint( particle->pos ) ) {
		return;
	}

	VectorCopy( particle->pos, start );

	sinTumbling = sin( particle->pos[2] * 0.03125f * ( 0.5f * particle->weight ) );
	cosTumbling = cos( ( particle->pos[2] + particle->pos[1] ) * 0.03125f * ( 0.5f * particle->weight ) );
	start[0] += 24 * ( 1 - particle->deltaNormalized[2] ) * sinTumbling;
	start[1] += 24 * ( 1 - particle->deltaNormalized[2] ) * cosTumbling;

	// Make sure it doesn't clip through surfaces
	groundHeight = BG_GetSkyGroundHeightAtPoint( start );
	len = particle->height;
	if ( start[2] <= groundHeight ) {
		// Stop snow going through surfaces.
		len = particle->height - groundHeight + start[2];
		VectorMA( start, len - particle->height, particle->deltaNormalized, start );
	}

	if ( len <= 0 ) {
//		rendertime += trap_Milliseconds() - msec;
		return;
	}

	line[0] = particle->pos[0] - cg.refdef.vieworg[0];
	line[1] = particle->pos[1] - cg.refdef.vieworg[1];

	dist = DistanceSquared( particle->pos, cg.refdef.vieworg );
	// dist becomes scale
	if ( dist > Square( 500.f ) ) {
		dist = 1.f + ( ( dist - Square( 500.f ) ) * ( 10.f / Square( 2000.f ) ) );
	} else {
		dist = 1.f;
	}

	len *= dist;

	VectorCopy( particle->deltaNormalized, forward );
	VectorMA( start, -( len /** sinTumbling*/ ), forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	particleWidth = dist * ( /*cosTumbling **/ particle->weight );

	VectorMA( finish, -particleWidth, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA( start, -particleWidth, right, verts[1].xyz );
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA( start, particleWidth, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	CG_AddPolyToPool( *particle->effectshader, verts );

//	rendertime += trap_Milliseconds() - msec;
}

/*
**	Set up gust parameters.
*/

static void CG_EffectGust( cg_atmosphericEffect_t *atmFx ) {
	// Generate random values for the next gust

	int diff;

	atmFx->baseEndTime          = cg.time                 + atmFx->baseMinTime      + ( rand() % ( atmFx->baseMaxTime - atmFx->baseMinTime ) );
	diff                        = atmFx->changeMaxTime    - atmFx->changeMinTime;
	atmFx->gustStartTime        = atmFx->baseEndTime      + atmFx->changeMinTime    + ( diff ? ( rand() % diff ) : 0 );
	diff                        = atmFx->gustMaxTime      - atmFx->gustMinTime;
	atmFx->gustEndTime          = atmFx->gustStartTime    + atmFx->gustMinTime      + ( diff ? ( rand() % diff ) : 0 );
	diff                        = atmFx->changeMaxTime    - atmFx->changeMinTime;
	atmFx->baseStartTime        = atmFx->gustEndTime      + atmFx->changeMinTime    + ( diff ? ( rand() % diff ) : 0 );
}

static qboolean CG_EffectGustCurrent( cg_atmosphericEffect_t *atmFx, vec3_t curr, float *weight, int *num ) {
	// Calculate direction for new drops.

	vec3_t temp;
	float frac;

	if ( cg.time < atmFx->baseEndTime ) {
		VectorCopy( atmFx->baseVec, curr );
		*weight = atmFx->baseWeight;
		*num = atmFx->baseDrops;
	} else {
		VectorSubtract( atmFx->gustVec, atmFx->baseVec, temp );
		if ( cg.time < atmFx->gustStartTime ) {
			frac = ( (float)( cg.time - atmFx->baseEndTime ) ) / ( (float)( atmFx->gustStartTime - atmFx->baseEndTime ) );
			VectorMA( atmFx->baseVec, frac, temp, curr );
			*weight = atmFx->baseWeight + ( atmFx->gustWeight - atmFx->baseWeight ) * frac;
			*num = atmFx->baseDrops + ( (float)( atmFx->gustDrops - atmFx->baseDrops ) ) * frac;
		} else if ( cg.time < atmFx->gustEndTime )    {
			VectorCopy( atmFx->gustVec, curr );
			*weight = atmFx->gustWeight;
			*num = atmFx->gustDrops;
		} else
		{
			frac = 1.0 - ( (float)( cg.time - atmFx->gustEndTime ) ) / ( (float)( atmFx->baseStartTime - atmFx->gustEndTime ) );
			VectorMA( atmFx->baseVec, frac, temp, curr );
			*weight = atmFx->baseWeight + ( atmFx->gustWeight - atmFx->baseWeight ) * frac;
			*num = atmFx->baseDrops + ( (float)( atmFx->gustDrops - atmFx->baseDrops ) ) * frac;
			if ( cg.time >= atmFx->baseStartTime ) {
				return( qtrue );
			}
		}
	}
	return( qfalse );
}

static void CG_EP_ParseFloats( char *floatstr, float *f1, float *f2 ) {
	// Parse the float or floats

	char *middleptr;
	char buff[64];

	Q_strncpyz( buff, floatstr, sizeof( buff ) );
	for ( middleptr = buff; *middleptr && *middleptr != ' '; middleptr++ ) ;
	if ( *middleptr ) {
		*middleptr++ = 0;
		*f1 = atof( floatstr );
		*f2 = atof( middleptr );
	} else {
		*f1 = *f2 = atof( floatstr );
	}
}

static void CG_EP_ParseInts( char *intstr, int *i1, int *i2 ) {
	// Parse the int or ints

	char *middleptr;
	char buff[64];

	Q_strncpyz( buff, intstr, sizeof( buff ) );
	for ( middleptr = buff; *middleptr && *middleptr != ' '; middleptr++ ) ;
	if ( *middleptr ) {
		*middleptr++ = 0;
		*i1 = atof( intstr );
		*i2 = atof( middleptr );
	} else {
		*i1 = *i2 = atof( intstr );
	}
}

void CG_EffectParseForPlayer( int localPlayerNum, const char *effectstr ) {
	// Split the string into it's component parts.

	cg_atmosphericEffect_t *atmFx = &cg_atmFx[localPlayerNum];
	float bmin, bmax, cmin, cmax, gmin, gmax, bdrop, gdrop /*, wsplash, lsplash*/;
	int count, bheight;
	char *startptr, *eqptr, *endptr;
	char workbuff[128];
	atmFXType_t atmFXType = ATM_NONE;
	int i;

	if ( CG_AtmosphericKludge() ) {
		return;
	}

	// Set up some default values
	atmFx->baseVec[0] = atmFx->baseVec[1] = 0;
	atmFx->gustVec[0] = atmFx->gustVec[1] = 100;
	bmin = 5;
	bmax = 10;
	cmin = 1;
	cmax = 1;
	gmin = 0;
	gmax = 2;
	bdrop = gdrop = 300;
	atmFx->baseWeight = 0.7f;
	atmFx->gustWeight = 1.5f;
	bheight = 0;

	// Parse the parameter string
	Q_strncpyz( workbuff, effectstr, sizeof( workbuff ) );
	for ( startptr = workbuff; *startptr; )
	{
		for ( eqptr = startptr; *eqptr && *eqptr != '=' && *eqptr != ','; eqptr++ ) ;
		if ( !*eqptr ) {
			break;          // No more string
		}
		if ( *eqptr == ',' ) {
			startptr = eqptr + 1;   // Bad argument, continue
			continue;
		}
		*eqptr++ = 0;
		for ( endptr = eqptr; *endptr && *endptr != ','; endptr++ ) ;
		if ( *endptr ) {
			*endptr++ = 0;
		}

		if ( atmFXType == ATM_NONE ) {
			if ( Q_stricmp( startptr, "T" ) ) {
				atmFx->numDrops = 0;
				CG_Printf( "Atmospheric effect must start with a type.\n" );
				return;
			}
			if ( !Q_stricmp( eqptr, "RAIN" ) ) {
				atmFXType = ATM_RAIN;
				atmFx->ParticleCheckVisible = &CG_RainParticleCheckVisible;
				atmFx->ParticleGenerate = &CG_RainParticleGenerate;
				atmFx->ParticleRender = &CG_RainParticleRender;

				atmFx->baseVec[2] = atmFx->gustVec[2] = -ATMOSPHERIC_RAIN_SPEED;
			} else if ( !Q_stricmp( eqptr, "SNOW" ) ) {
				atmFXType = ATM_SNOW;
				atmFx->ParticleCheckVisible = &CG_SnowParticleCheckVisible;
				atmFx->ParticleGenerate = &CG_SnowParticleGenerate;
				atmFx->ParticleRender = &CG_SnowParticleRender;

				atmFx->baseVec[2] = atmFx->gustVec[2] = -ATMOSPHERIC_SNOW_SPEED;
			} else {
				atmFx->numDrops = 0;
				CG_Printf( "Only effect type 'rain' and 'snow' are supported.\n" );
				return;
			}
		} else {
			if ( !Q_stricmp( startptr, "B" ) ) {
				CG_EP_ParseFloats( eqptr, &bmin, &bmax );
			} else if ( !Q_stricmp( startptr, "C" ) ) {
				CG_EP_ParseFloats( eqptr, &cmin, &cmax );
			} else if ( !Q_stricmp( startptr, "G" ) ) {
				CG_EP_ParseFloats( eqptr, &gmin, &gmax );
			} else if ( !Q_stricmp( startptr, "BV" ) ) {
				CG_EP_ParseFloats( eqptr, &atmFx->baseVec[0], &atmFx->baseVec[1] );
			} else if ( !Q_stricmp( startptr, "GV" ) ) {
				CG_EP_ParseFloats( eqptr, &atmFx->gustVec[0], &atmFx->gustVec[1] );
			} else if ( !Q_stricmp( startptr, "W" ) ) {
				CG_EP_ParseFloats( eqptr, &atmFx->baseWeight, &atmFx->gustWeight );
			} else if ( !Q_stricmp( startptr, "D" ) ) {
				CG_EP_ParseFloats( eqptr, &bdrop, &gdrop );
			} else if ( !Q_stricmp( startptr, "H" ) ) {
				CG_EP_ParseInts( eqptr, &bheight, &bheight );
			} else { CG_Printf( "Unknown effect key '%s'.\n", startptr );}
		}
		startptr = endptr;
	}

	if ( atmFXType == ATM_NONE || !BG_LoadTraceMap( cgs.mapname, cg.mapcoordsMins, cg.mapcoordsMaxs ) ) {
		// No effects
		atmFx->numDrops = -1;
		return;
	}

	atmFx->baseHeightOffset = bheight;
	if ( atmFx->baseHeightOffset < 0 ) {
		atmFx->baseHeightOffset = 0;
	}
	atmFx->baseMinTime = 1000 * bmin;
	atmFx->baseMaxTime = 1000 * bmax;
	atmFx->changeMinTime = 1000 * cmin;
	atmFx->changeMaxTime = 1000 * cmax;
	atmFx->gustMinTime = 1000 * gmin;
	atmFx->gustMaxTime = 1000 * gmax;
	atmFx->baseDrops = bdrop;
	atmFx->gustDrops = gdrop;

	atmFx->numDrops = ( atmFx->baseDrops > atmFx->gustDrops ) ? atmFx->baseDrops : atmFx->gustDrops;
	if ( atmFx->numDrops > MAX_ATMOSPHERIC_PARTICLES ) {
		atmFx->numDrops = MAX_ATMOSPHERIC_PARTICLES;
	}

	// Load graphics
	if ( atmFXType == ATM_RAIN ) {
		// Rain
		atmFx->effectshaders[0] = trap_R_RegisterShader( "gfx/misc/raindrop" );
		atmFx->numEffectShaders = 1;

		for ( i = 1; i < MAX_ATMOSPHERIC_EFFECTSHADERS; i++ ) {
			atmFx->effectshaders[i] = trap_R_RegisterShader( va("gfx/misc/raindrop%d", i) );
			if ( atmFx->effectshaders[i] ) {
				atmFx->numEffectShaders++;
			} else {
				break;
			}
		}
	} else if ( atmFXType == ATM_SNOW ) {
		// Snow
		atmFx->effectshaders[0] = trap_R_RegisterShader( "gfx/misc/snow" );
		atmFx->numEffectShaders = 1;

		for ( i = 1; i < MAX_ATMOSPHERIC_EFFECTSHADERS; i++ ) {
			atmFx->effectshaders[i] = trap_R_RegisterShader( va("gfx/misc/snow%d", i) );
			if ( atmFx->effectshaders[i] ) {
				atmFx->numEffectShaders++;
			} else {
				break;
			}
		}
	} else {
		// This really should never happen
		atmFx->numEffectShaders = 0;
	}

	if ( !atmFx->effectshaders[0] ) {
		atmFx->effectshaders[0] = -1;
		atmFx->numEffectShaders = 0;
	}

	// Initialise atmospheric effect to prevent all particles falling at the start
	for ( count = 0; count < atmFx->numDrops; count++ )
		atmFx->particles[count].nextDropTime = ATMOSPHERIC_DROPDELAY + ( rand() % ATMOSPHERIC_DROPDELAY );

	CG_EffectGust( atmFx );
}

void CG_EffectParse( const char *effectstr ) {
	int i;

	for ( i = 0; i < MAX_SPLITVIEW; i++ ) {
		CG_EffectParseForPlayer( i, effectstr );
	}
}

/*
** Main render loop
*/

void CG_AddAtmosphericEffects( void ) {
	// Add atmospheric effects (e.g. rain, snow etc.) to view

	cg_atmosphericEffect_t *atmFx = &cg_atmFx[cg.cur_localPlayerNum];
	int curr, max, currnum;
	cg_atmosphericParticle_t *particle;
	vec3_t currvec;
	float currweight;

	if ( atmFx->numDrops <= 0 || atmFx->numEffectShaders == 0 || cg_atmosphericEffects.value <= 0 ) {
		return;
	}

#ifndef ATM_NEW
	CG_ClearPolyPool();
#endif // ATM_NEW

	max = cg_atmosphericEffects.value < 1 ? cg_atmosphericEffects.value * atmFx->numDrops : atmFx->numDrops;
	if ( CG_EffectGustCurrent( atmFx, currvec, &currweight, &currnum ) ) {
		CG_EffectGust( atmFx );            // Recalculate gust parameters
	}
	// ydnar: allow parametric management of drop count for swelling/waning precip
	atmFx->oldDropsActive = atmFx->dropsActive;
	atmFx->dropsActive = 0;

	atmFx->dropsRendered = atmFx->dropsCreated = atmFx->dropsSkipped = 0;

//	getgroundtime = getskytime = rendertime = checkvisibletime = generatetime = 0;
//	n_getgroundtime = n_getskytime = n_rendertime = n_checkvisibletime = n_generatetime = 0;

	VectorSet( atmFx->viewDir, cg.refdef.viewaxis[0][0], cg.refdef.viewaxis[0][1], 0.f );

	for ( curr = 0; curr < max; curr++ )
	{
		particle = &atmFx->particles[curr];
		//%	if( !CG_SnowParticleCheckVisible( atmFx, particle ) )
		if ( !atmFx->ParticleCheckVisible( atmFx, particle ) ) {
			// Effect has terminated / fallen from screen view
			/*
			if( !particle->nextDropTime )
			{
				// Stop rain being synchronized
				particle->nextDropTime = cg.time + rand() % ATMOSPHERIC_DROPDELAY;
			}
			if( currnum < curr || particle->nextDropTime > cg.time )
			{
				atmFx->dropsRendered++;
				continue;
			} */
			//%	if( !CG_SnowParticleGenerate( atmFx, particle, currvec, currweight ) )
			if ( !atmFx->ParticleGenerate( atmFx, particle, currvec, currweight ) ) {
				// Ensure it doesn't attempt to generate every frame, to prevent
				// 'clumping' when there's only a small sky area available.
				particle->nextDropTime = cg.time + ATMOSPHERIC_DROPDELAY;
				continue;
			} else {
				atmFx->dropsCreated++;
			}
		}

		//%	CG_RainParticleRender( particle );
		atmFx->ParticleRender( atmFx, particle );
		atmFx->dropsActive++;
	}

#ifndef ATM_NEW
	CG_RenderPolyPool();
#endif

	atmFx->lastRainTime = cg.time;

//	CG_Printf( "Active: %d Generated: %d Rendered: %d Skipped: %d\n", atmFx->dropsActive, atmFx->dropsCreated, atmFx->dropsRendered, atmFx->dropsSkipped );
//	CG_Printf( "gg: %i gs: %i rt: %i cv: %i ge: %i\n", getgroundtime, getskytime, rendertime, checkvisibletime, generatetime );
//	CG_Printf( "\\-> %i \\-> %i \\-> %i \\-> %i \\-> %i\n", n_getgroundtime, n_getskytime, n_rendertime, n_checkvisibletime, n_generatetime );
}
