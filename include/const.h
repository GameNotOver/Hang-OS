#ifndef CONST_H
#define CONST_H

#ifndef NULL
#define NULL			0
#endif

#define COL8_000000		0 		// BLACK			
#define COL8_FF0000		1 		// LIGHT_RED		
#define COL8_00FF00		2 		// LIGHT_GREEN	
#define COL8_FFFF00		3 		// LIGHT_YELLOW	
#define COL8_0000FF		4 		// LIGHT_BLUE1	
#define COL8_FF00FF		5 		// LIGHT_PURPLE	
#define COL8_00FFFF		6 		// LIGHT_BLUE2	
#define COL8_FFFFFF		7 		// WHITE			
#define COL8_C6C6C6		8 		// LIGHT_GRAY		
#define COL8_840000		9 		// DARK_RED		
#define COL8_008400		10		// DARK_GREEN		
#define COL8_848400		11		// DARK_YELLOW	
#define COL8_000084		12		// DARK_BLUE1		
#define COL8_840084		13		// DARK_PURPLE	
#define COL8_008484		14		// DARK_BLUE2		
#define COL8_848484		15		// DARK_GRAY

#define TRSPRT_ON		0xff	// TRANSPARENT
#define TRSPRT_OFF		-1


#define ADR_BOOTINFO	0x00000ff0
#define ADR_DISKIMG		0x00100000

/* int.c */
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/* dsctbl.c */
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e
#define AR_LDT			0x0082
#define AR_TSS32		0x0089
#define AR_INTGATE32	0x008e

/* int.c bootpack.c*/
#define PORT_KEYDAT		0x0060
#define PORT_KEYSTA		0x0064
#define PORT_KEYCMD		0x0064
#define	KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4
#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000
#define	MEMMAN_FREES		4090
#define MEMMAN_ADDR			0x003c0000
#define MAX_SHEETS			256
#define CURSOR_SIZE_X		10
#define CURSOR_SIZE_Y		16
#define IRQALLOW_TIMER		0xfe	/* 1111 1110 */
#define IRQALLOW_KEYBD		0xfd	/* 1111 1101 */
#define IRQALLOW_PIC_1		0xfb	/* 1111 1011 */
#define IRQALLOW_MOUSE		0xef	/* 1110 1111 */
#define KEYCMD_LED			0xed


/* fifo.c */
#define FLAGS_OVERRUN	0x0001

/* sheet.c */
#define SHEET_USE		1

/* timer.c */
#define PIC_CTRL		0x0043
#define PIC_CNT0		0x0040
#define MAX_TIMER		500
#define TIMER_FLAGS_FREE	0
#define TIMER_FLAGS_ALLOC	1
#define TIMER_FLAGS_USING	2

#define TIMER_FLAGS_BASIC	0
#define TIMER_FLAGS_APP		1

/* multitask.c */
#define MAX_TASKS		1000
#define TASK_GDT0		3
#define TASK_FREE		0
#define TASK_ALLOC		1
#define TASK_RUN		2
#define MAX_TASKS_LV	100
#define MAX_TASKLEVELS	10

/* console.c */
#define CONS_WIDTH		256
#define CONS_LENGTH		165
#define CONS_TITLE_LEN	20	

typedef unsigned char * BUFFER;

typedef struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
} SEGMENT_DESCRIPTOR;

typedef struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
} GATE_DESCRIPTOR;

typedef struct BOOTINFO {	/* 0x0ff0 ~ 0x0fff */
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
} BOOTINFO;

typedef struct  FILEINFO {
	unsigned char name[8], ext[3];
	unsigned char type;
	char reverse[10];
	unsigned short time, data;
	unsigned short clustno;	/* ?????? */
	unsigned int size;
} FILEINFO;

typedef struct KEYBUF {
	unsigned char data[32];
	int next_rd, next_wt;
	int len;
} KEYBUF;

typedef struct FIFO8 {
	unsigned char *buf;
	int next_rd, next_wt;
	int size, free, flags;
} FIFO8;

typedef struct FIFO32 {
	int *buf;
	int next_rd, next_wt;
	int size, free, flags;
	struct TASK *task;
} FIFO32;

typedef struct MOUSE_DEC{
	unsigned char buf[3];
	unsigned char phase;
	int x, y, btn;
} MOUSE_DEC;

typedef struct FREEINFO {
	unsigned int addr;
	unsigned int size;
} FREEINFO;	/* 8B */

typedef struct MEMMAN {
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
} MEMMAN;	/* 16 + 8 * 4090 = 32736B ~ 32KB */

typedef struct SHEET {
	unsigned char *buf;	/* ???????????? */
	int bxsize, bysize;	/* ???????????? */
	int vx0, vy0;		/* ???????????? */
	int col_inv;		/* ??????????????? */
	int height;			/* ???????????? */
	int flags;			/* ?????????????????? */
	struct SHEETCTRL *ctrl;	/* sheet controller*/
	struct TASK *task;
} SHEET;

typedef struct SHEETCTRL {
	unsigned char *vram;
	unsigned char *map;
	int xsize, ysize;
	int top;			/* ???????????????????????? */
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
} SHEETCTRL;	/* 16 + 4 * 256 +  32 * 256 = 9232B*/

typedef struct TIMER {
	unsigned int timeout;
	unsigned int flags;
	unsigned int flags_basic;
	struct FIFO32 *fifo;
	int data;
	struct TIMER *next;
} TIMER;


typedef struct TIMER_CRTL {
	unsigned int count;
	unsigned int next_timeout;
	struct TIMER *head;
	struct TIMER timers[MAX_TIMER];
} TIMER_CRTL;

/* task struct segment */
typedef struct TSS32 {
	int backlink;
	int esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
} TSS32;	/* 104 B */

typedef struct TASK {
	int selector;		/* GDT????????? */
	int flags;	
	int level, priority;
	struct FIFO32 fifo;
	struct TSS32 tss;
	struct SEGMENT_DESCRIPTOR ldt[2];
	struct CONSOLE *cons;
	int ds_base, cons_stack;
	struct FILEHANDLE *fhandle;
	int *fat;
	char *cmdline;
	unsigned char langmode, langbyte1;
} TASK;

typedef struct TASKLEVEL {
	int running;
	int current;
	struct TASK *tasks[MAX_TASKS_LV];
} TASKLEVEL;

typedef struct TASKCTRL {
	int cur_lv;			/* ??????????????????level */
	char lv_change;		/* ??????????????????????????????????????????level */
	struct TASKLEVEL level[MAX_TASKLEVELS];
	TASK tasks0[MAX_TASKS];
} TASKCTRL;

typedef struct CONSOLE {
	struct SHEET *sheet;
	int cur_x, cur_y, cur_c;
	struct TIMER *timer;
} CONSOLE;

typedef struct FILEHANDLE {
	char *buf;
	int size;
	int pos;
} FILEHANDLE;


#endif
