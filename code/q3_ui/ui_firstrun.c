/*
===========================================================================
Copyright (C) 2009 Poul Sander
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

#include "ui_local.h"

#define ART_FRAMEL "menu/art/frame2_l"
#define ART_FRAMER "menu/art/frame1_r"
#define ART_FIGHT0 "menu/art/accept_0"
#define ART_FIGHT1 "menu/art/accept_1"

#define ID_GO				100
#define ID_NAME				101
#define ID_RATE				102
#define ID_ALLOWDOWNLOAD	103

#define MAX_NAMELENGTH 20

static const char *rate_items2[] = {
	"<= 28.8K",
	"33.6K",
	"56K",
	"ISDN",
	"LAN/Cable/xDSL",
	NULL
};

static char *art_artlist[] = {
	ART_FRAMEL,
	ART_FRAMER,
	ART_FIGHT0,
	ART_FIGHT1,
	NULL
};

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s framel;
	menubitmap_s framer;
	menubitmap_s go;
	menutext_s info;
	menutext_s info2;
	// important options
	menufield_s name;
	menulist_s rate;
	// optional options
	menuradiobutton_s allowdownload;
} firstrun_t;

static firstrun_t s_firstrun;

/*
=======================================================================================================================================
FirstRun_SetPlayerName
=======================================================================================================================================
*/
static void FirstRun_SetPlayerName(void *self) {
	menufield_s *f;
	qboolean focus;
	int style;
	float *color;
	int x, y;
	char name[32];

	f = (menufield_s *)self;
	x = f->generic.x;
	y = f->generic.y;
	focus = (f->generic.parent->cursor == f->generic.menuPosition);

	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;

	if (focus) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	UI_DrawProportionalString(x, y, "Name:", style, color);
	// draw the actual name
	x += 76;
	y += 4;

	if (focus) {
		style |= UI_FORCECOLOR;
	}

	MField_Draw(&f->field, x, y, style, colorWhite, focus);
}

/*
=======================================================================================================================================
FirstRun_StatusBar_Name
=======================================================================================================================================
*/
static void FirstRun_StatusBar_Name(void *ptr) {
	UI_DrawString(320, 410, "Your nickname", UI_CENTER|UI_SMALLFONT, colorWhite);
}

/*
=======================================================================================================================================
FirstRun_StatusBar_Rate
=======================================================================================================================================
*/
static void FirstRun_StatusBar_Rate(void *ptr) {
	UI_DrawString(320, 410, "Your connection speed", UI_CENTER|UI_SMALLFONT, colorWhite);
}

/*
=======================================================================================================================================
FirstRun_StatusBar_Download
=======================================================================================================================================
*/
static void FirstRun_StatusBar_Download(void *ptr) {
	UI_DrawString(320, 410, "Auto download missing maps and mods", UI_CENTER|UI_SMALLFONT, colorWhite);
}

/*
=======================================================================================================================================
FirstRun_SaveChanges
=======================================================================================================================================
*/
static void FirstRun_SaveChanges(void) {

	// name
	trap_Cvar_Set("name", MField_Buffer(&s_firstrun.name.field));
}

/*
=======================================================================================================================================
FirstRun_Event
=======================================================================================================================================
*/
static void FirstRun_Event(void *ptr, int event) {

	switch (((menucommon_s *)ptr)->id) {
		case ID_GO:
			if (event != QM_ACTIVATED) {
				break;
			}

			FirstRun_SaveChanges();

			UI_PopMenu();

			trap_Cvar_SetValue("ui_firstrun", 0);

			UI_ArenaServersMenu();
			break;
		case ID_RATE:
			if (s_firstrun.rate.curvalue == 0) {
				trap_Cvar_SetValue("rate", 2500);
			} else if (s_firstrun.rate.curvalue == 1) {
				trap_Cvar_SetValue("rate", 3000);
			} else if (s_firstrun.rate.curvalue == 2) {
				trap_Cvar_SetValue("rate", 4000);
			} else if (s_firstrun.rate.curvalue == 3) {
				trap_Cvar_SetValue("rate", 5000);
			} else if (s_firstrun.rate.curvalue == 4) {
				trap_Cvar_SetValue("rate", 25000);
			}

			break;
		case ID_ALLOWDOWNLOAD:
			trap_Cvar_SetValue("cl_allowDownload", s_firstrun.allowdownload.curvalue);
			trap_Cvar_SetValue("sv_allowDownload", s_firstrun.allowdownload.curvalue);
			break;
	}
}

/*
=======================================================================================================================================
FirstRun_SetMenuItems
=======================================================================================================================================
*/
static void FirstRun_SetMenuItems(void) {
	int rate;

	// name
	MField_SetText(&s_firstrun.name.field, CG_Cvar_VariableString("name"));

	rate = trap_Cvar_VariableValue("rate");

	if (rate <= 2500) {
		s_firstrun.rate.curvalue = 0;
	} else if (rate <= 3000) {
		s_firstrun.rate.curvalue = 1;
	} else if (rate <= 4000) {
		s_firstrun.rate.curvalue = 2;
	} else if (rate <= 5000) {
		s_firstrun.rate.curvalue = 3;
	} else {
		s_firstrun.rate.curvalue = 4;
	}

	s_firstrun.allowdownload.curvalue = trap_Cvar_VariableValue("cl_allowDownload") != 0;
}

/*
=======================================================================================================================================
FirstRun_MenuInit
=======================================================================================================================================
*/
void FirstRun_MenuInit(void) {
	int y;

	// zero set all our globals
	memset(&s_firstrun, 0, sizeof(firstrun_t));

	FirstRun_Cache();

	s_firstrun.menu.wrapAround = qtrue;
	s_firstrun.menu.fullscreen = qtrue;

	s_firstrun.banner.generic.type = MTYPE_BTEXT;
	s_firstrun.banner.generic.x = 320;
	s_firstrun.banner.generic.y = 16;
	s_firstrun.banner.string = "BASIC SETTINGS";
	s_firstrun.banner.color = color_white;
	s_firstrun.banner.style = UI_CENTER;

	s_firstrun.framel.generic.type = MTYPE_BITMAP;
	s_firstrun.framel.generic.name = ART_FRAMEL;
	s_firstrun.framel.generic.flags = QMF_INACTIVE;
	s_firstrun.framel.generic.x = 0;
	s_firstrun.framel.generic.y = 78;
	s_firstrun.framel.width = 256;
	s_firstrun.framel.height = 329;

	s_firstrun.framer.generic.type = MTYPE_BITMAP;
	s_firstrun.framer.generic.name = ART_FRAMER;
	s_firstrun.framer.generic.flags = QMF_INACTIVE;
	s_firstrun.framer.generic.x = 376;
	s_firstrun.framer.generic.y = 76;
	s_firstrun.framer.width = 256;
	s_firstrun.framer.height = 334;

	s_firstrun.go.generic.type = MTYPE_BITMAP;
	s_firstrun.go.generic.name = ART_FIGHT0;
	s_firstrun.go.generic.flags = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_firstrun.go.generic.callback = FirstRun_Event;
	s_firstrun.go.generic.id = ID_GO;
	s_firstrun.go.generic.x = 640;
	s_firstrun.go.generic.y = 480 - 64;
	s_firstrun.go.width = 128;
	s_firstrun.go.height = 64;
	s_firstrun.go.focuspic = ART_FIGHT1;

	y = 144;
	s_firstrun.name.generic.type = MTYPE_FIELD;
	s_firstrun.name.generic.flags = QMF_NODEFAULTINIT;
	s_firstrun.name.generic.ownerdraw = FirstRun_SetPlayerName;
	s_firstrun.name.field.widthInChars = MAX_NAMELENGTH;
	s_firstrun.name.field.maxchars = MAX_NAMELENGTH;
	s_firstrun.name.generic.x = 192;
	s_firstrun.name.generic.y = y;
	s_firstrun.name.generic.left = 192 - 8;
	s_firstrun.name.generic.top = y - 8;
	s_firstrun.name.generic.right = 192 + 200;
	s_firstrun.name.generic.bottom = y + 2 * PROP_HEIGHT;
	s_firstrun.name.generic.statusbar = FirstRun_StatusBar_Name;

	y += 4 * PROP_HEIGHT;
	s_firstrun.rate.generic.type = MTYPE_SPINCONTROL;
	s_firstrun.rate.generic.name = "Data Rate:";
	s_firstrun.rate.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_firstrun.rate.generic.callback = FirstRun_Event;
	s_firstrun.rate.generic.id = ID_RATE;
	s_firstrun.rate.generic.x = 320;
	s_firstrun.rate.generic.y = y;
	s_firstrun.rate.itemnames = rate_items2;
	s_firstrun.rate.generic.statusbar = FirstRun_StatusBar_Rate;

	y += BIGCHAR_HEIGHT + 2;
	s_firstrun.allowdownload.generic.type = MTYPE_RADIOBUTTON;
	s_firstrun.allowdownload.generic.name = "Automatic Downloading:";
	s_firstrun.allowdownload.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_firstrun.allowdownload.generic.callback = FirstRun_Event;
	s_firstrun.allowdownload.generic.id = ID_ALLOWDOWNLOAD;
	s_firstrun.allowdownload.generic.x = 320;
	s_firstrun.allowdownload.generic.y = y;
	s_firstrun.allowdownload.generic.statusbar = FirstRun_StatusBar_Download;

	s_firstrun.info.generic.type = MTYPE_TEXT;
	s_firstrun.info.generic.x = 320;
	s_firstrun.info.generic.y = 446;
	s_firstrun.info.color = color_white;
	s_firstrun.info.style = UI_CENTER|UI_TINYFONT;
	s_firstrun.info.string = "Note: All settings can be changed later in SETUP";

	s_firstrun.info2.generic.type = MTYPE_TEXT;
	s_firstrun.info2.generic.x = 320;
	s_firstrun.info2.generic.y = 100;
	s_firstrun.info2.color = color_white;
	s_firstrun.info2.style = UI_CENTER|UI_SMALLFONT;
	s_firstrun.info2.string = "Please verify these settings";

	Menu_AddItem(&s_firstrun.menu, &s_firstrun.banner);
	Menu_AddItem(&s_firstrun.menu, &s_firstrun.framel);
	Menu_AddItem(&s_firstrun.menu, &s_firstrun.framer);
	Menu_AddItem(&s_firstrun.menu, &s_firstrun.go);
	Menu_AddItem(&s_firstrun.menu, &s_firstrun.name);
	Menu_AddItem(&s_firstrun.menu, &s_firstrun.rate);
	Menu_AddItem(&s_firstrun.menu, &s_firstrun.allowdownload);
	Menu_AddItem(&s_firstrun.menu, &s_firstrun.info);
	Menu_AddItem(&s_firstrun.menu, &s_firstrun.info2);

	FirstRun_SetMenuItems();
}

/*
=======================================================================================================================================
FirstRun_Cache
=======================================================================================================================================
*/
void FirstRun_Cache(void) {
	int i;

	// touch all our pics
	for (i = 0;; i++) {
		if (!art_artlist[i]) {
			break;
		}

		trap_R_RegisterShaderNoMip(art_artlist[i]);
	}
}

/*
=======================================================================================================================================
UI_FirstRunMenu
=======================================================================================================================================
*/
void UI_FirstRunMenu(void) {

	FirstRun_MenuInit();
	UI_PushMenu(&s_firstrun.menu);
}
