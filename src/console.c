#include <stdio.h>
#include <string.h>
#include "../include/function.h"

void console_task(SHEET *sheet, unsigned int memtotal){

	TIMER *timer;
	TASK *task = task_current();
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
    CONSOLE cons;

	int i;
	char cmdline[30];
	int fifoBuf[128];

    unsigned char *img_fat = (unsigned char *) (ADR_DISKIMG + 0x000200);

    file_readfat(fat, img_fat);

	cons.sheet = sheet;
    cons.cur_x = 8;
    cons.cur_y = 28;
    cons.cur_c = -1;

	*((int *) 0x0fec) = (int) &cons;

	init_fifo32(&task->fifo, 128, fifoBuf, task);

	timer = timer_alloc();
	timer_init(timer, &task->fifo, 0);
	timer_settimer(timer, 50);

	/* 显示提示符 */
	cons_putchar(&cons, '>', 1);

	while(1){	
		io_cli();
		if(fifo32_status(&task->fifo) == 0){
			task_sleep(task);
			io_sti();
		}else{

			i = fifo32_get(&task->fifo);
			io_sti();

			switch(i){
				case 0: case 0xff:
					timer_init(timer, &task->fifo, ~i);
					timer_settimer(timer, 50);

					if(cons.cur_c >= 0)
						cons.cur_c = i == 0 ? COL8_000000 : COL8_FFFFFF;
					
					break;
				case 2: case 3 :
					cons.cur_c = i == 2 ? COL8_FFFFFF : -1;
					if(i == 3) 
						putBoxOnSheet(sheet, cons.cur_x, cons.cur_y, 8, 16, COL8_000000);
					break;
				case 256 ... 511:
					if(i == 8 + 256){	/* 退格键 */
						if(cons.cur_x > 2 * 8){
							cons_putchar(&cons, ' ', 0);
							cons.cur_x -= 8;
						}
					}else if(i == 10 + 256){	/* 回车键 */
						cons_putchar(&cons, ' ', 0);
						cmdline[cons.cur_x / 8 - 2] = 0;    /* 给字符串添加结尾标志 */
						cons_newline(&cons);

						cons_runcmd(cmdline, &cons, fat, memtotal);

						cons_putchar(&cons, '>', 1);
					}else{	/* 其他字符 */
						if(cons.cur_x < 240) {
							cmdline[cons.cur_x / 8 - 2] = i -256;
							cons_putchar(&cons, i -256, 1);
						}
					}
					break;
				default:
					break;
			}	

			if(cons.cur_c >= 0)
				putBoxOnSheet(sheet, cons.cur_x, cons.cur_y, 8, 16, cons.cur_c);

		}
	}
	
}

void cons_newline(CONSOLE *cons){
    int x, y;
    SHEET *sheet = cons->sheet;
    int cur_x_start = 8;
    int cur_x_end = cur_x_start + 240;
    int cur_y_start = CONS_TITLE_LEN + 8;
    int cur_y_end = cur_y_start + 128;
    if(cons->cur_y < cur_y_end - 16){
        cons->cur_y += 16;
    }else{	/* 滚动 */
        /* 除第一行往上移一行 */
        for(y = cur_y_start; y < cur_y_end - 16; y++)
            for(x = cur_x_start; x < cur_x_end; x++)
                sheet->buf[x + sheet->bxsize * y] = sheet->buf[x + sheet->bxsize * (y + 16)];
        /* 最后一行涂黑 */
        for(y = cur_y_end - 16; y < cur_y_end; y++)	
            for(x = cur_x_start; x < cur_x_end; x++)
                sheet->buf[x + sheet->bxsize * y] = COL8_000000;
        sheet_refresh(sheet, cur_x_start, cur_y_start, cur_x_end, cur_y_end);
    }
    cons->cur_x = 8;
}

void cons_putchar(CONSOLE *cons, char c, char x_move){
    char s[2];
    s[0] = c;
    s[1] = 0;
    switch(s[0]){
        case 0x09 : /* 水平制表符 */
            do {
                putStrOnSheet_BG(cons->sheet, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ");
                cons->cur_x += 8;
                if(cons->cur_x - 8 == CONS_WIDTH - 16)    
                    cons_newline(cons);
            } while (((cons->cur_x - 8) & 0x1f) != 0);   /* 减去左边框（8） */ 
            break;
        case 0x0a : /* 换行 */ 
            cons_newline(cons);
            break;
        case 0x0d : /* 回车 */
            break;
        default :   /* 一般字符 */
            putStrOnSheet_BG(cons->sheet, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s);
            if(x_move != 0){  /* move为0时光标不后移 */
                cons->cur_x += 8;
                if(cons->cur_x - 8 == CONS_WIDTH - 16)
                    cons_newline(cons);
            }
    }
} 

void cons_runcmd(char *cmdline, CONSOLE *cons, int *fat, unsigned int memtotal){
    char cmd[10];
    char para[20];
    cmd_getpara(cmdline, cmd, para);
	if(strcmp(cmd, "mem") == 0){
		cmd_mem(cons, memtotal);
	}else if(strcmp(cmd, "cls") == 0){
		cmd_cls(cons);
	}else if(strcmp(cmd, "dir") == 0){
		cmd_dir(cons);
	}else if(strcmp(cmd, "cat") == 0){
		cmd_cat(cons, fat, para);
	}else if(cmd[0] != 0){
		if(cmd_app(cons, fat, cmdline) == 0){
			cons_putstr(cons, "Bad command!\n\n");
		}
	}
	return;
}

void cmd_getpara(char *cmdline, char *cmd, char *para){
    int len = strlen(cmdline);
    int i;
    for(i = 0; i < len; i++){
        if(cmdline[i] == ' '){
            cmdline[i] = 0;
            strcpy(para, cmdline + i + 1);
            break;
        }
    }
    strcpy(cmd, cmdline);    
}

void cmd_mem(CONSOLE *cons, unsigned int memtotal){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	char s[30];
	sprintf(s, "total: %d MB\nfree: %d KB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr(cons, s);
	return;
}

void cmd_cls(CONSOLE *cons){
	int x, y;
    SHEET *sheet = cons->sheet;
    int cur_x_start = 8;
    int cur_x_end = cur_x_start + 240;
    int cur_y_start = CONS_TITLE_LEN + 8;
    int cur_y_end = cur_y_start + 128;
	for(y = cur_y_start; y < cur_y_end; y++)
		for(x = cur_x_start; x < cur_x_end; x++)
			sheet->buf[x + sheet->bxsize * y] = COL8_000000;
	sheet_refresh(sheet, cur_x_start, cur_y_start, cur_x_end, cur_y_end);
	cons->cur_y = cur_y_start;
	return;
}

void cmd_dir(CONSOLE *cons){
	int x, y;
	char s[30];
	FILEINFO *finfo = (FILEINFO *) (ADR_DISKIMG + 0x002600);

	for(x = 0; x < 224; x++){
		if(finfo[x].name[0] == 0x00)
			break;
		if(finfo[x].name[0] != 0xe5){
			if((finfo[x].type & 0x18) == 0){
				sprintf(s, "filename.ext %7d\n", finfo[x].size);
				for(y = 0; y < 8; y++){
					s[y] = finfo[x].name[y];
				}
				s[9] = finfo[x].ext[0];
				s[10] = finfo[x].ext[1];
				s[11] = finfo[x].ext[2];
				cons_putstr(cons, s);
			}
		}
	}
	cons_newline(cons);
	return;
}

void cmd_cat(CONSOLE *cons, int *fat, char *fname){
	
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	FILEINFO *finfo = file_search(fname, (FILEINFO *) (ADR_DISKIMG + 0x002600), 244);
	char *img_file = (char *) (ADR_DISKIMG + 0x003e00);
	char *fileBuf;

	if(finfo != NULL){	/* 找到文件的情况 */

		fileBuf = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, fileBuf, fat, img_file);

		cons_putstr_len(cons, fileBuf, finfo->size);

		memman_free(memman, (int) fileBuf, finfo->size);
	}else{	/* 没有找到文件的情况 */
		cons_putstr(cons, "File Not Found!\n");
	}
	cons_newline(cons);
	return;
}

int cmd_app(CONSOLE *cons, int *fat, char *cmdline){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	FILEINFO *finfo;
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *) ADR_GDT;
	char *img_file = (char *) (ADR_DISKIMG + 0x003e00);
	char *fileBuf;
	char *appBuf;

	TASK *task = task_current();

	char fname[18];
	char para[18];

	int segsiz, datsiz, esp, dathrb;

	cmd_getpara(cmdline, fname, para);

	finfo = file_search(fname, (FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if(finfo == 0 && fname[strlen(fname) - 1] != 0){
		strcat(fname, ".hrb");
		finfo = file_search(fname, (FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}

	if(finfo != NULL){	/* 找到文件的情况 */
		fileBuf = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, fileBuf, fat, img_file);
		if(finfo->size >= 8 && strcmp_len(fileBuf + 4, "Hari", 4) == 0){
			//JMP	0x1b		; e8 16 00 00 00 cb
			segsiz = *((int *) (fileBuf + 0x0000));
			esp    = *((int *) (fileBuf + 0x000c));
			datsiz = *((int *) (fileBuf + 0x0010));
			dathrb = *((int *) (fileBuf + 0x0014));
			appBuf = (char *) memman_alloc_4k(memman, segsiz);
			*((int *) 0xfe8) = (int) appBuf;	/* 存储代码段的起始位置 */
			set_segmdesc(gdt + 1003, finfo->size - 1, (int) fileBuf, AR_CODE32_ER + 0x60);
			set_segmdesc(gdt + 1004, segsiz - 1, (int) appBuf, AR_DATA32_RW + 0x60);
			int i;
			for(i = 0; i < datsiz; i++){
				appBuf[esp+i] = fileBuf[dathrb+i];
			}
			start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0));
			memman_free(memman, (int) appBuf, segsiz);
		}else{
			cons_putstr(cons, ".hrb file format error!\n");
		}
		
		memman_free(memman, (int) fileBuf, finfo->size);
		cons_newline(cons);
		return 1;
	}

	return 0;	/* 没有找到文件的情况 */
}

void cons_putstr(CONSOLE *cons, char *str){
	while(*str != 0){
		cons_putchar(cons, *str, 1);
		str++;
	}

	return;
}

void cons_putstr_len(CONSOLE *cons, char *str, int length){
	int i = 0;

	while (i < length && str[i] != 0){
		cons_putchar(cons, str[i], 1);
		i++;
	}
	
	return;
}

int *os_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax){
	CONSOLE *cons = (CONSOLE *) *((int *) 0xfec);
	int cs_base = *((int *) 0xfe8);
	TASK *task = task_current();
	
	switch(edx){
		case 1 :
			cons_putchar(cons, eax & 0xff, 1);
			break;
		case 2 :
			cons_putstr(cons, (char *) ebx + cs_base);
			break;
		case 3 :
			cons_putstr_len(cons, (char *) ebx + cs_base, ecx);
			break;
		case 4 :
			return &(task->tss.esp0);
		default :
			break;
	}
	return 0;
}

int *inthandler0c(int *esp){
	CONSOLE *cons = (CONSOLE *) *((int *) 0x0fec);
	TASK *task = task_current();
	char excp_addr[30];
	cons_putstr(cons, "\nINT 0C :\nStack Exception.\n");
	sprintf(excp_addr, "EIP = %08X\n", esp[11]);
	cons_putstr(cons, excp_addr);
	return &(task->tss.esp0);
}

int *inthandler0d(int *esp){
	CONSOLE *cons = (CONSOLE *) *((int *) 0x0fec);
	TASK *task = task_current();
	char excp_addr[30];
	cons_putstr(cons, "\nINT 0D :\nGeneral Protected Exception.\n");
	sprintf(excp_addr, "EIP = %08X\n", esp[11]);
	cons_putstr(cons, excp_addr);
	return &(task->tss.esp0);
}
