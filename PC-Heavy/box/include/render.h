/*
 *  Copyright (C) 2002-2013  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DOSBOX_RENDER_H
#define DOSBOX_RENDER_H

namespace dosbox {

#define RENDER_FRMSK myldbi->GetConfig()->frameskip

typedef struct {
	struct { 
		Bit8u red;
		Bit8u green;
		Bit8u blue;
		Bit8u unused;
	} rgb[256];
	union {
		Bit16u b16[256];
		Bit32u b32[256];
	} lut;
	bool changed;
	Bit8u modified[256];
	Bitu first;
	Bitu last;
} RenderPal_t;

typedef struct {
	struct {
		Bitu width, start;
		Bitu height;
		Bitu bpp;
		bool dblw,dblh;
		double ratio;
		float fps;
	} src;
	RenderPal_t pal;
	bool updating;
	bool active;
	bool aspect;
	bool fullFrame;
} Render_t;

typedef void (*RenderLineHandler_t)(const void*);

extern Render_t render;
extern RenderLineHandler_t RENDER_DrawLine;
void RENDER_SetSize(Bitu width,Bitu height,Bitu bpp,float fps,double ratio,bool dblw,bool dblh);
bool RENDER_StartUpdate(void);
void RENDER_EndUpdate(bool abort);
void RENDER_SetPal(Bit8u entry,Bit8u red,Bit8u green,Bit8u blue);

}

#endif
