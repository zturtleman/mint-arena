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

void UI_SetMenu( currentMenu_t *current, menudef_t *menu ) {
	current->numStacked = 0;
	current->menu = menu;
	current->selectedItem = 0;
}

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
}

void UI_PopMenu( currentMenu_t *current ) {
	if ( current->numStacked == 0 ) {
		if ( cg.connected ) {
			trap_Mouse_SetState( 0, MOUSE_CLIENT );
			trap_Key_SetCatcher( 0 );
			trap_Cvar_SetValue( "cl_paused", 0 );

			current->menu = NULL;
		}
		return;
	}

	// pop stack
	current->numStacked--;
	current->menu = current->stack[current->numStacked].menu;
	current->selectedItem = current->stack[current->numStacked].selectedItem;

	trap_S_StartLocalSound( uis.menuPopSound, CHAN_LOCAL_SOUND );
}

// dir -1 = up, 1 = down
void UI_MenuAdjustCursor( currentMenu_t *current, int dir ) {
	trap_S_StartLocalSound( uis.itemFocusSound, CHAN_LOCAL_SOUND );

	current->selectedItem += dir;

	if ( current->selectedItem >= current->menu->numItems )
		current->selectedItem = 0;
	else if ( current->selectedItem < 0 )
		current->selectedItem = current->menu->numItems - 1;
}

void UI_MenuAction( currentMenu_t *current, int itemNum ) {
	menuitem_t *item = &current->menu->items[itemNum];

	if ( item->flags & MIF_CALL ) {
		void    (*function)( int item );

		function = item->data;

		function( itemNum );
	} else if ( item->flags & MIF_SUBMENU ) {
		UI_PushMenu( current, item->data );
	} else {
		// no sound
		return;
	}

	trap_S_StartLocalSound( uis.itemActionSound, CHAN_LOCAL_SOUND );
}


