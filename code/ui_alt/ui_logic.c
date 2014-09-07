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

// clear stack, change to menu
void UI_SetMenu( currentMenu_t *current, menudef_t *menu ) {
	current->numStacked = 0;
	current->menu = menu;
	current->selectedItem = 0;
	current->mouseItem = -1;

	if ( !menu ) {
		if ( cg.connected ) {
			trap_Mouse_SetState( 0, MOUSE_CLIENT );
			trap_Key_SetCatcher( 0 );
			trap_Cvar_SetValue( "cl_paused", 0 );
		}

		return;
	}

	UI_BuildCurrentMenu( current );

	if ( !( current->items[ current->selectedItem ].flags & MIF_SELECTABLE ) ) {
		UI_MenuAdjustCursor( current, 1 );
	}
}

// change current menu, without changing stack
void UI_SwapMenu( currentMenu_t *current, menudef_t *menu ) {
	if ( current->menu == menu )
		return;

	current->menu = menu;
	current->selectedItem = 0;
	current->mouseItem = -1;

	if ( !menu )
		return;

	UI_BuildCurrentMenu( current );

	if ( !( current->items[ current->selectedItem ].flags & MIF_SELECTABLE ) ) {
		UI_MenuAdjustCursor( current, 1 );
	}
}

// add current menu to stask, then change to new menu
void UI_PushMenu( currentMenu_t *current, menudef_t *menu ) {
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

	UI_BuildCurrentMenu( current );

	if ( !( current->items[ current->selectedItem ].flags & MIF_SELECTABLE ) ) {
		UI_MenuAdjustCursor( current, 1 );
	}
}

// return to previous menu
void UI_PopMenu( currentMenu_t *current ) {
	if ( current->numStacked == 0 ) {
		if ( cg.connected ) {
			UI_SetMenu( current, NULL );
		}
		return;
	}

	// pop stack
	current->numStacked--;
	current->menu = current->stack[current->numStacked].menu;
	current->selectedItem = current->stack[current->numStacked].selectedItem;
	current->mouseItem = -1;

	trap_S_StartLocalSound( uis.menuPopSound, CHAN_LOCAL_SOUND );

	UI_BuildCurrentMenu( current );
}

// dir -1 = up, 1 = down
void UI_MenuAdjustCursor( currentMenu_t *current, int dir ) {
	int origial = current->selectedItem;

	do {
		current->selectedItem += dir;

		if ( current->selectedItem >= current->numItems )
			current->selectedItem = 0;
		else if ( current->selectedItem < 0 )
			current->selectedItem = current->numItems - 1;

		// nothing was selectable...
		if ( current->selectedItem == origial )
			break;

	} while ( !( current->items[ current->selectedItem ].flags & MIF_SELECTABLE ) );

	if ( current->selectedItem != origial ) {
		trap_S_StartLocalSound( uis.itemFocusSound, CHAN_LOCAL_SOUND );
	}
}

void UI_MenuCursorPoint( currentMenu_t *current, int x, int y ) {
	int origial = current->selectedItem;
	int i;

	current->mouseItem = -1;

	for ( i = 0; i < current->numItems; i++ ) {
		if ( !( current->items[i].flags & MIF_SELECTABLE ) ) {
			continue;
		}

		if ( x >= current->items[i].x && x <= current->items[i].x + current->items[i].width
			&& y >= current->items[i].y && y <= current->items[i].y + current->items[i].height ) {
			// inside item bbox
			current->selectedItem = i;
			current->mouseItem = i;
			break;
		}
	}

	if ( current->selectedItem != origial ) {
		trap_S_StartLocalSound( uis.itemFocusSound, CHAN_LOCAL_SOUND );
	}
}

void UI_MenuAction( currentMenu_t *current, int itemNum ) {
	currentMenuItem_t *item = &current->items[itemNum];
	qboolean popMenu = ( item->flags & MIF_POPMENU );

	if ( item->flags & MIF_CALL ) {
		void    (*function)( int item );

		function = item->data;

		function( itemNum );
	} else if ( item->flags & MIF_SWAPMENU ) {
		UI_SwapMenu( current, item->data );
		// warning, push menu replaces the current->items
	} else if ( item->flags & MIF_SUBMENU ) {
		UI_PushMenu( current, item->data );
		// warning, push menu replaces the current->items
	}

	// can use call and popmenu on the same item
	if ( popMenu ) {
		UI_PopMenu( current );
		return; // popmenu plays a sound
	}

	trap_S_StartLocalSound( uis.itemActionSound, CHAN_LOCAL_SOUND );
}

