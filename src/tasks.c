#include "../include/function.h"

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