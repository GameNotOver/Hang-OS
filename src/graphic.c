/*图形处理相关*/
#include <stdio.h>
#include "../include/function.h"

int y_pos = 0;

void init_palette(void)
{
	/* data.c */
	extern unsigned char table_rgb[16 * 3];

	unsigned char table2[216 * 3];
	int r, g, b;

	set_palette(0, 15, table_rgb);

	for(b = 0; b < 6; b++){
		for(g = 0; g < 6; g++){
			for(r = 0; r < 6; r++){
				table2[(r + g * 6 + b * 36) * 3 + 0] = r * 51;
				table2[(r + g * 6 + b * 36) * 3 + 1] = g * 51;
				table2[(r + g * 6 + b * 36) * 3 + 2] = b * 51;
			}
		}
	}
	set_palette(16, 231, table2);
	return;
	/*static char命令只能用于数据，但相当于DB命令*/
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	/* 记录中断允许标志的值 */
	io_cli(); 					/* 允许标志设为0，屏蔽中断 */
	io_out8(0x03c8, start);
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* 撤消中断许可标志 */
	return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}

void init_screen8(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

void putfontGB2312(char *vram, int xsize, int x, int y, char c, char *font){
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i * 2];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s){
	extern char hankaku[4096];
	TASK *task = task_current();
	char *songti = (char *) *((int *) 0x0fc8);
	char *font;
	unsigned int k, t;

	if(task->langmode == 0){	/* ASCII */
		for (; *s != 0x00; s++) {
			putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
			x += 8;
		}
	}
	if(task->langmode == 1){	/* GB2312 */
		for (; *s != 0x00; s++) {
			if(task->langbyte1 == 0){
				if(0xa1 <= *s && *s <= 0xfe)
					task->langbyte1 = *s;
				else
					putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
			}else{
				k = task->langbyte1 - 0xa0;
				t = *s - 0xa0;
				task->langbyte1 = 0;
				font = songti + (94 * (k - 1) + (t - 1)) * 32;
				putfontGB2312(vram, xsize, x - 8, y, c, font);		/* 左半部分 */
				putfontGB2312(vram, xsize, x    , y, c, font+1);	/* 右半部分 */
			}
			x += 8;
		}
	}
	
	return;
}

void init_mouse_cursor8(char *mouse, int bc){	/*准备鼠标光标（16×16）*/
	/* data.c */
	extern char cursor[16][16];

	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}

void putStrOnSheet(SHEET *sheet, int x, int y, int font_color, char *str){
	int strLen = stringlength(str);
	putfonts8_asc(sheet->buf, sheet->bxsize, x, y, font_color, str);
	sheet_refresh(sheet, x, y, x + strLen * 8, y + 16);
}

void putStrOnSheet_BG(SHEET *sheet, int x, int y, int font_color, int bg_color, char *str){
	int strLen = stringlength(str);
	int bgColor = bg_color;
	TASK *task = task_current();
	boxfill8(sheet->buf, sheet->bxsize, bgColor, x, y, x + strLen * 8 - 1, y + 16 - 1);
	if(task->langmode != 0 && task->langbyte1 != 0){
		putfonts8_asc(sheet->buf, sheet->bxsize, x, y, font_color, str);
		sheet_refresh(sheet, x-8, y, x + strLen * 8, y + 16);
	}else{
		putfonts8_asc(sheet->buf, sheet->bxsize, x, y, font_color, str);
		sheet_refresh(sheet, x, y, x + strLen * 8, y + 16);
	}	
	
	
}

void putBoxOnSheet(SHEET *sheet, int x, int y, int sx, int sy, int color){
	boxfill8(sheet->buf, sheet->bxsize, color, x, y, x + sx - 1, y + sy - 1);
	sheet_refresh(sheet, x, y, x + sx + 1, y + sy + 1);
}

void putLineOnSheet(SHEET *sheet, int x0, int y0, int x1, int y1, int color){
	int i, x, y, len, dx, dy;
	dx = x1 - x0;
	dy = y1 - y0;
	x = x0 << 10;
	y = y0 << 10;
	if(dx < 0)
		dx = -dx;
	if(dy < 0)
		dy = -dy;
	if(dx >= dy){
		len = dx + 1;

		if(x1 - x0 < 0)
			dx = -1024;
		else
			dx = 1024;
		
		if(y0 <= y1)
			dy = ((y1 - y0 + 1) << 10) / len;
		else
			dy = ((y1 - y0 - 1) << 10) / len;
	}else{
		len = dy + 1;

		if(y0 > y1)
			dy = -1024;
		else
			dy = 1024;
		
		if(x0 <= x1)
			dx = ((x1 - x0 + 1) << 10) / len;
		else
			dx = ((x1 - x0 - 1) << 10) / len;
	}

	for(i = 0; i < len; i++){
		sheet->buf[(y >> 10) * sheet->bxsize + (x >> 10)] = color;
		x += dx;
		y += dy;
	}

	return;
}

void showMousePosition(int mx, int my){
	int bgColor;
	char s[11];
	SHEET *sheetBack = (SHEET *) *((int *) 0x0fc4);
	sprintf(s, "(%3d, %3d)", mx, my);
	bgColor = sheetBack->buf[0 * sheetBack->bxsize + 0];
	putStrOnSheet_BG(sheetBack, 0, 0, COL8_000000, bgColor, s);
}
