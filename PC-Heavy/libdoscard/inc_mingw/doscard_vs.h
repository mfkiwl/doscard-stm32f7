/*
 *  Copyright (C) 2015  Dmitry Soloviov
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

#ifndef DOSCARD_VS_H_INCL_
#define DOSCARD_VS_H_INCL_

//#include "doscard_common.h"

#ifdef BUILD_LDC_DLL
#define LDC_DLL __declspec(dllexport)
#else
#define LDC_DLL 
//__declspec(dllimport)
#endif

extern "C" {

//void LDC_DLL WinDosCardCreate(bool autoload);
//void LDC_DLL WinDosCardDestroy();

/// Try to load binary blob with filename provided.
bool LDC_DLL WinDosCardTryLoad(const char* filename);

/// Returns current state of class instance.
doscard::EDOSCRDState LDC_DLL WinDosCardGetCurrentState();

/// Returns a pointer to libdoscard settings block.
dosbox::LDB_Settings* LDC_DLL WinDosCardGetSettings();

/// Returns a safe pointer (not needed to be destroyed) to 0-terminated string.
char* LDC_DLL WinDosCardGetVersionStringSafe();

/// Try to apply settings provided.
bool LDC_DLL WinDosCardApplySettings(dosbox::LDB_Settings* pset);

/// Apply some video post-process settings.
void LDC_DLL WinDosCardApplyPostProcess(doscard::LDBI_PostProcess* pset);

/// Prepare instance for execution.
bool LDC_DLL WinDosCardPrepare();

/// Run instance (which eventually will create a thread).
int LDC_DLL WinDosCardRun();

/// Pause/Resume VM.
void LDC_DLL WinDosCardSetPause(bool paused);

/// Interleaved mode setting.
void LDC_DLL WinDosCardSetInterleave(uint32_t cycles);

/// Try to use as many CPU resources as possible.
void LDC_DLL WinDosCardUnlockSpeed(bool on);

/// Try to set capabilities flags.
int LDC_DLL WinDosCardSetCapabilities(doscard::LDBI_caps flags);

/// Returns a pointer to a valid VM video framebuffer and modifies width and height args.
uint32_t* LDC_DLL WinDosCardGetFramebuffer(int* w, int* h);

/// Puts event to a FIFO buffer of VM.
void LDC_DLL WinDosCardPutEvent(dosbox::LDB_UIEvent e);

/// Puts a string to VM's TTY interface.
void LDC_DLL WinDosCardPutString(char* str);

/// Returns a pointer to a vector filled with VM's messages.
doscard::LDBI_MesgVec* LDC_DLL WinDosCardGetMessages();

/// Fills a sound format request structure.
bool LDC_DLL WinDosCardGetSoundFormat(dosbox::LDB_SoundInfo* format);

/// Fills up a buffer with sound data generated by VM.
uint32_t LDC_DLL WinDosCardFillSound(doscard::LDBI_SndSample* buf, uint32_t samples);

void LDC_DLL WinDosCardInit(int verb);
void LDC_DLL WinDosCardExit(void);

}

#endif
