/*
===========================================================================
Copyright (C) 2012-2013 Unvanquished Developers

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

#ifndef Q_UNICODE_H_
#define Q_UNICODE_H_

int Q_UTF8_Width( const char *str );
int Q_UTF8_WidthCP( int ch );
int Q_UTF8_Strlen( const char *str );
int Q_UTF8_PrintStrlen( const char *str );
qboolean Q_UTF8_ContByte( char c );
unsigned long Q_UTF8_CodePoint( const char **str );
char *Q_UTF8_Encode( unsigned long codepoint );
int Q_UTF8_Store( const char *s );
char *Q_UTF8_Unstore( int e );

qboolean Q_Unicode_IsAlpha( int ch );
qboolean Q_Unicode_IsUpper( int ch );
qboolean Q_Unicode_IsLower( int ch );
qboolean Q_Unicode_IsIdeo( int ch );
qboolean Q_Unicode_IsAlphaOrIdeo( int ch );
qboolean Q_Unicode_IsAlphaOrIdeoOrDigit( int ch );

int Q_Unicode_ToUpper( int ch );
int Q_Unicode_ToLower( int ch );

#endif /* Q_UNICODE_H_ */
