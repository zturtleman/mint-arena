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
	current->selectedItem = 0;
	current->mouseItem = -1;
	current->mouseClickDown = qfalse;

	if ( !menu ) {
		if ( cg.connected ) {
			trap_Mouse_SetState( 0, MOUSE_CLIENT );
			trap_Key_SetCatcher( 0 );
			trap_Cvar_SetValue( "cl_paused", 0 );
		}

		return;
	}

	UI_BuildCurrentMenu( current );
	UI_SetInitalSelection( current );
}

// change current menu, without changing stack
void UI_SwapMenu( currentMenu_t *current, menuId_t menu ) {
	if ( current->menu == menu )
		return;

	current->menu = menu;
	current->selectedItem = 0;
	current->mouseItem = -1;
	current->mouseClickDown = qfalse;

	if ( !menu )
		return;

	UI_BuildCurrentMenu( current );
	UI_SetInitalSelection( current );
}

// add current menu to stask, then change to new menu
void UI_PushMenu( currentMenu_t *current, menuId_t menu ) {
	if ( current->menu == menu )
		return;

	if ( current->menu ) {
		// push stack
		current->stack[current->numStacked].menu = current->menu;
		current->stack[current->numStacked].selectedItem = current->selectedItem;
		current->numStacked++;
	}

	current->menu = menu;
	current->selectedItem = 0;
	current->mouseItem = -1;
	current->mouseClickDown = qfalse;

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
	current->selectedItem = current->stack[current->numStacked].selectedItem;
	current->mouseItem = -1;
	current->mouseClickDown = qfalse;

	trap_S_StartLocalSound( uis.menuPopSound, CHAN_LOCAL_SOUND );

	UI_BuildCurrentMenu( current );
	UI_SetInitalSelection( current );
}

// dir -1 = up, 1 = down, 2 = left, 4 = right
void UI_MenuAdjustCursor( currentMenu_t *current, int dir ) {
	int original;

	if ( dir == 2 || dir == 4 ) {
		UI_MenuItemChangeValue( current, current->selectedItem, dir - 3 );
		return;
	}

	original = current->selectedItem;

	do {
		current->selectedItem += dir;

		if ( current->selectedItem >= current->numItems )
			current->selectedItem = 0;
		else if ( current->selectedItem < 0 )
			current->selectedItem = current->numItems - 1;

		// nothing was selectable...
		if ( current->selectedItem == original )
			break;

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
qboolean UI_MenuItemChangeValue( currentMenu_t *current, int itemNum, int dir ) {
	currentMenuItem_t *item;
	float value;

	item = &current->items[itemNum];

	if ( !item->cvarName || !item->cvarRange ) {
		return qtrue;
	}

	if ( item->cvarRange->numPairs > 0 ) {
		item->cvarPair += dir;

		if ( item->cvarPair >= item->cvarRange->numPairs )
			item->cvarPair = 0;
		else if ( item->cvarPair < 0 )
			item->cvarPair = item->cvarRange->numPairs - 1;

		value = item->cvarRange->pairs[ item->cvarPair ].value;
	}
	else
	{
		float min, max;
		qboolean reversed;
		//qboolean ratioButton;

		reversed = ( item->cvarRange->min > item->cvarRange->max );
		if ( reversed ) {
			min = item->cvarRange->max;
			max = item->cvarRange->min;
		} else {
			min = item->cvarRange->min;
			max = item->cvarRange->max;
		}

		// trap_Cvar_VariableValue( item->cvarName ) doesn't work with latched cvars
		value = item->vmCvar.value + dir * item->cvarRange->stepSize;

		if ( reversed ) {
			value = max - value;
		}

		//ratioButton = item->cvarRange->stepSize == 1 && max == 1 && min == 0;

		if ( UI_ItemIsSlider( item ) ) {
			// added elipse for min=0, max=1, step=0.1 not being able to go to max
			if ( value < min - 0.001f || value > max + 0.001f ) {
				// play buzz sound
				trap_S_StartLocalSound( uis.itemWarnSound, CHAN_LOCAL_SOUND );
				return qfalse;
			}
			value = Com_Clamp( min, max, value );
		}
		// ratio button, ..erm or any wrapping value
		else if ( min != max ) {
			// if cvar has min and max and out of range, wrap around
			if ( value < min ) value = max;
			if ( value > max ) value = min;
		}
	}

	// FIXME: What about cases when it should not take affect until 'Apply' is clicked?
	trap_Cvar_SetValue( item->cvarName, value );
	trap_Cvar_Update( &item->vmCvar );

	trap_S_StartLocalSound( uis.itemActionSound, CHAN_LOCAL_SOUND );

	return qtrue;
}

void UI_MenuAction( currentMenu_t *current, int itemNum ) {
	currentMenuItem_t item;

	if ( !UI_MenuItemChangeValue( current, itemNum, 1 ) ) {
		return;
	}

	// item is copied instead of a pointer to avoid issues when switching menus
	item = current->items[itemNum];

	if ( item.flags & MIF_CALL ) {
		if ( item.action ) {
			item.action( itemNum );
		}
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

	if ( UI_ItemIsSlider( item ) ) {
		float frac, sliderx, targetStep, targetValue, min, max;
		qboolean reversed;

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

		// click slider item outside of slider bar... eat action
		if ( ( x < sliderx || x > sliderx + 96 ) && state == MACTION_PRESS ) {
			return qtrue;
		} else {
			sliderx += 8;
			frac = Com_Clamp( 0, 1, ( x - 6 - sliderx ) / ( 96 - 16 ) );

			if ( reversed ) {
				frac = 1.0f - frac;
			}

			targetStep = frac * ( max - min );
			targetValue = targetStep + min;
		}

		if ( state == MACTION_RELEASE ) {
			float value, halfStep;

			halfStep = item->cvarRange->stepSize / 2.0f;

			// snap to step
			value = 0;
			while ( value + halfStep <= targetStep ) {
				value += item->cvarRange->stepSize;
			}
			value += min;

			value = Com_Clamp( min, max, value );
			trap_Cvar_SetValue( item->cvarName, value );

			// force update because dragging changes the vmCvar directly
			item->vmCvar.modificationCount = -1;
			trap_Cvar_Update( &item->vmCvar );
		} else {
			// change the value shown to the user
			item->vmCvar.value = targetValue;
		}
		return qtrue;
	}

	return qfalse;
}

qboolean UI_ItemIsSlider( currentMenuItem_t *item ) {
	return ( item->cvarRange && item->cvarRange->numPairs == 0 );
}

