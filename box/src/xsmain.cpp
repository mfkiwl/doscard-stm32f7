/*
 *  Copyright (C) 2002-2013  The DOSBox Team
 *  Copyright (C) 2013-2014  Soloviov Dmitry
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h> //vsprintf
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include "cross.h"
#include "dosbox.h"
#include "video.h"
#include "mouse.h"
#include "pic.h"
#include "timer.h"
#include "setup.h"
#include "support.h"
#include "debug.h"
#include "mapper.h"
#include "vga.h"
#include "keyboard.h"
#include "cpu.h"
#include "cross.h"
#include "control.h"

extern const char* RunningProgram;
extern bool CPU_CycleAutoAdjust;

void GFX_SetTitle(Bit32s cycles,Bits frameskip,bool paused)
{
	char title[200]={0};
	static Bit32s internal_cycles=0;
	static Bits internal_frameskip=0;
	if(cycles != -1) internal_cycles = cycles;
	if(frameskip != -1) internal_frameskip = frameskip;
	if(CPU_CycleAutoAdjust) {
		sprintf(title,"DOSBox %s, CPU speed: max %3d%% cycles, Frameskip %2d, Program: %8s",VERSION,internal_cycles,internal_frameskip,RunningProgram);
	} else {
		sprintf(title,"DOSBox %s, CPU speed: %8d cycles, Frameskip %2d, Program: %8s",VERSION,internal_cycles,internal_frameskip,RunningProgram);
	}

	if(paused) strcat(title," PAUSED");
//	SDL_WM_SetCaption(title,VERSION);
}

void GFX_ResetScreen(void)
{

}

Bitu GFX_SetSize(Bitu width,Bitu height,Bitu flags,double scalex,double scaley,GFX_CallBack_t callback)
{

}

void GFX_GetSize(int &width, int &height, bool &fullscreen)
{

}

bool GFX_StartUpdate(Bit8u * & pixels,Bitu & pitch)
{

}

void GFX_EndUpdate(const Bit16u *changedLines )
{

}

void GFX_SetPalette(Bitu start,Bitu count,GFX_PalEntry * entries)
{

}

static void GUI_StartUp(Section * sec)
{
}

void GFX_Events()
{
	LDB_UIEvent evt;
	while ((*libdosbox_callbacks[DBCB_PullUIEvents])(&evt,sizeof(LDB_UIEvent))) {
		switch (evt.t) {
		case LDB_UIE_KBD:
			KEYBOARD_AddKey(evt.key,evt.pressed);
			break;
		case LDB_UIE_MOUSE:
			LOG_MSG("Mouse event: STUB");
			break;
		case LDB_UIE_QUIT:
			throw 1;
		default:
			LOG_MSG("Unknown event (type %d) received",static_cast<int>(evt.t));
			break;
		}
	}
}

void GFX_ShowMsg(char const* format,...)
{
	char buf[512];
	va_list msg;
	va_start(msg,format);
	vsnprintf(buf,511,format,msg);
	va_end(msg);
	(*libdosbox_callbacks[DBCB_PushMessage])(buf,sizeof(buf));
}

void MAPPER_AddHandler(MAPPER_Handler * handler,MapKeys key,Bitu mods,char const * const eventname,char const * const buttonname)
{
	LOG_MSG("MAPPER_AddHandler()");
}

void Mouse_AutoLock(bool enable)
{
}