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


#include <sys/types.h>
#include <assert.h>
#include <math.h>
#include "dosbox.h"
#include "video.h"
#include "render.h"
#include "cross.h"
#include "support.h"

namespace dosbox {

Render_t render;
RenderLineHandler_t RENDER_DrawLine;
static bool init_ok,render_dummy;
static uint32_t frmsk_cnt;

#ifndef LDB_EMBEDDED
static uint32_t hungry_buf[VGA_MAXWIDTH];
#endif

static INLINE int32_t LDB_SendDWord(Bit32u x)
{
	return myldbi->Callback(DBCB_PushScreen,&x,4);
}
/*
static inline void LDB_SendWord(Bit16u x)
{
	(*libdosbox_callbacks[DBCB_PushScreen])(&x,2);
}
*/
static INLINE int32_t LDB_SendByte(Bit8u x)
{
	return myldbi->Callback(DBCB_PushScreen,&x,1);
}

void RENDER_SetPal(Bit8u entry,Bit8u red,Bit8u green,Bit8u blue)
{
	render.pal.rgb[entry].red=red;
	render.pal.rgb[entry].green=green;
	render.pal.rgb[entry].blue=blue;
	if (render.pal.first>entry) render.pal.first=entry;
	if (render.pal.last<entry) render.pal.last=entry;
}

static void RENDER_FullLine(const void* src)
{
	if (!src) return;
	Bit8u* px8 = ((Bit8u*)src);
	Bit32u x,s;
	Bit32u* pb32;
	void* p_temp;

	for (x=0; x<render.src.width; x++) {
		switch (render.src.bpp) {
		case 8:
			s = px8[x];
#ifdef LDB_EMBEDDED
			LDB_SendByte(render.pal.rgb[s].blue);
			LDB_SendByte(render.pal.rgb[s].green);
			LDB_SendByte(render.pal.rgb[s].red);
#else
			hungry_buf[x] = render.pal.rgb[s].blue;
			hungry_buf[x] |= render.pal.rgb[s].green << 8;
			hungry_buf[x] |= render.pal.rgb[s].red << 16;
#endif
			break;
		case 15:
			s = ((Bit16u*)px8)[x];
#ifdef LDB_EMBEDDED
			LDB_SendByte(((s & 0x001f) * 0x21) >> 2);
			LDB_SendByte(((s & 0x03e0) * 0x21) >> 7);
			LDB_SendByte(((s & 0x7c00) * 0x21) >> 12);
#else
			hungry_buf[x] = ((s & 0x001f) * 0x21) >> 2;
			hungry_buf[x] |= ((s & 0x03e0) * 0x21) >> 7 << 8;
			hungry_buf[x] |= ((s & 0x7c00) * 0x21) >> 12 << 16;
#endif
			break;
		case 16:
			s = ((Bit16u*)px8)[x];
#ifdef LDB_EMBEDDED
			LDB_SendByte(((s & 0x001f) * 0x21) >> 2);
			LDB_SendByte(((s & 0x07e0) * 0x41) >> 9);
			LDB_SendByte(((s & 0xf800) * 0x21) >> 13);
#else
			hungry_buf[x] = ((s & 0x001f) * 0x21) >> 2;
			hungry_buf[x] |= ((s & 0x07e0) * 0x41) >> 9 << 8;
			hungry_buf[x] |= ((s & 0xf800) * 0x21) >> 13 << 16;
#endif
			break;
		case 32:
#ifdef LDB_EMBEDDED
			LDB_SendByte((((Bit32u*)src)[x]) & 255);
			LDB_SendByte((((Bit32u*)src)[x] >> 8) & 255);
			LDB_SendByte((((Bit32u*)src)[x] >> 16) & 255);
#else
			p_temp = const_cast<void*>(src);
			pb32 = reinterpret_cast<Bit32u*>(p_temp);
			myldbi->Callback(DBCB_PushScreen,pb32,render.src.width*4);
			return;
#endif
			break;
		}
	}
#ifndef LDB_EMBEDDED
	myldbi->Callback(DBCB_PushScreen,hungry_buf,render.src.width*4);
#endif
}

static void RENDER_EmptyLine(const void*)
{
}

bool RENDER_StartUpdate(void)
{
	if (!init_ok) {
		init_ok = true;
		frmsk_cnt = 0;
		return false;
	}
	if (render_dummy) {
		RENDER_DrawLine = RENDER_EmptyLine;
		return true;
	}
	RENDER_DrawLine = (frmsk_cnt < RENDER_FRMSK)? RENDER_EmptyLine:RENDER_FullLine;
	LDB_SendDWord(DISPLAY_NFRM_SIGNATURE);
	Bit32u hdr = ((Bit16u)render.src.width) << 16;
	hdr |= ((Bit16u)render.src.height);
	int ret = LDB_SendDWord(hdr);
	if (ret == DISPLAY_RET_BUSY)
		RENDER_DrawLine = RENDER_EmptyLine;
	return true;
}

void RENDER_EndUpdate(bool abort)
{
	if ((abort) || (frmsk_cnt < RENDER_FRMSK)) {
		//Do NOT update anything
		LDB_SendDWord(DISPLAY_ABOR_SIGNATURE);
	} else {
		//normal update
#ifdef LDB_EMBEDDED
		LDB_SendByte(0x01); //stop byte
#endif
	}
	if (frmsk_cnt < RENDER_FRMSK) frmsk_cnt++;
	else frmsk_cnt = 0;
}

void RENDER_SetSize(Bitu width,Bitu height,Bitu bpp,float fps,double ratio,bool dblw,bool dblh)
{
	render.src.width=width;
	render.src.height=height;
	render.src.bpp=bpp;
	render.src.dblw=dblw;
	render.src.dblh=dblh;
	render.src.fps=fps;
	render.src.ratio=ratio;
}

void RENDER_Init()
{
	render_dummy = (LDB_SendDWord(DISPLAY_INIT_SIGNATURE) == LDB_CALLBACK_RET_NOT_FOUND);
	if (render_dummy)
		LOG_MSG("RENDER: Dummy mode");
	init_ok = false;
}

}
