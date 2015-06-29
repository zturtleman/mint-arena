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

// 256x256 image with characters that are 16x16
void CG_InitBitmapFont( fontInfo_t *font, int charHeight, int charWidth ) {
	int			i, col, row;
	const char 	*shaderName;
	qhandle_t	hShader;
	float		aspect;

	shaderName = "gfx/2d/bigchars";
	hShader = trap_R_RegisterShaderNoMip( shaderName );

	Com_sprintf( font->name, sizeof ( font->name ), "bitmapfont_%dx%d", charHeight, charWidth );
	font->glyphScale = 48.0f / 16;

	aspect = (float)charWidth / (float)charHeight;

	for ( i = 0; i < GLYPHS_PER_FONT; i++ ) {
		i &= 255;
		row = i>>4;
		col = i&15;

		font->glyphs[i].height = 16;
		font->glyphs[i].top = 16;
		font->glyphs[i].left = 0;
		font->glyphs[i].pitch = 16*aspect;
		font->glyphs[i].xSkip = 16*aspect;
		font->glyphs[i].imageWidth = 16*aspect;
		font->glyphs[i].imageHeight = 16;
		font->glyphs[i].s = col*0.0625; // note: 0.0625 = 16 / 256.0f
		font->glyphs[i].t = row*0.0625;
		font->glyphs[i].s2 = col*0.0625 + 0.0625;
		font->glyphs[i].t2 = row*0.0625 + 0.0625;
		font->glyphs[i].glyph = hShader;
		Q_strncpyz( font->glyphs[i].shaderName, shaderName, sizeof ( font->glyphs[i].shaderName ) );
	}
}

qboolean CG_InitTrueTypeFont( const char *name, int pointSize, fontInfo_t *font ) {
	int i;
	qboolean oldFont;

	trap_R_RegisterFont( name, pointSize, font );

	if ( !font->name[0] ) {
		return qfalse;
	}

	// check if it's a dynamicly rendered font (which has cursors)
	oldFont = qtrue;
	for ( i = 0; i < GLYPHS_PER_FONT; i++ ) {
		if ( font->glyphs[i].left != 0 ) {
			oldFont = qfalse;
			break;
		}
	}

	if ( oldFont ) {
		// Team Arena per-rendered fonts don't have cursor characters (they're just transparent space)
		Com_Memcpy( &font->glyphs[10], &font->glyphs[(int)'_'], sizeof ( glyphInfo_t ) );
		Com_Memcpy( &font->glyphs[11], &font->glyphs[(int)'|'], sizeof ( glyphInfo_t ) );

		// Make the '|' into a full width block
		font->glyphs[11].glyph = cgs.media.whiteShader;
		font->glyphs[11].imageWidth = font->glyphs[(int)'M'].left + font->glyphs[(int)'M'].xSkip;
		font->glyphs[11].s = 0;
		font->glyphs[11].s2 = 1;

		// character 13 is used as a selection marker in q3_ui
		Com_Memcpy( &font->glyphs[13], &font->glyphs[(int)'>'], sizeof ( glyphInfo_t ) );

		// Team Arena's fonts don't have character 255,
		// it's suppose to be a lowercase y with two dots over it
		if ( font->glyphs[255].shaderName[0] == '\0' ) {
			Com_Memcpy( &font->glyphs[255], &font->glyphs[(int)'y'], sizeof ( glyphInfo_t ) );
		}
	}

	return qtrue;
}

// Q3A UI also uses additional fonts (font1_prop, font2_prop, ..)
// Team Arena HUD/UI also use separate fonts (specified in .menu files).
// The truetype font names here are the same as in Team Arena (except tiny font that does not exist)
void CG_TextInit( void ) {
	cgs.media.smallFontHeight = 12;
	cgs.media.bigFontHeight = 20;

	if ( !CG_InitTrueTypeFont( "fonts/tinyfont", 8, &cgs.media.tinyFont ) ) {
		CG_InitBitmapFont( &cgs.media.tinyFont, 8, 8 );
	}

	if ( !CG_InitTrueTypeFont( "fonts/smallfont", 12, &cgs.media.smallFont ) ) {
		// quake 3 bitmap style
		cgs.media.smallFontHeight = 16;
		CG_InitBitmapFont( &cgs.media.smallFont, 16, 8 );
		// team arena truetype style
		//CG_InitBitmapFont( &cgs.media.smallFont, 12, 6 );
	}

	if ( !CG_InitTrueTypeFont( "fonts/font", 16, &cgs.media.textFont ) ) {
		CG_InitBitmapFont( &cgs.media.textFont, 16, 16 );
	}

	if ( !CG_InitTrueTypeFont( "fonts/bigfont", 20, &cgs.media.bigFont ) ) {
		// quake 3 bitmap style
		cgs.media.bigFontHeight = 48;
		CG_InitBitmapFont( &cgs.media.bigFont, 48, 32 );
		// team arena truetype style
		//CG_InitBitmapFont( &cgs.media.bigFont, 20, 10 );
	}
}

const glyphInfo_t *Text_GetGlyph( const fontInfo_t *font, unsigned long index ) {
	if ( index == 0 || index >= GLYPHS_PER_FONT )
		return &font->glyphs[(int)'.'];

	return &font->glyphs[index];
}

int Text_Width(const char *text, const fontInfo_t *font, float scale, int limit) {
  int count,len;
	float out;
	const glyphInfo_t *glyph;
	float useScale;
	unsigned long cp;
	const char *s = text;

	useScale = scale * font->glyphScale;
  out = 0;
  if (text) {
		len = Q_UTF8_PrintStrlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if ( Q_IsColorString(s) ) {
				s += 2;
				continue;
			} else {
				cp = Q_UTF8_CodePoint( &s );
				glyph = Text_GetGlyph( font, cp );
				out += glyph->xSkip;
				count++;
			}
    }
  }
  return out * useScale;
}

int Text_Height(const char *text, const fontInfo_t *font, float scale, int limit) {
  int len, count;
	float max;
	const glyphInfo_t *glyph;
	float useScale;
	unsigned long cp;
	const char *s = text;

	useScale = scale * font->glyphScale;
  max = 0;
  if (text) {
		len = Q_UTF8_PrintStrlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if ( Q_IsColorString(s) ) {
				s += 2;
				continue;
			} else {
				cp = Q_UTF8_CodePoint( &s );
				glyph = Text_GetGlyph( font, cp );
	      if (max < glyph->height) {
		      max = glyph->height;
			  }
				count++;
			}
    }
  }
  return max * useScale;
}

void Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader) {
  float w, h;
  w = width * scale;
  h = height * scale;
  CG_AdjustFrom640( &x, &y, &w, &h );
  trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

void Text_Paint(float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *text, float adjust, int limit, float shadowOffset, qboolean forceColor) {
  int len, count;
	vec4_t newColor;
	const glyphInfo_t *glyph;
	float yadj, xadj;
	float useScale;
	unsigned long cp;

	useScale = scale * font->glyphScale;
  if (text) {
    const char *s = text;
		trap_R_SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = Q_UTF8_PrintStrlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
      //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
			if ( Q_IsColorString( s ) ) {
				if ( !forceColor ) {
					memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
					newColor[3] = color[3];
					trap_R_SetColor( newColor );
				}
				s += 2;
				continue;
			}

			cp = Q_UTF8_CodePoint( &s );
			glyph = Text_GetGlyph( font, cp );

				yadj = useScale * glyph->top;
				xadj = useScale * glyph->left;
				if (shadowOffset) {
					colorBlack[3] = newColor[3];
					trap_R_SetColor( colorBlack );
					Text_PaintChar(x + xadj + shadowOffset, y - yadj + shadowOffset,
														glyph->imageWidth,
														glyph->imageHeight,
														useScale,
														glyph->s,
														glyph->t,
														glyph->s2,
														glyph->t2,
														glyph->glyph);
					trap_R_SetColor( newColor );
					colorBlack[3] = 1.0;
				}
				Text_PaintChar(x + xadj, y - yadj, 
													glyph->imageWidth,
													glyph->imageHeight,
													useScale,
													glyph->s,
													glyph->t,
													glyph->s2,
													glyph->t2,
													glyph->glyph);

				x += (glyph->xSkip * useScale) + adjust;
				count++;
    }
	  trap_R_SetColor( NULL );
  }
}

void Text_PaintWithCursor(float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *text, int cursorPos, char cursor, float adjust, int limit, float shadowOffset, qboolean forceColor) {
  int len, count;
	vec4_t newColor;
	const glyphInfo_t *glyph, *glyph2;
	float yadj, xadj;
	float useScale;
	unsigned long cp;

	useScale = scale * font->glyphScale;
  if (text) {
    const char *s = text;
		trap_R_SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = Q_UTF8_Strlen(text); // note: doesn't use Q_UTF8_PrintStrlen because this function draws color codes
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		glyph2 = &font->glyphs[cursor & 255];
		while (s && *s && count < len) {
      //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
			if ( Q_IsColorString( s ) ) {
				if ( !forceColor ) {
					memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
					newColor[3] = color[3];
					trap_R_SetColor( newColor );
				}
				// display color codes in edit fields
				//s += 2;
				//continue;
			}

			cp = Q_UTF8_CodePoint( &s );
			glyph = Text_GetGlyph( font, cp );

				if (count == cursorPos && !((cg.realTime/BLINK_DIVISOR) & 1)) {
					yadj = useScale * glyph2->top;

					Text_PaintChar(x, y - yadj,
														glyph->left + glyph->xSkip, // use horizontal width of text character
														glyph2->imageHeight,
														useScale,
														glyph2->s,
														glyph2->t,
														glyph2->s2,
														glyph2->t2,
														glyph2->glyph);
				}

				yadj = useScale * glyph->top;
				xadj = useScale * glyph->left;
				if (shadowOffset) {
					colorBlack[3] = newColor[3];
					trap_R_SetColor( colorBlack );
					Text_PaintChar(x + xadj + shadowOffset, y - yadj + shadowOffset,
														glyph->imageWidth,
														glyph->imageHeight,
														useScale,
														glyph->s,
														glyph->t,
														glyph->s2,
														glyph->t2,
														glyph->glyph);
					colorBlack[3] = 1.0;
					trap_R_SetColor( newColor );
				}

				// make overstrike cursor invert color
				if (count == cursorPos && !((cg.realTime/BLINK_DIVISOR) & 1) && cursor == 11) {
					// invert color
					vec4_t invertedColor;

					invertedColor[0] = 1.0f - newColor[0];
					invertedColor[1] = 1.0f - newColor[1];
					invertedColor[2] = 1.0f - newColor[2];
					invertedColor[3] = color[3];

					trap_R_SetColor( invertedColor );
				}

				Text_PaintChar(x + xadj, y - yadj, 
													glyph->imageWidth,
													glyph->imageHeight,
													useScale, 
													glyph->s,
													glyph->t,
													glyph->s2,
													glyph->t2,
													glyph->glyph);

				if (count == cursorPos && !((cg.realTime/BLINK_DIVISOR) & 1) && cursor == 11) {
					// restore color
					trap_R_SetColor( newColor );
				}

				x += (glyph->xSkip * useScale) + adjust;
				count++;
    }
    // need to paint cursor at end of text
    if (cursorPos == len && !((cg.realTime/BLINK_DIVISOR) & 1)) {
        yadj = useScale * glyph2->top;
        Text_PaintChar(x, y - yadj, 
                          glyph2->imageWidth,
                          glyph2->imageHeight,
                          useScale, 
                          glyph2->s,
                          glyph2->t,
                          glyph2->s2,
                          glyph2->t2,
                          glyph2->glyph);

    }

	  trap_R_SetColor( NULL );
  }
}


void Text_Paint_Limit(float *maxX, float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char* text, float adjust, int limit) {
  int len, count;
	vec4_t newColor;
	const glyphInfo_t *glyph;
  if (text) {
    const char *s = text;
		float max = *maxX;
		float yadj, xadj;
		float useScale;
		unsigned long cp;

		useScale = scale * font->glyphScale;
		trap_R_SetColor( color );
		len = Q_UTF8_PrintStrlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if ( Q_IsColorString( s ) ) {
				memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
				newColor[3] = color[3];
				trap_R_SetColor( newColor );
				s += 2;
				continue;
			}

			cp = Q_UTF8_CodePoint( &s );
			glyph = Text_GetGlyph( font, cp );

				yadj = useScale * glyph->top;
				xadj = useScale * glyph->left;
				if (Text_Width(s, font, useScale, 1) + x > max) {
					*maxX = 0;
					break;
				}
		    Text_PaintChar(x + xadj, y - yadj, 
			                 glyph->imageWidth,
				               glyph->imageHeight,
				               useScale, 
						           glyph->s,
								       glyph->t,
								       glyph->s2,
									     glyph->t2,
										   glyph->glyph);
	      x += (glyph->xSkip * useScale) + adjust;
				*maxX = x;
				count++;
		}
	  trap_R_SetColor( NULL );
  }

}

