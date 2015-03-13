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
 * name:		ai_ea.c
 *
 * desc:		elementary actions
 *
 * $Archive: /MissionPack/code/game/ai_ea.c $
 *
 *****************************************************************************/

#include "g_local.h"
#include "../botlib/botlib.h"
#include "../botlib/be_aas.h"
//
#include "ai_char.h"
#include "ai_chat_sys.h"
#include "ai_ea.h"
#include "ai_gen.h"
#include "ai_goal.h"
#include "ai_move.h"
#include "ai_weap.h"
#include "ai_weight.h"
//
#include "ai_main.h"
#include "ai_dmq3.h"
#include "ai_chat.h"
#include "ai_cmd.h"
#include "ai_vcmd.h"
#include "ai_dmnet.h"
#include "ai_team.h"
//
#include "chars.h"				//characteristics
#include "inv.h"				//indexes into the inventory
#include "syn.h"				//synonyms
#include "match.h"				//string matching types and vars

#define MAX_USERMOVE				400
#define MAX_COMMANDARGUMENTS		10

bot_input_t botinputs[MAX_CLIENTS];

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_Say(int playerNum, char *str)
{
	trap_ClientCommand(playerNum, va("say %s", str));
} //end of the function EA_Say
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_SayTeam(int playerNum, char *str)
{
	trap_ClientCommand(playerNum, va("say_team %s", str));
} //end of the function EA_SayTeam
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_Tell(int playerNum, int playerto, char *str)
{
	trap_ClientCommand(playerNum, va("tell %d, %s", playerto, str));
} //end of the function EA_SayTeam
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_UseItem(int playerNum, char *it)
{
	trap_ClientCommand(playerNum, va("use %s", it));
} //end of the function EA_UseItem
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_DropItem(int playerNum, char *it)
{
	trap_ClientCommand(playerNum, va("drop %s", it));
} //end of the function EA_DropItem
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_UseInv(int playerNum, char *inv)
{
	trap_ClientCommand(playerNum, va("invuse %s", inv));
} //end of the function EA_UseInv
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_DropInv(int playerNum, char *inv)
{
	trap_ClientCommand(playerNum, va("invdrop %s", inv));
} //end of the function EA_DropInv
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Gesture(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_GESTURE;
} //end of the function EA_Gesture
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_Command(int playerNum, char *command)
{
	trap_ClientCommand(playerNum, command);
} //end of the function EA_Command
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_SelectWeapon(int playerNum, int weapon)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->weapon = weapon;
} //end of the function EA_SelectWeapon
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Attack(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_ATTACK;
} //end of the function EA_Attack
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Talk(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_TALK;
} //end of the function EA_Talk
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Use(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_USE;
} //end of the function EA_Use
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Respawn(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_RESPAWN;
} //end of the function EA_Respawn
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Jump(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	if (bi->actionflags & ACTION_JUMPEDLASTFRAME)
	{
		bi->actionflags &= ~ACTION_JUMP;
	} //end if
	else
	{
		bi->actionflags |= ACTION_JUMP;
	} //end if
} //end of the function EA_Jump
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_DelayedJump(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	if (bi->actionflags & ACTION_JUMPEDLASTFRAME)
	{
		bi->actionflags &= ~ACTION_DELAYEDJUMP;
	} //end if
	else
	{
		bi->actionflags |= ACTION_DELAYEDJUMP;
	} //end if
} //end of the function EA_DelayedJump
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Crouch(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_CROUCH;
} //end of the function EA_Crouch
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Action(int playerNum, int action)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= action;
} //end of function EA_Action
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveUp(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_MOVEUP;
} //end of the function EA_MoveUp
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveDown(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_MOVEDOWN;
} //end of the function EA_MoveDown
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveForward(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_MOVEFORWARD;
} //end of the function EA_MoveForward
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveBack(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_MOVEBACK;
} //end of the function EA_MoveBack
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveLeft(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_MOVELEFT;
} //end of the function EA_MoveLeft
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveRight(int playerNum)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	bi->actionflags |= ACTION_MOVERIGHT;
} //end of the function EA_MoveRight
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Move(int playerNum, vec3_t dir, float speed)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	VectorCopy(dir, bi->dir);
	//cap speed
	if (speed > MAX_USERMOVE) speed = MAX_USERMOVE;
	else if (speed < -MAX_USERMOVE) speed = -MAX_USERMOVE;
	bi->speed = speed;

	if (speed <= 200) {
		bi->actionflags |= ACTION_WALK;
	}
} //end of the function EA_Move
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_View(int playerNum, vec3_t viewangles)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];

	VectorCopy(viewangles, bi->viewangles);
} //end of the function EA_View
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_EndRegular(int playerNum, float thinktime)
{
} //end of the function EA_EndRegular
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_GetInput(int playerNum, float thinktime, bot_input_t *input)
{
	bot_input_t *bi;

	bi = &botinputs[playerNum];
	bi->thinktime = thinktime;
	Com_Memcpy(input, bi, sizeof(bot_input_t));
} //end of the function EA_GetInput
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_ResetInput(int playerNum)
{
	bot_input_t *bi;
	int jumped = qfalse;

	bi = &botinputs[playerNum];

	bi->thinktime = 0;
	VectorClear(bi->dir);
	bi->speed = 0;
	jumped = bi->actionflags & ACTION_JUMP;
	bi->actionflags = 0;
	if (jumped) bi->actionflags |= ACTION_JUMPEDLASTFRAME;
} //end of the function EA_ResetInput
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int EA_Setup(void)
{
	//initialize the bot inputs
	Com_Memset(botinputs, 0, sizeof (botinputs));
	return BLERR_NOERROR;
} //end of the function EA_Setup
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Shutdown(void)
{

} //end of the function EA_Shutdown
