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


#ifndef DOSBOX_VIDEO_H
#define DOSBOX_VIDEO_H

namespace dosbox {

#define VGA_PARTS 4
#define VGA_MAXWIDTH 800

struct GFX_PalEntry {
	Bit8u r;
	Bit8u g;
	Bit8u b;
	Bit8u unused;
};

void GFX_Events(void);

/* Mouse related */
void GFX_CaptureMouse(void);
extern bool mouselocked; //true if mouse is confined to window

}

#endif
