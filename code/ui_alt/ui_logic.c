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
// handles menu and menu item logic

#include "ui_local.h"

// called when a new menu is shown to make selection valid
static void UI_SetInitalSelection( currentMenu_t *current ) {
	// if cursor is over an item select it
	UI_MenuCursorPoint( current, uis.cursors[0].x, uis.cursors[0].y );

	// if selected item, bail out
	if ( current->mouseItem != -1 ) {
		return;
	}

	// if initial item is not selectable, use to next
	if ( !( current->items[ current->selectedItem ].flags & MIF_SELECTABLE ) ) {
		UI_MenuAdjustCursor( current, 1 );
	}
}

// clear stack, change to menu
void UI_SetMenu( currentMenu_t *current, menuId_t menu ) {
	current->numStacked = 0;
	current->menu = menu;
	current->panel = 1;
	current->selectedItem = 0;

	current->mouseItem = -1;
	current->mouseClickDown = 0;

	if ( menu == M_NONE ) {
		if ( cg.connected ) {
			trap_Mouse_SetState( 0, MOUSE_CLIENT );
			Key_SetCatcher( 0 );
			trap_Cvar_SetValue( "cl_paused", 0 );
		}

		return;
	}

	UI_BuildCurrentMenu( current );
	UI_SetInitalSelection( current );
}

// change current menu, without changing stack
void UI_SwapMenu( currentMenu_t *current, menuId_t menu ) {
	if ( current->menu == menu ) {
		return;
	}

	current->menu = menu;
	current->panel = 1;
	current->selectedItem = 0;

	current->mouseItem = -1;
	current->mouseClickDown = 0;

	if ( menu == M_NONE ) {
		return;
	}

	UI_BuildCurrentMenu( current );
	UI_SetInitalSelection( current );
}

// add current menu to stask, then change to new menu
void UI_PushMenu( currentMenu_t *current, menuId_t menu ) {
	if ( current->menu == menu ) {
		return;
	}

	if ( current->menu ) {
		// push stack
		current->stack[current->numStacked].menu = current->menu;
		current->stack[current->numStacked].panel = current->panel;
		current->stack[current->numStacked].selectedItem = current->selectedItem;
		current->numStacked++;
	}

	current->menu = menu;
	current->panel = 1;
	current->selectedItem = 0;

	current->mouseItem = -1;
	current->mouseClickDown = 0;

	UI_BuildCurrentMenu( current );
	UI_SetInitalSelection( current );
}

// return to previous menu
void UI_PopMenu( currentMenu_t *current ) {
	if ( current->numStacked == 0 ) {
		if ( cg.connected ) {
			UI_SetMenu( current, M_NONE );
		}
		return;
	}

	// pop stack
	current->numStacked--;
	current->menu = current->stack[current->numStacked].menu;
	current->panel = current->stack[current->numStacked].panel;
	current->selectedItem = current->stack[current->numStacked].selectedItem;

	current->mouseItem = -1;
	current->mouseClickDown = 0;

	trap_S_StartLocalSound( uis.menuPopSound, CHAN_LOCAL_SOUND );

	UI_BuildCurrentMenu( current );
	UI_SetInitalSelection( current );
}

void UI_SetMenuPanel( currentMenu_t *current, int itemNum ) {
	int i;
	currentMenuItem_t *item;
	int panelNum = 0;

	for ( i = 0, item = current->items; i < current->numItems; i++, item++ ) {
		if ( item->flags & MIF_PANEL ) {
			panelNum++;
		}
		if ( i == itemNum ) {
			break;
		}
	}

	if ( current->panel == panelNum ) {
		return;
	}

	current->panel = panelNum;
	UI_BuildCurrentMenu( current );
	UI_SetInitalSelection( current );

	// select the panel button
	panelNum = 0;
	for ( i = 0, item = current->items; i < current->numItems; i++, item++ ) {
		if ( item->flags & MIF_PANEL ) {
			panelNum++;
		}
		if ( panelNum == current->panel ) {
			break;
		}
	}

	current->selectedItem = i;
}

// dir -1 = up, 1 = down, 2 = left, 4 = right
void UI_MenuAdjustCursor( currentMenu_t *current, int dir ) {
	int original;

	if ( dir == 2 || dir == 4 ) {
		// was UI_MenuItemChangeValue, but want the action function to get run
		// ZTM: FIXME: causes button action to run, only meant to affect lists / sliders
		UI_MenuAction( current, current->selectedItem, dir - 3 );
		return;
	}

	original = current->selectedItem;

	do {
		current->selectedItem += dir;

		if ( current->selectedItem >= current->numItems ) {
			current->selectedItem = 0;
		} else if ( current->selectedItem < 0 ) {
			current->selectedItem = current->numItems - 1;
		}

		// nothing was selectable...
		if ( current->selectedItem == original ) {
			break;
		}

	} while ( !( current->items[ current->selectedItem ].flags & MIF_SELECTABLE ) );

	if ( current->selectedItem != original ) {
		trap_S_StartLocalSound( uis.itemFocusSound, CHAN_LOCAL_SOUND );
	}
}

void UI_MenuCursorPoint( currentMenu_t *current, int x, int y ) {
	currentMenuItem_t *item;
	int original;
	int i;

	original = current->selectedItem;
	current->mouseItem = -1;

	for ( i = 0, item = current->items; i < current->numItems; i++, item++ ) {
		if ( !( item->flags & MIF_SELECTABLE ) ) {
			continue;
		}

		if ( x >= item->clickPos.x && x <= item->clickPos.x + item->clickPos.width
			&& y >= item->clickPos.y && y <= item->clickPos.y + item->clickPos.height ) {
			// inside item bbox
			current->selectedItem = i;
			current->mouseItem = i;
			break;
		}
	}

	if ( current->selectedItem != original ) {
		trap_S_StartLocalSound( uis.itemFocusSound, CHAN_LOCAL_SOUND );
	}
}

// returns qfalse if item is a cvar and did not change value
// FIXME: What about cases when it should not take affect until 'Apply' is clicked?
qboolean UI_MenuItemChangeValue( currentMenu_t *current, int itemNum, int dir ) {
	currentMenuItem_t *item;

	item = &current->items[itemNum];

	if ( item->cvarPairs && item->numPairs > 0 ) {
		const char *value;

		item->cvarPair += dir;

		if ( item->cvarPair >= item->numPairs ) {
			item->cvarPair = 0;
		} else if ( item->cvarPair < 0 ) {
			item->cvarPair = item->numPairs - 1;
		}

		value = item->cvarPairs[ item->cvarPair ].value;

		if ( item->cvarPairs[ item->cvarPair ].type == CVT_CMD ) {
			trap_Cmd_ExecuteText( EXEC_APPEND, value );
		} else if ( item->cvarName ) {
			trap_Cvar_Set( item->cvarName, value );
		} else {
			Com_Printf( S_COLOR_YELLOW "WARNING: menu item '%s' doesn't have a cvarName but it's value requires it\n", item->caption );
		}
	}
	else if ( item->cvarRange )
	{
		float min, max, value;
		qboolean reversed;

		reversed = ( item->cvarRange->min > item->cvarRange->max );
		if ( reversed ) {
			min = item->cvarRange->max;
			max = item->cvarRange->min;
		} else {
			min = item->cvarRange->min;
			max = item->cvarRange->max;
		}

		// NOTE: trap_Cvar_VariableValue( item->cvarName ) doesn't work with latched cvars
		value = item->vmCvar.value + dir * item->cvarRange->stepSize;

		if ( reversed ) {
			value = max - value;
		}

		// FIXME: slider will always be true here .. is the other case going to be needed? ratio buttons will use cvarPairs.
		if ( item->widgetType == UIW_SLIDER ) {
			// added elipse for min=0, max=1, step=0.1 not being able to go to max
			if ( value < min - 0.001f || value > max + 0.001f ) {
				// play buzz sound
				trap_S_StartLocalSound( uis.itemWarnSound, CHAN_LOCAL_SOUND );
				return qfalse;
			}
			value = Com_Clamp( min, max, value );
		}
		// wrapping value
		else if ( min != max ) {
			// if cvar has min and max and out of range, wrap around
			if ( value < min ) {
				value = max;
			} else if ( value > max ) {
				value = min;
			}
		}

		if ( item->cvarName ) {
			trap_Cvar_SetValue( item->cvarName, value );
		} else {
			Com_Printf( S_COLOR_YELLOW "WARNING: menu item '%s' doesn't have a cvarName but it's value requires it\n", item->caption );
		}
	}

	return qtrue;
}

void UI_MenuAction( currentMenu_t *current, int itemNum, int dir ) {
	currentMenuItem_t item;

	if ( !UI_MenuItemChangeValue( current, itemNum, dir ) ) {
		return;
	}

	// item is copied instead of a pointer to avoid issues when switching menus
	item = current->items[itemNum];

	if ( item.flags & MIF_CALL ) {
		if ( item.action ) {
			item.action( itemNum );
		}
	}

	if ( item.flags & MIF_PANEL ) {
		// switch to panel
		UI_SetMenuPanel( current, itemNum );
	}

	if ( item.flags & MIF_SWAPMENU ) {
		UI_SwapMenu( current, item.menuid );
		// warning, push menu replaces the current->items
	} else if ( item.flags & MIF_SUBMENU ) {
		UI_PushMenu( current, item.menuid );
		// warning, push menu replaces the current->items
	}

	// can use call and popmenu on the same item
	if ( item.flags & MIF_POPMENU ) {
		UI_PopMenu( current );
		return; // popmenu plays a sound
	}

	trap_S_StartLocalSound( uis.itemActionSound, CHAN_LOCAL_SOUND );
}

qboolean UI_MenuMouseAction( currentMenu_t *current, int itemNum, int x, int y, mouseActionState_t state ) {
	currentMenuItem_t *item;

	item = &current->items[itemNum];

	if ( item->widgetType == UIW_SLIDER || item->widgetType == UIW_COLORBAR ) {
		float frac, sliderx, targetStep, targetValue, min, max;
		qboolean reversed;
		int sliderWidth;
		float value, halfStep;

		// clicked slider caption, ignore -- still allow clicking it to run an action
		//if ( x < item->captionPos.x + item->captionPos.width && state == MACTION_PRESS )
		//	return qfalse;

		sliderx = item->captionPos.x + item->captionPos.width + BIGCHAR_WIDTH;

		reversed = ( item->cvarRange->min > item->cvarRange->max );
		if ( reversed ) {
			min = item->cvarRange->max;
			max = item->cvarRange->min;
		} else {
			min = item->cvarRange->min;
			max = item->cvarRange->max;
		}

		if ( item->widgetType == UIW_COLORBAR ) {
			sliderWidth = 128;
		} else {
			sliderWidth = 96;
		}

		// click slider item outside of slider bar... eat action
		if ( ( x < sliderx || x > sliderx + sliderWidth ) && state == MACTION_PRESS ) {
			return qtrue;
		} else {
			sliderx += 8;
			frac = Com_Clamp( 0, 1, ( x - 6 - sliderx ) / ( sliderWidth - 16 ) );

			if ( reversed ) {
				frac = 1.0f - frac;
			}

			targetStep = frac * ( max - min );
			targetValue = targetStep + min;
		}

		//
		// round to nearest
		//
		halfStep = item->cvarRange->stepSize / 2.0f;

		// snap to step
		value = 0;
		while ( value + halfStep <= targetStep ) {
			value += item->cvarRange->stepSize;
		}
		value += min;

		value = Com_Clamp( min, max, value );

		// color bar uses values in a non-linear order
		if ( item->numPairs ) {
			item->cvarPair = Com_Clamp( 0, item->numPairs, ( value - min ) / item->cvarRange->stepSize );

			// ZTM: TODO: handle CVT_CMD and CVT_STRING
			// display the cvar value, not the index value
			targetValue = value = atof( item->cvarPairs[item->cvarPair].value );
		}

		if ( state == MACTION_RELEASE ) {
			trap_Cvar_SetValue( item->cvarName, value );

#if 0 // ZTM: Old code. Now the cvar will automatically update to latched value each frame (derp, but only if the string changes -- i.e. not when snaps to existing value)
			// force update because dragging changes the vmCvar directly
			item->vmCvar.modificationCount = -1;
			trap_Cvar_Update( &item->vmCvar );
#endif

			// change the value shown to the user
			// HACK?: this is required because if the value snapped to existing the vm cvar doesn't get updated (for latched r_picmip at least)
			item->vmCvar.value = value;
		} else {
			// change the value shown to the user
			item->vmCvar.value = targetValue;
		}
		return qtrue;
	}

	return qfalse;
}

int UI_NumCvarPairs( cvarValuePair_t *cvarPairs ) {
	int pair;

	if ( !cvarPairs ) {
		return 0;
	}

	for ( pair = 0; cvarPairs[pair].type != CVT_NONE; pair++ ) {
		// count
	}

	return pair;
}

static void UI_SetMenuCvarValue( currentMenuItem_t *item ) {
	float dist, bestDist = 10000;
	int closestPair = 0;
	int pair;

	if ( !item->cvarName || !item->cvarPairs ) {
		return;
	}

	// HACK: TODO: need to override this for Graphics Settings
	if ( !Q_stricmp( item->cvarName, "ui_glCustom" ) ) {
		item->cvarPair = 0;
		return;
	}

	for ( pair = 0; pair < item->numPairs; pair++ ) {
		if ( item->cvarPairs[ pair ].type == CVT_STRING ) {
			if ( Q_stricmp( item->vmCvar.string, item->cvarPairs[ pair ].value ) == 0 ) {
				closestPair = pair;
				break;
			}
		} else if ( item->cvarPairs[ pair ].type == CVT_INT || item->cvarPairs[ pair ].type == CVT_FLOAT ) {
			dist = fabs( item->vmCvar.value - atof( item->cvarPairs[ pair ].value ) );
			if ( dist < bestDist ) {
				bestDist = dist;
				closestPair = pair;
			}
		}
	}

	item->cvarPair = closestPair;
}

void UI_RegisterMenuCvars( currentMenu_t *current ) {
	currentMenuItem_t *item;
	int i;

	for ( i = 0, item = current->items; i < current->numItems; i++, item++ ) {
		item->cvarPair = 0;

		// HACK: need to override this for resolution (r_mode)
		if ( item->cvarPairs == cp_resolution ) {
			item->cvarPair = uis.currentResPair;
		}

		if ( !item->cvarName ) {
			continue;
		}

		trap_Cvar_Register( &item->vmCvar, item->cvarName, "", 0 );

		// get latched value, if not latched gets current value
		trap_Cvar_LatchedVariableStringBuffer( item->cvarName, item->vmCvar.string, sizeof(item->vmCvar.string) );
		item->vmCvar.value = atof( item->vmCvar.string );
		item->vmCvar.integer = atoi( item->vmCvar.string );

		UI_SetMenuCvarValue( item );

		// cvar for demos/mods/cinematics list box needs to be set initially
		if ( item->widgetType == UIW_LISTBOX ) {
			// FIXME: should CVT_CMD be run?
			if ( item->cvarPairs[ item->cvarPair ].type != CVT_CMD ) {
				trap_Cvar_Set( item->cvarName, item->cvarPairs[ item->cvarPair ].value );
			}
		}
	}
}

// check if cvar have been modified via console or menu action function
void UI_UpdateMenuCvars( currentMenu_t *current ) {
	currentMenuItem_t *item;
	int i;
	int modCount;
	//float oldValue;
	char oldString[MAX_CVAR_VALUE_STRING];

	for ( i = 0, item = current->items; i < current->numItems; i++, item++ ) {
		if ( !item->cvarName ) {
			continue;
		}

		modCount = item->vmCvar.modificationCount;
		//oldValue = item->vmCvar.value;
		Q_strncpyz( oldString, item->vmCvar.string, sizeof (oldString) );
		trap_Cvar_Update( &item->vmCvar );

		// get latched value, if not latched gets current value
		trap_Cvar_LatchedVariableStringBuffer( item->cvarName, item->vmCvar.string, sizeof(item->vmCvar.string) );

		// The vmCvar.value is overriden for display while dragging sliders, so only update if needed
		if ( strcmp( item->vmCvar.string, oldString ) != 0 ) {
			item->vmCvar.value = atof( item->vmCvar.string );
			item->vmCvar.integer = atoi( item->vmCvar.string );

			//Com_Printf("DEBUG: Cvar changed! %s: %s -> %s. %f -> %f.\n", item->cvarName, oldString, item->vmCvar.string, oldValue, item->vmCvar.value );
			UI_SetMenuCvarValue( item );
			// should action function get run? well, if add it here it would run twice. and currently I don't want it _only_ here.
		}

		// should there just a string compare? could get rid of the trap_Cvar_Update call.
		// also, why does this not always get changed?
		if ( modCount != item->vmCvar.modificationCount ) {
			//Com_Printf("DEBUG: Cvar mod cound changed! %s: %s -> %s. %f -> %f.\n", item->cvarName, oldString, item->vmCvar.string, oldValue, item->vmCvar.value );
		}
	}
}

static int UI_CvarPairsStringCompare( const void *a, const void *b ) {
	return Q_stricmp( ((const cvarValuePair_t*)a)->string, ((const cvarValuePair_t*)b)->string );
}

void UI_InitFileList( currentMenu_t *current, currentMenuItem_t *item, const char *extData ) {
	char	dirName[MAX_QPATH], extension[MAX_QPATH], defaultMessage[128];
	char	*name;
	int		i, len, file, numFilenames;
	cvarValuePair_t *filePairs;
	int		maxFilePairs;
	char	*fileNames;
	int		fileNamesSize;

	filePairs = &current->filePairs[current->numFilePairs];
	maxFilePairs = ARRAY_LEN( current->filePairs ) - current->numFilePairs;

	fileNames = &current->fileText[current->fileTextLength];
	fileNamesSize = ARRAY_LEN( current->fileText ) - current->fileTextLength;

	Q_strncpyz( dirName, Info_ValueForKey( extData, "dir" ), sizeof (dirName) );
	Q_strncpyz( extension, Info_ValueForKey( extData, "ext" ), sizeof (extension) );
	// defaultMessage cannot be local memory...
	Q_strncpyz( defaultMessage, Info_ValueForKey( extData, "empty" ), sizeof (defaultMessage) );

	numFilenames = trap_FS_GetFileList( dirName, extension, fileNames, fileNamesSize );

	// mod list is "gamedir\0description\0"
	if ( !Q_stricmp( dirName, "$modlist" ) ) {
		numFilenames /= 2;
		if ( numFilenames >= maxFilePairs ) {
			numFilenames = maxFilePairs - 1;
		}
		name = fileNames;
		for ( i = 0; i < numFilenames; i++ ) {
			filePairs[i].type = CVT_STRING;

			// gamedir
			filePairs[i].value = name;
			name += strlen( name ) + 1;

			// description
			filePairs[i].string = name;
			name += strlen( name ) + 1;
		}

		// sort list by displayed names
		qsort( filePairs, numFilenames, sizeof ( filePairs[0] ), UI_CvarPairsStringCompare );
	} else {
		if ( numFilenames >= maxFilePairs ) {
			numFilenames = maxFilePairs - 1;
		}
		name = fileNames;
		for ( file = 0, i = 0; file < numFilenames; file++ ) {
			len = strlen( name );

			// special handling for list of directories
			// ZTM: FIXME: directories in pk3dirs do not have slash at end, but from pk3s do. causes duplicates if exist in both
			if ( *extension == '/' ) {
				if ( name[ len - 1 ] == '/' ) {
					name[ len - 1 ] = 0;
				} else if ( Q_stricmp( name, "." ) == 0 || Q_stricmp( name, ".." ) == 0 ) {
					name[0] = '\0';
				}
			} else {
				COM_StripExtension( name, name, len+1 );
			}

			if ( *name ) {
				filePairs[i].type = CVT_STRING;
				filePairs[i].value = name;
				filePairs[i].string = name;
				i++;
			}

			name += len + 1;
		}
	}

	// create default message placeholder and store text in menu's fileText
	if ( !numFilenames && defaultMessage[0] ) {
		filePairs[numFilenames].type = CVT_STRING;
		filePairs[numFilenames].value = "";
		filePairs[numFilenames].string = name;
		Q_strncpyz( name, defaultMessage, fileNamesSize );
		name += strlen( name ) + 1;

		numFilenames++;
		item->flags &= ~MIF_SELECTABLE;
	}

	filePairs[numFilenames].type = CVT_NONE;
	filePairs[numFilenames].value = NULL;
	filePairs[numFilenames].string = NULL;

	item->cvarPairs = filePairs;
	item->numPairs = numFilenames;

	// don't overwrite the CVT_NONE list terminator
	numFilenames++;

	current->fileTextLength += (int)(name - fileNames);
	current->numFilePairs += numFilenames;
}

