#include "../include/function.h"

// struct FIFO8  mousefifo;
FIFO32 *mousefifo;
int mousedata_start;

void enable_mouse(MOUSE_DEC *mdec, FIFO32 *fifo, int data_start){
	/* 讲FIFO缓冲区的信息保存到全局变量里 */
	mousefifo = fifo;
	mousedata_start = data_start;
	/* 鼠标有效 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/*顺利的话，键盘会返送回ACK（0xfa）*/
	mdec->phase = 0;	/* 等待0xfa阶段 */
	return;	
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat){
	switch (mdec->phase){
		case 0 :
			/* 等待鼠标的0xfa的状态 */
			if(dat == 0xfa)
				mdec->phase = 1;
			break;
		case 1 :
			/* 等待鼠标的第一个字节 */
			if((dat & 0xc8) == 0x08){
				mdec->buf[0] = dat;
				mdec->phase = 2;
			}
			break;
		case 2 :
			/* 等待鼠标的第二个字节 */
			mdec->buf[1] = dat;
			mdec->phase = 3;
			break;
		case 3 :
			/* 等待鼠标的第三个字节 */
			mdec->buf[2] = dat;
			mdec->phase = 1;

			mdec->btn = mdec->buf[0] & 0x07;
			mdec->x = mdec->buf[1];
			mdec->y = mdec->buf[2];

			if((mdec->buf[0] & 0x10) != 0)
				mdec->x |= 0xffffff00;
			if((mdec->buf[0] & 0x20) != 0)
				mdec->y |= 0xffffff00;
			mdec->y = -mdec->y;

			return 1;
		default:
			return -1;
	}
	return 0;
}

/*PS/2来自鼠标的中断*/
void inthandler2c(int *esp){
    unsigned char data;
    io_out8(PIC1_OCW2, 0x64);   /* 通知PIC1 IRQ-12的受理已经完成 */
    io_out8(PIC0_OCW2, 0x62);   /* 通知PIC0 IRQ-02的受理已经完成 */
    data = io_in8(PORT_KEYDAT);
    fifo32_put(mousefifo, data + mousedata_start);
    return;
}
