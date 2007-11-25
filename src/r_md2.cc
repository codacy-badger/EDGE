//----------------------------------------------------------------------------
//  MD2 Models
//----------------------------------------------------------------------------
//
//  Copyright (c) 2002-2007  The EDGE Team.
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
//  Based on "qfiles.h" and "anorms.h" from the GPL'd quake 2 source
//  release.  Copyright (C) 1997-2001 Id Software, Inc.
//
//  Based on MD2 loading and rendering code (C) 2004 David Henry.
//
//----------------------------------------------------------------------------

#include "i_defs.h"
#include "i_defs_gl.h"

#include "epi/types.h"
#include "epi/endianess.h"

#include "r_md2.h"
#include "r_gldefs.h"
#include "r_colormap.h"
#include "r_effects.h"
#include "r_misc.h"
#include "r_modes.h"
#include "r_state.h"
#include "r_shader.h"
#include "r_units.h"
#include "p_blockmap.h"
#include "m_math.h"



extern float P_ApproxDistance(float dx, float dy, float dz);


// #define DEBUG_MD2_LOAD  1


/*============== FORMAT DEFINITIONS ====================*/


// format uses float pointing values, but to allow for endianness
// conversions they are represented here as unsigned integers.
typedef u32_t f32_t;

#define MD2_IDENTIFIER  "IDP2"

typedef struct
{
	char ident[4];

	s32_t version;

	s32_t skin_width;
	s32_t skin_height;

	s32_t frame_size;

	s32_t num_skins;
	s32_t num_vertices;  // per frame
	s32_t num_st;
	s32_t num_tris;
	s32_t num_glcmds;
	s32_t num_frames;

	s32_t ofs_skins;
	s32_t ofs_st;
	s32_t ofs_tris;
	s32_t ofs_frames;
	s32_t ofs_glcmds;  
	s32_t ofs_end;
} 
raw_md2_header_t;

typedef struct
{
	u16_t s, t;
} 
raw_md2_texcoord_t;

typedef struct 
{
	u16_t index_xyz[3];
	u16_t index_st[3];
} 
raw_md2_triangle_t;

typedef struct
{
	u8_t x, y, z;
	u8_t light_normal;
} 
raw_md2_vertex_t;

typedef struct
{
	f32_t s, t;
	s32_t vert_index;
}
raw_md2_glcmd_t;

typedef struct
{
	f32_t scale[3];
	f32_t translate[3];

	char name[16];

//	raw_md2_vertex_t verts[1];  /* variable sized */
} 
raw_md2_frame_t;

typedef struct
{
	char name[64];
}
raw_md2_skin_t;


/* ---- normals ---- */

#define MD2_NUM_NORMALS  162

static vec3_t md2_normals[MD2_NUM_NORMALS] =
{
	{ -0.525731f,  0.000000f,  0.850651f },
	{ -0.442863f,  0.238856f,  0.864188f },
	{ -0.295242f,  0.000000f,  0.955423f },
	{ -0.309017f,  0.500000f,  0.809017f },
	{ -0.162460f,  0.262866f,  0.951056f },
	{  0.000000f,  0.000000f,  1.000000f },
	{  0.000000f,  0.850651f,  0.525731f },
	{ -0.147621f,  0.716567f,  0.681718f },
	{  0.147621f,  0.716567f,  0.681718f },
	{  0.000000f,  0.525731f,  0.850651f },
	{  0.309017f,  0.500000f,  0.809017f },
	{  0.525731f,  0.000000f,  0.850651f },
	{  0.295242f,  0.000000f,  0.955423f },
	{  0.442863f,  0.238856f,  0.864188f },
	{  0.162460f,  0.262866f,  0.951056f },
	{ -0.681718f,  0.147621f,  0.716567f },
	{ -0.809017f,  0.309017f,  0.500000f },
	{ -0.587785f,  0.425325f,  0.688191f },
	{ -0.850651f,  0.525731f,  0.000000f },
	{ -0.864188f,  0.442863f,  0.238856f },
	{ -0.716567f,  0.681718f,  0.147621f },
	{ -0.688191f,  0.587785f,  0.425325f },
	{ -0.500000f,  0.809017f,  0.309017f },
	{ -0.238856f,  0.864188f,  0.442863f },
	{ -0.425325f,  0.688191f,  0.587785f },
	{ -0.716567f,  0.681718f, -0.147621f },
	{ -0.500000f,  0.809017f, -0.309017f },
	{ -0.525731f,  0.850651f,  0.000000f },
	{  0.000000f,  0.850651f, -0.525731f },
	{ -0.238856f,  0.864188f, -0.442863f },
	{  0.000000f,  0.955423f, -0.295242f },
	{ -0.262866f,  0.951056f, -0.162460f },
	{  0.000000f,  1.000000f,  0.000000f },
	{  0.000000f,  0.955423f,  0.295242f },
	{ -0.262866f,  0.951056f,  0.162460f },
	{  0.238856f,  0.864188f,  0.442863f },
	{  0.262866f,  0.951056f,  0.162460f },
	{  0.500000f,  0.809017f,  0.309017f },
	{  0.238856f,  0.864188f, -0.442863f },
	{  0.262866f,  0.951056f, -0.162460f },
	{  0.500000f,  0.809017f, -0.309017f },
	{  0.850651f,  0.525731f,  0.000000f },
	{  0.716567f,  0.681718f,  0.147621f },
	{  0.716567f,  0.681718f, -0.147621f },
	{  0.525731f,  0.850651f,  0.000000f },
	{  0.425325f,  0.688191f,  0.587785f },
	{  0.864188f,  0.442863f,  0.238856f },
	{  0.688191f,  0.587785f,  0.425325f },
	{  0.809017f,  0.309017f,  0.500000f },
	{  0.681718f,  0.147621f,  0.716567f },
	{  0.587785f,  0.425325f,  0.688191f },
	{  0.955423f,  0.295242f,  0.000000f },
	{  1.000000f,  0.000000f,  0.000000f },
	{  0.951056f,  0.162460f,  0.262866f },
	{  0.850651f, -0.525731f,  0.000000f },
	{  0.955423f, -0.295242f,  0.000000f },
	{  0.864188f, -0.442863f,  0.238856f },
	{  0.951056f, -0.162460f,  0.262866f },
	{  0.809017f, -0.309017f,  0.500000f },
	{  0.681718f, -0.147621f,  0.716567f },
	{  0.850651f,  0.000000f,  0.525731f },
	{  0.864188f,  0.442863f, -0.238856f },
	{  0.809017f,  0.309017f, -0.500000f },
	{  0.951056f,  0.162460f, -0.262866f },
	{  0.525731f,  0.000000f, -0.850651f },
	{  0.681718f,  0.147621f, -0.716567f },
	{  0.681718f, -0.147621f, -0.716567f },
	{  0.850651f,  0.000000f, -0.525731f },
	{  0.809017f, -0.309017f, -0.500000f },
	{  0.864188f, -0.442863f, -0.238856f },
	{  0.951056f, -0.162460f, -0.262866f },
	{  0.147621f,  0.716567f, -0.681718f },
	{  0.309017f,  0.500000f, -0.809017f },
	{  0.425325f,  0.688191f, -0.587785f },
	{  0.442863f,  0.238856f, -0.864188f },
	{  0.587785f,  0.425325f, -0.688191f },
	{  0.688191f,  0.587785f, -0.425325f },
	{ -0.147621f,  0.716567f, -0.681718f },
	{ -0.309017f,  0.500000f, -0.809017f },
	{  0.000000f,  0.525731f, -0.850651f },
	{ -0.525731f,  0.000000f, -0.850651f },
	{ -0.442863f,  0.238856f, -0.864188f },
	{ -0.295242f,  0.000000f, -0.955423f },
	{ -0.162460f,  0.262866f, -0.951056f },
	{  0.000000f,  0.000000f, -1.000000f },
	{  0.295242f,  0.000000f, -0.955423f },
	{  0.162460f,  0.262866f, -0.951056f },
	{ -0.442863f, -0.238856f, -0.864188f },
	{ -0.309017f, -0.500000f, -0.809017f },
	{ -0.162460f, -0.262866f, -0.951056f },
	{  0.000000f, -0.850651f, -0.525731f },
	{ -0.147621f, -0.716567f, -0.681718f },
	{  0.147621f, -0.716567f, -0.681718f },
	{  0.000000f, -0.525731f, -0.850651f },
	{  0.309017f, -0.500000f, -0.809017f },
	{  0.442863f, -0.238856f, -0.864188f },
	{  0.162460f, -0.262866f, -0.951056f },
	{  0.238856f, -0.864188f, -0.442863f },
	{  0.500000f, -0.809017f, -0.309017f },
	{  0.425325f, -0.688191f, -0.587785f },
	{  0.716567f, -0.681718f, -0.147621f },
	{  0.688191f, -0.587785f, -0.425325f },
	{  0.587785f, -0.425325f, -0.688191f },
	{  0.000000f, -0.955423f, -0.295242f },
	{  0.000000f, -1.000000f,  0.000000f },
	{  0.262866f, -0.951056f, -0.162460f },
	{  0.000000f, -0.850651f,  0.525731f },
	{  0.000000f, -0.955423f,  0.295242f },
	{  0.238856f, -0.864188f,  0.442863f },
	{  0.262866f, -0.951056f,  0.162460f },
	{  0.500000f, -0.809017f,  0.309017f },
	{  0.716567f, -0.681718f,  0.147621f },
	{  0.525731f, -0.850651f,  0.000000f },
	{ -0.238856f, -0.864188f, -0.442863f },
	{ -0.500000f, -0.809017f, -0.309017f },
	{ -0.262866f, -0.951056f, -0.162460f },
	{ -0.850651f, -0.525731f,  0.000000f },
	{ -0.716567f, -0.681718f, -0.147621f },
	{ -0.716567f, -0.681718f,  0.147621f },
	{ -0.525731f, -0.850651f,  0.000000f },
	{ -0.500000f, -0.809017f,  0.309017f },
	{ -0.238856f, -0.864188f,  0.442863f },
	{ -0.262866f, -0.951056f,  0.162460f },
	{ -0.864188f, -0.442863f,  0.238856f },
	{ -0.809017f, -0.309017f,  0.500000f },
	{ -0.688191f, -0.587785f,  0.425325f },
	{ -0.681718f, -0.147621f,  0.716567f },
	{ -0.442863f, -0.238856f,  0.864188f },
	{ -0.587785f, -0.425325f,  0.688191f },
	{ -0.309017f, -0.500000f,  0.809017f },
	{ -0.147621f, -0.716567f,  0.681718f },
	{ -0.425325f, -0.688191f,  0.587785f },
	{ -0.162460f, -0.262866f,  0.951056f },
	{  0.442863f, -0.238856f,  0.864188f },
	{  0.162460f, -0.262866f,  0.951056f },
	{  0.309017f, -0.500000f,  0.809017f },
	{  0.147621f, -0.716567f,  0.681718f },
	{  0.000000f, -0.525731f,  0.850651f },
	{  0.425325f, -0.688191f,  0.587785f },
	{  0.587785f, -0.425325f,  0.688191f },
	{  0.688191f, -0.587785f,  0.425325f },
	{ -0.955423f,  0.295242f,  0.000000f },
	{ -0.951056f,  0.162460f,  0.262866f },
	{ -1.000000f,  0.000000f,  0.000000f },
	{ -0.850651f,  0.000000f,  0.525731f },
	{ -0.955423f, -0.295242f,  0.000000f },
	{ -0.951056f, -0.162460f,  0.262866f },
	{ -0.864188f,  0.442863f, -0.238856f },
	{ -0.951056f,  0.162460f, -0.262866f },
	{ -0.809017f,  0.309017f, -0.500000f },
	{ -0.864188f, -0.442863f, -0.238856f },
	{ -0.951056f, -0.162460f, -0.262866f },
	{ -0.809017f, -0.309017f, -0.500000f },
	{ -0.681718f,  0.147621f, -0.716567f },
	{ -0.681718f, -0.147621f, -0.716567f },
	{ -0.850651f,  0.000000f, -0.525731f },
	{ -0.688191f,  0.587785f, -0.425325f },
	{ -0.587785f,  0.425325f, -0.688191f },
	{ -0.425325f,  0.688191f, -0.587785f },
	{ -0.425325f, -0.688191f, -0.587785f },
	{ -0.587785f, -0.425325f, -0.688191f },
	{ -0.688191f, -0.587785f, -0.425325f }  
};


/*============== EDGE REPRESENTATION ====================*/

struct md2_vertex_c
{
	float x, y, z;

	short normal_idx;
};

struct md2_frame_c
{
	md2_vertex_c *vertices;

	// list of normals which are used.  Terminated by -1.
	short *used_normals;
};

struct md2_point_c
{
	float skin_s, skin_t;

	// index into frame's vertex array (md2_frame_c::verts)
	int vert_idx;
};

struct md2_strip_c
{
	// either GL_TRIANGLE_STRIP or GL_TRIANGLE_FAN
	GLenum mode;

	// number of points in this strip / fan
	int count;

	// index to the first point (within md2_model_c::points).
	// All points for the strip are contiguous in that array.
	int first;
};

class md2_model_c
{
public:
	int num_frames;
	int num_points;
	int num_strips;

	md2_frame_c *frames;
	md2_point_c *points;
	md2_strip_c *strips;

	int verts_per_frame;

public:
	md2_model_c(int _nframe, int _npoint, int _nstrip) :
		num_frames(_nframe), num_points(_npoint),
		num_strips(_nstrip), verts_per_frame(0)
	{
		frames = new md2_frame_c[num_frames];
		points = new md2_point_c[num_points];
		strips = new md2_strip_c[num_strips];
	}

	~md2_model_c()
	{
		delete[] frames;
		delete[] points;
		delete[] strips;
	}
};


/*============== LOADING CODE ====================*/

static short *CreateNormalList(byte *which_normals)
{
	int count = 0;
	int i;
	
	for (i=0; i < MD2_NUM_NORMALS; i++)
		if (which_normals[i])
			count++;

	short *n_list = new short[count+1];

	count = 0;

	for (i=0; i < MD2_NUM_NORMALS; i++)
		if (which_normals[i])
			n_list[count++] = i;

	n_list[count] = -1;
	
	return n_list;
}


md2_model_c *MD2_LoadModel(epi::file_c *f)
{
	int i;

	raw_md2_header_t header;

	/* read header */
	f->Read(&header, sizeof (raw_md2_header_t));

	//!!!! FIXME:
	if ( /* (header.ident != 844121161) || */
		(header.version != 8))
	{
		I_Error("MD2_LoadModel: bad header or version!");
		return NULL; /* NOT REACHED */
	}

	int num_frames = EPI_LE_S32(header.num_frames);
	int num_points = 0;
	int num_strips = 0;

	/* PARSE GLCMDS */

	int num_glcmds = EPI_LE_S32(header.num_glcmds);

	s32_t *glcmds = new s32_t[num_glcmds];

	f->Seek(EPI_LE_S32(header.ofs_glcmds), epi::file_c::SEEKPOINT_START);
	f->Read(glcmds, num_glcmds * sizeof(s32_t));

	for (int aa = 0; aa < num_glcmds; aa++)
		glcmds[aa] = EPI_LE_S32(glcmds[aa]);

	// determine total number of strips and points
	for (i = 0; i < num_glcmds && glcmds[i] != 0; )
	{
		int count = glcmds[i++];

		if (count < 0)
			count = -count;

		num_strips += 1;
		num_points += count;

		i += count*3;
	}

	L_WriteDebug("MODEL INFO:\n");
	L_WriteDebug("  frames:%d  points:%d  strips: %d\n",
			num_frames, num_points, num_strips);

	md2_model_c *md = new md2_model_c(num_frames, num_points, num_strips);

	md->verts_per_frame = EPI_LE_S32(header.num_vertices);

	L_WriteDebug("  verts_per_frame:%d  glcmds:%d\n", md->verts_per_frame, num_glcmds);

	// convert glcmds into strips and points
	md2_strip_c *strip = md->strips;
	md2_point_c *point = md->points;

	for (i = 0; i < num_glcmds && glcmds[i] != 0; )
	{
		int count = glcmds[i++];

		SYS_ASSERT(strip < md->strips + md->num_strips);
		SYS_ASSERT(point < md->points + md->num_points);

		strip->mode = (count < 0) ? GL_TRIANGLE_FAN : GL_TRIANGLE_STRIP;

		if (count < 0)
			count = -count;

		strip->count = count;
		strip->first = point - md->points;

		strip++;

		for (; count > 0; count--, point++, i += 3)
		{
			float *f_ptr = (float *) &glcmds[i];

			point->skin_s   = f_ptr[0];
			point->skin_t   = 1.0 - f_ptr[1];
			point->vert_idx = glcmds[i+2];

			SYS_ASSERT(point->vert_idx >= 0);
			SYS_ASSERT(point->vert_idx < md->verts_per_frame);
		}
	}

	SYS_ASSERT(strip == md->strips + md->num_strips);
	SYS_ASSERT(point == md->points + md->num_points);

	delete[] glcmds;

	/* PARSE FRAMES */

	byte which_normals[MD2_NUM_NORMALS];

	raw_md2_vertex_t *raw_verts = new raw_md2_vertex_t[md->verts_per_frame];

	f->Seek(EPI_LE_S32(header.ofs_frames), epi::file_c::SEEKPOINT_START);

	for (i = 0; i < num_frames; i++)
	{
		raw_md2_frame_t raw_frame;

		f->Read(&raw_frame, sizeof(raw_frame));

		for (int j = 0; j < 3; j++)
		{
			raw_frame.scale[j]     = EPI_LE_U32(raw_frame.scale[j]);
			raw_frame.translate[j] = EPI_LE_U32(raw_frame.translate[j]);
		}

		float *f_ptr = (float *) raw_frame.scale;

		float scale[3];
		float translate[3];

		scale[0] = f_ptr[0];
		scale[1] = f_ptr[1];
		scale[2] = f_ptr[2];

		translate[0] = f_ptr[3];
		translate[1] = f_ptr[4];
		translate[2] = f_ptr[5];

#ifdef DEBUG_MD2_LOAD
		L_WriteDebug("  __FRAME_%d__\n", i);
		L_WriteDebug("    scale: %1.2f, %1.2f, %1.2f\n", scale[0], scale[1], scale[2]);
		L_WriteDebug("    translate: %1.2f, %1.2f, %1.2f\n", translate[0], translate[1], translate[2]);
#endif

		f->Read(raw_verts, md->verts_per_frame * sizeof(raw_md2_vertex_t));

		md->frames[i].vertices = new md2_vertex_c[md->verts_per_frame];

		memset(which_normals, 0, sizeof(which_normals));

		for (int v = 0; v < md->verts_per_frame; v++)
		{
			raw_md2_vertex_t *raw_V  = raw_verts + v;
			md2_vertex_c     *good_V = md->frames[i].vertices + v;

			good_V->x = (int)raw_V->x * scale[0] + translate[0];
			good_V->y = (int)raw_V->y * scale[1] + translate[1];
			good_V->z = (int)raw_V->z * scale[2] + translate[2];

#ifdef DEBUG_MD2_LOAD
			L_WriteDebug("    __VERT_%d__\n", v);
			L_WriteDebug("      raw: %d,%d,%d\n", raw_V->x, raw_V->y, raw_V->z);
			L_WriteDebug("      normal: %d\n", raw_V->light_normal);
			L_WriteDebug("      good: %1.2f, %1.2f, %1.2f\n", good_V->x, good_V->y, good_V->z);
#endif
			good_V->normal_idx = raw_V->light_normal;

			SYS_ASSERT(good_V->normal_idx >= 0);
			SYS_ASSERT(good_V->normal_idx < MD2_NUM_NORMALS);

			which_normals[good_V->normal_idx] = 1;
		}

		md->frames[i].used_normals = CreateNormalList(which_normals);
	}

	delete[] raw_verts;

	return md;
}


/*============== MODEL RENDERING ====================*/


typedef struct
{
	md2_model_c *model;

	const md2_frame_c *frame1;
	const md2_frame_c *frame2;
	const md2_strip_c *strip;

	float lerp;
	float x, y, z;

	bool is_weapon;

	// scaling
	float xy_scale;
	float  z_scale;
	float bias;

	// mlook vectors
	vec2_t kx_mat;
	vec2_t kz_mat;

	// rotation vectors
	vec2_t rx_mat;
	vec2_t ry_mat;

	mobj_t *mo;

	multi_color_c nm_colors[MD2_NUM_NORMALS];

	short * used_normals;

	bool is_additive;

public:
	void CalcPos(vec3_t *pos, const md2_vertex_c *vert) const
	{
		float x1 = vert->x * xy_scale;
		float y1 = vert->y * xy_scale;
		float z1 = (vert->z + bias) * z_scale;

		float x2 = x1 * kx_mat.x + z1 * kx_mat.y;
		float z2 = x1 * kz_mat.x + z1 * kz_mat.y;
		float y2 = y1;

		pos->x = x + x2 * rx_mat.x + y2 * rx_mat.y;
		pos->y = y + x2 * ry_mat.x + y2 * ry_mat.y;
		pos->z = z + z2;
	}

	void CalcNormal(vec3_t *normal, const md2_vertex_c *vert) const
	{
		short n = vert->normal_idx;

		float nx1 = md2_normals[n].x;
		float ny1 = md2_normals[n].y;
		float nz1 = md2_normals[n].z;

		float nx2 = nx1 * kx_mat.x + nz1 * kx_mat.y;
		float nz2 = nx1 * kz_mat.x + nz1 * kz_mat.y;
		float ny2 = ny1;

		normal->x = nx2 * rx_mat.x + ny2 * rx_mat.y;
		normal->y = nx2 * ry_mat.x + ny2 * ry_mat.y;
		normal->z = nz2;
	}
}
model_coord_data_t;


static void InitNormalColors(model_coord_data_t *data)
{
	short *n_list = data->used_normals;

	for (; *n_list >= 0; n_list++)
	{
		data->nm_colors[*n_list].Clear();
	}
}

static void ShadeNormals(abstract_shader_c *shader,
		 model_coord_data_t *data)
{
	short *n_list = data->used_normals;

	for (; *n_list >= 0; n_list++)
	{
		short n = *n_list;

		// FIXME !!!! pre-rotate normals too
		float nx, ny, nz;
		{
			float nx1 = md2_normals[n].x;
			float ny1 = md2_normals[n].y;
			float nz1 = md2_normals[n].z;

			float nx2 = nx1 * data->kx_mat.x + nz1 * data->kx_mat.y;
			float nz2 = nx1 * data->kz_mat.x + nz1 * data->kz_mat.y;
			float ny2 = ny1;

			nx = nx2 * data->rx_mat.x + ny2 * data->rx_mat.y;
			ny = nx2 * data->ry_mat.x + ny2 * data->ry_mat.y;
			nz = nz2;
		}

		shader->Corner(data->nm_colors + n, nx, ny, nz, data->mo, data->is_weapon);
	}
}

static void DLIT_Model(mobj_t *mo, void *dataptr)
{
	model_coord_data_t *data = (model_coord_data_t *)dataptr;

	// dynamic lights do not light themselves up!
	if (mo == data->mo)
		return;

	SYS_ASSERT(mo->dlight.shader);

	ShadeNormals(mo->dlight.shader, data);
}

static int MD2_MulticolMaxRGB(model_coord_data_t *data, bool additive)
{
	int result = 0;

	short *n_list = data->used_normals;

	for (; *n_list >= 0; n_list++)
	{
		multi_color_c *col = &data->nm_colors[*n_list];

		int mx = additive ? col->add_MAX() : col->mod_MAX();

		result = MAX(result, mx);
	}

	return result;
}

static void UpdateMulticols(model_coord_data_t *data)
{
	short *n_list = data->used_normals;

	for (; *n_list >= 0; n_list++)
	{
		multi_color_c *col = &data->nm_colors[*n_list];

		col->mod_R -= 256;
		col->mod_G -= 256;
		col->mod_B -= 256;
	}
}


static void ModelCoordFunc(void *d, int v_idx, vec3_t *pos,
				float *rgb, vec2_t *texc, vec3_t *normal)
{
	model_coord_data_t *data = (model_coord_data_t *)d;

	const md2_model_c *md = data->model;

	const md2_frame_c *frame1 = data->frame1;
	const md2_frame_c *frame2 = data->frame2;
	const md2_strip_c *strip  = data->strip;

	SYS_ASSERT(strip->first + v_idx >= 0);
	SYS_ASSERT(strip->first + v_idx < md->num_points);

	const md2_point_c *point = &md->points[strip->first + v_idx];

	const md2_vertex_c *vert1 = &frame1->vertices[point->vert_idx];
	const md2_vertex_c *vert2 = &frame2->vertices[point->vert_idx];

	
	vec3_t pos1, pos2;

	data->CalcPos(&pos1, vert1);
	data->CalcPos(&pos2, vert2);

	pos->x = pos1.x * (1.0f - data->lerp) + pos2.x * data->lerp;
	pos->y = pos1.y * (1.0f - data->lerp) + pos2.y * data->lerp;
	pos->z = pos1.z * (1.0f - data->lerp) + pos2.z * data->lerp;

	
	texc->Set(point->skin_s, point->skin_t);

// FUZZ TEST !!!
if (true)
{
	if (data->is_weapon)
	{
		texc->x *= 0.8;
		texc->y *= 0.8;
	}
	else if (! viewiszoomed)
	{
		float dist = P_ApproxDistance(data->mo->x - viewx, data->mo->y - viewy, data->mo->z - viewz);
		float factor = 70.0 / CLAMP(35, dist, 700);

		texc->x *= factor;
		texc->y *= factor;
	}

	FUZZ_Adjust(texc, data->mo);
}

	const md2_vertex_c *n_vert = (data->lerp < 0.5) ? vert1 : vert2;

	data->CalcNormal(normal, n_vert);


	multi_color_c *col = &data->nm_colors[n_vert->normal_idx];

	if (! data->is_additive)
	{
		rgb[0] = col->mod_R / 255.0;
		rgb[1] = col->mod_G / 255.0;
		rgb[2] = col->mod_B / 255.0;
	}
	else
	{
		rgb[0] = col->add_R / 255.0;
		rgb[1] = col->add_G / 255.0;
		rgb[2] = col->add_B / 255.0;
	}
}


void MD2_RenderModel(md2_model_c *md, GLuint skin_tex, bool is_weapon,
		             int frame1, int frame2, float lerp,
		             float x, float y, float z, mobj_t *mo,
					 region_properties_t *props,
					 float scale, float aspect, float bias)
{
	// check if frames are valid
	if (frame1 < 0 || frame1 >= md->num_frames)
	{
I_Debugf("Render model: bad frame %d\n", frame1);
		return;
	}
	if (frame2 < 0 || frame2 >= md->num_frames)
	{
I_Debugf("Render model: bad frame %d\n", frame1);
		return;
	}

	int fuzzy = (mo->flags & MF_FUZZY);

	float trans = fuzzy ? 1.0f : mo->visibility;

	int blending = BL_CullBack | (trans < 0.99 ? BL_Alpha : 0);


	model_coord_data_t data;

	data.mo = mo;
	data.model = md;

	data.frame1 = & md->frames[frame1];
	data.frame2 = & md->frames[frame2];

	data.lerp = lerp;

	data.x = x;
	data.y = y;
	data.z = z;

	data.is_weapon = is_weapon;

	data.xy_scale = scale * aspect;
	data. z_scale = scale;
	data.bias = bias;

	M_Angle2Matrix(is_weapon ? ~mo->vertangle : 0, &data.kx_mat, &data.kz_mat);

	
	angle_t ang = mo->angle;

	MIR_Angle(ang);

	M_Angle2Matrix(~ ang, &data.rx_mat, &data.ry_mat);


	data.used_normals = (lerp < 0.5) ? data.frame1->used_normals : data.frame2->used_normals;

	InitNormalColors(&data);


	if (! fuzzy)
	{
		abstract_shader_c *shader = R_GetColormapShader(props, mo->state->bright);

		ShadeNormals(shader, &data);

		if (use_dlights)
		{
			float r = mo->radius;
			
			P_DynamicLightIterator(mo->x - r, mo->y - r, mo->z,
					               mo->x + r, mo->y + r, mo->z + mo->height,
								   DLIT_Model, &data);

			P_SectorGlowIterator(mo->subsector->sector,
					             mo->x - r, mo->y - r, mo->z,
					             mo->x + r, mo->y + r, mo->z + mo->height,
								 DLIT_Model, &data);
		}
	}

blending |= BL_Alpha;
trans=1.00; //!!!!! FUZZ TEST


	/* draw the model */

	for (int pass = 0; pass < 1; pass++) //!!!!!! pass < 4
	{
		if (pass > 0 && pass < 3)
		{
			UpdateMulticols(&data);
			if (MD2_MulticolMaxRGB(&data, false) <= 0)
				continue;
		}

		if (pass == 3 && MD2_MulticolMaxRGB(&data, true) <= 0)
			continue;

		if (pass >= 1)
		{
			blending &= ~BL_Alpha;
			blending |=  BL_Add;
		}

		data.is_additive = (pass == 3);

		if (pass == 2 && detail_level < 2 && !is_weapon)
			continue;

		for (int i = 0; i < md->num_strips; i++)
		{
			data.strip = & md->strips[i];

			local_gl_vert_t * glvert = RGL_BeginUnit(
					 data.strip->mode, data.strip->count,
					 GL_MODULATE, data.is_additive ? 0 : skin_tex,
					 ENV_NONE, 0, pass, blending);

			for (int v_idx=0; v_idx < md->strips[i].count; v_idx++)
			{
				local_gl_vert_t *dest = glvert + v_idx;

				ModelCoordFunc(&data, v_idx, &dest->pos, dest->rgba,
						&dest->texc[0], &dest->normal);

				dest->rgba[3] = trans;
			}

			RGL_EndUnit(md->strips[i].count);
		}
	}
}


void MD2_RenderModel_2D(md2_model_c *md, GLuint skin_tex, int frame,
		                float x, float y, float xscale, float yscale,
		                const mobjtype_c *info)
{
	// check if frame is valid
	if (frame < 0 || frame >= md->num_frames)
		return;

	xscale = yscale * info->model_scale * info->model_aspect;
	yscale = yscale * info->model_scale;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, skin_tex);
 
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	if (info->flags & MF_FUZZY)
		glColor4f(0, 0, 0, 0.5f);
	else
		glColor4f(1, 1, 1, 1.0f);


	for (int i = 0; i < md->num_strips; i++)
	{
		const md2_strip_c *strip = & md->strips[i];

		glBegin(strip->mode);

		for (int v_idx=0; v_idx < md->strips[i].count; v_idx++)
		{
			const md2_frame_c *frame_ptr = & md->frames[frame];

			SYS_ASSERT(strip->first + v_idx >= 0);
			SYS_ASSERT(strip->first + v_idx < md->num_points);

			const md2_point_c *point = &md->points[strip->first + v_idx];
			const md2_vertex_c *vert = &frame_ptr->vertices[point->vert_idx];

			glTexCoord2f(point->skin_s, point->skin_t);


			short n = vert->normal_idx;

			float norm_x = md2_normals[n].x;
			float norm_y = md2_normals[n].y;
			float norm_z = md2_normals[n].z;

			glNormal3f(norm_y, norm_z, norm_x);


			float dx = vert->x * xscale;
			float dy = vert->y * xscale;
			float dz = (vert->z + info->model_bias) * yscale;

			glVertex3f(x + dy, y + dz, dx / 256.0f);
		}

		glEnd();
	}

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
}

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
