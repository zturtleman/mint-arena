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
// bsp_q3test106.c -- Q3Test 1.05 BSP Level Loading

#include "q_shared.h"
#include "qcommon.h"
#include "bsp.h"

#define BSP_IDENT	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
		// little-endian "IBSP"

#define BSP_VERSION			44

typedef struct {
	int		fileofs, filelen;
} lump_t;

							  // 1.06 -> 1.05
#define	LUMP_ENTITIES		0 // 0 -> 0		// CORRECT
		// does not have a shaders lump
#define	LUMP_PLANES			1 // 2 -> 1		// CORRECT
#define	LUMP_NODES			2 // 3 -> 2		// CORRECT
#define	LUMP_LEAFS			3 // 4 -> 3		// CORRECT
#define	LUMP_LEAFSURFACES	4 // 5 -> 4		// CORRECT
#define	LUMP_LEAFBRUSHES	5 // 6 -> 5		// CORRECT
#define	LUMP_MODELS			6 // 7 -> 6		// CORRECT?
#define	LUMP_BRUSHES		7 // 8 -> 7		// CORRECT
#define	LUMP_BRUSHSIDES		8 // 9 -> 8		// CORRECT

#define	LUMP_LIGHTMAPS		9 // 14 -> 9	// UNVERIFIED
#define	LUMP_DRAWINDEXES	10 // 11 -> 10	// UNVERIFIED
#define	LUMP_DRAWVERTS		11 // 10 -> 11	// looks ok
#define	LUMP_SURFACES		12 // 14 -> 12	// check more
#define	LUMP_FOGS			13	// 12 -> 13
							// 14 ???
#define	HEADER_LUMPS		15

#define	LUMP_LIGHTGRID		-1 // 15 -> gone?
#define	LUMP_SHADERS		-2 // 1 -> gone? shader names are in LUMP_SURFACES. Not sure where surfaceFlags and contentFlags are stored.
#define	LUMP_VISIBILITY		-3 // 16 -> gone?

typedef struct {
	int			ident;
	int			version;

	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef struct {
	float		mins[3], maxs[3];
	int			unknownZeros[2]; // ZTM: These are always 0
	int			firstSurface, numSurfaces;
	int			firstBrush, numBrushes;
} realDmodel_t;

typedef struct {
	//char		shader[MAX_QPATH];
	int			drawSurfNum; // ZTM: now I'm really guessing at crap
	int			surfaceFlags;
	int			contentFlags;
} realDshader_t;

// planes x^1 is allways the opposite of plane x

typedef struct {
	float		normal[3];
	float		dist;
	int			unknown; // ZTM: this might be 'type' like in q2 bsp -- I haven't tried to check
} realDplane_t;

typedef struct {
	int			planeNum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	int			mins[3];		// for frustom culling
	int			maxs[3];
} realDnode_t;

typedef struct {
	int			cluster;			// -1 = opaque cluster (do I still store these?)
	int			area;

	int			mins[3];			// for frustum culling
	int			maxs[3];

	int			firstLeafSurface;
	int			numLeafSurfaces;

	int			firstLeafBrush;
	int			numLeafBrushes;
} realDleaf_t;

typedef struct {
	int			planeNum;			// positive plane side faces out of the leaf
	int			shaderNum;
} realDbrushside_t;

typedef struct {
	int			firstSide;
	int			numSides;
	int			shaderNum;		// the shader that determines the contents flags
} realDbrush_t;

typedef struct {
	char		shader[MAX_QPATH];
	int			brushNum;
	//int			visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
} realDfog_t;

typedef struct {
	vec3_t		xyz;
	float		st[2];
	float		lightmap[2];
	vec3_t		normal;
	byte		color[4];
} realDrawVert_t;

// When adding a new BSP format make sure the surfaceType variables mean the same thing as Q3 or remap them on load!
#if 0
typedef enum {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE,
	MST_FOLIAGE
} mapSurfaceType_t;
#endif

typedef struct {
	//int			shaderNum;
	char		shader[MAX_QPATH];
	int			fogNum;
	int			surfaceType;	// ZTM: FIXME: This is wrong. Is it shaderNum? -- everything else seems to be correct.

	int			firstVert;
	int			numVerts; // ydnar: num verts + foliage origins (for cleaner lighting code in q3map)

	int			firstIndex;
	int			numIndexes;

#if 1
	int			patchWidth; // ydnar: num foliage instances
	int			patchHeight; // ydnar: num foliage mesh verts
#endif

	int			lightmapNum;
	int			lightmapX, lightmapY;
	int			lightmapWidth, lightmapHeight;

	vec3_t		lightmapOrigin;
	vec3_t		lightmapVecs[3];	// for patches, [0] and [1] are lodbounds -- ZTM: This comment does not appear to be true for q3test 1.05

#if 0
	int			patchWidth; // ydnar: num foliage instances
	int			patchHeight; // ydnar: num foliage mesh verts
#endif
} realDsurface_t;

#define VIS_HEADER 8

#define LIGHTING_GRIDSIZE_X 64
#define LIGHTING_GRIDSIZE_Y 64
#define LIGHTING_GRIDSIZE_Z 128

#define	SUBDIVIDE_DISTANCE	16	//4	// never more than this units away from curve

/****************************************************
*/

static int GetLumpElements( dheader_t *header, int lump, int size ) {
	int i;

	if ( 0 && size > 1 && ( lump < 0 || ( header->lumps[ lump ].filelen % size ) ) ) {
		if ( lump < 0 )
			Com_Printf( "Logical lump %d (element size %d) matches physical lumps...\n", lump, size );
		else
			Com_Printf( "Logical lump %d (element size %d) size %d matches physical lumps...\n", lump, size, header->lumps[ lump ].filelen );

		for ( i = 0; i < HEADER_LUMPS; i++ ) {
			if ( ( header->lumps[ i ].filelen % size ) == 0 ) {
				Com_Printf( "   lump %d, size %d\n", i, header->lumps[ i ].filelen );
			}
		}
	}

	if ( lump < 0 )
		return 0;

	/* check for odd size */
	if ( header->lumps[ lump ].filelen % size ) {
		Com_Printf( "GetLumpElements: odd lump size (%d) in lump %d\n", header->lumps[ lump ].filelen, lump );
		return 0;
	}

	//Com_Printf( "GetLumpElements: lump %d has %d elements\n", lump, header->lumps[ lump ].filelen / size );

	/* return element count */
	return header->lumps[ lump ].filelen / size;
}

static void CopyLump( dheader_t *header, int lump, const void *src, void *dest, int size, qboolean swap ) {
	int length;

	length = GetLumpElements( header, lump, size ) * size;

	/* handle erroneous cases */
	if ( length <= 0 ) {
		return;
	}

	if ( swap ) {
		BSP_SwapBlock( dest, (int *)((byte*) src + header->lumps[lump].fileofs), length );
	} else {
		Com_Memcpy( dest, (byte*) src + header->lumps[lump].fileofs, length );
	}
}

static void *GetLump( dheader_t *header, const void *src, int lump ) {
	return (void*)( (byte*) src + header->lumps[ lump ].fileofs );
}

#define MAX_LUMPS 64
static void BSP_Unknown( const char *name, const void *data, int length ) {
	const byte *data_p;
	int		headerSize;
	lump_t	lumps[MAX_LUMPS];
	int		numLumps = 0;
	int		bestAsciiLump = 0, bestAsciiDist = 99999;
	int		i, j, k;

	//if ( !com_developer->integer )
	//	return;

	// all BSP formats are assumed to have ident and version
	data_p = (const byte*)data;
	data_p += 4; // ident
	data_p += 4; // version
	//data_p += 4; // fakk/alice/ef2 have a checksum.

	headerSize = (int)(data_p - (const byte*)data);

	for ( i = 0; i < MAX_LUMPS; i++ ) {
#if 0 // NOTE: call of duty as flipped length and offset
		lumps[i].filelen = LittleLong( *(int*)data_p ); data_p += 4;
		lumps[i].fileofs = LittleLong( *(int*)data_p ); data_p += 4;
#else
		lumps[i].fileofs = LittleLong( *(int*)data_p ); data_p += 4;
		lumps[i].filelen = LittleLong( *(int*)data_p ); data_p += 4;
#endif

		// if lump starts in header, has negative size, goes beyond end of data, or off+len causes int overflow
		if ( lumps[i].fileofs < headerSize || lumps[i].filelen < 0 || lumps[i].fileofs + lumps[i].filelen > length
			|| lumps[i].fileofs + lumps[i].filelen < lumps[i].fileofs ) {
			// BAD LUMP
			Com_Printf("DEBUG: Bad Lump %d: off: %d, len: %d\n", i, lumps[i].fileofs, lumps[i].filelen );
			data_p -= 8;
			break;
		}

		// if this lump over lap any perious lumps, it's invalid
		for ( j = 0; j < i; j++ ) {
			if ( ( lumps[i].fileofs > lumps[j].fileofs && lumps[i].fileofs < lumps[j].fileofs + lumps[j].filelen )
				|| ( lumps[j].fileofs > lumps[i].fileofs && lumps[j].fileofs < lumps[i].fileofs + lumps[i].filelen ) ) {
				Com_Printf("DEBUG: Bad Lump %d: off: %d, len: %d (over laps lump %d)\n", i, lumps[i].fileofs, lumps[i].filelen, j );
				break;
			}
		}

		if ( j != i )
			break;

		if ( i > 0 && lumps[i].fileofs < lumps[i-1].fileofs+lumps[i-1].filelen ) {
			// not an error, might be useful to know?
			Com_Printf("DEBUG: Lump %d before lump %d\n", i, i-1 );
		}
		Com_Printf("DEBUG: Lump %d: off: %d, len: %d\n", i, lumps[i].fileofs, lumps[i].filelen );
		numLumps++;
	}

	Com_Printf("DEBUG: %s appears to have %d lumps\n", name, numLumps );

	// find entity lump
	for ( i = 0; i < HEADER_LUMPS; i++ ) {
		char *lumpdata = (void*)( (byte*) data + lumps[ i ].fileofs ); // GetLump( &header, data, i );

		if ( lumps[ i ].filelen <= 0 )
			continue;

		k = 0;
		for ( j = 0; j < lumps[ i ].filelen; j++ ) {
			if ( isprint( lumpdata[j] ) ) {
				k++;
			}
		}

		if ( k > lumps[ i ].filelen * 0.75f && lumps[ i ].filelen - k < bestAsciiDist ) {
			bestAsciiLump = i;
			bestAsciiDist = lumps[ i ].filelen - k;
		}

		Com_Printf("DEBUG: Lump %d is %d / %d ASCII\n", i, k, lumps[ i ].filelen );
	}

	Com_Printf("DEBUG: Best Entities Guess: Lump %d is %d / %d ASCII\n", bestAsciiLump, lumps[ bestAsciiLump ].filelen - bestAsciiDist, lumps[ bestAsciiLump ].filelen );
}

bspFile_t *BSP_LoadQ3Test105( const bspFormat_t *format, const char *name, const void *data, int length ) {
	int				i, j, k;
	dheader_t		header;
	bspFile_t		*bsp;

	BSP_SwapBlock( (int *) &header, (int *)data, sizeof ( dheader_t ) );

	if ( header.ident != format->ident || header.version != format->version ) {
		return NULL;
	}

	BSP_Unknown( name, data, length );

	bsp = malloc( sizeof ( bspFile_t ) );
	Com_Memset( bsp, 0, sizeof ( bspFile_t ) );

	// ...
	bsp->checksum = LittleLong (Com_BlockChecksum (data, length));
	bsp->defaultLightGridSize[0] = LIGHTING_GRIDSIZE_X;
	bsp->defaultLightGridSize[1] = LIGHTING_GRIDSIZE_Y;
	bsp->defaultLightGridSize[2] = LIGHTING_GRIDSIZE_Z;


	//
	// count and alloc
	//
	bsp->entityStringLength = GetLumpElements( &header, LUMP_ENTITIES, 1 );
	bsp->entityString = malloc( bsp->entityStringLength );

	bsp->numShaders = GetLumpElements( &header, LUMP_SHADERS, sizeof ( realDshader_t ) );
	bsp->shaders = malloc( bsp->numShaders * sizeof ( *bsp->shaders ) );

	bsp->numPlanes = GetLumpElements( &header, LUMP_PLANES, sizeof ( realDplane_t ) );
	bsp->planes = malloc( bsp->numPlanes * sizeof ( *bsp->planes ) );

	bsp->numNodes = GetLumpElements( &header, LUMP_NODES, sizeof ( realDnode_t ) );
	bsp->nodes = malloc( bsp->numNodes * sizeof ( *bsp->nodes ) );

	bsp->numLeafs = GetLumpElements( &header, LUMP_LEAFS, sizeof ( realDleaf_t ) );
	bsp->leafs = malloc( bsp->numLeafs * sizeof ( *bsp->leafs ) );

	bsp->numLeafSurfaces = GetLumpElements( &header, LUMP_LEAFSURFACES, sizeof ( int ) );
	bsp->leafSurfaces = malloc( bsp->numLeafSurfaces * sizeof ( *bsp->leafSurfaces ) );

	bsp->numLeafBrushes = GetLumpElements( &header, LUMP_LEAFBRUSHES, sizeof ( int ) );
	bsp->leafBrushes = malloc( bsp->numLeafBrushes * sizeof ( *bsp->leafBrushes ) );

	bsp->numSubmodels = GetLumpElements( &header, LUMP_MODELS, sizeof ( realDmodel_t ) );
	bsp->submodels = malloc( bsp->numSubmodels * sizeof ( *bsp->submodels ) );

	bsp->numBrushes = GetLumpElements( &header, LUMP_BRUSHES, sizeof ( realDbrush_t ) );
	bsp->brushes = malloc( bsp->numBrushes * sizeof ( *bsp->brushes ) );

	bsp->numBrushSides = GetLumpElements( &header, LUMP_BRUSHSIDES, sizeof ( realDbrushside_t ) );
	bsp->brushSides = malloc( bsp->numBrushSides * sizeof ( *bsp->brushSides ) );

	bsp->numDrawVerts = GetLumpElements( &header, LUMP_DRAWVERTS, sizeof ( realDrawVert_t ) );
	bsp->drawVerts = malloc( bsp->numDrawVerts * sizeof ( *bsp->drawVerts ) );

	bsp->numDrawIndexes = GetLumpElements( &header, LUMP_DRAWINDEXES, sizeof ( int ) );
	bsp->drawIndexes = malloc( bsp->numDrawIndexes * sizeof ( *bsp->drawIndexes ) );

	bsp->numFogs = GetLumpElements( &header, LUMP_FOGS, sizeof ( realDfog_t ) );
	bsp->fogs = malloc( bsp->numFogs * sizeof ( *bsp->fogs ) );

	bsp->numSurfaces = GetLumpElements( &header, LUMP_SURFACES, sizeof ( realDsurface_t ) );
	bsp->surfaces = malloc( bsp->numSurfaces * sizeof ( *bsp->surfaces ) );

	bsp->numLightmaps = GetLumpElements( &header, LUMP_LIGHTMAPS, 128 * 128 * 3 );
	bsp->lightmapData = malloc( bsp->numLightmaps * 128 * 128 * 3 );

	bsp->numGridPoints = GetLumpElements( &header, LUMP_LIGHTGRID, 8 );
	bsp->lightGridData = malloc( bsp->numGridPoints * 8 );

	bsp->visibilityLength = GetLumpElements( &header, LUMP_VISIBILITY, 1 ) - VIS_HEADER;
	if ( bsp->visibilityLength > 0 )
		bsp->visibility = malloc( bsp->visibilityLength );
	else
		bsp->visibilityLength = 0;

	//
	// copy and swap and convert data
	//
	CopyLump( &header, LUMP_ENTITIES, data, (void *) bsp->entityString, sizeof ( *bsp->entityString ), qfalse ); /* NO SWAP */

	{
		realDshader_t *in = GetLump( &header, data, LUMP_SHADERS );
		dshader_t *out = bsp->shaders;

		Com_Printf( "DEBUG: Num shaders %d\n", bsp->numShaders );

		for ( i = 0; i < bsp->numShaders; i++, in++, out++ ) {
			//Q_strncpyz( out->shader, in->shader, sizeof ( out->shader ) );
			Q_strncpyz( out->shader, "*noname", sizeof ( out->shader ) ); // ZTM: FIXME:
			out->contentFlags = LittleLong( in->contentFlags );
			out->surfaceFlags = LittleLong( in->surfaceFlags );

			if ( i < 3 || i > bsp->numShaders - 4 ) {
				Com_Printf( "DEBUG: Shader %d: surface %d, contents %d, surfaceFlags %d\n",
						i, in->drawSurfNum, out->contentFlags, out->surfaceFlags );
			}
		}

		// ZTM: Temp dummy
		if ( bsp->numShaders == 0 ) {
			bsp->numShaders = 16000;
			bsp->shaders = malloc( bsp->numShaders * sizeof ( *bsp->shaders ) );
			Com_Memset( bsp->shaders, 0, bsp->numShaders * sizeof ( *bsp->shaders ) );
		}
	}

	{
		realDplane_t *in = GetLump( &header, data, LUMP_PLANES );
		dplane_t *out = bsp->planes;

		for ( i = 0; i < bsp->numPlanes; i++, in++, out++) {
			for (j=0 ; j<3 ; j++) {
				out->normal[j] = LittleFloat (in->normal[j]);
			}

			out->dist = LittleFloat (in->dist);

			if ( i < 3 || i > bsp->numPlanes - 4 ) {
				Com_Printf( "DEBUG: Plane %d, normal %f, %f, %f, dist %f\n", i, out->normal[0], out->normal[1], out->normal[2], out->dist );
			}
		}
	}

	{
		realDnode_t *in = GetLump( &header, data, LUMP_NODES );
		dnode_t *out = bsp->nodes;

		for ( i = 0; i < bsp->numNodes; i++, in++, out++ ) {
			out->planeNum = LittleLong( in->planeNum );

			for ( j = 0; j < 2; j++ ) {
				out->children[j] = LittleLong( in->children[j] );
			}

			for ( j = 0; j < 3; j++ ) {
				out->mins[j] = LittleLong( in->mins[j] );
				out->maxs[j] = LittleLong( in->maxs[j] );
			}

			if ( i < 3 || i > bsp->numNodes - 4 ) {
				Com_Printf( "DEBUG: Node %d, plane %d, children %d, %d, mins %d, %d, %d, maxs %d, %d, %d\n",
					i, out->planeNum, out->children[0], out->children[1], out->mins[0], out->mins[1], out->mins[2],
					out->maxs[0], out->maxs[1], out->maxs[2] );
			}
		}
	}

	{
		realDleaf_t *in = GetLump( &header, data, LUMP_LEAFS );
		dleaf_t *out = bsp->leafs;
		int maxUsedLeafBrush = 0, maxUsedLeafSurface = 0;

		Com_Printf("DEBUG: Num leafs %d, leaf brushes %d, leaf surfaces %d\n", bsp->numLeafs, bsp->numLeafBrushes, bsp->numLeafSurfaces );

		for ( i = 0; i < bsp->numLeafs; i++, in++, out++ ) {
			out->cluster = LittleLong (in->cluster);
			out->area = LittleLong (in->area);

			for ( j = 0; j < 3; j++ ) {
				out->mins[j] = LittleLong( in->mins[j] );
				out->maxs[j] = LittleLong( in->maxs[j] );
			}

			out->firstLeafBrush = LittleLong (in->firstLeafBrush);
			out->numLeafBrushes = LittleLong (in->numLeafBrushes);
			out->firstLeafSurface = LittleLong (in->firstLeafSurface);
			out->numLeafSurfaces = LittleLong (in->numLeafSurfaces);

			if ( i < 3 || i > bsp->numLeafs - 4 ) {
				Com_Printf( "DEBUG: Leaf %d, mins %d, %d, %d, maxs %d, %d, %d\n",
					i, out->mins[0], out->mins[1], out->mins[2],
					out->maxs[0], out->maxs[1], out->maxs[2] );
			}

			if ( maxUsedLeafBrush < out->firstLeafBrush + out->numLeafBrushes ) {
				maxUsedLeafBrush = out->firstLeafBrush + out->numLeafBrushes;
			}

			if ( maxUsedLeafSurface < out->firstLeafSurface + out->numLeafSurfaces ) {
				maxUsedLeafSurface = out->firstLeafSurface + out->numLeafSurfaces;
			}
		}

		if ( maxUsedLeafBrush != bsp->numLeafBrushes ) {
			Com_Printf("WARNING: highest used leaf brush %d, max %d\n", maxUsedLeafBrush, bsp->numLeafBrushes );
		}

		if ( maxUsedLeafSurface != bsp->numLeafSurfaces ) {
			Com_Printf("WARNING: highest used leaf surface %d, max %d\n", maxUsedLeafSurface, bsp->numLeafSurfaces );
		}
	}

	CopyLump( &header, LUMP_LEAFSURFACES, data, (void *) bsp->leafSurfaces, sizeof ( *bsp->leafSurfaces ), qtrue );
	CopyLump( &header, LUMP_LEAFBRUSHES, data, (void *) bsp->leafBrushes, sizeof ( *bsp->leafBrushes ), qtrue );

	{
		realDmodel_t *in = GetLump( &header, data, LUMP_MODELS );
		dmodel_t *out = bsp->submodels;
		int maxUsedSurface = 0, maxUsedBrush = 0;

		Com_Printf("DEBUG: Num BSP models %d\n", bsp->numSubmodels );
		for ( i = 0; i < bsp->numSubmodels; i++, in++, out++ ) {
			for ( j = 0; j < 3; j++ ) {
				out->mins[j] = LittleFloat( in->mins[j] );
				out->maxs[j] = LittleFloat( in->maxs[j] );
			}

			out->firstSurface = LittleLong (in->firstSurface);
			out->numSurfaces = LittleLong (in->numSurfaces);
			out->firstBrush = LittleLong (in->firstBrush);
			out->numBrushes = LittleLong (in->numBrushes);

			if ( i < 3 || i > bsp->numSubmodels - 4 ) {
				Com_Printf( "DEBUG: Model %d, mins %f, %f, %f, maxs %f, %f, %f\n",
					i, out->mins[0], out->mins[1], out->mins[2],
					out->maxs[0], out->maxs[1], out->maxs[2] );
			}

			if ( maxUsedSurface < out->firstSurface + out->numSurfaces )
				maxUsedSurface = out->firstSurface + out->numSurfaces;

			if ( maxUsedBrush < out->firstBrush + out->numBrushes )
				maxUsedBrush = out->firstBrush + out->numBrushes;
		}

		if ( maxUsedBrush != bsp->numBrushes ) {
			Com_Printf("WARNING: highest used model brush %d, max %d\n", maxUsedBrush, bsp->numBrushes );
		}

		if ( maxUsedSurface != bsp->numSurfaces ) {
			Com_Printf("WARNING: highest used model surface %d, max %d\n", maxUsedSurface, bsp->numSurfaces );
		}
	}

	{
		realDbrush_t *in = GetLump( &header, data, LUMP_BRUSHES );
		dbrush_t *out = bsp->brushes;

		Com_Printf("DEBUG: Num BSP brushes %d\n", bsp->numBrushes );

		for ( i = 0; i < bsp->numBrushes; i++, in++, out++ )
		{
			out->firstSide = LittleLong (in->firstSide);
			out->numSides = LittleLong (in->numSides);
			out->shaderNum = LittleLong (in->shaderNum);

			if ( i < 3 || i > bsp->numBrushes - 4 ) {
				Com_Printf( "DEBUG: Brush %d, first side %d, num sides %d, shader num %d\n",
					i, out->firstSide, out->numSides, out->shaderNum );
			}
		}
	}

	{
		realDbrushside_t *in = GetLump( &header, data, LUMP_BRUSHSIDES );
		dbrushside_t *out = bsp->brushSides;

		Com_Printf("DEBUG: Num BSP brush sides %d\n", bsp->numBrushSides );

		for ( i = 0; i < bsp->numBrushSides; i++, in++, out++ ) {
			out->planeNum = LittleLong (in->planeNum);
			out->shaderNum = LittleLong (in->shaderNum);
			out->surfaceNum = -1;

			if ( i < 3 || i > bsp->numBrushSides - 4 ) {
				Com_Printf( "DEBUG: Brush side %d, plane num %d, shader num %d\n",
					i, out->planeNum, out->shaderNum );
			}
		}
	}

	{
		realDrawVert_t *in = GetLump( &header, data, LUMP_DRAWVERTS );
		drawVert_t *out = bsp->drawVerts;

		Com_Printf("DEBUG: Num BSP draw verts %d\n", bsp->numDrawVerts );

		for ( i = 0; i < bsp->numDrawVerts; i++, in++, out++ ) {
			for ( j = 0 ; j < 3 ; j++ ) {
				out->xyz[j] = LittleFloat( in->xyz[j] );
				out->normal[j] = LittleFloat( in->normal[j] );
			}
			for ( j = 0 ; j < 2 ; j++ ) {
				out->st[j] = LittleFloat( in->st[j] );
				out->lightmap[j] = LittleFloat( in->lightmap[j] );
			}

			/* NO SWAP */
			for ( j = 0; j < 4; j++ ) {
				out->color[j] = in->color[j];
			}

			if ( i < 3 || i > bsp->numDrawVerts - 4 ) {
				Com_Printf( "DEBUG: Draw vert %d, xyz %f %f %f, normal %f %f %f, st %f %f\n",
					i, out->xyz[0], out->xyz[1], out->xyz[2], out->normal[0], out->normal[1], out->normal[2], out->st[0], out->st[1] );
			}
		}
	}

	CopyLump( &header, LUMP_DRAWINDEXES, data, (void *) bsp->drawIndexes, sizeof ( *bsp->drawIndexes ), qtrue );

	{
		realDfog_t *in = GetLump( &header, data, LUMP_FOGS );
		dfog_t *out = bsp->fogs;

		Com_Printf("DEBUG: Num BSP fogs %d\n", bsp->numFogs );

		for ( i = 0; i < bsp->numFogs; i++, in++, out++ ) {
			Q_strncpyz( out->shader, in->shader, sizeof ( out->shader ) );
			out->brushNum = LittleLong (in->brushNum);
			// ZTM: Hard code visibleSide to top. I'm unsure if this is the correct handling.
			out->visibleSide = 5; //LittleLong (in->visibleSide);

			if ( i < 3 ) {
				Com_Printf( "DEBUG: Fog %d, shader %s, brush num %d\n",
					i, out->shader, out->brushNum );
			}
		}
	}

	{
		realDsurface_t *in = GetLump( &header, data, LUMP_SURFACES );
		dsurface_t *out = bsp->surfaces;

		Com_Printf("DEBUG: Num BSP surfaces %d, num lightmaps %d\n", bsp->numSurfaces, bsp->numLightmaps );

		for ( i = 0; i < bsp->numSurfaces; i++, in++, out++ ) {
			out->shaderNum = 0; //LittleLong (in->shaderNum);	// ZTM: FIXME
			out->fogNum = LittleLong (in->fogNum);
			out->surfaceType = LittleLong (in->surfaceType);
			out->firstVert = LittleLong (in->firstVert);
			out->numVerts = LittleLong (in->numVerts);
			out->firstIndex = LittleLong (in->firstIndex);
			out->numIndexes = LittleLong (in->numIndexes);
			out->lightmapNum = LittleLong (in->lightmapNum);
			out->lightmapX = LittleLong (in->lightmapX);
			out->lightmapY = LittleLong (in->lightmapY);
			out->lightmapWidth = LittleLong (in->lightmapWidth);
			out->lightmapHeight = LittleLong (in->lightmapHeight);

			for ( j = 0; j < 3; j++ ) {
				out->lightmapOrigin[j] = LittleFloat( in->lightmapOrigin[j] );
				for ( k = 0; k < 3; k++ ) {
					out->lightmapVecs[j][k] = LittleFloat( in->lightmapVecs[j][k] );
				}
			}

			out->patchWidth = LittleLong (in->patchWidth);
			out->patchHeight = LittleLong (in->patchHeight);

			out->subdivisions = SUBDIVIDE_DISTANCE;


			// ZTM: is this really not included or am I missing something?
			if ( out->patchWidth > 0 && out->patchHeight > 0 ) {
				out->surfaceType = MST_PATCH;
			} else if ( out->surfaceType == 0 ) { // ZTM: can out->numIndexes != 0 be checked instead? though, planar should have it too...
				out->surfaceType = MST_TRIANGLE_SOUP;
			} else {
				out->surfaceType = MST_PLANAR;
			}

#if 0
			if ( i < 20/*3*/ || i > bsp->numSurfaces - 4 || out->numIndexes != 0 ) {
				Com_Printf( "DEBUG: Surface %d, shader %s, fogNum %d, surfaceType %d, firstVert %d, numVerts %d, firstIndex %d, numIndexes %d, lightmapNum %d, X %d, Y %d, W %d, H %d\n",
					i, in->shader, out->fogNum, out->surfaceType, out->firstVert, out->numVerts, out->firstIndex, out->numIndexes, out->lightmapNum, out->lightmapX, out->lightmapY, out->lightmapWidth, out->lightmapHeight );
				Com_Printf( "    lightmapOrigin: %f, %f, %f\n", out->lightmapOrigin[0], out->lightmapOrigin[1], out->lightmapOrigin[2] );
				Com_Printf( "    lightmapVecs[0]: %f, %f, %f\n", out->lightmapVecs[0][0], out->lightmapVecs[0][1], out->lightmapVecs[0][2] );
				Com_Printf( "    lightmapVecs[1]: %f, %f, %f\n", out->lightmapVecs[1][0], out->lightmapVecs[1][1], out->lightmapVecs[1][2] );
				Com_Printf( "    lightmapVecs[2]: %f, %f, %f\n", out->lightmapVecs[2][0], out->lightmapVecs[2][1], out->lightmapVecs[2][2] );
				Com_Printf( "    patchWidth %d, patchHeight %d\n", out->patchWidth, out->patchHeight );

#if 0 // these don't make sense, though that assumes q3test even used the same surface flag values
				Com_Printf("    surfacetype is flags?" );
				if ( out->surfaceType & SURF_NODAMAGE ) {
					Com_Printf( " NODAMAGE" );
				}
				if ( out->surfaceType & SURF_SLICK ) {
					Com_Printf( " SLICK" );
				}
				if ( out->surfaceType & SURF_SKY ) {
					Com_Printf( " SKY" );
				}
				if ( out->surfaceType & SURF_LADDER ) {
					Com_Printf( " LADDER" );
				}
				if ( out->surfaceType & SURF_NOIMPACT ) {
					Com_Printf( " NOIMPACT" );
				}
				if ( out->surfaceType & SURF_NOMARKS ) {
					Com_Printf( " NOMARKS" );
				}
				if ( out->surfaceType & SURF_FLESH ) {
					Com_Printf( " FLESH" );
				}
				if ( out->surfaceType & SURF_NODRAW ) {
					Com_Printf( " NODRAW" );
				}
				if ( out->surfaceType & SURF_HINT ) {
					Com_Printf( " HINT" );
				}
				if ( out->surfaceType & SURF_SKIP ) {
					Com_Printf( " SKIP" );
				}
				if ( out->surfaceType & SURF_NOLIGHTMAP ) {
					Com_Printf( " NOLIGHTMAP" );
				}
				if ( out->surfaceType & SURF_POINTLIGHT ) {
					Com_Printf( " POINTLIGHT" );
				}
				if ( out->surfaceType & SURF_METALSTEPS ) {
					Com_Printf( " METALSTEPS" );
				}
				if ( out->surfaceType & SURF_NOSTEPS ) {
					Com_Printf( " NOSTEPS" );
				}
				if ( out->surfaceType & SURF_NONSOLID ) {
					Com_Printf( " NONSOLID" );
				}
				if ( out->surfaceType & SURF_LIGHTFILTER ) {
					Com_Printf( " LIGHTFILTER" );
				}
				if ( out->surfaceType & SURF_ALPHASHADOW ) {
					Com_Printf( " ALPHASHADOW" );
				}
				if ( out->surfaceType & SURF_NODLIGHT ) {
					Com_Printf( " NODLIGHT" );
				}
				if ( out->surfaceType & SURF_DUST ) {
					Com_Printf( " DUST" );
				}
				Com_Printf("\n");
#endif
			}

			if ( out->firstVert < 0 || out->numVerts < 0 || out->firstVert + out->numVerts > bsp->numDrawVerts ) {
				Com_Printf( "DEBUG: Surface %d: Invalid verts: first vert %d, num verts %d, max verts %d\n", i, out->firstVert, out->numVerts, bsp->numDrawVerts );
			}
			if ( out->firstIndex < 0 || out->numIndexes < 0 || out->firstIndex + out->numIndexes > bsp->numDrawIndexes ) {
				Com_Printf( "DEBUG: Surface %d: Invalid vert indexes: first index %d, num indexes %d, max indexes %d\n", i, out->firstIndex, out->numIndexes, bsp->numDrawIndexes );
			}
			if ( out->fogNum < -1 || out->fogNum >= bsp->numFogs ) {
				Com_Printf( "DEBUG: Surface %d: Invalid fog num: fogNum %d, max fogs %d\n", i, out->fogNum, bsp->numFogs );
			}
			// ZTM: Surface num points to wrong data
			if ( out->surfaceType < 0 || out->surfaceType >= MST_FOLIAGE ) {
			//	Com_Printf( "DEBUG: Surface %d: Invalid surface type %d\n", i, out->surfaceType );
			}
#endif
		}
	}

	CopyLump( &header, LUMP_LIGHTMAPS, data, (void *) bsp->lightmapData, sizeof ( *bsp->lightmapData ), qfalse ); /* NO SWAP */
	CopyLump( &header, LUMP_LIGHTGRID, data, (void *) bsp->lightGridData, sizeof ( *bsp->lightGridData ), qfalse ); /* NO SWAP */

	if ( bsp->visibilityLength )
	{
		byte *in = GetLump( &header, data, LUMP_VISIBILITY );

		bsp->numClusters = LittleLong( ((int *)in)[0] );
		bsp->clusterBytes = LittleLong( ((int *)in)[1] );

		Com_Memcpy( bsp->visibility, in + VIS_HEADER, bsp->visibilityLength ); /* NO SWAP */
	}

	return bsp;
}


/****************************************************
*/

bspFormat_t q3Test105BspFormat = {
	"Q3Test 1.05",
	BSP_IDENT,
	BSP_VERSION,
	BSP_LoadQ3Test105,
};
