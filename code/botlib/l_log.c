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

/*****************************************************************************
 * name:		l_log.c
 *
 * desc:		log file
 *
 * $Archive: /MissionPack/CODE/botlib/l_log.c $
 *
 *****************************************************************************/

#include "../qcommon/q_shared.h"
#include "botlib.h"
#include "be_interface.h"			//for botimport.Print
#include "l_libvar.h"
#include "l_log.h"

#define MAX_LOGFILENAMESIZE		1024

typedef struct logfile_s
{
	char filename[MAX_LOGFILENAMESIZE];
	qhandle_t fp;
	int numwrites;
} logfile_t;

logfile_t logfile;

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Log_Open(char *filename)
{
	if (!LibVarValue("log", "0")) return;
	if (!filename || !strlen(filename))
	{
		botimport.Print(PRT_MESSAGE, "openlog <filename>\n");
		return;
	} //end if
	if (logfile.fp)
	{
		botimport.Print(PRT_ERROR, "log file %s is already opened\n", logfile.filename);
		return;
	} //end if
	botimport.FS_FOpenFile(filename, &logfile.fp, FS_APPEND_SYNC);
	if (!logfile.fp)
	{
		botimport.Print(PRT_ERROR, "can't open the log file %s\n", filename);
		return;
	} //end if
	strncpy(logfile.filename, filename, MAX_LOGFILENAMESIZE);
	botimport.Print(PRT_MESSAGE, "Opened log %s\n", logfile.filename);
} //end of the function Log_Open
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Log_Close(void)
{
	if (!logfile.fp) return;
	botimport.FS_FCloseFile(logfile.fp);
	logfile.fp = 0;
	botimport.Print(PRT_MESSAGE, "Closed log %s\n", logfile.filename);
} //end of the function Log_Close
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Log_Shutdown(void)
{
	if (logfile.fp) Log_Close();
} //end of the function Log_Shutdown
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void QDECL Log_Write(char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	if ( !logfile.fp ) return;

	va_start( argptr, fmt );
	Q_vsnprintf( string, sizeof(string), fmt, argptr );
	va_end( argptr );

	botimport.FS_Write( string, strlen( string ), logfile.fp );
} //end of the function Log_Write
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void QDECL Log_WriteTimeStamped(char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	int			length;

	Com_sprintf( string, sizeof(string), "%d   %02d:%02d:%02d:%02d   ",
					logfile.numwrites,
					(int) (botlibglobals.time / 60 / 60),
					(int) (botlibglobals.time / 60),
					(int) (botlibglobals.time),
					(int) ((int) (botlibglobals.time * 100)) -
							((int) botlibglobals.time) * 100);

	// time stamp length
	length = strlen( string );

	va_start( argptr, fmt );
	Q_vsnprintf(string + length, sizeof(string) - length, fmt, argptr);
	va_end( argptr );

	Q_strcat( string, sizeof(string), "\r\n" );

	logfile.numwrites++;
	botimport.FS_Write( string, strlen( string ), logfile.fp );
} //end of the function Log_Write
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qhandle_t Log_FileHandle(void)
{
	return logfile.fp;
} //end of the function Log_FileHandle
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Log_Flush(void)
{

} //end of the function Log_Flush

