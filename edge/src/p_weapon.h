//----------------------------------------------------------------------------
//  EDGE Weapon (player sprites) Action Code
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2000  The EDGE Team.
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------
//
//  Based on the DOOM source code, released by Id Software under the
//  following copyright:
//
//    Copyright (C) 1993-1996 by id Software, Inc.
//
//----------------------------------------------------------------------------

#ifndef __P_PSPR__
#define __P_PSPR__

// Basic data types.
// Needs fixed point, and BAM angles.
#include "lu_math.h"
#include "m_fixed.h"

#include "ddf_main.h"

//
// Overlay psprites are scaled shapes
// drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
//
typedef enum
{
  ps_weapon,
  ps_flash,
  ps_crosshair,
  NUMPSPRITES
}
psprnum_t;

typedef struct
{
  // current state.  NULL state means not active
  state_t *state;

  // state to enter next.
  int next_state;

  // time (in tics) remaining for current state
  int tics;
  
  // screen position values
  float_t sx;
  float_t sy;

  // translucency values
  float_t visibility;
  float_t vis_target;
}
pspdef_t;

//
// Per-player Weapon Info.
// 
// -AJA- 1999/08/11: added this.

typedef struct
{
  weaponinfo_t *info;

  // player has this weapon.
  boolean_t owned;

  // how many times the player has fired it.
  int fire_count;
  int sa_fire_count;
}
playerweapon_t;

#endif
