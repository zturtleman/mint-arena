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
#include "cg_local.h"
#include "../qcommon/q_unicode.h"

/*
===================
MField_Draw

Handles horizontal scrolling and cursor blinking
x, y, charWidth, charHeight, are in 640*480 virtual screen size
===================
*/
void MField_Draw( mfield_t *edit, int x, int y, int style, vec4_t color, qboolean drawCursor ) {
	int		i;
	int		len;
	int		drawLen;
	int		prestep;
	int		cursorChar;
	char	str[MAX_STRING_CHARS];

	drawLen = edit->widthInChars;
	len     = edit->len + 1;

	// guarantee that cursor will be visible
	if ( len <= drawLen ) {
		prestep = 0;
	} else {
		if ( edit->scroll + drawLen > len ) {
			edit->scroll = len - drawLen;
			if ( edit->scroll < 0 ) {
				edit->scroll = 0;
			}
		}
		prestep = edit->scroll;
	}

	if ( prestep + drawLen > len ) {
		drawLen = len - prestep;
	}

	// extract <drawLen> characters from the field at <prestep>
	str[0] = 0;
	for ( i = 0; i < drawLen; i++ ) {
		Q_strcat( str, sizeof( str ), Q_UTF8_Encode( edit->buffer[prestep+i] ) );
	}

	if ( drawCursor ) {
		// if overstrike and not at end of buffer
		if ( trap_Key_GetOverstrikeMode() && edit->cursor != edit->len ) {
			cursorChar = GLYPH_OVERSTRIKE;
		} else {
			cursorChar = GLYPH_INSERT;
		}
	} else {
		cursorChar = -1;
	}

	CG_DrawStringWithCursor( x, y, str, style, color, ( edit->cursor - prestep ), cursorChar );
}

/*
================
MField_Buffer

Returns a UTF-8 encoded string.
================
*/
const char *MField_Buffer( mfield_t *edit ) {
	static char	str[MAX_STRING_CHARS];
	int i;

	str[0] = 0;
	for ( i = 0; i < edit->len; i++ ) {
		Q_strcat( str, sizeof( str ), Q_UTF8_Encode( edit->buffer[i] ) );
	}

	return str;
}

/*
================
MField_AddText
================
*/
void MField_AddText( mfield_t *edit, const char *text ) {
	int codePoint;

	while ( *text ) {
		codePoint = Q_UTF8_CodePoint( &text );

		if ( codePoint != 0 ) {
			MField_CharEvent( edit, codePoint );
		}
	}
}

/*
================
MField_SetText
================
*/
void MField_SetText( mfield_t *edit, const char *text ) {
	MField_Clear( edit );
	MField_AddText( edit, text );
}

/*
================
MField_Paste
================
*/
void MField_Paste( mfield_t *edit ) {
	char	pasteBuffer[MAX_EDIT_LINE*4];

	trap_GetClipboardData( pasteBuffer, sizeof ( pasteBuffer ) );

	MField_AddText( edit, pasteBuffer );
}

/*
=================
MField_KeyDownEvent

Performs the basic line editing functions for the console,
in-game talk, and menu fields

Key events are used for non-printable characters, others are gotten from char events.
=================
*/
void MField_KeyDownEvent( mfield_t *edit, int key ) {
	// shift-insert is paste
	if ( ( ( key == K_INS ) || ( key == K_KP_INS ) ) && trap_Key_IsDown( K_SHIFT ) ) {
		MField_Paste( edit );
		return;
	}

	if ( key == K_DEL || key == K_KP_DEL ) {
		int i;

		if ( edit->cursor < edit->len ) {
			for ( i = edit->cursor; i < edit->len; i++ ) {
				edit->buffer[i] = edit->buffer[i+1];
			}
			edit->len--;
		}
		return;
	}

	if ( key == K_RIGHTARROW || key == K_KP_RIGHTARROW ) 
	{
		if ( edit->cursor < edit->len ) {
			edit->cursor++;
		}
		if ( edit->cursor >= edit->scroll + edit->widthInChars && edit->cursor <= edit->len )
		{
			edit->scroll++;
		}
		return;
	}

	if ( key == K_LEFTARROW || key == K_KP_LEFTARROW ) 
	{
		if ( edit->cursor > 0 ) {
			edit->cursor--;
		}
		if ( edit->cursor < edit->scroll )
		{
			edit->scroll--;
		}
		return;
	}

	if ( key == K_HOME || key == K_KP_HOME || ( tolower(key) == 'a' && trap_Key_IsDown( K_CTRL ) ) ) {
		edit->cursor = 0;
		edit->scroll = 0;
		return;
	}

	if ( key == K_END || key == K_KP_END || ( tolower(key) == 'e' && trap_Key_IsDown( K_CTRL ) ) ) {
		edit->cursor = edit->len;
		edit->scroll = edit->len - edit->widthInChars + 1;
		if (edit->scroll < 0)
			edit->scroll = 0;
		return;
	}

	if ( key == K_INS || key == K_KP_INS ) {
		trap_Key_SetOverstrikeMode( !trap_Key_GetOverstrikeMode() );
		return;
	}
}

/*
==================
MField_CharEvent
==================
*/
void MField_CharEvent( mfield_t *edit, int ch ) {
	int i;

	if ( ch == 'v' - 'a' + 1 ) {	// ctrl-v is paste
		MField_Paste( edit );
		return;
	}

	if ( ch == 'c' - 'a' + 1 ) {	// ctrl-c clears the field
		MField_Clear( edit );
		return;
	}

	if ( ch == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
		if ( edit->cursor > 0 ) {
			for ( i = edit->cursor; i < edit->len+1; i++ ) {
				edit->buffer[i-1] = edit->buffer[i];
			}

			edit->cursor--;
			if ( edit->cursor < edit->scroll )
			{
				edit->scroll--;
			}
			edit->len--;
		}
		return;
	}

	if ( ch == 'a' - 'a' + 1 ) {	// ctrl-a is home
		edit->cursor = 0;
		edit->scroll = 0;
		return;
	}

	if ( ch == 'e' - 'a' + 1 ) {	// ctrl-e is end
		edit->cursor = edit->len;
		edit->scroll = edit->cursor - edit->widthInChars + 1;
		if (edit->scroll < 0)
			edit->scroll = 0;
		return;
	}

	//
	// ignore any other non printable chars
	//
	if ( ch < 32 ) {
		return;
	}

	if ( trap_Key_GetOverstrikeMode() ) {
		if ((edit->cursor == MAX_EDIT_LINE - 1) || (edit->maxchars && edit->cursor >= edit->maxchars))
			return;
	} else {
		// insert mode
		if (( edit->len == MAX_EDIT_LINE - 1 ) || (edit->maxchars && edit->len >= edit->maxchars))
			return;

		for ( i = edit->len + 1; i >= edit->cursor; i-- ) {
			edit->buffer[i+1] = edit->buffer[i];
		}

		edit->len++;
	}

	edit->buffer[edit->cursor] = ch;
	if (!edit->maxchars || edit->cursor < edit->maxchars-1)
		edit->cursor++;

	if ( edit->cursor >= edit->widthInChars )
	{
		edit->scroll++;
	}

	if ( edit->cursor == edit->len + 1) {
		edit->buffer[edit->cursor] = 0;
		edit->len++;
	}
}

/*
==================
MField_Clear
==================
*/
void MField_Clear( mfield_t *edit ) {
	edit->buffer[0] = 0;
	edit->len = 0;
	edit->cursor = 0;
	edit->scroll = 0;
}

