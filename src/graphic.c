/*图形处理相关*/
#include "../include/function.h"

void init_palette(void)
{
	/* data.c */
	extern unsigned char table_rgb[16 * 3];
	set_palette(0, 15, table_rgb);
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

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	for (; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

void init_mouse_cursor8(char *mouse, char bc)
/*准备鼠标光标（16×16）*/
{
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

void make_window8(MEMMAN *man, SHEET *sheet, int xsize, int ysize, char *title, char act){

	BUFFER buf = (BUFFER) memman_alloc_4k(man, xsize * ysize);
	sheet_setbuf(sheet, buf, xsize, ysize, -1);

	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);

	set_win_title_bar(sheet, title, act);
}

void set_win_title_bar(SHEET *sheet, char *title, char act){
	int x, y;
	char c, title_color, title_bar_color;
	if(act != 0){
		title_color = COL8_FFFFFF;
		title_bar_color = COL8_000084;
	}else{
		title_color = COL8_C6C6C6;
		title_bar_color = COL8_848484;
	}
	extern char closebtn[14][16];
	boxfill8(sheet->buf, sheet->bxsize, title_bar_color, 3, 3, sheet->bxsize - 4, 20);
	putfonts8_asc(sheet->buf, sheet->bxsize, 24, 4, title_color, title);
	for(y = 0; y < 14; y++){
		for(x = 0; x < 16; x++){
			c = closebtn[y][x];
			switch (c){
				case '@':
					c = COL8_000000;
					break;
				case '$':
					c = COL8_848484;
					break;
				case 'Q':
					c = COL8_C6C6C6;
					break;
				case 'O':
					c = COL8_FFFFFF;
					break;
				default:
					break;
			}
			sheet->buf[(5 + y) * sheet->bxsize + (sheet->bxsize - 21 + x)] = c;
		}
	}
}

void make_textbox8(SHEET *sheet, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sheet->buf, sheet->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sheet->buf, sheet->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sheet->buf, sheet->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sheet->buf, sheet->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sheet->buf, sheet->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sheet->buf, sheet->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sheet->buf, sheet->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sheet->buf, sheet->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sheet->buf, sheet->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void putStrOnSheet(SHEET *sheet, int x, int y, int font_color, char *str){
	int strLen = stringlength(str);
	int bgColor = sheet->buf[y * sheet->bxsize + x];
	boxfill8(sheet->buf, sheet->bxsize, bgColor, x, y, x + strLen * 8 - 1, y + 16 - 1);
	putfonts8_asc(sheet->buf, sheet->bxsize, x, y, font_color, str);
	sheet_refresh(sheet, x, y, x + strLen * 8, y + 16);
}

void putStrOnSheet_BG(SHEET *sheet, int x, int y, int font_color, int bg_color, char *str){
	int strLen = stringlength(str);
	int bgColor = bg_color;
	boxfill8(sheet->buf, sheet->bxsize, bgColor, x, y, x + strLen * 8 - 1, y + 16 - 1);
	putfonts8_asc(sheet->buf, sheet->bxsize, x, y, font_color, str);
	sheet_refresh(sheet, x, y, x + strLen * 8, y + 16);
}

void putBoxOnSheet(SHEET *sheet, int x, int y, int sx, int sy, int color){
	boxfill8(sheet->buf, sheet->bxsize, color, x, y, x + sx - 1, y + sy - 1);
	sheet_refresh(sheet, x, y, x + sx + 1, y + sy + 1);
}
