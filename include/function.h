#ifndef FUNCTION_H
#define FUNCTION_H

#include "const.h"

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void asm_inthandler0c(void);
void asm_inthandler0d(void);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

void asm_cons_putchar(void);
void asm_os_api(void);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
void farjmp(int epi, int cs);
void farcall(int epi, int cs);
void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
void asm_end_app(void);

/* graphic.c */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen8(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, int bc);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);
void putStrOnSheet(SHEET *sheet, int x, int y, int font_color, char *str);
void putBoxOnSheet(SHEET *sheet, int x, int y, int sx, int sy, int color);
void putStrOnSheet_BG(SHEET *sheet, int x, int y, int font_color, int bg_color, char *str);
void putLineOnSheet(SHEET *sheet, int x0, int y0, int x1, int y1, int color);
void putfontGB2312(char *vram, int xsize, int x, int y, char c, char *font);

/* windows.c */
void make_window8(SHEET *sheet, int xsize, int ysize, char *title, char act);
void set_win_title_bar(SHEET *sheet, char *title, char act);
void make_textbox8(SHEET *sheet, int x0, int y0, int sx, int sy, int c);
void make_window8_buf(SHEET *sheet, char* buf, int xsize, int ysize, char *title, char act);
void keyWinOff(SHEET *keyRecvWin);
void keyWinOn(SHEET *keyRecvWin);
void changeWinTitle(SHEET *sheet, char act);

/* dsctbl.c */
void init_gdtidt(void);
void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

/* int.c */
void init_pic(void);
void inthandler27(int *esp);

/* fifo.c */
// void init_fifo8(FIFO8 *fifo, int size, unsigned char *buf);
// int fifo8_put(FIFO8 *fifo, unsigned char data);
// int fifo8_get(FIFO8 *fifo);
// int fifo8_status(FIFO8 *fifo);
void init_fifo32(FIFO32 *fifo, int size, int *buf, TASK *task);
int fifo32_put(FIFO32 *fifo, int data);
int fifo32_get(FIFO32 *fifo);
int fifo32_status(FIFO32 *fifo);

/* keyboaard.c */
void wait_KBC_sendready(void);
void init_keyboard(FIFO32 *fifo, int data_start);
void inthandler21(int *esp);

/* mouse.c */
void enable_mouse(MOUSE_DEC *mdec, FIFO32 *fifo, int data_start);
int mouse_decode(MOUSE_DEC *mdec, unsigned char dat);
void inthandler2c(int *esp);

/* memory.c */
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void memman_init(MEMMAN *man);
unsigned int memman_total(MEMMAN *man);

unsigned int memman_alloc(MEMMAN *man, unsigned int size);
int memman_free(MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(MEMMAN *man, unsigned int size);
int memman_free_4k(MEMMAN *man, unsigned int addr, unsigned int size);

/* math.c */
int max(int a, int b);
int min(int a, int b);

/* string.c */
int stringlength(char *str);
int strcmp_len(char *str_1, char *str_2, int len);
void stringcat(char *dist, char *src);

/* sheet.c */
SHEETCTRL *sheetctrl_init(MEMMAN *man, unsigned char *vram, int xsize, int ysize);
SHEET *sheet_alloc(SHEETCTRL *ctrl);
void sheet_refreshsub(SHEETCTRL *ctrl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_setbuf(SHEET *sheet, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(SHEET *sheet, int height);
void sheet_refresh(SHEET *sheet, int bx0, int by0, int bx1, int by1);
void sheet_slide(SHEET *sheet, int vx0, int vy0);
void sheet_free(SHEET *sheet);
void sheet_refreshmap(SHEETCTRL *ctrl, int vx0, int vy0, int vx1, int vy1, int h0);

/* timer.c */
void init_pit();
void inthandler20(int *esp);
TIMER *timer_alloc(void);
void timer_free(TIMER *timer);
void timer_init(TIMER *timer, FIFO32 *fifo, int data);
void timer_settimer(TIMER *timer, unsigned int timeout);
int timer_cancel(TIMER *timer);
void timer_cancelall(FIFO32 *fifo);

/* multitask.c */
TASK *task_init(MEMMAN *man);
TASK *task_alloc(void);
void task_run(TASK *task, int level, int priority);
void task_switch(void);
void task_sleep(TASK *task);
TASK *task_current(void);
void task_add(TASK *task);
void task_remove(TASK *task);
void task_switch_preset(void);

/* console.c */
void console_task(SHEET *sheet, unsigned int memtotal);
TASK *open_constask(SHEET *sheet);
SHEET *open_console();
void close_constask(TASK *taskCmd);
void close_console(SHEET *sheetCmd);
void cons_newline(CONSOLE *cons);
void cons_putchar(CONSOLE *cons, char c, char x_move);
void cons_runcmd(char *cmdline, CONSOLE *cons, int *fat, unsigned int memtotal);
void cmd_getpara(char *cmdline, char *cmd, char *para);
void cmd_mem(CONSOLE *cons, unsigned int memtotal);
void cmd_cls(CONSOLE *cons);
void cmd_dir(CONSOLE *cons);
void cmd_cat(CONSOLE *cons, int *fat, char *fname);
void cmd_exit(CONSOLE *cons, int *fat);
void cmd_start(CONSOLE *cons, char *para);
void cmd_ncst(CONSOLE *cons, char *para);
void cmd_langmode(CONSOLE *cons, char *para);
int cmd_app(CONSOLE *cons, int *fat, char *cmdline);
void cons_putstr(CONSOLE *cons, char *str);
void cons_putstr_len(CONSOLE *cons, char *str, int length);
int *os_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
int *inthandler0c(int *esp);
int *inthandler0d(int *esp);

/* file.c */
void file_readfat(int *fat, unsigned char *img);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
FILEINFO  *file_search(char *fname, FILEINFO *finfo, int max);

#endif
