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

#include "g_local.h"


/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *ent ) {
	gplayer_t	*player;
	float	count;
	vec3_t	angles;

	player = ent->player;
	if ( player->ps.pm_type == PM_DEAD ) {
		return;
	}

	// total points of damage shot at the player this frame
	count = player->damage_blood + player->damage_armor;
	if ( count == 0 ) {
		return;		// didn't take any damage
	}

	if ( count > 255 ) {
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( player->damage_fromWorld ) {
		player->ps.damagePitch = 255;
		player->ps.damageYaw = 255;

		player->damage_fromWorld = qfalse;
	} else {
		vectoangles( player->damage_from, angles );
		player->ps.damagePitch = angles[PITCH]/360.0 * 256;
		player->ps.damageYaw = angles[YAW]/360.0 * 256;
	}

	// play an appropriate pain sound
	if ( (level.time > ent->pain_debounce_time) && !(ent->flags & FL_GODMODE) ) {
		ent->pain_debounce_time = level.time + 700;
		G_AddEvent( ent, EV_PAIN, ent->health );
		player->ps.damageEvent++;
	}


	player->ps.damageCount = count;

	//
	// clear totals
	//
	player->damage_blood = 0;
	player->damage_armor = 0;
	player->damage_knockback = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	qboolean	envirosuit;
	int			waterlevel;

	if ( ent->player->noclip ) {
		ent->player->airOutTime = level.time + 12000;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	envirosuit = ent->player->ps.powerups[PW_BATTLESUIT] > level.time;

	//
	// check for drowning
	//
	if ( waterlevel == 3 ) {
		// envirosuit give air
		if ( envirosuit ) {
			ent->player->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if ( ent->player->airOutTime < level.time) {
			// drown!
			ent->player->airOutTime += 1000;
			if ( ent->health > 0 ) {
				// take more damage the longer underwater
				ent->damage += 2;
				if (ent->damage > 15)
					ent->damage = 15;

				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage (ent, NULL, NULL, NULL, NULL, 
					ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	} else {
		ent->player->airOutTime = level.time + 12000;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if (waterlevel && 
		(ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		if (ent->health > 0
			&& ent->pain_debounce_time <= level.time	) {

			if ( envirosuit ) {
				G_AddEvent( ent, EV_POWERUP_BATTLESUIT, 0 );
			} else {
				if (ent->watertype & CONTENTS_LAVA) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						30*waterlevel, 0, MOD_LAVA);
				}

				if (ent->watertype & CONTENTS_SLIME) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						10*waterlevel, 0, MOD_SLIME);
				}
			}
		}
	}
}



/*
===============
G_SetPlayerSound
===============
*/
void G_SetPlayerSound( gentity_t *ent ) {
#ifdef MISSIONPACK
	if( ent->s.eFlags & EF_TICKING ) {
		ent->player->ps.loopSound = G_SoundIndex( "sound/weapons/proxmine/wstbtick.wav");
	}
	else
#endif
	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		ent->player->ps.loopSound = level.snd_fry;
	} else {
		ent->player->ps.loopSound = 0;
	}
}



//==============================================================

/*
==============
PlayerImpacts
==============
*/
void PlayerImpacts( gentity_t *ent, pmove_t *pm ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}

		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( gentity_t *ent ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs;
	static vec3_t	range = { 40, 40, 52 };

	if ( !ent->player ) {
		return;
	}

	// dead players don't activate triggers!
	if ( ent->player->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->player->ps.origin, range, mins );
	VectorAdd( ent->player->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->player->ps.origin, ent->s.mins, mins );
	VectorAdd( ent->player->ps.origin, ent->s.maxs, maxs );

	for ( i=0 ; i<num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if ( !( hit->s.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}

		// ignore most entities if a spectator
		if ( ent->player->sess.sessionTeam == TEAM_SPECTATOR ) {
			if ( hit->s.eType != ET_TELEPORT_TRIGGER &&
				// this is ugly but adding a new ET_? type will
				// most likely cause network incompatibilities
				hit->touch != Touch_DoorTrigger) {
				continue;
			}
		}

		// use separate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->player->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else {
			if ( !trap_EntityContact( mins, maxs, hit ) ) {
				continue;
			}
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->touch ) {
			hit->touch (hit, ent, &trace);
		}

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, hit, &trace );
		}
	}

	// if we didn't touch a jump pad this pmove frame
	if ( ent->player->ps.jumppad_frame != ent->player->ps.pmove_framecount ) {
		ent->player->ps.jumppad_frame = 0;
		ent->player->ps.jumppad_ent = 0;
	}
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t	pm;
	gplayer_t	*player;

	player = ent->player;

	if ( player->sess.spectatorState != SPECTATOR_FOLLOW || !( player->ps.pm_flags & PMF_FOLLOW ) ) {
		if ( player->sess.spectatorState == SPECTATOR_FREE ) {
			if ( player->noclip ) {
				player->ps.pm_type = PM_NOCLIP;
			} else {
				player->ps.pm_type = PM_SPECTATOR;
			}
		} else {
			player->ps.pm_type = PM_FREEZE;
		}
		player->ps.speed = 400;	// faster than normal

		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &player->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		if (player->ps.collisionType == CT_CAPSULE) {
			pm.trace = trap_TraceCapsule;
		} else {
			pm.trace = trap_Trace;
		}
		pm.pointcontents = trap_PointContents;

		// perform a pmove
		Pmove (&pm);
		// save results of pmove
		VectorCopy( player->ps.origin, ent->s.origin );

		G_TouchTriggers( ent );
		trap_UnlinkEntity( ent );
	}

	player->oldbuttons = player->buttons;
	player->buttons = ucmd->buttons;

	// attack button cycles through spectators
	if ( ( player->buttons & BUTTON_ATTACK ) && ! ( player->oldbuttons & BUTTON_ATTACK ) ) {
		Cmd_FollowCycle_f( ent, 1 );
	}
}



/*
=================
PlayerInactivityTimer

Returns qfalse if the player is dropped
=================
*/
qboolean PlayerInactivityTimer( gplayer_t *player ) {
	if ( ! g_inactivity.integer ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		player->inactivityTime = level.time + 60 * 1000;
		player->inactivityWarning = qfalse;
	} else if ( player->pers.cmd.forwardmove || 
		player->pers.cmd.rightmove || 
		player->pers.cmd.upmove ||
		(player->pers.cmd.buttons & BUTTON_ATTACK) ) {
		player->inactivityTime = level.time + g_inactivity.integer * 1000;
		player->inactivityWarning = qfalse;
	} else if ( !player->pers.localClient ) {
		if ( level.time > player->inactivityTime ) {
			trap_DropPlayer( player - level.players, "Dropped due to inactivity" );
			return qfalse;
		}
		if ( level.time > player->inactivityTime - 10000 && !player->inactivityWarning ) {
			player->inactivityWarning = qtrue;
			trap_SendServerCommand( player - level.players, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}

/*
==================
PlayerTimerActions

Actions that happen once a second
==================
*/
void PlayerTimerActions( gentity_t *ent, int msec ) {
	gplayer_t	*player;
	int			maxHealth;

	player = ent->player;
	player->timeResidual += msec;

	while ( player->timeResidual >= 1000 ) {
		player->timeResidual -= 1000;

		// regenerate
#ifdef MISSIONPACK
		if( BG_ItemForItemNum( player->ps.stats[STAT_PERSISTANT_POWERUP] )->giTag == PW_GUARD ) {
			maxHealth = player->ps.stats[STAT_MAX_HEALTH] / 2;
		}
		else
#endif
		if ( player->ps.powerups[PW_REGEN] ) {
			maxHealth = player->ps.stats[STAT_MAX_HEALTH];
		} else {
			maxHealth = 0;
		}
		if( maxHealth ) {
			if ( ent->health < maxHealth ) {
				ent->health += 15;
				if ( ent->health > maxHealth * 1.1 ) {
					ent->health = maxHealth * 1.1;
				}
				G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
			} else if ( ent->health < maxHealth * 2) {
				ent->health += 5;
				if ( ent->health > maxHealth * 2 ) {
					ent->health = maxHealth * 2;
				}
				G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
			}
		} else {
			// count down health when over max
			if ( ent->health > player->ps.stats[STAT_MAX_HEALTH] ) {
				ent->health--;
			}
		}

		// count down armor when over max
		if ( player->ps.stats[STAT_ARMOR] > player->ps.stats[STAT_MAX_HEALTH] ) {
			player->ps.stats[STAT_ARMOR]--;
		}
	}
#ifdef MISSIONPACK
	if( BG_ItemForItemNum( player->ps.stats[STAT_PERSISTANT_POWERUP] )->giTag == PW_AMMOREGEN ) {
		int w, max, inc, t, i;
    int weapList[]={WP_MACHINEGUN,WP_SHOTGUN,WP_GRENADE_LAUNCHER,WP_ROCKET_LAUNCHER,WP_LIGHTNING,WP_RAILGUN,WP_PLASMAGUN,WP_BFG,WP_NAILGUN,WP_PROX_LAUNCHER,WP_CHAINGUN};
    int weapCount = ARRAY_LEN( weapList );
		//
    for (i = 0; i < weapCount; i++) {
		  w = weapList[i];

		  switch(w) {
			  case WP_MACHINEGUN: max = 50; inc = 4; t = 1000; break;
			  case WP_SHOTGUN: max = 10; inc = 1; t = 1500; break;
			  case WP_GRENADE_LAUNCHER: max = 10; inc = 1; t = 2000; break;
			  case WP_ROCKET_LAUNCHER: max = 10; inc = 1; t = 1750; break;
			  case WP_LIGHTNING: max = 50; inc = 5; t = 1500; break;
			  case WP_RAILGUN: max = 10; inc = 1; t = 1750; break;
			  case WP_PLASMAGUN: max = 50; inc = 5; t = 1500; break;
			  case WP_BFG: max = 10; inc = 1; t = 4000; break;
			  case WP_NAILGUN: max = 10; inc = 1; t = 1250; break;
			  case WP_PROX_LAUNCHER: max = 5; inc = 1; t = 2000; break;
			  case WP_CHAINGUN: max = 100; inc = 5; t = 1000; break;
			  default: max = 0; inc = 0; t = 1000; break;
		  }
		  player->ammoTimes[w] += msec;
		  if ( player->ps.ammo[w] >= max ) {
			  player->ammoTimes[w] = 0;
		  }
		  if ( player->ammoTimes[w] >= t ) {
			  while ( player->ammoTimes[w] >= t )
				  player->ammoTimes[w] -= t;
			  player->ps.ammo[w] += inc;
			  if ( player->ps.ammo[w] > max ) {
				  player->ps.ammo[w] = max;
			  }
		  }
    }
	}
#endif
}

/*
====================
PlayerIntermissionThink
====================
*/
void PlayerIntermissionThink( gplayer_t *player ) {
	player->ps.eFlags &= ~EF_TALK;
	player->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	player->oldbuttons = player->buttons;
	player->buttons = player->pers.cmd.buttons;
	if ( player->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) & ( player->oldbuttons ^ player->buttons ) ) {
		// this used to be an ^1 but once a player says ready, it should stick
		player->readyToExit = 1;
	}
}


/*
================
PlayerEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void PlayerEvents( gentity_t *ent, int oldEventSequence ) {
	int		i;
	int		event;
	gplayer_t *player;
	int		damage;
	vec3_t	origin, angles;

	player = ent->player;

	if ( oldEventSequence < player->ps.eventSequence - MAX_PS_EVENTS ) {
		oldEventSequence = player->ps.eventSequence - MAX_PS_EVENTS;
	}
	for ( i = oldEventSequence ; i < player->ps.eventSequence ; i++ ) {
		event = player->ps.events[ i & (MAX_PS_EVENTS-1) ];

		switch ( event ) {
		case EV_FALL_MEDIUM:
		case EV_FALL_FAR:
			if ( ent->s.eType != ET_PLAYER ) {
				break;		// not in the player model
			}
			if ( g_dmflags.integer & DF_NO_FALLING ) {
				break;
			}
			if ( event == EV_FALL_FAR ) {
				damage = 10;
			} else {
				damage = 5;
			}
			ent->pain_debounce_time = level.time + 200;	// no normal pain sound
			G_Damage (ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
			break;

		case EV_FIRE_WEAPON:
			FireWeapon( ent );
			break;

		case EV_USE_ITEM0:
		case EV_USE_ITEM1:
		case EV_USE_ITEM2:
		case EV_USE_ITEM3:
		case EV_USE_ITEM4:
		case EV_USE_ITEM5:
		case EV_USE_ITEM6:
		case EV_USE_ITEM7:
		case EV_USE_ITEM8:
		case EV_USE_ITEM9:
		case EV_USE_ITEM10:
		case EV_USE_ITEM11:
		case EV_USE_ITEM12:
		case EV_USE_ITEM13:
		case EV_USE_ITEM14:
		case EV_USE_ITEM15:
		{
			int itemNum = (event & ~EV_EVENT_BITS) - EV_USE_ITEM0;

			switch ( itemNum ) {
			default:
			case HI_NONE:
				break;

			case HI_TELEPORTER:
				TossPlayerGametypeItems( ent );
				SelectSpawnPoint( ent->player->ps.origin, origin, angles, qfalse );
				TeleportPlayer( ent, origin, angles );
				break;

			case HI_MEDKIT:
				ent->health = ent->player->ps.stats[STAT_MAX_HEALTH] + 25;
				break;

#ifdef MISSIONPACK
			case HI_KAMIKAZE:
				// make sure the invulnerability is off
				ent->player->invulnerabilityTime = 0;
				// start the kamikze
				G_StartKamikaze( ent );
				break;

			case HI_PORTAL:
				if ( ent->player->portalID ) {
					DropPortalSource( ent );
				}
				else {
					DropPortalDestination( ent );
				}
				break;

			case HI_INVULNERABILITY:
				ent->player->invulnerabilityTime = level.time + 10000;
				break;
#endif // MISSIONPACK
			}
			break;
		}

		default:
			break;
		}
	}

}

#ifdef MISSIONPACK
/*
==============
StuckInOtherPlayer
==============
*/
static int StuckInOtherPlayer(gentity_t *ent) {
	int i;
	gentity_t	*ent2;

	ent2 = &g_entities[0];
	for ( i = 0; i < MAX_CLIENTS; i++, ent2++ ) {
		if ( ent2 == ent ) {
			continue;
		}
		if ( !ent2->inuse ) {
			continue;
		}
		if ( !ent2->player ) {
			continue;
		}
		if ( ent2->health <= 0 ) {
			continue;
		}
		//
		if (ent2->r.absmin[0] > ent->r.absmax[0])
			continue;
		if (ent2->r.absmin[1] > ent->r.absmax[1])
			continue;
		if (ent2->r.absmin[2] > ent->r.absmax[2])
			continue;
		if (ent2->r.absmax[0] < ent->r.absmin[0])
			continue;
		if (ent2->r.absmax[1] < ent->r.absmin[1])
			continue;
		if (ent2->r.absmax[2] < ent->r.absmin[2])
			continue;
		return qtrue;
	}
	return qfalse;
}
#endif

void BotTestSolid(vec3_t origin);

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
	gentity_t *t;
	int event, seq, i;
	int extEvent, number;
	gconnection_t *connection;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->playerNum;
		t->s.contents = 0;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_PLAYERMASK;
		Com_ClientListAll( &t->r.sendPlayers );
		connection = &level.connections[ level.players[ ps->playerNum ].pers.connectionNum ];
		for (i = 0; i < connection->numLocalPlayers; i++ ) {
			Com_ClientListRemove( &t->r.sendPlayers, connection->localPlayerNums[i] );
		}
		// set back external event
		ps->externalEvent = extEvent;
	}
}

/*
==============
PlayerThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void PlayerThink_real( gentity_t *ent ) {
	gplayer_t	*player;
	pmove_t		pm;
	int			oldEventSequence;
	int			msec;
	usercmd_t	*ucmd;

	player = ent->player;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (player->pers.connected != CON_CONNECTED) {
		return;
	}

	// frameOffset should be about the number of milliseconds into a frame
	// this command packet was received, depending on how fast the server
	// does a G_RunFrame()
	player->frameOffset = trap_Milliseconds() - level.frameStartTime;

	// mark the time, so the connection sprite can be removed
	ucmd = &ent->player->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
//		G_Printf("serverTime <<<<<\n" );
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
//		G_Printf("serverTime >>>>>\n" );
	} 

	player->lastCmdServerTime = ucmd->serverTime;

	msec = ucmd->serverTime - player->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && player->sess.spectatorState != SPECTATOR_FOLLOW ) {
		return;
	}
	if ( msec > 200 ) {
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_SetValue( "pmove_msec", 8 );
		trap_Cvar_Update( &pmove_msec );
	}
	else if ( pmove_msec.integer > 33 ) {
		trap_Cvar_SetValue( "pmove_msec", 33 );
		trap_Cvar_Update( &pmove_msec );
	}

	if ( pmove_fixed.integer || player->pers.pmoveFixed ) {
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - player->ps.commandTime <= 0)
		//	return;
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) {
		PlayerIntermissionThink( player );
		return;
	}

	// spectators don't do much
	if ( player->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( player->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			return;
		}
		SpectatorThink( ent, ucmd );
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !PlayerInactivityTimer( player ) ) {
		return;
	}

	// clear the rewards if time
	if ( level.time > player->rewardTime ) {
		player->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
	}

	if ( player->noclip ) {
		player->ps.pm_type = PM_NOCLIP;
	} else if ( player->ps.stats[STAT_HEALTH] <= 0 ) {
		player->ps.pm_type = PM_DEAD;
	} else {
		player->ps.pm_type = PM_NORMAL;
	}

	player->ps.gravity = g_gravity.value;

	// set speed
	player->ps.speed = g_speed.value;

#ifdef MISSIONPACK
	if( BG_ItemForItemNum( player->ps.stats[STAT_PERSISTANT_POWERUP] )->giTag == PW_SCOUT ) {
		player->ps.speed *= 1.5;
	}
	else
#endif
	if ( player->ps.powerups[PW_HASTE] ) {
		player->ps.speed *= 1.3;
	}

	// Let go of the hook if we aren't firing
	if ( player->ps.weapon == WP_GRAPPLING_HOOK &&
		player->hook && !( ucmd->buttons & BUTTON_ATTACK ) ) {
		Weapon_HookFree(player->hook);
	}

	// set up for pmove
	oldEventSequence = player->ps.eventSequence;

	memset (&pm, 0, sizeof(pm));

	// check for the hit-scan gauntlet, don't let the action
	// go through as an attack unless it actually hits something
	if ( player->ps.weapon == WP_GAUNTLET && !( ucmd->buttons & BUTTON_TALK ) &&
		( ucmd->buttons & BUTTON_ATTACK ) && player->ps.weaponTime <= 0 ) {
		pm.gauntletHit = CheckGauntletAttack( ent );
	}

	if ( ent->flags & FL_FORCE_GESTURE ) {
		ent->flags &= ~FL_FORCE_GESTURE;
		ent->player->pers.cmd.buttons |= BUTTON_GESTURE;
	}

#ifdef MISSIONPACK
	// check for invulnerability expansion before doing the Pmove
	if (player->ps.powerups[PW_INVULNERABILITY] ) {
		if ( !(player->ps.pm_flags & PMF_INVULEXPAND) ) {
			vec3_t mins = { -42, -42, -42 };
			vec3_t maxs = { 42, 42, 42 };
			vec3_t oldmins, oldmaxs;

			VectorCopy (ent->s.mins, oldmins);
			VectorCopy (ent->s.maxs, oldmaxs);
			// expand
			VectorCopy (mins, ent->s.mins);
			VectorCopy (maxs, ent->s.maxs);
			trap_LinkEntity(ent);
			// check if this would get anyone stuck in this player
			if ( !StuckInOtherPlayer(ent) ) {
				// set flag so the expanded size will be set in PM_CheckDuck
				player->ps.pm_flags |= PMF_INVULEXPAND;
			}
			// set back
			VectorCopy (oldmins, ent->s.mins);
			VectorCopy (oldmaxs, ent->s.maxs);
			trap_LinkEntity(ent);
		}
	}
#endif

	pm.ps = &player->ps;
	pm.cmd = *ucmd;
	if ( pm.ps->pm_type == PM_DEAD ) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else if ( ent->r.svFlags & SVF_BOT ) {
		pm.tracemask = MASK_PLAYERSOLID | CONTENTS_BOTCLIP;
	}
	else {
		pm.tracemask = MASK_PLAYERSOLID;
	}
	if (player->ps.collisionType == CT_CAPSULE) {
		pm.trace = trap_TraceCapsule;
	} else {
		pm.trace = trap_Trace;
	}
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pm.pmove_fixed = pmove_fixed.integer | player->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	pm.pmove_overbounce = pmove_overbounce.integer;

	VectorCopy( player->ps.origin, player->oldOrigin );

#ifdef MISSIONPACK
	if ( level.intermissionQueued != 0 && g_singlePlayer.integer ) {
		if ( level.time - level.intermissionQueued >= 1000 ) {
			pm.cmd.buttons = 0;
			pm.cmd.forwardmove = 0;
			pm.cmd.rightmove = 0;
			pm.cmd.upmove = 0;
			if ( level.time - level.intermissionQueued >= 2000 && level.time - level.intermissionQueued <= 2500 ) {
				trap_Cmd_ExecuteText( EXEC_APPEND, "centerview\n" );
			}
			ent->player->ps.pm_type = PM_SPINTERMISSION;
		}
	}
#endif

	Pmove (&pm);

	// save results of pmove
	if ( ent->player->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
	}
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->player->ps, &ent->s, ent->player->ps.commandTime, qtrue );
	}
	else {
		BG_PlayerStateToEntityState( &ent->player->ps, &ent->s, qtrue );
	}
	SendPendingPredictableEvents( &ent->player->ps );

	if ( !( ent->player->ps.eFlags & EF_FIRING ) ) {
		player->fireHeld = qfalse;		// for grapple
	}

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	// execute player events
	PlayerEvents( ent, oldEventSequence );

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity (ent);
	if ( !ent->player->noclip ) {
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise players can be snapped into solid
	VectorCopy( ent->player->ps.origin, ent->r.currentOrigin );

	//test for solid areas in the AAS file
	BotTestAAS(ent->r.currentOrigin);

	// touch other objects
	PlayerImpacts( ent, &pm );

	// save results of triggers and player events
	if (ent->player->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	player->oldbuttons = player->buttons;
	player->buttons = ucmd->buttons;
	player->latched_buttons |= player->buttons & ~player->oldbuttons;

	// check for respawning
	if ( player->ps.stats[STAT_HEALTH] <= 0 ) {
		// wait for the attack button to be pressed
		if ( level.time > player->respawnTime ) {
			// forcerespawn is to prevent users from waiting out powerups
			if ( g_forcerespawn.integer > 0 && 
				( level.time - player->respawnTime ) > g_forcerespawn.integer * 1000 ) {
				PlayerRespawn( ent );
				return;
			}
		
			// pressing attack or use is the normal respawn method
			if ( ucmd->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) ) {
				PlayerRespawn( ent );
			}
		}
		return;
	}

	// perform once-a-second actions
	PlayerTimerActions( ent, msec );
}

/*
==================
PlayerThink

A new command has arrived from the client
==================
*/
void PlayerThink( int playerNum ) {
	gentity_t *ent;

	ent = g_entities + playerNum;
	trap_GetUsercmd( playerNum, &ent->player->pers.cmd );

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->player->lastCmdTime = level.time;

	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
		PlayerThink_real( ent );
	}
}


void G_RunPlayer( gentity_t *ent ) {
	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
		return;
	}
	ent->player->pers.cmd.serverTime = level.time;
	PlayerThink_real( ent );
}


/*
==================
SpectatorPlayerEndFrame

==================
*/
void SpectatorPlayerEndFrame( gentity_t *ent ) {
	gplayer_t	*cl;

	// if we are doing a chase cam or a remote view, grab the latest info
	if ( ent->player->sess.spectatorState == SPECTATOR_FOLLOW ) {
		int		playerNum, flags;

		playerNum = ent->player->sess.spectatorPlayer;

		// team follow1 and team follow2 go to whatever players are playing
		if ( playerNum == -1 ) {
			playerNum = level.follow1;
		} else if ( playerNum == -2 ) {
			playerNum = level.follow2;
		}
		if ( playerNum >= 0 ) {
			cl = &level.players[ playerNum ];
			if ( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR ) {
				flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (ent->player->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
				ent->player->ps = cl->ps;
				ent->player->ps.pm_flags |= PMF_FOLLOW;
				ent->player->ps.eFlags = flags;
				return;
			}
		}

		if ( ent->player->ps.pm_flags & PMF_FOLLOW ) {
			// drop them to free spectators unless they are dedicated camera followers
			if ( ent->player->sess.spectatorPlayer >= 0 ) {
				ent->player->sess.spectatorState = SPECTATOR_FREE;
			}

			PlayerBegin( ent->player - level.players );
		}
	}

	if ( ent->player->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		ent->player->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->player->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

/*
==============
PlayerEndFrame

Called at the end of each server frame for each connected player
A fast client will have multiple PlayerThink for each PlayerEndFrame,
while a slow client may have multiple PlayerEndFrame between PlayerThink.
==============
*/
void PlayerEndFrame( gentity_t *ent ) {
	int			i;

	if ( ent->player->sess.sessionTeam == TEAM_SPECTATOR ) {
		SpectatorPlayerEndFrame( ent );
		return;
	}

	// turn off any expired powerups
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ent->player->ps.powerups[ i ] < level.time ) {
			ent->player->ps.powerups[ i ] = 0;
		}
	}

#ifdef MISSIONPACK
	// set powerup for player animation
	if( BG_ItemForItemNum( ent->player->ps.stats[STAT_PERSISTANT_POWERUP] )->giTag == PW_GUARD ) {
		ent->player->ps.powerups[PW_GUARD] = level.time;
	}
	if( BG_ItemForItemNum( ent->player->ps.stats[STAT_PERSISTANT_POWERUP] )->giTag == PW_SCOUT ) {
		ent->player->ps.powerups[PW_SCOUT] = level.time;
	}
	if( BG_ItemForItemNum( ent->player->ps.stats[STAT_PERSISTANT_POWERUP] )->giTag == PW_DOUBLER ) {
		ent->player->ps.powerups[PW_DOUBLER] = level.time;
	}
	if( BG_ItemForItemNum( ent->player->ps.stats[STAT_PERSISTANT_POWERUP] )->giTag == PW_AMMOREGEN ) {
		ent->player->ps.powerups[PW_AMMOREGEN] = level.time;
	}
	if ( ent->player->invulnerabilityTime > level.time ) {
		ent->player->ps.powerups[PW_INVULNERABILITY] = level.time;
	}
#endif

	// save network bandwidth
#if 0
	if ( !g_synchronousClients->integer && ent->player->ps.pm_type == PM_NORMAL ) {
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear( ent->player->ps.viewangles );
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) {
		return;
	}

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->player->lastCmdTime > 1000 ) {
		ent->player->ps.eFlags |= EF_CONNECTION;
	} else {
		ent->player->ps.eFlags &= ~EF_CONNECTION;
	}

	ent->player->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

	G_SetPlayerSound (ent);

	// set the latest infor
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->player->ps, &ent->s, ent->player->ps.commandTime, qtrue );
	}
	else {
		BG_PlayerStateToEntityState( &ent->player->ps, &ent->s, qtrue );
	}
	SendPendingPredictableEvents( &ent->player->ps );

	// store the client's position for backward reconciliation later
	G_StoreHistory( ent );

	// set the bit for the reachability area the player is currently in
//	i = trap_AAS_PointReachabilityAreaIndex( ent->player->ps.origin );
//	ent->player->areabits[i >> 3] |= 1 << (i & 7);
}


