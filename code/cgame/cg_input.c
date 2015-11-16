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
// usercmd_t creation

#include "cg_local.h"

unsigned in_frameMsec;
int in_frameTime;

vmCvar_t	cg_freelook;

vmCvar_t	m_pitch;
vmCvar_t	m_yaw;
vmCvar_t	m_forward;
vmCvar_t	m_side;

vmCvar_t	cg_yawspeed[MAX_SPLITVIEW];
vmCvar_t	cg_pitchspeed[MAX_SPLITVIEW];

vmCvar_t	cg_yawspeedanalog[MAX_SPLITVIEW];
vmCvar_t	cg_pitchspeedanalog[MAX_SPLITVIEW];

vmCvar_t	cg_anglespeedkey[MAX_SPLITVIEW];

vmCvar_t	cg_run[MAX_SPLITVIEW];

vmCvar_t	cg_joystickUseAnalog[MAX_SPLITVIEW];
vmCvar_t	cg_joystickThreshold[MAX_SPLITVIEW];

/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as argv(1) so it can be matched up with the release.

argv(2) will be set to the time the event happened, which allows exact
control even at low framerates when the down and up events may both get qued
at the same time.

===============================================================================
*/

typedef struct {
	int			down[2];		// key nums holding it down
	int			joystickNum[2];	// player joystick number or -1 if key is not on a joystick
	int			axisNum[2];		// analog joystick axis + 1 (possibly negated for negative axis)
	unsigned	downtime;		// msec timestamp
	unsigned	msec;			// msec down this frame if both a down and up happened
	qboolean	active;			// current state
	qboolean	wasPressed;		// set when down, not cleared when up
	float		lastFraction[2];	// last fraction
} kbutton_t;

typedef struct
{
	kbutton_t	in_left, in_right, in_forward, in_back;
	kbutton_t	in_lookup, in_lookdown, in_moveleft, in_moveright;
	kbutton_t	in_strafe, in_speed;
	kbutton_t	in_up, in_down;

	kbutton_t	in_buttons[15];

	qboolean	in_mlooking;
} clientInput_t;

clientInput_t cis[MAX_SPLITVIEW];

float CL_AxisFraction( int localPlayerNum, int joystickNum, int axisNum );

void IN_MLookDown( int localPlayerNum ) {
	cis[localPlayerNum].in_mlooking = qtrue;
}

void IN_MLookUp( int localPlayerNum ) {
	cis[localPlayerNum].in_mlooking = qfalse;
	if ( !cg_freelook.integer ) {
		IN_CenterView ( localPlayerNum );
	}
}

// arg1 keynum, arg2 frametime, arg3 analog joystick axis number
void IN_KeyDown( kbutton_t *b ) {
	int		k;
	int		localPlayerNum;
	int		joystickNum;
	int		axisNum;
	char	c[20];
	
	trap_Argv( 1, c, sizeof (c) );
	if ( c[0] ) {
		k = atoi(c);
	} else {
		k = -1;		// typed manually at the console for continuous down
	}

	if ( k == b->down[0] || k == b->down[1] ) {
		return;		// repeating key
	}
	
	trap_Argv( 3, c, sizeof (c) );
	joystickNum = *c ? atoi( c ) : -1;

	trap_Argv( 4, c, sizeof (c) );
	axisNum = atoi( c );

	if ( joystickNum < 0 || joystickNum >= MAX_SPLITVIEW || ( abs( axisNum ) - 1 ) >= MAX_JOYSTICK_AXIS ) {
		joystickNum = -1;
		axisNum = 0;
	}

	localPlayerNum = Com_LocalPlayerForCvarName( CG_Argv( 0 ) );

	if ( !b->down[0] ) {
		b->down[0] = k;
		b->joystickNum[0] = joystickNum;
		b->axisNum[0] = axisNum;
		b->lastFraction[0] = CL_AxisFraction( localPlayerNum, joystickNum, axisNum );
	} else if ( !b->down[1] ) {
		b->down[1] = k;
		b->joystickNum[1] = joystickNum;
		b->axisNum[1] = axisNum;
		b->lastFraction[1] = CL_AxisFraction( localPlayerNum, joystickNum, axisNum );
	} else {
		Com_Printf ("Three keys down for a button!\n");
		return;
	}
	
	if ( b->active ) {
		return;		// still down
	}

	// save timestamp for partial frame summing
	trap_Argv( 2, c, sizeof (c) );
	b->downtime = atoi(c);

	b->active = qtrue;
	b->wasPressed = qtrue;
}

void IN_KeyUp( kbutton_t *b ) {
	int		k;
	char	c[20];
	unsigned	uptime;
	
	trap_Argv( 1, c, sizeof (c) );
	if ( c[0] ) {
		k = atoi(c);
	} else {
		// typed manually at the console, assume for unsticking, so clear all
		b->down[0] = b->down[1] = 0;
		b->active = qfalse;
		return;
	}

	if ( b->down[0] == k ) {
		b->down[0] = 0;
	} else if ( b->down[1] == k ) {
		b->down[1] = 0;
	} else {
		return;		// key up without coresponding down (menu pass through)
	}
	if ( b->down[0] || b->down[1] ) {
		return;		// some other key is still holding it down
	}

	b->active = qfalse;

	// save timestamp for partial frame summing
	trap_Argv( 2, c, sizeof (c) );
	uptime = atoi(c);
	if ( uptime ) {
		b->msec += uptime - b->downtime;
	} else {
		b->msec += in_frameMsec / 2;
	}

	b->active = qfalse;
}



/*
===============
CL_AxisFraction

Returns the fraction of the axis press
===============
*/
float CL_AxisFraction( int localPlayerNum, int joystickNum, int axisNum ) {
	float fraction;
	int axis;

	// non-analog keystate or analog disabled
	if ( axisNum == 0 || !cg_joystickUseAnalog[joystickNum].integer ) {
		return 1;
	}

	if ( axisNum < 0 ) {
		axis = -axisNum - 1;
	} else {
		axis = axisNum - 1;
	}

	// sign flip shouldn't ever happen, key should be released first
	if ( !!( axisNum < 0 ) != !!( cg.localPlayers[ joystickNum ].joystickAxis[ axis ] < 0 ) ) {
		fraction = 0;
		CG_Printf("WARNING: Cmd for player %d (axis %d on joystick for player %d): axis fraction is 0, but input system still thinks it's pressed\n", localPlayerNum+1, axisNum, joystickNum+1);
	} else {
		float threshold = 32767.0f * cg_joystickThreshold[joystickNum].value;

		fraction = ( (float)abs( cg.localPlayers[ joystickNum ].joystickAxis[ axis ] ) - threshold ) / ( 32767.0f - threshold );
	}

	return fraction;
}

/*
===============
CL_KeyState

Gets fraction of the frame that the key was down as digital and analog input
===============
*/
void CL_KeyStateSeparate( localPlayer_t *player, kbutton_t *key, float *pDigitalFrac, float *pAnalogFrac ) {
	float		val;
	int			msec;
	int			i;
	float		fraction[2];
	float		digitalFrac, analogFrac;
	int			localPlayerNum;

	localPlayerNum = player - cg.localPlayers;

	msec = key->msec;
	key->msec = 0;

	if ( key->active ) {
		// still down
		if ( !key->downtime ) {
			msec = in_frameTime;
		} else {
			msec += in_frameTime - key->downtime;
		}
		key->downtime = in_frameTime;

		for ( i = 0; i < 2; ++i ) {
			if ( key->down[i] ) {
				fraction[i] = CL_AxisFraction( localPlayerNum, key->joystickNum[i], key->axisNum[i] );
			} else {
				fraction[i] = 0;
			}

			key->lastFraction[i] = fraction[i];
		}
	} else {
		// use fraction from last frame to avoid boost frame after releasing joystick axis
		fraction[0] = key->lastFraction[0];
		fraction[1] = key->lastFraction[1];
	}

	analogFrac = 0;
	digitalFrac = 0;

	for ( i = 0; i < 2; ++i ) {
		if ( key->axisNum[i] && cg_joystickUseAnalog[ key->joystickNum[i] ].integer ) {
			analogFrac += fraction[i];
		} else {
			digitalFrac += fraction[i];
		}
	}

	if ( analogFrac > 1 ) {
		analogFrac = 1;
	}

	if ( digitalFrac > 1 ) {
		digitalFrac = 1;
	}

#if 0
	if (msec) {
		Com_Printf ("%i ", msec);
	}
#endif

	val = (float)msec / in_frameMsec;

	*pDigitalFrac = Com_Clamp( 0, 1, val * digitalFrac );
	*pAnalogFrac = Com_Clamp( 0, 1, val * analogFrac );
}

/*
===============
CL_KeyState

Returns the fraction of the frame that the key was down
===============
*/
float CL_KeyState( localPlayer_t *player, kbutton_t *key ) {
	float digital, analog;

	CL_KeyStateSeparate( player, key, &digital, &analog );

	return Com_Clamp( 0, 1, digital + analog );
}


void IN_UpDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_up);}
void IN_UpUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_up);}
void IN_DownDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_down);}
void IN_DownUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_down);}
void IN_LeftDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_left);}
void IN_LeftUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_left);}
void IN_RightDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_right);}
void IN_RightUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_right);}
void IN_ForwardDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_forward);}
void IN_ForwardUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_forward);}
void IN_BackDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_back);}
void IN_BackUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_back);}
void IN_LookupDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_lookup);}
void IN_LookupUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_lookup);}
void IN_LookdownDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_lookdown);}
void IN_LookdownUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_lookdown);}
void IN_MoveleftDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_moveleft);}
void IN_MoveleftUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_moveleft);}
void IN_MoverightDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_moveright);}
void IN_MoverightUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_moveright);}

void IN_SpeedDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_speed);}
void IN_SpeedUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_speed);}
void IN_StrafeDown( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_strafe);}
void IN_StrafeUp( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_strafe);}

void IN_Button0Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[0]);}
void IN_Button0Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[0]);}
void IN_Button1Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[1]);}
void IN_Button1Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[1]);}
void IN_Button2Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[2]);}
void IN_Button2Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[2]);}
void IN_Button3Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[3]);}
void IN_Button3Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[3]);}
void IN_Button4Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[4]);}
void IN_Button4Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[4]);}
void IN_Button5Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[5]);}
void IN_Button5Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[5]);}
void IN_Button6Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[6]);}
void IN_Button6Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[6]);}
void IN_Button7Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[7]);}
void IN_Button7Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[7]);}
void IN_Button8Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[8]);}
void IN_Button8Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[8]);}
void IN_Button9Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[9]);}
void IN_Button9Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[9]);}
void IN_Button10Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[10]);}
void IN_Button10Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[10]);}
void IN_Button11Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[11]);}
void IN_Button11Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[11]);}
void IN_Button12Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[12]);}
void IN_Button12Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[12]);}
void IN_Button13Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[13]);}
void IN_Button13Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[13]);}
void IN_Button14Down( int localPlayerNum ) {IN_KeyDown(&cis[localPlayerNum].in_buttons[14]);}
void IN_Button14Up( int localPlayerNum ) {IN_KeyUp(&cis[localPlayerNum].in_buttons[14]);}

void IN_CenterView( int localPlayerNum ) {
	playerState_t *ps;

	if ( !cg.snap || cg.snap->playerNums[localPlayerNum] == -1 ) {
		return;
	}

	ps = &cg.snap->pss[localPlayerNum];

	cg.localPlayers[localPlayerNum].viewangles[PITCH] = -SHORT2ANGLE(ps->delta_angles[PITCH]);
}

//==========================================================================

/*
================
CG_AdjustAngles

Moves the local angle positions
================
*/
void CG_AdjustAngles( localPlayer_t *player, clientInput_t *ci ) {
	float	speed;
	float	digital, analog;
	int		localPlayerNum = player - cg.localPlayers;
	
	if ( ci->in_speed.active ) {
		speed = 0.001 * cg.frametime * cg_anglespeedkey[localPlayerNum].value;
	} else {
		speed = 0.001 * cg.frametime;
	}

	if ( !ci->in_strafe.active ) {
		CL_KeyStateSeparate (player, &ci->in_right, &digital, &analog);
		player->viewangles[YAW] -= speed*cg_yawspeed[localPlayerNum].value * digital;
		player->viewangles[YAW] -= speed*cg_yawspeedanalog[localPlayerNum].value * analog;

		CL_KeyStateSeparate (player, &ci->in_left, &digital, &analog);
		player->viewangles[YAW] += speed*cg_yawspeed[localPlayerNum].value * digital;
		player->viewangles[YAW] += speed*cg_yawspeedanalog[localPlayerNum].value * analog;
	}

	CL_KeyStateSeparate (player, &ci->in_lookup, &digital, &analog);
	player->viewangles[PITCH] -= speed*cg_pitchspeed[localPlayerNum].value * digital;
	player->viewangles[PITCH] -= speed*cg_pitchspeedanalog[localPlayerNum].value * analog;

	CL_KeyStateSeparate (player, &ci->in_lookdown, &digital, &analog);
	player->viewangles[PITCH] += speed*cg_pitchspeed[localPlayerNum].value * digital;
	player->viewangles[PITCH] += speed*cg_pitchspeedanalog[localPlayerNum].value * analog;
}

/*
================
CG_KeyMove

Sets the usercmd_t based on key states
================
*/
void CG_KeyMove( localPlayer_t *player, clientInput_t *ci, usercmd_t *cmd ) {
	int		movespeed;
	int		forward, side, up;

	//
	// adjust for speed key / running
	// the walking flag is to keep animations consistant
	// even during acceleration and develeration
	//
	if ( ci->in_speed.active ^ cg_run[ci-cis].integer ) {
		movespeed = 127;
		cmd->buttons &= ~BUTTON_WALKING;
	} else {
		cmd->buttons |= BUTTON_WALKING;
		movespeed = 64;
	}

	forward = 0;
	side = 0;
	up = 0;
	if ( ci->in_strafe.active ) {
		side += movespeed * CL_KeyState (player, &ci->in_right);
		side -= movespeed * CL_KeyState (player, &ci->in_left);
	}

	side += movespeed * CL_KeyState (player, &ci->in_moveright);
	side -= movespeed * CL_KeyState (player, &ci->in_moveleft);


	up += movespeed * CL_KeyState (player, &ci->in_up);
	up -= movespeed * CL_KeyState (player, &ci->in_down);

	forward += movespeed * CL_KeyState (player, &ci->in_forward);
	forward -= movespeed * CL_KeyState (player, &ci->in_back);

	cmd->forwardmove = ClampChar( forward );
	cmd->rightmove = ClampChar( side );
	cmd->upmove = ClampChar( up );
}


/*
=================
CG_MouseMove
=================
*/
void CG_MouseMove( localPlayer_t *player, clientInput_t *ci, usercmd_t *cmd, float mx, float my )
{
	// ingame FOV
	mx *= player->zoomSensitivity;
	my *= player->zoomSensitivity;

	// add mouse X/Y movement to cmd
	if(ci->in_strafe.active)
	{
		cmd->rightmove = ClampChar(cmd->rightmove + m_side.value * mx);
	}
	else
		player->viewangles[YAW] -= m_yaw.value * mx;

	if ((ci->in_mlooking || cg_freelook.integer) && !ci->in_strafe.active)
		player->viewangles[PITCH] += m_pitch.value * my;
	else
		cmd->forwardmove = ClampChar(cmd->forwardmove - m_forward.value * my);
}

/*
==============
CG_CmdButtons
==============
*/
void CG_CmdButtons( clientInput_t *ci, usercmd_t *cmd, qboolean anykeydown ) {
	int		i;

	//
	// figure button bits
	// send a button bit even if the key was pressed and released in
	// less than a frame
	//	
	for (i = 0 ; i < 15 ; i++) {
		if ( ci->in_buttons[i].active || ci->in_buttons[i].wasPressed ) {
			cmd->buttons |= 1 << i;
		}
		ci->in_buttons[i].wasPressed = qfalse;
	}

	if ( Key_GetCatcher( ) ) {
		cmd->buttons |= BUTTON_TALK;
	}
	// allow the game to know if any key at all is
	// currently pressed, even if it isn't bound to anything
	else if ( anykeydown ) {
		cmd->buttons |= BUTTON_ANY;
	}
}

/*
==============
CG_FinishMove
==============
*/
void CG_FinishMove( localPlayer_t *player, usercmd_t *cmd ) {
	int i;

	cmd->stateValue = BG_ComposeUserCmdValue( player->weaponSelect );

	for (i=0 ; i<3 ; i++) {
		cmd->angles[i] = ANGLE2SHORT( player->viewangles[i] );
	}
}

/*
=================
CG_CreateUserCmd
=================
*/
usercmd_t *CG_CreateUserCmd( int localPlayerNum, int frameTime, unsigned frameMsec, float mx, float my, qboolean anykeydown ) {
	static usercmd_t cmd;
	vec3_t		oldAngles;
	localPlayer_t *player;
	clientInput_t *ci;

	in_frameTime = frameTime;
	in_frameMsec = frameMsec;

	player = &cg.localPlayers[localPlayerNum];
	ci = &cis[localPlayerNum];

	VectorCopy( player->viewangles, oldAngles );

	// keyboard angle adjustment
	CG_AdjustAngles( player, ci );

	Com_Memset( &cmd, 0, sizeof( cmd ) );

	CG_CmdButtons( ci, &cmd, anykeydown );

	// get basic movement from keyboard
	CG_KeyMove( player, ci, &cmd );

	// get basic movement from mouse
	CG_MouseMove( player, ci, &cmd, mx, my );

	// check to make sure the angles haven't wrapped
	if ( player->viewangles[PITCH] - oldAngles[PITCH] > 90 ) {
		player->viewangles[PITCH] = oldAngles[PITCH] + 90;
	} else if ( oldAngles[PITCH] - player->viewangles[PITCH] > 90 ) {
		player->viewangles[PITCH] = oldAngles[PITCH] - 90;
	}

	// store out the final values
	CG_FinishMove( player, &cmd );

	return &cmd;
}

/*
=================
CG_RegisterInputCvars
=================
*/
void CG_RegisterInputCvars( void ) {
	int i;

	trap_Cvar_Register( &cg_freelook, "cl_freelook", "1", CVAR_ARCHIVE ); // ZTM: NOTE: changing name breaks team arena menu scripts

	trap_Cvar_Register( &m_pitch, "m_pitch", "0.022", CVAR_ARCHIVE );
	trap_Cvar_Register( &m_yaw, "m_yaw", "0.022", CVAR_ARCHIVE );
	trap_Cvar_Register( &m_forward, "m_forward", "0.25", CVAR_ARCHIVE );
	trap_Cvar_Register( &m_side, "m_side", "0.25", CVAR_ARCHIVE );

	for (i = 0; i < CG_MaxSplitView(); i++) {
		trap_Cvar_Register( &cg_yawspeed[i], Com_LocalPlayerCvarName(i, "cg_yawspeed"), "140", CVAR_ARCHIVE );
		trap_Cvar_Register( &cg_pitchspeed[i], Com_LocalPlayerCvarName(i, "cg_pitchspeed"), "140", CVAR_ARCHIVE );
		trap_Cvar_Register( &cg_yawspeedanalog[i], Com_LocalPlayerCvarName(i, "cg_yawspeedanalog"), "200", CVAR_ARCHIVE );
		trap_Cvar_Register( &cg_pitchspeedanalog[i], Com_LocalPlayerCvarName(i, "cg_pitchspeedanalog"), "200", CVAR_ARCHIVE );
		trap_Cvar_Register( &cg_anglespeedkey[i], Com_LocalPlayerCvarName(i, "cg_anglespeedkey"), "1.5", 0 );
		trap_Cvar_Register( &cg_run[i], Com_LocalPlayerCvarName(i, "cl_run"), "1", CVAR_ARCHIVE ); // ZTM: NOTE: changing name breaks team arena menu scripts
		trap_Cvar_Register( &cg_joystickUseAnalog[i], Com_LocalPlayerCvarName(i, "in_joystickUseAnalog"), "1", CVAR_ARCHIVE );
		trap_Cvar_Register( &cg_joystickThreshold[i], Com_LocalPlayerCvarName(i, "in_joystickThreshold"), "0.15", CVAR_ARCHIVE );
	}
}

/*
=================
CG_UpdateInputCvars
=================
*/
void CG_UpdateInputCvars( void ) {
	int i;

	trap_Cvar_Update( &cg_freelook );

	trap_Cvar_Update( &m_pitch );
	trap_Cvar_Update( &m_yaw );
	trap_Cvar_Update( &m_forward );
	trap_Cvar_Update( &m_side );

	for (i = 0; i < CG_MaxSplitView(); i++) {
		trap_Cvar_Update( &cg_yawspeed[i] );
		trap_Cvar_Update( &cg_pitchspeed[i] );
		trap_Cvar_Update( &cg_yawspeedanalog[i] );
		trap_Cvar_Update( &cg_pitchspeedanalog[i] );
		trap_Cvar_Update( &cg_anglespeedkey[i] );
		trap_Cvar_Update( &cg_run[i] );
		trap_Cvar_Update( &cg_joystickUseAnalog[i] );
		trap_Cvar_Update( &cg_joystickThreshold[i] );
	}
}

