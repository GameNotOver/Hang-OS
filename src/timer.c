#include "../include/function.h"
#include <stdio.h>
TIMER_CRTL timerCtrl;

/* Programmable Interval Timer */
void init_pit(){
    int i;
    io_out8(PIC_CTRL, 0x34);
    io_out8(PIC_CNT0, 0x9c);
    io_out8(PIC_CNT0, 0x2e);        /* 0x2e9c = 11932,f = 1.19318KHZ / 11932 = 100HZ, T = 1/f =  0.01s = 10ms*/
    timerCtrl.count = 0;
    timerCtrl.head->timeout = 0xffffffff;
    for(i = 0; i < MAX_TIMER; i++)
        timerCtrl.timers[i].flags = TIMER_FLAGS_FREE;
    return;
}

void inthandler20(int *esp){

    TIMER *timer;
    char taskswitch = 0;
    extern TIMER *taskTimer;     /* multitask.c */

    io_out8(PIC0_OCW2, 0x60);   /* 把IRQ-00信号接收完了的消息通知给PIC */
    timerCtrl.count++;

    if(timerCtrl.next_timeout > timerCtrl.count)
        return;

    timer = timerCtrl.head;
    while(1){
        if(timerCtrl.count  < timer->timeout)
            break;

        timer->flags = TIMER_FLAGS_ALLOC;

        if(timer != taskTimer){
            fifo32_put(timer->fifo, timer->data);
        }else{
            taskswitch = 1;
        }

        timer = timer->next;
    }
    
    timerCtrl.head = timer; /* 更新链表 */

    timerCtrl.next_timeout = timerCtrl.head->timeout;

    /* 多任务 */
    if(taskswitch != 0){
        task_switch();
    }

    return;
}

TIMER *timer_alloc(void){
    int i;
    for(i = 0; i < MAX_TIMER; i++){
        if(timerCtrl.timers[i].flags == TIMER_FLAGS_FREE){
            timerCtrl.timers[i].flags = TIMER_FLAGS_ALLOC;
            timerCtrl.timers[i].flags_basic = TIMER_FLAGS_BASIC;
            return &timerCtrl.timers[i];
        }
    }
    return 0;
}

void timer_free(TIMER *timer){
    timer->flags = TIMER_FLAGS_FREE;
    return;
}

void timer_init(TIMER *timer, FIFO32 *fifo, int data){
    timer->fifo = fifo;
    timer->data = data;
    return;
}

void timer_settimer(TIMER *timer, unsigned int timeout){
    int eflags;
    TIMER *temp_timer;

    timer->timeout = timeout + timerCtrl.count;
    timer->flags = TIMER_FLAGS_USING;

    eflags = io_load_eflags();
    io_cli();

    temp_timer = timerCtrl.head;
    if(timer->timeout <= temp_timer->timeout){
        timer->next = temp_timer;
        timerCtrl.head = timer;
        timerCtrl.next_timeout = timerCtrl.head->timeout;
        io_store_eflags(eflags);
        return;
    }

    while (timer->timeout > temp_timer->next->timeout) {
        temp_timer = temp_timer->next;
    }
    timer->next = temp_timer->next;
    temp_timer->next = timer;

    io_store_eflags(eflags);

    return;
}


int timer_cancel(TIMER *timer){
    int eflags;
    TIMER *t;

    eflags = io_load_eflags();
    
    io_cli();   /* 在设置中禁止改变定时器状态 */
    if(timer->flags == TIMER_FLAGS_USING){
        
        if(timer == timerCtrl.head){    /* 如果是第一个定时器 */

            timerCtrl.head = timer->next;
            timerCtrl.next_timeout = timerCtrl.head->timeout;
        }else{
            t = timerCtrl.head;
            while(t->next != timer){
                t = t->next;
            }
            t->next = t->next->next;
        }
        timer->flags = TIMER_FLAGS_ALLOC;
        io_store_eflags(eflags);
        return 1;
    }

    io_store_eflags(eflags);
    return 0;
}

void timer_cancelall(FIFO32 *fifo){
    int eflags;
    int i;
    TIMER *t;
    eflags = io_load_eflags();

    io_cli();   
    for(i = 0; i < MAX_TIMER; i++){
        t = &timerCtrl.timers[i];
        if(t->flags != TIMER_FLAGS_FREE && t->flags_basic != TIMER_FLAGS_BASIC && t->fifo == fifo){
            timer_cancel(t);
            timer_free(t);
        }
    }
    io_store_eflags(eflags);
    return;
}
