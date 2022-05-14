/* bootpack的主要部分 */

#include <stdio.h>
#include "../include/function.h"

void task_b_main(SHEET *sheet);
void console_task(SHEET *sheet);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	char s[40]/* , mcursor[256] */;
	// char keybuf[32], mousebuf[128];
	int mx, my;
	// extern FIFO8 keyfifo, mousefifo;

	/* 	fifo32 分配：
		0~1：	光标闪烁用定时器
		3： 	3秒定时器
		10：	10秒定时器
		256~511	键盘输入（256 bits）
		512~767	鼠标输入（256 bits）
		*/
	FIFO32 fifo32;
	int fifo32buf[128];

	MOUSE_DEC mdec;

	unsigned int memtotal;
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	SHEETCTRL *sheetCtrl;
	SHEET *sheetBack, *sheetMouse, *sheetWin, *sheet_win_b[3];
	unsigned char *bufBack, bufMouse[256], *bufWin, *buf_win_b;

	TIMER *timer[11];

	extern char keytable[0x54];

	TASK *task_a, *task_b[3];

	SHEET *sheetCmd;
	unsigned *bufCmd;
	TASK *taskCmd;


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

	/* 10s定时器 */
	timer[10] = timer_alloc();
	timer_init(timer[10], &fifo32, 10);
	timer_settimer(timer[10], 10 * 100);
	/* 3s定时器 */
	timer[3] = timer_alloc();
	timer_init(timer[3], &fifo32, 3);
	timer_settimer(timer[3], 3 * 100);

	/* 频闪定时器 */
	timer[0] = timer_alloc();
	timer_init(timer[0], &fifo32, 1);
	timer_settimer(timer[0], 50);
	int cursor_x = 8;
	int cursor_c = COL8_FFFFFF;
	
	/* 窗口sheet */
	sheetWin = sheet_alloc(sheetCtrl);
	make_window8(memman, sheetWin, 144, 52, "Count", 1);

	/* Notepad sheet */
	SHEET *sheetWinNotepad = sheet_alloc(sheetCtrl);
	make_window8(memman, sheetWinNotepad, 160, 20 + 30, "NotePad", 0);
	make_textbox8(sheetWinNotepad, 8, 28, 144, 16, COL8_FFFFFF);

	task_a = task_init(memman);
	fifo32.task = task_a;
	task_run(task_a, 1, 0);

	for(i = 0; i < 3; i++){
		sheet_win_b[i] = sheet_alloc(sheetCtrl);
		buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
		sheet_setbuf(sheet_win_b[i], buf_win_b, 144, 52, -1);
		sprintf(s, "task_b_%d", i);
		make_window8(memman, sheet_win_b[i], 144, 52, s, 0);
		task_b[i] = task_alloc();
		task_b[i]->tss.eip = (int) &task_b_main;
		task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
		task_b[i]->tss.es = 1 * 8;
		task_b[i]->tss.cs = 2 * 8;	/* GDT的2号 */
		task_b[i]->tss.ss = 1 * 8;
		task_b[i]->tss.ds = 1 * 8;
		task_b[i]->tss.fs = 1 * 8;
		task_b[i]->tss.gs = 1 * 8;
		*((int *) (task_b[i]->tss.esp + 4)) = (int) sheet_win_b[i];
		// task_run(task_b[i], 2, i + 1);
	}


	sheetCmd = sheet_alloc(sheetCtrl);
	make_window8(memman, sheetCmd, 256, 165, "console", 0);
	make_textbox8(sheetCmd, 8, 28, 240, 128, COL8_000000);
	taskCmd = task_alloc();
	taskCmd->tss.eip = (int) &console_task;
	taskCmd->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	taskCmd->tss.es = 1 * 8;
	taskCmd->tss.cs = 2 * 8;	/* GDT的2号 */
	taskCmd->tss.ss = 1 * 8;
	taskCmd->tss.ds = 1 * 8;
	taskCmd->tss.fs = 1 * 8;
	taskCmd->tss.gs = 1 * 8;
	*((int *) (taskCmd->tss.esp + 4)) = (int) sheetCmd;
	task_run(taskCmd, 2, 2);
	// task_remove(taskCmd);
	

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
	
	sprintf(s, "[memery: %dMB; free: %dKB;]", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putStrOnSheet(sheetBack, 0, 32, COL8_000084, s);

	int count = 0;
	
	for (;;) {
		count++;
		sprintf(s, "%010d", count);
		putStrOnSheet(sheetWin, 40, 28, COL8_000000, s);

		io_cli();

		if(fifo32_status(&fifo32) == 0){
			task_sleep(task_a);
			io_sti();
		}else{
			i = fifo32_get(&fifo32);
			io_sti();
			switch (i){
				case 0 : case 1 :
					// putBoxOnSheet(sheetBack, 10, 80, 5, 15, COL8_008484);
					timer_init(timer[0], &fifo32, i ^ 0x0000001);
					timer_settimer(timer[0], 50);
					cursor_c = i == 0 ? COL8_000000 : COL8_FFFFFF;
					putBoxOnSheet(sheetWinNotepad, cursor_x, 28, 8, 16, cursor_c);
					break;
				case 3 :
					putStrOnSheet(sheetBack, 0, 64, COL8_FFFFFF, "3 SEC");
					
					break;
				case 10 :
					putStrOnSheet(sheetBack, 0, 64, COL8_FFFFFF, "10 SEC");
					// task_sleep(task_b);
					break;
				case 256 ... 511 :	/* 键盘 */										
					sprintf(s, "%02X", i - 256);
					sprintf(s, "%d", cursor_x);
					putStrOnSheet(sheetBack, 0, 16, COL8_FFFFFF, s);
					if(i < 256 + 0x54){
						if(keytable[i - 256] != 0 && cursor_x < 8 * 18){
							s[0] = keytable[i - 256];
							s[1] = 0;
							putStrOnSheet_BG(sheetWinNotepad, cursor_x, 20 + 8, COL8_000000, COL8_FFFFFF, s);
							cursor_x += 8;
						}
					}
					if(i - 256 == 0x0e && cursor_x > 8)	{	/* 退格键 */
						putStrOnSheet_BG(sheetWinNotepad, cursor_x, 20 + 8, COL8_FFFFFF, COL8_FFFFFF, " ");
						cursor_x -= 8;
					}
					break;
				case 512 ... 767 :	/* 鼠标 */		
					if(mouse_decode(&mdec, i - 512) != 0){
						sprintf(s, "[    %4d %4d]", mdec.x, mdec.y);
						if((mdec.btn & 0x01) != 0)
							s[1] = 'L';
						if((mdec.btn & 0x02) != 0)
							s[3] = 'R';
						if((mdec.btn & 0x04) != 0)
							s[2] = 'C';

						putStrOnSheet(sheetBack, 32, 16, COL8_FFFFFF, s);

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
							sheet_slide(sheetWinNotepad, mx - 80, my -8);
						}
					}
					break;
				default:
					break;
			}
			
		}
	}
}

void task_b_main(SHEET *sheet){
	FIFO32 fifo;
	TIMER *timer_put_str;
	int fifobuf[128];
	int i;
	int count = 0, count0 = 0;
	char s[40];

	init_fifo32(&fifo, 128, fifobuf, NULL);

	timer_put_str = timer_alloc();
	timer_init(timer_put_str, &fifo, 100);
	timer_settimer(timer_put_str, 100);

	for(;;) { 
		count++;

		io_cli();
		if(fifo32_status(&fifo) == 0){
			io_sti();
		}else{
			i = fifo32_get(&fifo);
			io_sti();
			if(i == 100){
				sprintf(s, "%11d", count - count0);
				putStrOnSheet(sheet, 24, 28, COL8_000000, s);
				count0 = count;
				timer_settimer(timer_put_str, 100);
			}
		}
	}
}

void console_task(SHEET *sheet){
	FIFO32 fifo;
	TIMER *timer;
	TASK *task = task_current();

	int i;
	int fifobuf[128];
	int cursor_x = 8, cursor_c = COL8_FFFFFF;

	init_fifo32(&fifo, 128, fifobuf, task);

	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settimer(timer, 50);

	char s[40];
	while(1){
		
		io_cli();
		if(fifo32_status(&fifo) == 0){
			task_sleep(task);
			io_sti();
		}else{
			// putStrOnSheet(sheet, 20, 28, COL8_FFFFFF, "kkkkk");
			i = fifo32_get(&fifo);
			io_sti();
			if(i <= 1){
				if(i != 0){
					timer_init(timer, &fifo, 0);
					cursor_c = COL8_FFFFFF;
				}else{
					timer_init(timer, &fifo, 1);
					cursor_c = COL8_000000;
				}
				timer_settimer(timer, 50);
				putBoxOnSheet(sheet, cursor_x, 28, 8, 16, cursor_c);
			}
		}
	}
	
}


