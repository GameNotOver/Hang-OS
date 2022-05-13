#include "../include/function.h"

// struct FIFO8 keyfifo;
FIFO32 *keyfifo;
int keydata_start;

void wait_KBC_sendready(void){
	for(;;){
		if((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0)
			break;
	}
	return;
}

void init_keyboard(FIFO32 *fifo, int data_start){
	keyfifo = fifo;
	keydata_start = data_start;

	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

//来自 PS/2 键盘的中断
void inthandler21(int *esp){ 

    unsigned char data;
    io_out8(PIC0_OCW2, 0x61);   /* 通知PIC “IRQ-01已经受理完毕” */
    data = io_in8(PORT_KEYDAT);
    fifo32_put(keyfifo, data + keydata_start);

    return;
}
