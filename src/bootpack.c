/* bootpack的主要部分 */

#include <stdio.h>
#include "../include/function.h"

#define cmdNum 2

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	extern TASKCTRL *taskCtrl;
	char s[40];
	int mx, my;
	int new_mx, new_my;
	int new_wx, new_wy;

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
	SHEET *keyRecvWin;		/* 接收键盘值的Sheet */
	SHEET *sheetBack, *sheetMouse;
	unsigned char *bufBack, bufMouse[256];

	TIMER *timer_1, *timer_3, *timer_10;

	extern char hankaku[4096];
	extern char keytable0[0x80];
	extern char keytable1[0x80];

	int key_shift = 0, key_ctrl = 0;
	int key_leds = (binfo->leds >> 4) & 7;
	int keycmd_wait = -1;

	TASK *task_a;

	TASK *keyTask;

	int mInShtX, mInShtY;
	int mmx = -1, mmy = -1, mmx2 = 0;
	SHEET *tempSheet = 0;

	unsigned char *nihong;
	int *fat;
	FILEINFO *finfo;

	int i, j;
	
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

	*((int *) 0x0fe4) = (int) sheetCtrl;

	*((int *) 0x0fe8) = memtotal;

	*((int *) 0x0fec) = (int) &fifo32;

	/* 背景sheet */
	sheetBack = sheet_alloc(sheetCtrl);
	bufBack = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sheetBack, bufBack, binfo->scrnx, binfo->scrny, TRSPRT_OFF); /* 没有透明色 */
	init_screen8(bufBack, binfo->scrnx, binfo->scrny);

	*((int *) 0x0fc4) = (int) sheetBack;

	/* 鼠标sheet */
	sheetMouse = sheet_alloc(sheetCtrl);
	sheet_setbuf(sheetMouse, bufMouse, 16, 16, TRSPRT_ON);	/* 透明色号99 */
	init_mouse_cursor8(bufMouse, TRSPRT_ON);	/* 背景色号99 */
	
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

	task_a = task_init(memman);
	fifo32.task = task_a;
	task_run(task_a, 1, 0);
	task_a->langmode = 0;

	keyRecvWin = open_console();

	sheet_slide(sheetBack, 0, 0);
	sheet_slide(keyRecvWin, 32, 4);
	mx = binfo->scrnx / 2;
	my = binfo->scrny / 2;
	sheet_slide(sheetMouse, mx, my);

	sheet_updown(sheetBack, 0);
	sheet_updown(keyRecvWin, 1);
	sheet_updown(sheetMouse, 3);

	new_mx = -1;
	new_my = 0;
	new_wx = 0x7fffffff;
	new_wy = 0;

	int second_counter = 0;

	int bgColor;
	int x, y;

	keyWinOn(keyRecvWin);

	/* 加载字体 */
	nihong = (unsigned char *) memman_alloc_4k(memman, 16 * 256 + 32 * 94 * 47);
	fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
	finfo = file_search("nihong.fnt", (FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if(finfo != NULL){
		file_loadfile(finfo->clustno, finfo->size, nihong, fat, (char *) (ADR_DISKIMG + 0x003e00));
	}else{
		for(i = 0; i < 16 * 256; i++)	/* 没有字库，半角部分直接复制英文字库 */
			nihong[i] = hankaku[i];
		for(i = 16 * 256; i < 16 * 256 + 32 * 94 * 47; i++)	/* 没有字库，全角部分0xff填充 */
			nihong[i] = 0xff;
	}

	*((int *) 0x0fc8) = (int) nihong;
	memman_free_4k(memman, (int) fat, 4 * 2880);
	

	for (;;) {
		if(fifo32_status(&keyCmdFifo) > 0 && keycmd_wait < 0){
			keycmd_wait = fifo32_get(&keyCmdFifo);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}

		io_cli();

		if(fifo32_status(&fifo32) == 0){
			/* fifo32为空，当存在搁置的绘图操作时立即执行 */
			if(new_mx >= 0){
				io_sti();
				sheet_slide(sheetMouse, new_mx, new_my);
				new_mx = -1;
			}else if(new_wx != 0x7fffffff){
				io_sti();
				sheet_slide(tempSheet, new_wx, new_wy);
				new_wx = 0x7fffffff;
			}else{
				task_sleep(task_a);
				io_sti();
			}
		}else{
			i = fifo32_get(&fifo32);
			io_sti();
			if(keyRecvWin != NULL && keyRecvWin->flags == 0){	/* 输入窗口被关闭 */
				if(sheetCtrl->top == 1){	/* 当画面上只剩鼠标和背景时 */
					keyRecvWin = NULL;
				}else{
					keyRecvWin = sheetCtrl->sheets[sheetCtrl->top - 1];
					keyWinOn(keyRecvWin);
				}
			}
			switch (i){
				case 1:
					second_counter++;
					sprintf(s, "COUNT: %d SEC", second_counter);

					x = 500; y = 40;
					bgColor = sheetBack->buf[y * sheetBack->bxsize + x];
					putStrOnSheet_BG(sheetBack, x, y, COL8_FFFFFF, bgColor, s);

					timer_settimer(timer_1, 1 * 100);
					break;
				case 256 ... 511 :															/* 键盘 */		

					sprintf(s, "{{%02X}}", (i - 256));
					x = 0; y = 16;
					bgColor = sheetBack->buf[y * sheetBack->bxsize + x];
					putStrOnSheet_BG(sheetBack, x, y, COL8_FFFFFF, bgColor, s);

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
					if(s[0] != 0 && key_ctrl != 1 && keyRecvWin != NULL){		/* 一般字符、退格键、回车键 */
						/* 发送至命令行窗口 */
						/* 为了不与键盘数据冲突， 在写入fifo时将键盘数值加256 */ 
						fifo32_put(&keyRecvWin->task->fifo, s[0] + 256);
					}
					// if(i - 256 == 0x0e && keyRecvWin != NULL)	{	/* 退格键 */
					// 	/* 发送至命令行窗口 */
					// 	fifo32_put(&keyRecvWin->task->fifo, 8 + 256);
					// }
					if(i - 256 == 0x0f && keyRecvWin != NULL){	/* Tab键 */
						keyWinOff(keyRecvWin);
						keyRecvWin = sheetCtrl->sheets[1];
						sheet_updown(keyRecvWin, sheetCtrl->top - 1);
						keyWinOn(keyRecvWin);
					}
					// if(i - 256 == 0x1c){	/* 回车键 */
					// 	fifo32_put(&keyRecvWin->task->fifo, 10 + 256);
					// }

					if(i - 256 == 0x1d)
						key_ctrl |= 1;		/* ctrl键 ON */
					if(i - 256 == 0x9d)
						key_ctrl &= ~1;		/* ctrl键 OFF */
					if(i - 256 == 0x2a)	
						key_shift |= 1;		/* 左shift键 ON */
					if(i - 256 == 0xaa)	
						key_shift &= ~1;	/* 左shift键 OFF */
					if(i - 256 == 0x36)	
						key_shift |= 2;		/* 右shift键 ON */
					if(i - 256 == 0xb6)	
						key_shift &= ~2;	/* 右shift键 OFF */

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
					if (i - 256 == 0x2e && key_ctrl == 1 && keyRecvWin != NULL){
						keyTask = keyRecvWin->task;
						if(keyTask != NULL && keyTask->tss.ss0 != 0){
							cons_putstr(keyTask->cons, "\nCtrl^C\n");
							io_cli();	/* 改变寄存器值之前先关中断 */
							keyTask->tss.eax = (int) &(keyTask->tss.esp0);
							keyTask->tss.eip = (int) asm_end_app;
							io_sti();
							task_run(keyTask, -1, 0);
						}
					}

					if (i - 256 == 0x3c){
						if(keyRecvWin != NULL)
							keyWinOff(keyRecvWin);
						keyRecvWin = open_console();
						sheet_slide(keyRecvWin, 32, 4);
						sheet_updown(keyRecvWin, sheetCtrl->top);
						keyWinOn(keyRecvWin);
					}
					
					break;
				case 512 ... 767 :															/* 鼠标 */		
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

						new_mx = mx;
						new_my = my;

						if((mdec.btn & 0x01) != 0){	/* 按下左键 */
							
							if(mmx < 0){
								/* 如果处于普通模式 */
								for(j = sheetCtrl->top - 1; j > 0; j--){	/* 从上往下找鼠标指向的图层 */
									tempSheet = sheetCtrl->sheets[j];
									mInShtX = mx - tempSheet->vx0;
									mInShtY = my - tempSheet->vy0;
									/* 窗口矩形点击判断 */
									if(0 <= mInShtX && mInShtX < tempSheet->bxsize 
									&& 0 <= mInShtY && mInShtY < tempSheet->bysize){
										/* 窗口点击判断 */
										if(tempSheet->buf[mInShtY * tempSheet->bxsize + mInShtX] != tempSheet->col_inv){	

											sheet_updown(tempSheet, sheetCtrl->top - 1);

											keyWinOff(keyRecvWin);
											keyRecvWin = tempSheet;
											keyWinOn(keyRecvWin);

											if(3 <= mInShtX && mInShtX < tempSheet->bxsize - 3 
											&& 0 <= mInShtY && mInShtY < 21){	/* 窗口标题栏点击判断 */
												mmx = mx;
												mmy = my;
												mmx2 = tempSheet->vx0;
												new_wy = tempSheet->vy0;
											}

											if(tempSheet->bxsize - 21 <= mInShtX && mInShtX < tempSheet->bxsize - 5
											&& 5 <= mInShtY && mInShtY < 19 ){	/* 窗口标题栏关闭按钮点击判断 */

												if((tempSheet->flags & 0x10) != 0){	/* 该窗口是否为应用窗口 */
													keyTask = tempSheet->task;
													cons_putstr(keyTask->cons, "\nBreak Mouse\n");
													io_cli();
													keyTask->tss.eax = (int) &(keyTask->tss.esp0);
													keyTask->tss.eip = (int) asm_end_app;
													io_sti();
													task_run(keyTask, -1, 0);
												}else{
													keyTask = tempSheet->task;
													sheet_updown(tempSheet, -1);	/* 先将窗口隐藏 */

													keyWinOff(keyRecvWin);
													keyRecvWin = sheetCtrl->sheets[sheetCtrl->top - 1];
													keyWinOn(keyRecvWin);

													io_cli();
													fifo32_put(&keyTask->fifo, 4);
													io_sti();
												}
											}
											break;
										}
									}
								}
							}else{
								/* 如果处于窗口移动模式 */
								mInShtX = mx - mmx;
								mInShtY = my - mmy;
								/* 为了发挥refreshmap函数，x坐标对4的倍数向下取整 */
								new_wx = (mmx2 + mInShtX + 2) & ~3;
								new_wy = new_wy + mInShtY;
								// sheet_slide(tempSheet, (mmx2 + mInShtX + 2) & ~3, tempSheet->vy0 + mInShtY);
								mmy = my;
							}
						}else{	/* 没有按下左键 */
							mmx = -1;
							if(new_wx != 0x7fffffff){
								sheet_slide(tempSheet, new_wx, new_wy);	/* 固定图层位置 */
								new_wx = 0x7fffffff;
							}
						}
					}
					break;
				case 768 ... 1023 :															/* 命令行窗口关闭处理 */
					close_console(sheetCtrl->sheets0 + (i - 768));
					break;
				case 1024 ... 2023 :														/* 无窗口命令行关闭处理 */
					close_constask(taskCtrl->tasks0 + (i - 1024));
					break;																	
				case 2024 ... 2279 :														/* 只关闭命令行窗口 */
					tempSheet = sheetCtrl->sheets0 + (i - 2024);							
					memman_free_4k(memman, (int) tempSheet->buf, 256 * 165);
					sheet_free(tempSheet);
					break;
				default:
					break;
			}
			
		}
	}
}
