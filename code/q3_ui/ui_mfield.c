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
#include "ui_local.h"

/*
==================
MenuField_Init
==================
*/
void MenuField_Init( menufield_s* m ) {
	int	l;
	int	w;
	int	h;
	int	style;

	MField_Clear( &m->field );

	if (m->generic.flags & QMF_SMALLFONT)
	{
		w = SMALLCHAR_WIDTH;
		h = SMALLCHAR_HEIGHT;
		style = UI_SMALLFONT;
	}
	else
	{
		w = BIGCHAR_WIDTH;
		h = BIGCHAR_HEIGHT;
		style = UI_BIGFONT;
	}	

	l = CG_DrawStrlen( m->generic.name, style ) + w;

	m->generic.left   = m->generic.x - l;
	m->generic.top    = m->generic.y;
	m->generic.right  = m->generic.x + w + m->field.widthInChars*w;
	m->generic.bottom = m->generic.y + h;
}

/*
==================
MenuField_Draw
==================
*/
void MenuField_Draw( menufield_s *f )
{
	int		x;
	int		y;
	int		w;
	int		style;
	qboolean focus;
	float	*color;

	x =	f->generic.x;
	y =	f->generic.y;

	if (f->generic.flags & QMF_SMALLFONT)
	{
		w = SMALLCHAR_WIDTH;
		style = UI_SMALLFONT;
	}
	else
	{
		w = BIGCHAR_WIDTH;
		style = UI_BIGFONT;
	}	

	if (Menu_ItemAtCursor( f->generic.parent ) == f) {
		focus = qtrue;
		style |= UI_PULSE;
	}
	else {
		focus = qfalse;
	}

	if (f->generic.flags & QMF_GRAYED)
		color = text_color_disabled;
	else if (focus)
		color = text_color_highlight;
	else
		color = text_color_normal;

	if ( focus )
	{
		// draw cursor
		CG_FillRect( f->generic.left, f->generic.top, f->generic.right-f->generic.left+1, f->generic.bottom-f->generic.top+1, listbar_color ); 
		UI_DrawChar( x, y, GLYPH_ARROW, UI_CENTER|UI_BLINK|style, color);
	}

	if ( f->generic.name ) {
		UI_DrawString( x - w, y, f->generic.name, style|UI_RIGHT, color );
	}

	MField_Draw( &f->field, x + w, y, style, color, focus );
}

/*
==================
MenuField_Key
==================
*/
sfxHandle_t MenuField_Key( menufield_s* m, int* key )
{
	int keycode;

	keycode = *key;

	switch ( keycode )
	{
		case K_KP_ENTER:
		case K_ENTER:
		case K_JOY_A:
		case K_2JOY_A:
		case K_3JOY_A:
		case K_4JOY_A:
			// have enter go to next cursor point
			*key = K_TAB;
			break;

		case K_TAB:
		case K_KP_DOWNARROW:
		case K_DOWNARROW:
		case K_KP_UPARROW:
		case K_UPARROW:
			break;

		default:
			if ( keycode & K_CHAR_FLAG )
			{
				keycode &= ~K_CHAR_FLAG;

				if ((m->generic.flags & QMF_UPPERCASE) && Q_islower( keycode ))
					keycode -= 'a' - 'A';
				else if ((m->generic.flags & QMF_LOWERCASE) && Q_isupper( keycode ))
					keycode -= 'A' - 'a';
				else if ((m->generic.flags & QMF_NUMBERSONLY) && Q_isalpha( keycode ))
					return (menu_buzz_sound);

				MField_CharEvent( &m->field, keycode);
			}
			else
				MField_KeyDownEvent( &m->field, keycode );
			break;
	}

	return (0);
}


