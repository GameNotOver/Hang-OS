#include <stdio.h>
#include <string.h>
#include "../include/function.h"

void console_task(SHEET *sheet, unsigned int memtotal){

	TASK *task = task_current();
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
    CONSOLE cons;
	FILEHANDLE fhandle[8];

	unsigned char *songti = (unsigned char *) *((int *) 0x0fc8);

	int i;
	char cmdline[30];

    unsigned char *img_fat = (unsigned char *) (ADR_DISKIMG + 0x000200);

    file_readfat(fat, img_fat);

	cons.sheet = sheet;
    cons.cur_x = 8;
    cons.cur_y = 28;
    cons.cur_c = -1;

	task->cons = &cons;
	task->cmdline = cmdline;

	for(i = 0; i < 8; i++)
		fhandle[i].buf = NULL;
	task->fhandle = fhandle;
	task->fat = fat;

	if(cons.sheet != NULL){
		cons.timer = timer_alloc();
		timer_init(cons.timer, &task->fifo, 0);
		timer_settimer(cons.timer, 50);
	}


	/* 显示提示符 */
	cons_putchar(&cons, '>', 1);

	if(songti[4096] != 0xff){
		task->langmode = 1;
	}else{
		task->langmode = 0;
	}
	task->langbyte1 = 0;

	while(1){	
		io_cli();
		if(fifo32_status(&task->fifo) == 0){
			task_sleep(task);
			io_sti();
		}else{

			i = fifo32_get(&task->fifo);
			io_sti();

			switch(i){
				case 0: case 0xffffffff:
					if(cons.sheet != NULL){
						timer_init(cons.timer, &task->fifo, ~i);
						timer_settimer(cons.timer, 50);

						if(cons.cur_c >= 0)
							cons.cur_c = i == 0 ? COL8_000000 : COL8_FFFFFF;
					}
					break;
				case 2: case 3 :	/* 控制光标启用（2：启用，3：禁用） */
					cons.cur_c = i == 2 ? COL8_FFFFFF : -1;
					if(i == 3 && cons.sheet != NULL)
						putBoxOnSheet(cons.sheet, cons.cur_x, cons.cur_y, 8, 16, COL8_000000);
					break;
				case 4:		/* 点击命令行关闭按钮 */
					cmd_exit(&cons, fat);
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

						cons_runcmd(cmdline, &cons, fat, memtotal);	/* 执行命令 */

						if(cons.sheet == NULL)	/* 若是无窗口程序则程序结束时关闭命令行任务 */
							cmd_exit(&cons, fat);
						/* 显示提示符 */
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

			if(cons.cur_c >= 0 && cons.sheet != NULL)
				putBoxOnSheet(cons.sheet, cons.cur_x, cons.cur_y, 8, 16, cons.cur_c);

		}
	}
	
}

SHEET *open_console(){
	SHEETCTRL *sheetCtrl = (SHEETCTRL *) *((int *) 0x0fe4);
	SHEET *sheetCmd = sheet_alloc(sheetCtrl);

	make_window8(sheetCmd, 256, 165, "console", 0);
	make_textbox8(sheetCmd, 8, 28, 240, 128, COL8_000000);

	sheetCmd->task = open_constask(sheetCmd);
	sheetCmd->flags |= 0x20;	/* 有光标 */

	return sheetCmd;
}

void close_constask(TASK *taskCmd){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	task_sleep(taskCmd);
	memman_free_4k(memman, taskCmd->cons_stack, 64 * 1024);
	memman_free_4k(memman, (int) taskCmd->fifo.buf, 128 * sizeof(int));
	taskCmd->flags = TASK_FREE;
	return;
}

void close_console(SHEET *sheetCmd){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	TASK *taskCmd = sheetCmd->task;
	memman_free(memman, (int) sheetCmd->buf, 256 * 165);
	sheet_free(sheetCmd);
	close_constask(taskCmd);
	return;
}

TASK *open_constask(SHEET *sheet){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	int memtotal = *((int *) 0x0fe8);
	TASK *taskCmd = task_alloc();
	int *fifoBufCmd = (int *) memman_alloc_4k(memman, 128 * sizeof(int));

	taskCmd->cons_stack = memman_alloc_4k(memman, 64 * 1024);

	taskCmd->tss.eip = (int) &console_task;
	taskCmd->tss.esp = taskCmd->cons_stack + 64 * 1024 - 12;
	taskCmd->tss.es = 1 * 8;
	taskCmd->tss.cs = 2 * 8;	/* GDT的2号 */
	taskCmd->tss.ss = 1 * 8;
	taskCmd->tss.ds = 1 * 8;
	taskCmd->tss.fs = 1 * 8;
	taskCmd->tss.gs = 1 * 8;
	*((int *) (taskCmd->tss.esp + 4)) = (int) sheet;
	*((int *) (taskCmd->tss.esp + 8)) = memtotal;
	task_run(taskCmd, 2, 2);	/* level = 2; priority = 2 */

	init_fifo32(&taskCmd->fifo, 128, fifoBufCmd, taskCmd);

	return taskCmd;
}

void cons_newline(CONSOLE *cons){
    int x, y;
    SHEET *sheet = cons->sheet;
	TASK *task = task_current();
    int cur_x_start = 8;
    int cur_x_end = cur_x_start + 240;
    int cur_y_start = CONS_TITLE_LEN + 8;
    int cur_y_end = cur_y_start + 128;
    if(cons->cur_y < cur_y_end - 16){
        cons->cur_y += 16;
    }else{	/* 滚动 */
        /* 除第一行往上移一行 */
		if(sheet != NULL){
			for(y = cur_y_start; y < cur_y_end - 16; y++)
            for(x = cur_x_start; x < cur_x_end; x++)
                sheet->buf[x + sheet->bxsize * y] = sheet->buf[x + sheet->bxsize * (y + 16)];
			/* 最后一行涂黑 */
			for(y = cur_y_end - 16; y < cur_y_end; y++)	
				for(x = cur_x_start; x < cur_x_end; x++)
					sheet->buf[x + sheet->bxsize * y] = COL8_000000;
			sheet_refresh(sheet, cur_x_start, cur_y_start, cur_x_end, cur_y_end);
		}
    }
    cons->cur_x = 8;
	if(task->langmode == 1 && task->langbyte1 != 0)
		cons->cur_x += 8;
	return;
}

void cons_putchar(CONSOLE *cons, char c, char x_move){
    char s[2];
    s[0] = c;
    s[1] = 0;
    switch(s[0]){
        case 0x09 : /* 水平制表符 */
            do {
				if(cons->sheet != NULL)
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
			if(cons->sheet != NULL)
            	putStrOnSheet_BG(cons->sheet, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s);
				// putfonts8_asc_sht(cons->sheet, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
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
	}else if(strcmp(cmd, "exit") == 0){
		cmd_exit(cons, fat);
	}else if(strcmp(cmd, "start") == 0){
		cmd_start(cons, para);
	}else if(strcmp(cmd, "ncst") == 0){
		cmd_ncst(cons, para);
	}else if(strcmp(cmd, "langmode") == 0){
		cmd_langmode(cons, para);
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
	strcpy(cmd, cmdline);
	para[0] = 0;
    for(i = 0; cmd[i] != ' ' && i < len; i++){
        if(cmd[i+1] == ' '){
            cmd[i+1] = 0;
            strcpy(para, cmd + i + 2);
            break;
        }
    }
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

void cmd_exit(CONSOLE *cons, int *fat){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	TASK *task = task_current();
	SHEETCTRL *sheetCtrl = (SHEETCTRL *) *((int *) 0x0fe4);
	FIFO32 *fifo32 = (FIFO32 *) *((int *) 0x0fec);
	extern TASKCTRL *taskCtrl;
	if(cons->sheet != NULL)
		timer_cancel(cons->timer);
	memman_free_4k(memman, (int) fat, 4 * 2880);
	io_cli();
	if(cons->sheet != NULL)
		fifo32_put(fifo32, cons->sheet - sheetCtrl->sheets0 + 768); 	/* 768 ~ 1023 */
	else
		fifo32_put(fifo32, task - taskCtrl->tasks0 + 1024); 	/* 1024 ~ 2023 */
	io_sti();
	for(;;){
		task_sleep(task);
	}
	return;
}

void cmd_start(CONSOLE *cons, char *para){
	SHEETCTRL *sheetCtrl = (SHEETCTRL *) *((int *) 0x0fe4);
	SHEET *sheetCmd = open_console();
	FIFO32 *fifo32 = &sheetCmd->task->fifo;
	int i = 0;
	sheet_slide(sheetCmd, 320, 4);
	sheet_updown(sheetCmd, sheetCtrl->top);
	/* 将命令行输入的字符复制到新的窗口 */
	while(para[i++] != 0)
		fifo32_put(fifo32, para[i-1] + 256);
	fifo32_put(fifo32, 0x0a + 256);	/* 回车 */
	cons_newline(cons);
	return;
}

void cmd_ncst(CONSOLE *cons, char *para){
	TASK *task = open_constask(NULL);
	FIFO32 *fifo32 = &task->fifo;
	int i = 0;
	/* 将命令行输入的字符复制到新的窗口 */
	while(para[i++] != 0)
		fifo32_put(fifo32, para[i-1] + 256);
	fifo32_put(fifo32, 0x0a + 256);	/* 回车 */
	cons_newline(cons);
	return;
}

void cmd_langmode(CONSOLE *cons, char *para){
	TASK *task = task_current();
	unsigned char mode = para[0] - '0';
	char s[3];
	if(para[0] == 0){
		sprintf(s, "%d", task->langmode);
		cons_putstr(cons, s);
		cons_newline(cons);
		return;
	}
	if(mode <= 1){
		task->langmode = mode;
	}else{
		cons_putstr(cons, "mode number error\n");
	}
	cons_newline(cons);
	return;
}

int cmd_app(CONSOLE *cons, int *fat, char *cmdline){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	FILEINFO *finfo;
	SHEETCTRL *sheetCtrl = (SHEETCTRL *) *((int *) 0x0fe4);
	SHEET *sheet;
	char *img_file = (char *) (ADR_DISKIMG + 0x003e00);
	char *fileBuf;
	char *appBuf;

	TASK *task = task_current();

	char fname[18];
	char para[18];

	int segsiz, datsiz, esp, dathrb;

	cmd_getpara(cmdline, fname, para);
	
	finfo = file_search(fname, (FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if(finfo == NULL){
		strcat(fname, ".hrb");
		finfo = file_search(fname, (FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}

	if(finfo != NULL){	/* 找到文件的情况 */
		fileBuf = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, fileBuf, fat, img_file);
		if(finfo->size >= 36 && strcmp_len(fileBuf + 4, "Hari", 4) == 0 && *fileBuf == 0x00){						/* os_api程序启动 */
			//JMP	0x1b		; e8 16 00 00 00 cb
			segsiz = *((int *) (fileBuf + 0x0000));
			esp    = *((int *) (fileBuf + 0x000c));
			datsiz = *((int *) (fileBuf + 0x0010));
			dathrb = *((int *) (fileBuf + 0x0014));
			appBuf = (char *) memman_alloc_4k(memman, segsiz);
			//*((int *) 0xfe8) = (int) appBuf;	/* 存储代码段的起始位置 */
			task->ds_base = (int) appBuf;	/* 存储代码段的起始位置 */
			set_segmdesc(task->ldt + 0, finfo->size - 1, (int) fileBuf, AR_CODE32_ER + 0x60);
			set_segmdesc(task->ldt + 1, segsiz - 1, (int) appBuf, AR_DATA32_RW + 0x60);
			int i;
			for(i = 0; i < datsiz; i++){
				appBuf[esp+i] = fileBuf[dathrb+i];
			}

			start_app(0x1b, 0 * 8 + 4, esp, 1 * 8 + 4, &(task->tss.esp0));
			
			for(i = 0; i < MAX_SHEETS; i++){
				sheet = &(sheetCtrl->sheets0[i]);
				/* SHEET_USE | APP_WIN == 0x11 */
				if((sheet->flags & 0x11) == 0x11 && sheet->task == task){	
					/* 找到被应用程序遗漏的窗口 */
					sheet_free(sheet);	/* 关闭 */
				}
			}

			for(i = 0; i < 8; i++){		/* 将未关闭的窗口关闭 */
				if(task->fhandle[i].buf != NULL){
					memman_free_4k(memman, (int) task->fhandle[i].buf, task->fhandle[i].size);
					task->fhandle[i].buf = NULL;
				}
			}

			timer_cancelall(&task->fifo);
			memman_free_4k(memman, (int) appBuf, segsiz);
			task->langbyte1 = 0;
		}else{
			cons_putstr(cons, ".hrb file format error!\n");
		}
		
		memman_free_4k(memman, (int) fileBuf, finfo->size);
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
	
	TASK *task = task_current();
	// int ds_base = *((int *) 0xfe8);
	int ds_base = task->ds_base;
	FIFO32 *sys_fifo = (FIFO32 *) *((int *) 0x0fec);
	CONSOLE *cons = task->cons;
	SHEETCTRL *sheetCtrl = (SHEETCTRL *) *((int *) 0x0fe4);
	SHEET *sheet;

	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	FILEINFO *finfo;
	FILEHANDLE *fh;
	char *img_file = (char *) (ADR_DISKIMG + 0x003e00);

	// SHEET *sb = (SHEET *) *((int *) 0x0fc4);
	int *reg = &eax + 1;	/* eax后面的地址 */
		/* 强行改写通过PUSHAD保存的值 */
		/* reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP */
		/* reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */

	int len;
	int i;

	TIMER *tempTimer;
	
	switch(edx){
		case 1:		/* api_putchar */
			cons_putchar(cons, eax & 0xff, 1);
			break;
		case 2:		/* api_putstr */
			cons_putstr(cons, (char *) ebx + ds_base);
			break;
		case 3:		/*  */
			cons_putstr_len(cons, (char *) ebx + ds_base, ecx);
			break;
		case 4:		/* api_end */
			return &(task->tss.esp0);
		case 5:		/* api_openwin */
			sheet = sheet_alloc(sheetCtrl);
			sheet->task = task;
			sheet->flags |= 0x10;	/* 标记为应用窗口 */
			make_window8(sheet, esi, edi, (char *) ecx + ds_base, 0);
			/* 为了发挥refreshmap函数，x坐标对4的倍数向下取整 */
			sheet_slide(sheet, ((sheetCtrl->xsize - esi) / 2) & ~3, (sheetCtrl->ysize - edi) / 2);
			sheet_updown(sheet, sheetCtrl->top);
			reg[7] = (int) sheet;
			break;
		case 6:		/* api_putstrwin */
			sheet = (SHEET *) (ebx & 0xfffffffe);
			len = strlen((char *) ebp + ds_base);
			putStrOnSheet(sheet, esi, edi, eax, (char *) ebp + ds_base);
			if((ebx & 1 )== 0)
				sheet_refresh(sheet, esi, edi, esi + len * 8, edi + 16);
			break;
		case 7:		/* api_boxfilwin */
			sheet = (SHEET *) (ebx & 0xfffffffe);
			len = strlen((char *) ebp + ds_base);
			boxfill8(sheet->buf, sheet->bxsize, ebp, eax, ecx, esi, edi);
			if((ebx & 1) == 0)
				sheet_refresh(sheet, eax, ecx, esi + 1, edi + 1);
			break;
		case 8:		/* api_initmalloc */
			memman_init((MEMMAN *) (ebx + ds_base));
			ecx &= 0xfffffff0;	/* 以16字节为单位 */
			memman_free((MEMMAN *) (ebx + ds_base), eax, ecx);
			break;
		case 9:		/* api_malloc */
			ecx = (ecx + 0x0f) & 0xfffffff0;	/* 以16字节为单位进位取整 */
			reg[7] = memman_alloc((MEMMAN *) (ebx + ds_base), ecx);
			break;
		case 10:	/* api_free */
			ecx = (ecx + 0x0f) & 0xfffffff0;
			memman_free((MEMMAN *) (ebx + ds_base), eax, ecx);
			break;
		case 11:	/* api_point */
			sheet = (SHEET *) (ebx & 0xfffffffe);
			sheet->buf[sheet->bxsize * edi + esi] = eax;
			if((ebx & 1 )== 0)
				sheet_refresh(sheet, esi, edi, esi + 1, edi + 1);
			break;
		case 12:	/* api_refreshwin */
			sheet = (SHEET *) ebx;
			sheet_refresh(sheet, eax, ecx, esi, edi);
			break;
		case 13:	/* api_line */
			sheet = (SHEET *) (ebx & 0xfffffffe);
			putLineOnSheet(sheet, eax, ecx, esi, edi, ebp);
			if((ebx & 1) == 0){
				if(eax > esi){
					i = eax;
					eax = esi;
					esi = i;
				}
				if(ecx > edi){
					i = ecx;
					ecx = edi;
					edi = i;
				}
				sheet_refresh(sheet, eax, ecx, esi + 1, edi + 1);
			}
			break;
		case 14:	/* api_closewin */
			sheet = (SHEET *) ebx;
			sheet_free(sheet);
			break;
		case 15:	/* api_getkey */
			while(1){
				io_cli();
				if(fifo32_status(&task->fifo) == 0){
					if(eax != 0)
						task_sleep(task);
					else{
						io_sti();
						reg[7] = -1;
						return 0;
					}
				}
				i = fifo32_get(&task->fifo);
				io_sti();
				if(i == 0 || i == 0xffffffff){
					/* 应用程序运行时不需要显示光标， 因此总是将下次显示用的值置1 */
					timer_init(cons->timer, &task->fifo, 0xffffffff);
					timer_settimer(cons->timer, 50);
				}
				if(i == 2)	/* cursor on */
					cons->cur_c = COL8_FFFFFF;
				if(i == 3)	/* cursor off */
					cons->cur_c = -1;
				if(i == 4){	/* 只关闭命令行窗口 */
					timer_cancel(cons->timer);
					io_cli();
					fifo32_put(sys_fifo, cons->sheet - sheetCtrl->sheets0 + 2024);	/* 2024 ~ 2279 */
					cons->sheet = NULL;
					io_sti();
				}
				if(i >= 256){	/* 键盘数据 */
					
					reg[7] = i - 256;
					return 0;
				}
			}
			break;
		case 16:	/* api_alloctimer */
			tempTimer = timer_alloc();
			reg[7] = (int) tempTimer;
			tempTimer->flags_basic = TIMER_FLAGS_APP;	/* 允许自动取消 */
			break;
		case 17:	/* api_inittimer */
			timer_init((TIMER *) ebx, &task->fifo, eax + 256);
			break;
		case 18:	/* api_settimer */
			timer_settimer((TIMER *) ebx, eax);
			break;
		case 19:	/* api_freetimer */
			timer_free((TIMER *) ebx);
			break;
		case 20:
			if(eax == 0){
				i = io_in8(0x61);
				io_out8(0x61, i & 0x0d);
			}else{
				i = 1193180000 / eax;
				io_out8(0x43, 0xb6);
				io_out8(0x42, i & 0xff);
				io_out8(0x42, i >> 8);
				i = io_in8(0x61);
				io_out8(0x61, (i | 0x03) & 0x0f);
			}
			break;
		case 21:
			for(i = 0; i < 8; i++)
				if(task->fhandle[i].buf == NULL)
					break;
			fh = &task->fhandle[i];
			reg[7] = (int) NULL;
			if(i < 8){
				finfo = file_search((char *) ebx + ds_base, (FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
				if(finfo != NULL){
					reg[7] = (int) fh;
					fh->buf = (char *) memman_alloc_4k(memman, finfo->size);
					fh->size = finfo->size;
					fh->pos = 0;
					file_loadfile(finfo->clustno, finfo->size, fh->buf, task->fat, img_file);
				}
			}
			break;
		case 22:
			fh = (FILEHANDLE *) eax;
			memman_free_4k(memman, (int) fh->buf, fh->size);
			fh->buf = NULL;
			break;
		case 23:
			fh = (FILEHANDLE *) eax;
			if(ecx == 0)
				fh->pos = ebx;
			else if(ecx == 1)
				fh->pos += ebx;
			else if(ecx == 2)
				fh->pos = fh->size + ebx;
			if(fh->pos < 0)
				fh->pos = 0;
			if(fh->pos > fh->size)
				fh->pos = fh->size;
			break;
		case 24:
		 	fh = (FILEHANDLE *) eax;
			if(ecx == 0)
				reg[7] = fh->size;
			else if(ecx == 1)
				reg[7] = fh->pos;
			else if(ecx == 2)
				reg[7] = fh->pos - fh->size;
			break;
		case 25:
		 	fh = (FILEHANDLE *) eax;
			for(i = 0; i < ecx; i++){
				if(fh->pos == fh->size)
					break;
				*((char *) ebx + ds_base + i) = fh->buf[fh->pos];
				fh->pos++;
			}
			reg[7] = i;
			break;
		case 26:
		 	i = 0;
			for(;;){
				*((char *) ebx + ds_base + i) = task->cmdline[i];
				if(task->cmdline[i] == 0)
					break;
				if(i > ecx)
					break;
				i++;
			}
			reg[7] = i;
			break;
		case 27:
			reg[7] = task->langmode;
			break;
		case 0xff:	/* api_openwin_buf */
			sheet = sheet_alloc(sheetCtrl);
			sheet->task = task;
			sheet->flags |= 0x10;	/* 标记为应用窗口 */
			make_window8_buf(sheet, (char *) ebx + ds_base, esi, edi, (char *) ecx + ds_base, 0);
			/* 为了发挥refreshmap函数，x坐标对4的倍数向下取整 */
			sheet_slide(sheet, ((sheetCtrl->xsize - esi) / 2) & ~3, (sheetCtrl->ysize - edi) / 2);
			sheet_updown(sheet, sheetCtrl->top);
			reg[7] = (int) sheet;
			break;
		default :
			break;
	}
	return 0;
}

int *inthandler0c(int *esp){
	
	TASK *task = task_current();
	CONSOLE *cons = task->cons;
	char excp_addr[30];
	cons_putstr(cons, "\nINT 0C :\nStack Exception.\n");
	sprintf(excp_addr, "EIP = %08X\n", esp[11]);
	cons_putstr(cons, excp_addr);
	return &(task->tss.esp0);
}

int *inthandler0d(int *esp){
	
	TASK *task = task_current();
	CONSOLE *cons = task->cons;
	char excp_addr[30];
	cons_putstr(cons, "\nINT 0D :\nGeneral Protected Exception.\n");
	sprintf(excp_addr, "EIP = %08X\n", esp[11]);
	cons_putstr(cons, excp_addr);
	return &(task->tss.esp0);
}
