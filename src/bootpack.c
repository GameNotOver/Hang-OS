/* bootpack的主要部分 */

#include <stdio.h>
#include <string.h>
#include "../include/function.h"

void console_task(SHEET *sheet, unsigned int memtotal);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	char s[40];
	int mx, my;

	/* 	fifo32 分配：
		0x00、0xff：光标闪烁用定时器
		1 ~ 10		1 ~ 10秒定时器
		256~511		键盘输入（256 bits）
		512~767		鼠标输入（256 bits）
		*/
	FIFO32 fifo32, keyCmdFifo;
	int fifo32buf[128], keyCmdBuf[32];

	MOUSE_DEC mdec;

	unsigned int memtotal;
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	SHEETCTRL *sheetCtrl;
	SHEET *sheetBack, *sheetMouse, *sheetWin, *sheet_win_b[3];
	unsigned char *bufBack, bufMouse[256];

	TIMER *timer_1, *timer_3, *timer_10, *timer_cursor;

	extern char keytable0[0x80];
	extern char keytable1[0x80];
	int key_shift = 0;
	int key_to = 0;
	int key_leds = (binfo->leds >> 4) & 7;
	int keycmd_wait = -1;

	TASK *task_a;

	SHEET *sheetCmd;
	TASK *taskCmd;

	SHEET *mouseMove[2];

	int i;
	
	init_gdtidt();
	init_pic();
	init_pit();

	io_sti();

	/* 注册内存 */
	memtotal = memtest(0x00400000, 0xbfffffff) /* B */ ;
	memman_init(memman);
	/* 以下两次共注册 632KB + 28MB, 即 29304KB 大小的空间*/
	memman_free(memman, 0x00001000, 0x0009e000);	/* 0x00001000 ~ 0x0009efff 共632KB*/
	memman_free(memman, 0x00400000, memtotal - 0x00400000);	/* 共(memtotal-0x00400000)B，即 32MB - 4MB = 28MB*/

	init_fifo32(&fifo32, 128, fifo32buf, NULL);
	init_keyboard(&fifo32, 256);
	enable_mouse(&mdec, &fifo32, 512);

	init_fifo32(&keyCmdFifo, 32, keyCmdBuf, 0);
	fifo32_put(&keyCmdFifo, KEYCMD_LED);
	fifo32_put(&keyCmdFifo, key_leds);

	io_out8(PIC0_IMR, IRQALLOW_TIMER & IRQALLOW_PIC_1 & IRQALLOW_KEYBD); /* 允许PIT、PIC1和键盘 */
	io_out8(PIC1_IMR, IRQALLOW_MOUSE); /* 允许鼠标 */

	/* 调色板初始化 */
	init_palette();

	/* sheet控制器初始化 */
	sheetCtrl = sheetctrl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

	/* 背景sheet */
	sheetBack = sheet_alloc(sheetCtrl);
	bufBack = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sheetBack, bufBack, binfo->scrnx, binfo->scrny, -1); /* 没有透明色 */
	init_screen8(bufBack, binfo->scrnx, binfo->scrny);

	/* 鼠标sheet */
	sheetMouse = sheet_alloc(sheetCtrl);
	sheet_setbuf(sheetMouse, bufMouse, 16, 16, COL8_TRSPAR);	/* 透明色号99 */
	init_mouse_cursor8(bufMouse, COL8_TRSPAR);	/* 背景色号99 */
	
	/* 1s定时器 */
	timer_1 = timer_alloc();
	timer_init(timer_1, &fifo32, 1);
	timer_settimer(timer_1, 1 * 100);
	/* 3s定时器 */
	timer_3 = timer_alloc();
	timer_init(timer_3, &fifo32, 3);
	timer_settimer(timer_3, 3 * 100);
	/* 10s定时器 */
	timer_10 = timer_alloc();
	timer_init(timer_10, &fifo32, 10);
	timer_settimer(timer_10, 10 * 100);

	/* 频闪定时器 */
	timer_cursor = timer_alloc();
	timer_init(timer_cursor, &fifo32, 0);
	timer_settimer(timer_cursor, 50);
	int cursor_x = 8;
	int cursor_c = COL8_FFFFFF;
	
	/* 窗口sheet */
	sheetWin = sheet_alloc(sheetCtrl);
	make_window8(sheetWin, 144, 52, "Count", 0);

	/* Notepad sheet */
	SHEET *sheetWinNotepad = sheet_alloc(sheetCtrl);
	make_window8(sheetWinNotepad, 160, 20 + 30, "NotePad", 1);
	make_textbox8(sheetWinNotepad, 8, 28, 144, 16, COL8_FFFFFF);

	task_a = task_init(memman);
	fifo32.task = task_a;
	task_run(task_a, 1, 0);

	sheetCmd = sheet_alloc(sheetCtrl);
	make_window8(sheetCmd, 256, 165, "console", 0);
	make_textbox8(sheetCmd, 8, 28, 240, 128, COL8_000000);
	taskCmd = task_alloc();
	taskCmd->tss.eip = (int) &console_task;
	taskCmd->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	taskCmd->tss.es = 1 * 8;
	taskCmd->tss.cs = 2 * 8;	/* GDT的2号 */
	taskCmd->tss.ss = 1 * 8;
	taskCmd->tss.ds = 1 * 8;
	taskCmd->tss.fs = 1 * 8;
	taskCmd->tss.gs = 1 * 8;
	*((int *) (taskCmd->tss.esp + 4)) = (int) sheetCmd;
	*((int *) (taskCmd->tss.esp + 8)) = memtotal;
	task_run(taskCmd, 2, 2);	

	sheet_slide(sheetBack, 0, 0);
	sheet_slide(sheetWinNotepad, 80 + 160 + 5, 72 + 68 + 5);
	sheet_slide(sheetWin, 108, 56);
	sheet_slide(sheetCmd, 70, 200);
	sheet_slide(sheet_win_b[0], 268, 56);
	sheet_slide(sheet_win_b[1], 108, 116);
	sheet_slide(sheet_win_b[2], 268, 116);

	mx = binfo->scrnx / 2;
	my = binfo->scrny / 2;
	sheet_slide(sheetMouse, mx, my);

	sheet_updown(sheetBack, 0);
	sheet_updown(sheet_win_b[0], 1);
	sheet_updown(sheet_win_b[1], 2);
	sheet_updown(sheet_win_b[2], 3);
	sheet_updown(sheetWin, 4);
	sheet_updown(sheetCmd, 5);
	sheet_updown(sheetWinNotepad, 6);
	sheet_updown(sheetMouse, 7);

	int count = 0;
	int second_counter = 0;

	mouseMove[0] = sheetWinNotepad;
	mouseMove[1] = sheetCmd;
	int curMove = 0;

	for (;;) {
		count++;
		sprintf(s, "%010d", count);
		putStrOnSheet(sheetWin, 40, 28, COL8_000000, s);

		if(fifo32_status(&keyCmdFifo) > 0 && keycmd_wait < 0){
			keycmd_wait = fifo32_get(&keyCmdFifo);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}

		io_cli();

		if(fifo32_status(&fifo32) == 0){
			task_sleep(task_a);
			io_sti();
		}else{
			i = fifo32_get(&fifo32);
			io_sti();
			switch (i){
				case 0 : case 0xff :
					timer_init(timer_cursor, &fifo32, ~i);
					timer_settimer(timer_cursor, 50);

					if(cursor_c >= 0){
						cursor_c = i == 0 ? COL8_000000 : COL8_FFFFFF;
						putBoxOnSheet(sheetWinNotepad, cursor_x, 28, 8, 16, cursor_c);
					}
					
					break;
				case 1:
					second_counter++;
					sprintf(s, "%d SEC", second_counter);
					putStrOnSheet(sheetBack, 500, 40, COL8_FFFFFF, s);
					timer_settimer(timer_1, 1 * 100);
					break;
				case 256 ... 511 :	/* 键盘 */										
					sprintf(s, "%02X", (i - 256));
					putStrOnSheet(sheetBack, 0, 16, COL8_FFFFFF, s);
					if(i < 256 + 0x80 ){	
						if(key_shift == 0){
							s[0] = keytable0[i - 256];
						}else{
							s[0] = keytable1[i - 256];
						}
					}else{
						s[0] = 0;
					}
					if( 'A' <= s[0] && s[0] <= 'Z'){
						if(((key_leds & 4) == 0 && key_shift == 0) || 
						   ((key_leds & 4) != 0 && key_shift != 0)){
							   s[0] += 0x20;	/* 将大写字母转换为小写 */
						   }
					}
					if(s[0] != 0){		/* 一般字符 */
						if(key_to == 0){		/* 发送给任务a */
							if(cursor_x < 8 * 18){
								s[1] = 0;
								putStrOnSheet_BG(sheetWinNotepad, cursor_x, 20 + 8, COL8_000000, COL8_FFFFFF, s);
								cursor_x += 8;
							}
						}else{
							/* 为了不与键盘数据冲突， 在写入fifo时将键盘数值加256 */ 
							fifo32_put(&taskCmd->fifo, s[0] + 256);
						}
					}
					if(i - 256 == 0x0e)	{	/* 退格键 */
						if(key_to == 0){	/* 发送给任务a */
							if(cursor_x > 8){
								putStrOnSheet_BG(sheetWinNotepad, cursor_x, 20 + 8, COL8_FFFFFF, COL8_FFFFFF, " ");
								cursor_x -= 8;
							}
						}else{
							fifo32_put(&taskCmd->fifo, 8 + 256);
						}
						
					}
					if(i - 256 == 0x0f){	/* Tab键 */
						curMove = curMove == 0 ? 1 : 0;
						// sheet_updown(mouseMove[curMove], 6);
						if(key_to == 0){
							key_to = 1;
							set_win_title_bar(sheetWinNotepad, "NotePad", 0);
							set_win_title_bar(sheetCmd, "Console", 1);
							cursor_c = -1;	/* Notepad不显示光标 */
							putBoxOnSheet(sheetWinNotepad, cursor_x, 28, 8, 16, COL8_FFFFFF);
							fifo32_put(&taskCmd->fifo, 2);	/* 命令行窗口显示光标 */
						}else{
							key_to = 0;
							set_win_title_bar(sheetWinNotepad, "NotePad", 1);
							set_win_title_bar(sheetCmd, "Console", 0);
							cursor_c = COL8_000000;	/* Notepad显示光标 */
							fifo32_put(&taskCmd->fifo, 3);	/* 命令行窗口不显示光标 */
						}
						sheet_refresh(sheetWinNotepad, 0, 0, sheetWinNotepad->bxsize, 21);
						sheet_refresh(sheetCmd, 0, 0, sheetCmd->bxsize, 21);
					}
					if(i - 256 == 0x1c){	/* 回车键 */
						if(key_to != 0){
							fifo32_put(&taskCmd->fifo, 10 + 256);
						}
					}

					if(i - 256 == 0x2a){	/* 左shift键 ON */
						key_shift |= 1;
					}
					if(i - 256 == 0xaa){	/* 左shift键 OFF */
						key_shift &= ~1;
					}
					if(i - 256 == 0x36){	/* 右shift键 ON */
						key_shift |= 2;
					}
					if(i - 256 == 0xb6){	/* 右shift键 OFF */
						key_shift &= ~2;
					}
					/* Lock 设置 */
					if (i == 256 + 0x3a) {	/* CapsLock */
						key_leds ^= 4;
						fifo32_put(&keyCmdFifo, KEYCMD_LED);
						fifo32_put(&keyCmdFifo, key_leds);
					}
					if (i == 256 + 0x45) {	/* NumLock */
						key_leds ^= 2;
						fifo32_put(&keyCmdFifo, KEYCMD_LED);
						fifo32_put(&keyCmdFifo, key_leds);
					}
					if (i == 256 + 0x46) {	/* ScrollLock */
						key_leds ^= 1;
						fifo32_put(&keyCmdFifo, KEYCMD_LED);
						fifo32_put(&keyCmdFifo, key_leds);
					}
					if (i == 256 + 0xfa) {	/* 键盘成功收到数据 */
						keycmd_wait = -1;
					}
					if (i == 256 + 0xfe) {	/* 键盘没有成功收到数据 */
						wait_KBC_sendready();
						io_out8(PORT_KEYDAT, keycmd_wait);
					}

					if(cursor_c >= 0){
						putBoxOnSheet(sheetWinNotepad, cursor_x, 28, 8, 16, cursor_c);
					}
					
					break;
				case 512 ... 767 :	/* 鼠标 */		
					/* L : mdec.btn & 0x01 != 0 */
					/* R : mdec.btn & 0x02 != 0 */
					/* C : mdec.btn & 0x04 != 0 */
					if(mouse_decode(&mdec, i - 512) != 0){	
						
						mx += mdec.x;
						my += mdec.y;
						mx = max(mx, 0);
						my = max(my, 0);
						mx = min(mx, binfo->scrnx);
						my = min(my, binfo->scrny);
						sprintf(s, "(%3d, %3d)", mx, my);

						putStrOnSheet(sheetBack, 0, 0, COL8_000000, s);

						sheet_slide(sheetMouse, mx, my);

						if((mdec.btn & 0x01) != 0){	/* 按下左键 */
							sheet_slide(mouseMove[curMove], mx - 80, my -8);
						}
					}
					break;
				default:
					break;
			}
			
		}
	}
}


void console_task(SHEET *sheet, unsigned int memtotal){

	TIMER *timer;
	TASK *task = task_current();
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	FILEINFO *finfo = (FILEINFO *) (ADR_DISKIMG + 0x002600);
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *) ADR_GDT;
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);

	int i, x, y;
	char s[30], cmdline[30], *p;
	int fifobuf[128];
	int cursor_x = 16, cursor_y = 28, cursor_c = -1;

	init_fifo32(&task->fifo, 128, fifobuf, task);

	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settimer(timer, 50);

	/* 显示提示符 */
	putStrOnSheet(sheet, 8, cursor_y, COL8_FFFFFF, ">");

	unsigned char *img_fat = (unsigned char *) (ADR_DISKIMG + 0x000200);
	char *img_file = (char *) (ADR_DISKIMG + 0x003e00);
	file_readfat(fat, img_fat);

	while(1){	
		io_cli();
		if(fifo32_status(&task->fifo) == 0){
			task_sleep(task);
			io_sti();
		}else{

			i = fifo32_get(&task->fifo);
			io_sti();

			switch(i){
				case 0: case 1:
					timer_init(timer, &task->fifo, i ^ 0x0000001);
					timer_settimer(timer, 50);

					if(cursor_c >= 0)
						cursor_c = i == 0 ? COL8_000000 : COL8_FFFFFF;
					
					break;
				case 2: case 3 :
					cursor_c = i == 2 ? COL8_FFFFFF : -1;
					if(i == 3) 
						putBoxOnSheet(sheet, cursor_x, 28, 8, 16, COL8_000000);
					break;
				case 256 ... 511:
					if(i == 8 + 256){	/* 退格键 */
						if(cursor_x > 2 * 8){
							putStrOnSheet_BG(sheet, cursor_x, cursor_y, COL8_000000, COL8_000000, " ");
							cursor_x -= 8;
						}
					}else if(i == 10 + 256){	/* 回车键 */
						putStrOnSheet_BG(sheet, cursor_x, cursor_y, COL8_000000, COL8_000000, " ");

						cmdline[cursor_x / 8 - 2] = 0;
						cursor_y = cons_newline(cursor_y, sheet);
						/* 执行命令 */
						if(strcmp(cmdline, "mem") == 0){
							sprintf(s, "totle: %dMB", memtotal / (1024 * 1024));
							putStrOnSheet(sheet, 8, cursor_y, COL8_FFFFFF, s);
							cursor_y = cons_newline(cursor_y, sheet);
							sprintf(s, "free: %dKB", memman_total(memman) / 1024);
							putStrOnSheet(sheet, 8, cursor_y, COL8_FFFFFF, s);
							cursor_y = cons_newline(cursor_y, sheet);
							cursor_y = cons_newline(cursor_y, sheet);
						}else if(strcmp(cmdline, "cls") == 0){
							for(y = 28; y < 28 + 128; y++)
								for(x = 8; x < 8 + 240; x++)
									sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
							cursor_y = 28;
						}else if(strcmp(cmdline, "dir") == 0){
							for(x = 0; x < 224; x++){
								if(finfo[x].name[0] == 0x00)
									break;
								if(finfo[x].name[0] != 0xe5){
									if((finfo[x].type & 0x18) == 0){
										sprintf(s, "filename.ext%7d", finfo[x].size);
										for(y = 0; y < 8; y++){
											s[y] = finfo[x].name[y];
										}
										s[9] = finfo[x].ext[0];
										s[10] = finfo[x].ext[1];
										s[11] = finfo[x].ext[2];
										putStrOnSheet(sheet, 8, cursor_y, COL8_FFFFFF, s);
										cursor_y = cons_newline(cursor_y, sheet);
									}
								}
							}
							cursor_y = cons_newline(cursor_y, sheet);
						}else if(cmdline[0] == 'c' && cmdline[1] == 'a' && cmdline[2] == 't' && cmdline[3] == ' '){
							for(y = 0; y <11; y++){
								s[y] = ' ';
							}
							y = 0;
							s[11] = 0;
							for(x = 4; y < 11 && cmdline[x] != 0; x++){
								if(cmdline[x] == '.' && y <= 8){
									y = 8;
								}else{
									s[y] = cmdline[x];
									if('a' <= s[y] && s[y] <= 'z')
										s[y] -= 0x20;
									y++;
								}
							}
							// putStrOnSheet(sheet, 30, 30, COL8_FFFFFF, s);
							/* 寻找文件 */
							for(x = 0; x < 244; ){
								if(finfo[x].name[0] == 0x00)
									break;
								if((finfo[x].type & 0x18) == 0){
									for(y = 0; y < 11; y++){
										if(finfo[x].name[y] != s[y])
											goto type_next_file;
									}
									break; /* 找到文件 */
								}
							type_next_file:
								x++;
							}
							if(x < 224 && finfo[x].name[0] != 0x00){	/* 找到文件的情况 */
								// y = finfo[x].size;
								// p = (char *) (finfo[x].clustno * 512 + 0x003e00 + ADR_DISKIMG);
								p = (char *) memman_alloc_4k(memman, finfo[x].size);
								file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, img_file);
								cursor_x = 8;
								for(y = 0; y < finfo[x].size; y++){
									s[0] = p[y];
									s[1] = 0;
									if(s[0] == 0x09){ /* 水平制表符 */
										for(;;){
											putStrOnSheet(sheet, cursor_x, cursor_y, COL8_FFFFFF, " ");
											cursor_x += 8;
											if(cursor_x == 8 +240){
												cursor_x = 8;
												cursor_y = cons_newline(cursor_y, sheet);
											}
											if(((cursor_x - 8) & 0x1f) == 0)
												break;	/* 能被32整除则break; */
										}
									}else if(s[0] == 0x0a){	/* 换行 */
										cursor_x = 8;
										cursor_y = cons_newline(cursor_y, sheet);
									}else if(s[0] == 0x0d){	/* 回车 */

									}else{	/* 一般字符 */
										putStrOnSheet(sheet, cursor_x, cursor_y, COL8_FFFFFF, s);
										cursor_x += 8;
										if(cursor_x == 8 + 240){	/* 到达最右端换行 */
											cursor_x = 8;
											cursor_y = cons_newline(cursor_y, sheet);
										}
									}
									
								}
								memman_free(memman, (int) p, finfo[x].size);
							}else{	/* 没有找到文件的情况 */
								putStrOnSheet(sheet, 8, cursor_y, COL8_FFFFFF, "File Not Found!");
								cursor_y = cons_newline(cursor_y, sheet);
							}
							cursor_y = cons_newline(cursor_y, sheet);
						}else if(strcmp(cmdline, "hlt") == 0){
							strcpy(s, "HLT     HRB");
							for(x = 0; x < 224;){
								if(finfo[x].name[0] == 0x00)
									break;
								if((finfo[x].type & 0x18) == 0){
									for(y = 0; y < 11; y++){
										if(finfo[x].name[y] != s[y])
											goto hlt_next_file;
									}
									break;
								}
						hlt_next_file:
								x++;
							}
							if(x < 224 && finfo[x].name[0] != 0x00){	/* 找到文件的情况 */
								p = (char *) memman_alloc_4k(memman, finfo[x].size);
								file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, img_file);
								set_segmdesc(gdt + 1003, finfo[x].size - 1, (int) p, AR_CODE32_ER);
								farjmp(0, 1003 * 8);
								memman_free(memman, (int) p, finfo[x].size);
							}else{	/* 没有找到文件的情况 */
								putStrOnSheet(sheet, 8, cursor_y, COL8_FFFFFF, "File Not Found!");
								cursor_y = cons_newline(cursor_y, sheet);
							}
						}else if(cmdline[0] != 0){
							/* 既不是命令也不是空行 */
							putStrOnSheet(sheet, 8, cursor_y, COL8_FFFFFF, "Bad command!");
							cursor_y = cons_newline(cursor_y, sheet);
							cursor_y = cons_newline(cursor_y, sheet);
						}


						/* 显示提示符 */
						putStrOnSheet(sheet, 8, cursor_y, COL8_FFFFFF, ">");
						cursor_x = 16;

					}else{	/* 一般字符 */
						if(cursor_x < 30 * 8){
							s[0] = i - 256;
							s[1] = 0;
							cmdline[cursor_x / 8 - 2] = i - 256;
							putStrOnSheet_BG(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s);
							cursor_x += 8;
						}
					}
					break;
				default:
					break;
			}	

			if(cursor_c >= 0)
				putBoxOnSheet(sheet, cursor_x, cursor_y, 8, 16, cursor_c);

		}
	}
	
}


