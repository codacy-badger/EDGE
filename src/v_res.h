//----------------------------------------------------------------------------
//  EDGE Video Code
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2001  The EDGE Team.
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
//
// Original Author: Chi Hoang
//

#ifndef __MULTIRES_H__
#define __MULTIRES_H__

#include "dm_type.h"
#include "dm_defs.h"
#include "r_data.h"
#include "v_screen.h"
#include "v_video1.h"

boolean_t V_MultiResInit(void);
void V_InitResolution(void);
void V_AddAvailableResolution(screenmode_t *mode);
int V_FindClosestResolution(screenmode_t *mode,
    boolean_t samesize, boolean_t samedepth);
int V_CompareModes(screenmode_t *A, screenmode_t *B);

//
//start with v_video.h
//
#define CENTERY                 (SCREENHEIGHT/2)

#define FROM_320(x)  ((x) * SCREENWIDTH  / 320)
#define FROM_200(y)  ((y) * SCREENHEIGHT / 200)

extern screen_t *main_scr;
extern screen_t *back_scr;

#if 0  // OLD STUFF

// Allocates buffer screens, call before R_Init.
extern void (*V_Init) (void);
extern void (*V_DrawPatch) (screen_t * scr, int x, int y, const patch_t * patch);
extern void (*V_DrawPatchFlipped) (screen_t * scr, int x, int y, const patch_t * patch);
extern void (*V_DrawPatchDirect) (screen_t * scr, int x, int y, const patch_t * patch);
extern void (*V_DrawPatchInDirect) (screen_t * scr, int x, int y, const patch_t * patch);
extern void (*V_DrawPatchInDirectFlipped) (screen_t * scr, int x, int y, const patch_t * patch);
extern void (*V_DrawPatchTrans) (screen_t * scr, int x, int y, const byte *trans, const patch_t * patch);
extern void (*V_DrawPatchInDirectTrans) (screen_t * scr, int x, int y, const byte *trans, const patch_t * patch);
extern void (*V_DrawPatchInDirectAlpha) (screen_t * scr, int x, int y, const byte *trans, const patch_t * patch, fixed_t alpha);
extern void (*V_DrawPatchShrink) (screen_t * scr, int x, int y, const patch_t * patch);
extern void (*V_DrawBlock) (screen_t * dest, int x, int y, int width, int height, byte * src);
extern void (*V_GetBlock) (screen_t * scr, int x, int y, int width, int height, byte * dest);
extern void (*V_DarkenScreen) (screen_t * scr);
extern void (*V_TextureBackScreen) (screen_t * scr, const char *flatname, int left, int top, int right, int bottom);
extern void (*V_DrawPatchClip) (screen_t * scr, int x, int y, int left, int top, int right, int bottom, boolean_t flip, const patch_t * patch);

#endif  // OLD STUFF


extern void (*V_CopyRect) (screen_t * dest, screen_t * src, int srcx, int srcy, int width, int height, int destx, int desty);
extern void (*V_CopyScreen) (screen_t * dest, screen_t * src);

extern void (*V_DrawPixel) (screen_t * scr, int x, int y, int c);
extern void (*V_DrawLine) (screen_t * scr, int x1, int y1, int x2, int y2, int c);
extern void (*V_DrawBox) (screen_t * scr, int x, int y, int w, int h, int c);
extern void (*V_DrawBoxAlpha) (screen_t * scr, int x, int y, int w, int h, int c, fixed_t alpha);

//
// now with r_draw.h
//
extern const coltable_t *dc_colourmap;
extern int dc_x;
extern int dc_yl;
extern int dc_yh;
extern int dc_width, dc_height;
extern fixed_t dc_yfrac;
extern fixed_t dc_ystep;
extern fixed_t dc_translucency;

extern fixed_t dc_iscale;       // !!! FIXME: redundant
extern fixed_t dc_texturemid;   // !!! FIXME: redundant

#ifndef NOSMOOTHING
extern boolean_t dc_usesmoothing;
extern boolean_t ds_usesmoothing;
#endif

// first pixel in a column
extern const byte *dc_source;

#ifndef NOSMOOTHING
extern fixed_t dc_xfrac;
extern const byte *dc_source2;
#endif

extern byte **ylookup;
extern int *columnofs;

extern void (*resinit_r_draw_c) (void);
extern void (*R_DrawColumn) (void);
extern void (*R_DrawColumn_MIP) (void);
extern void (*R_DrawFuzzColumn) (void);
extern void (*R_DrawTranslucentColumn) (void);
extern void (*R_DrawTranslucentColumn_MIP) (void);
extern void (*R_DrawTranslatedColumn) (void);
extern void (*R_DrawTranslucentTranslatedColumn) (void);
extern void (*R_VideoErase) (unsigned ofs, int count); 

extern int ds_y;
extern int ds_x1;
extern int ds_x2;

extern const coltable_t *ds_colourmap;

extern fixed_t ds_xfrac, ds_yfrac;
extern fixed_t ds_xstep, ds_ystep;
extern int ds_width, ds_height;

// start of a 64*64 tile image
extern const byte *ds_source;

extern const byte *dc_translation;

extern void (*R_DrawSpan) (void);
extern void (*R_DrawSpan_MIP) (void);
extern void (*R_DrawTranslucentSpan) (void);
extern void (*R_DrawTranslucentSpan_MIP) (void);
extern void (*R_DrawHoleySpan_MIP) (void);
extern void V_ClearPageBackground(screen_t * scr);

void R_DrawViewBorder(void);
void R_FillBackScreen(void);

// WPost stuff

extern int dp_x;
extern int dp_yl, dp_yh;

extern fixed_t dp_yfrac;
extern fixed_t dp_ystep;
extern fixed_t dp_yscale;

// Screen Modes
extern screenmode_t *scrmode;
extern int numscrmodes;

// FIXME:
#define V_MarkRect(x,y,w,h)  do {} while (0)

#endif // __MULTIRES_H__
