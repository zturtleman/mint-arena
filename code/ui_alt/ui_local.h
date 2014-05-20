/*
===========================================================================
Copyright (C) 2014 Zack Middleton

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

#include "../cgame/cg_local.h"
#include "../ui/ui_public.h"

#ifdef MISSIONPACK
#define CURSOR_SHADER "ui/assets/3_cursor3"

// ZTM: I'm pretty sure these aren't used...
#define CURSOR_SIZE_SHADER "ui/assets/sizecursor"
#define CURSOR_SELECT_SHADER "ui/assets/selectcursor"
#else
#define CURSOR_SHADER "menu/art/3_cursor2"
#define CURSOR_SIZE_SHADER CURSOR_SHADER
#define CURSOR_SELECT_SHADER CURSOR_SHADER
#endif

typedef struct {
	int x, y;
	qhandle_t shader;
	qboolean show;

} cursor_t;


typedef struct {

	qhandle_t cursorShader;
	cursor_t cursors[MAX_SPLITVIEW];


} uiStatic_t;

