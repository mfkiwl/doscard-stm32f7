/*
 * This file is a part of the DOSCard project.
 *
 * (C) Copyright Dmitry 'MatrixS_Master' Soloviov, 2018
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "VM86Wrapper.h"
#include "VM86conf.h"
#include "fatfs.h"
#include "os.h"
#include "usart.h"
#include "pfont.h"

uint32_t OS_Last_Address = 0;
uint8_t OS_VK_CurSym = 'a';
static uint8_t OS_VK_Enabled = 0;
static uint8_t OS_VK_SymLoaded = 0;
static uint32_t OS_VK_Timestamp = 0;

static void OS_DrawRect(__IO uint16_t* to, int x0, int y0, int x1, int y1, const uint16_t col)
{
	for (to += y0*TFT_LCD_WIDTH; y0 < y1; y0++, to += TFT_LCD_WIDTH) {
		__IO uint16_t* ptr = to + x0;
		for (int i = x0; i < x1; i++,ptr++) *ptr = col;
	}
}

static void OS_DrawLargeChar(char x)
{
	g_frame_cnt = 1;
	__IO uint16_t* buf = (__IO uint16_t*)&(g_frames[g_frame_cnt*TFT_TOTAL_PIXELS]);
	memset((void*)buf,0,TFT_TOTAL_BYTES);

	if (x < OS_FONT_MINCODE || x > OS_FONT_MAXCODE) return;

	const int step = TFT_LCD_HEIGHT / 8;
	int cx = 8*step;
	int cy = 0;
	for (int a,j,i = 0; i < 8; i++) {
		uint8_t sym = font8x4[(x-33)*8+i];
		for (j = 0; j < 4; j++) {
			cx -= step;
			if (sym & 0x08) OS_DrawRect(buf,cx-step,cy,cx,cy+step,OS_OSD_COLOR);
			sym <<= 1;
		}
		cx = 8*step;
		cy += step;
	}
}

void OS_UpdateInput(int key)
{
	if (!OS_VK_Enabled) return;

	if (key) {
		//check for timer
		if ((key > 0) || (!OS_VK_SymLoaded && HAL_GetTick() - OS_VK_Timestamp > OS_VK_TIMEOUT)) {
			VM86_InsertKey((key > 0)? key:OS_VK_CurSym);
			OS_VK_SymLoaded = 1;
		}

	} else {
		OS_VK_SymLoaded = 0;
		//update current selection
		OS_DrawLargeChar(OS_VK_CurSym);

#if USB_DEBUG_EN
		char buf[64];
		snprintf(buf,sizeof(buf),"sym = %c\r\n",OS_VK_CurSym);
		send(buf);
#endif
	}

	if (key >= 0) OS_VK_Timestamp = HAL_GetTick();
}

static void OS_DrawChar(int col, int row, char x)
{
	if (x < OS_FONT_MINCODE || x > OS_FONT_MAXCODE) return;
	if (col < 0 || col > 79 || row < 0 || row > 24) return;


	g_frame_cnt = 0;
	__IO uint16_t* buf = (__IO uint16_t*)g_frames;
	int cx = (80-col)*4;
	int cy = row*8;
	for (int a,j,i = 0; i < 8; i++) {
		uint8_t sym = font8x4[(x-33)*8+i];
		for (j = 0; j < 4; j++) {
			cx--;
			if (sym & 0x08) {
				a = TFT_LCD_WIDTH*cy + cx;
				if (a >= TFT_TOTAL_BYTES) return;
				buf[a] = OS_FONT_COLOR;
			}
			sym <<= 1;
		}
		cx = (80-col)*4;
		cy++;
	}
}

uint8_t* OS_InitDisk(const char* name, uint32_t* len)
{
	FIL fp;
	UINT rb;
	uint8_t* to = (uint8_t*)OS_Last_Address;
	uint8_t* ret = to;
	memset(&fp,0,sizeof(fp));

	if (f_open(&fp,name,FA_READ) != FR_OK) return NULL;

	*len = 0;
	while (f_read(&fp,to,16,&rb) == FR_OK && rb > 0) {
		to += rb;
		*len += rb;
	}
	f_close(&fp);

	char dbg[64];
	snprintf(dbg,sizeof(dbg),"%lu bytes read\r\n",*len);
	HAL_UART_Transmit(&huart1,(uint8_t*)dbg,strlen(dbg),100);

	OS_Last_Address += *len;

	return ret;
}

void OS_PrintString(char* str)
{
	int col = 0, row = 1;
	for (unsigned i = 0; i < strlen(str); i++) {
		OS_DrawChar(col,row,str[i]);
		if (++col >= 80) {
			col = 0;
			row++;
		}
	}
}

void OS(os_callback_t cb)
{
	memset((void*)SDRAM_BANK_ADDR,0,TFT_TOTAL_BYTES);
	VM86_Start(OS_Last_Address);
	uint32_t ctm,timestp = HAL_GetTick();
	OS_PrintString("VM Init complete");
	OS_VK_Enabled = 1;

	for (int cyc = 0, l = 1, p = 0, r = 0;;cyc++) {
//		ctm = HAL_GetTick() - timestp;
//		if (ctm) {
//			rtc_millis += ctm;
//			timestp = HAL_GetTick();
//		}

		r = VM86_FullStep();
		if (r < 0) break;

		if (r > 0x01000000) {
			l = (r & 0x00FF0000) >> 16;
			p = (r & 0x0000FF00) >> 8;
			char bp = r & 0xFF;
			if (bp) OS_DrawChar(p,l,bp);

		} else if (r == 1) {
			memset((void*)SDRAM_BANK_ADDR,0,TFT_TOTAL_BYTES);
			char* bp = VM86_GetShadowBuf();
			for (l = 0; l < VM86W_GEOMH; l++)
				for (p = 0; p < VM86W_GEOMW; p++,bp++) {
					if (*bp) OS_DrawChar(p,l,*bp);
				}
		}

		if (cb && cb() < 0) break;
	}

	OS_VK_Enabled = 0;
	VM86_Stop();
}
