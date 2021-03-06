#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "VM86Wrapper.h"
#include "fatfs.h"
#include "os.h"
#include "usart.h"
#include "pfont.h"

uint32_t OS_Last_Address = 0;

void OS_DrawChar(int col, int row, char x)
{
	if (x < 33 || x > 126) return;
	char _s[] = "char x\r\n";
	_s[5] = x;
	send(_s);

	__IO uint16_t* buf = (__IO uint16_t*)g_frames;
	int cx = (25-row)*8;
	int cy = col*4;
	for (int a,j,i = 0; i < 4; i++) {
		uint8_t sym = font8x4_rotated[(x-33)*4+i];
		for (j = 0; j < 8; j++) {
			if (sym & 0x80) {
				a = TFT_LCD_WIDTH*2*cy + cx*2;
				if (a >= TFT_TOTAL_BYTES) {
					send("Achtung!\r\n");
					for(;;);
				}
				buf[a] = 0x0700;
				buf[++a] = 0xE000;
			}
			sym <<= 1;
			cx++;
		}
		cx = (25-row)*8;
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
	snprintf(dbg,sizeof(dbg),"%u bytes read\r\n",*len);
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

void OS()
{
	memset(SDRAM_BANK_ADDR,0,TFT_TOTAL_BYTES);
	VM86_Start(OS_Last_Address);
	OS_PrintString("VM Init complete");

	for (int cyc = 0, l = 1, p = 0, r = 0;;cyc++) {
		r = VM86_FullStep();
		if (r < 0) break;

		if (r > 0x01000000) {
			l = (r & 0x00FF0000) >> 16;
			p = (r & 0x0000FF00) >> 8;
			char bp = r & 0xFF;
			if (bp) OS_DrawChar(p,l,bp);

		} else if (r == 1) {
			memset(SDRAM_BANK_ADDR,0,TFT_TOTAL_BYTES);
			char* bp = VM86_GetShadowBuf();
			for (l = 0; l < VM86W_GEOMH; l++)
				for (p = 0; p < VM86W_GEOMW; p++,bp++) {
					if (*bp) OS_DrawChar(p,l,*bp);
				}
		}
	}

	VM86_Stop();
}
