#include "../include/function.h"

void make_window8(SHEET *sheet, int xsize, int ysize, char *title, char act){

	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;

	BUFFER buf = (BUFFER) memman_alloc_4k(memman, xsize * ysize);
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

void make_window8_buf(SHEET *sheet, char* buf, int xsize, int ysize, char *title, char act){

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

void keyWinOff(SHEET *keyWin, SHEET *sheetNotepad, int *cur_c, int cur_x){
    changeWinTitle(keyWin, 0);
    if(keyWin == sheetNotepad){
        *cur_c = -1;
        putBoxOnSheet(keyWin, cur_x, 28, 8, 16, COL8_FFFFFF);
    }else{
        if((keyWin->flags & 0x20) != 0){    /* 是命令行窗口 */
            fifo32_put(&keyWin->task->fifo, 3);
        }
    }
    return;
}

void keyWinOn(SHEET *keyWin, SHEET *sheetNotepad, int *cur_c){
    changeWinTitle(keyWin, 1);
    if(keyWin == sheetNotepad)
        *cur_c = COL8_000000;
    else{
        if((keyWin->flags & 0x20) != 0){    /* 是命令行窗口 */
            fifo32_put(&keyWin->task->fifo, 2);
        }
    }
    return;
}

void changeWinTitle(SHEET *sheet, char act){
    int x, y;
    int xsize = sheet->bxsize;
	char tc_new, tbc_new, tc_old, tbc_old;
    char c;
	if(act != 0){
		tc_new = COL8_FFFFFF;
		tbc_new = COL8_000084;
        tc_old = COL8_C6C6C6;
		tbc_old = COL8_848484;
	}else{
        tc_new = COL8_C6C6C6;
		tbc_new = COL8_848484;
		tc_old = COL8_FFFFFF;
		tbc_old = COL8_000084;
	}
    for(y = 3; y <=20; y++){
        for(x = 3; x < xsize - 4; x++){
            c = sheet->buf[y * xsize + x];
            if(c == tc_old && x <= xsize - 22)
                c = tc_new;
            else if(c == tbc_old)
                c = tbc_new;
            sheet->buf[y * xsize + x] = c;
        }
    }
	sheet_refresh(sheet, 3, 3, xsize, 21);
    return;
}

