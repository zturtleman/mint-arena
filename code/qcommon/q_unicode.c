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

// Unicode & UTF-8 handling

#include "q_shared.h"
#include "q_unicode.h"

#ifdef Q3_VM
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
#endif

// never returns more than 4
int Q_UTF8_Width( const char *str )
{
  int                 ewidth;
  const unsigned char *s = (const unsigned char *)str;

  if( !str )
    return 0;

  if     (               *s <= 0x7F )
    ewidth = 0;
  else if( 0xC2 <= *s && *s <= 0xDF )
    ewidth = 1;
  else if( 0xE0 <= *s && *s <= 0xEF )
    ewidth = 2;
  else if( 0xF0 <= *s && *s <= 0xF4 )
    ewidth = 3;
  else
    ewidth = 0;

  for( ; *s && ewidth > 0; s++, ewidth-- );

  return s - (const unsigned char *)str + 1;
}

int Q_UTF8_WidthCP( int ch )
{
	if ( ch <=   0x007F ) { return 1; }
	if ( ch <=   0x07FF ) { return 2; }
	if ( ch <=   0xFFFF ) { return 3; }
	if ( ch <= 0x10FFFF ) { return 4; }
	return 0;
}

int Q_UTF8_Strlen( const char *str )
{
  int l = 0;

  while( *str )
  {
    l++;

    str += Q_UTF8_Width( str );
  }

  return l;
}

int Q_UTF8_PrintStrlen( const char *str )
{
  int l = 0;

  while( *str )
  {
    if( Q_IsColorString( str ) )
    {
      str += 2;
      continue;
    }
    if( *str == Q_COLOR_ESCAPE && str[1] == Q_COLOR_ESCAPE )
    {
      ++str;
    }

    l++;

    str += Q_UTF8_Width( str );
  }

  return l;
}

qboolean Q_UTF8_ContByte( char c )
{
  return (unsigned char )0x80 <= (unsigned char)c && (unsigned char)c <= (unsigned char )0xBF;
}

// step str over next unicode character and return the character code point
unsigned long Q_UTF8_CodePoint( const char **str )
{
  unsigned long utf32 = 0;
  const char *c;

  c = *str;

  // Quick and dirty UTF-8 to UTF-32 conversion
  if( ( *c & 0x80 ) == 0 )
    utf32 = *c++;
  else if( ( *c & 0xE0 ) == 0xC0 ) // 110x xxxx
  {
    utf32 |= ( *c++ & 0x1F ) << 6;
    utf32 |= ( *c++ & 0x3F );
  }
  else if( ( *c & 0xF0 ) == 0xE0 ) // 1110 xxxx
  {
    utf32 |= ( *c++ & 0x0F ) << 12;
    utf32 |= ( *c++ & 0x3F ) << 6;
    utf32 |= ( *c++ & 0x3F );
  }
  else if( ( *c & 0xF8 ) == 0xF0 ) // 1111 0xxx
  {
    utf32 |= ( *c++ & 0x07 ) << 18;
    utf32 |= ( *c++ & 0x3F ) << 12;
    utf32 |= ( *c++ & 0x3F ) << 6;
    utf32 |= ( *c++ & 0x3F );
  }
  else
  {
    // Unrecognised UTF-8 lead byte.
    // Don't print a debug message as it may cause an infinite loop / segfault.
    c++;
  }

  *str = c;

  return utf32;
}

char *Q_UTF8_Encode( unsigned long codepoint )
{
  static char sbuf[2][5];
  static int index = 0;
  char *buf = sbuf[index++ & 1];

  if     (                        codepoint <= 0x007F )
  {
    buf[0] = codepoint;
    buf[1] = 0;
  }
  else if( 0x0080 <= codepoint && codepoint <= 0x07FF )
  {
    buf[0] = 0xC0 | ((codepoint & 0x07C0) >> 6);
    buf[1] = 0x80 | (codepoint & 0x003F);
    buf[2] = 0;
  }
  else if( 0x0800 <= codepoint && codepoint <= 0xFFFF )
  {
    buf[0] = 0xE0 | ((codepoint & 0xF000) >> 12);
    buf[1] = 0x80 | ((codepoint & 0x0FC0) >> 6);
    buf[2] = 0x80 | (codepoint & 0x003F);
    buf[3] = 0;
  }
  else if( 0x010000 <= codepoint && codepoint <= 0x10FFFF )
  {
    buf[0] = 0xF0 | ((codepoint & 0x1C0000) >> 18);
    buf[1] = 0x80 | ((codepoint & 0x03F000) >> 12);
    buf[2] = 0x80 | ((codepoint & 0x000FC0) >> 6);
    buf[3] = 0x80 | (codepoint & 0x00003F);
    buf[4] = 0;
  }
  else
  {
    buf[0] = 0;
  }

  return buf;
}

// stores a single UTF8 char inside an int
int Q_UTF8_Store( const char *s )
{
	int r = 0;
	const uint8_t *us = ( const uint8_t * ) s;

	if ( !us )
	{
		return 0;
	}

	if ( !( us[ 0 ] & 0x80 ) ) // 0xxxxxxx
	{
		r = us[ 0 ];
	}
	else if ( ( us[ 0 ] & 0xE0 ) == 0xC0 ) // 110xxxxx
	{
		r = us[ 0 ];
		r |= ( uint32_t ) us[ 1 ] << 8;
	}
	else if ( ( us[ 0 ] & 0xF0 ) == 0xE0 ) // 1110xxxx
	{
		r = us[ 0 ];
		r |= ( uint32_t ) us[ 1 ] << 8;
		r |= ( uint32_t ) us[ 2 ] << 16;
	}
	else if ( ( us[ 0 ] & 0xF8 ) == 0xF0 ) // 11110xxx
	{
		r = us[ 0 ];
		r |= ( uint32_t ) us[ 1 ] << 8;
		r |= ( uint32_t ) us[ 2 ] << 16;
		r |= ( uint32_t ) us[ 3 ] << 24;
	}

	return r;
}

// converts a single UTF8 char stored as an int into a byte array
char *Q_UTF8_Unstore( int e )
{
	static unsigned char sbuf[2][5];
	static int index = 0;
	unsigned char *buf;

	index = ( index + 1 ) & 1;
	buf = sbuf[ index ];

	buf[ 0 ] = e & 0xFF;
	buf[ 1 ] = ( e >> 8 ) & 0xFF;
	buf[ 2 ] = ( e >> 16 ) & 0xFF;
	buf[ 3 ] = ( e >> 24 ) & 0xFF;
	buf[ 4 ] = 0;

	return ( char * ) buf;
}


#include "unicode_data.h"

static int uc_search_range( const void *chp, const void *memb )
{
  int ch = *(int *)chp;
  const ucs2_pair_t *item = (ucs2_pair_t*) memb;

  return ( ch < item->c1 ) ? -1 : ( ch >= item->c2 ) ? 1 : 0;
}

#define Q_UC_IS(label, array) \
  qboolean Q_Unicode_Is##label( int ch ) \
  { \
    return bsearch( &ch, array, ARRAY_LEN( array ), sizeof( array[ 0 ] ), uc_search_range ) ? qtrue : qfalse; \
  }

Q_UC_IS( Alpha, uc_prop_alphabetic  )
Q_UC_IS( Upper, uc_prop_uppercase   )
Q_UC_IS( Lower, uc_prop_lowercase   )
Q_UC_IS( Ideo,  uc_prop_ideographic )
Q_UC_IS( Digit, uc_prop_digit       )

qboolean Q_Unicode_IsAlphaOrIdeo( int ch )
{
  return Q_Unicode_IsAlpha( ch ) || Q_Unicode_IsIdeo( ch );
}

qboolean Q_Unicode_IsAlphaOrIdeoOrDigit( int ch )
{
  return Q_Unicode_IsAlpha( ch ) || Q_Unicode_IsIdeo( ch ) || Q_Unicode_IsDigit( ch );
}

static int uc_search_cp( const void *chp, const void *memb )
{
  int ch = *(int *)chp;
  const ucs2_pair_t *item = (ucs2_pair_t*) memb;

  return ( ch < item->c1 ) ? -1 : ( ch > item->c1 ) ? 1 : 0;
}

#define Q_UC_TO(label, array) \
  int Q_Unicode_To##label( int ch ) \
  { \
    const ucs2_pair_t *converted = (ucs2_pair_t*) bsearch( &ch, array, ARRAY_LEN( array ), sizeof( array[ 0 ] ), uc_search_cp ); \
    return converted ? converted->c2 : ch; \
  }

Q_UC_TO( Upper, uc_case_upper )
Q_UC_TO( Lower, uc_case_lower )
