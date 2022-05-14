#include "../include/function.h"

// void init_fifo8(struct FIFO8 *fifo, int size, unsigned char *buf){
//     fifo->size = size;
//     fifo->buf = buf;
//     fifo->free = size;
//     fifo->flags = 0;
//     fifo->next_wt = 0;
//     fifo->next_rd = 0;
//     return;
// }

// int fifo8_put(struct FIFO8 *fifo, unsigned char data){
//     if(fifo->free == 0){
//         fifo->flags |= FLAGS_OVERRUN;
//         return -1;
//     }
//     fifo->buf[fifo->next_wt] = data;
//     fifo->next_wt++;
//     if(fifo->next_wt == fifo->size)
//         fifo->next_wt = 0;
//     fifo->free--;
//     return 0;
// }

// int fifo8_get(struct FIFO8 *fifo){
//     int data;
//     if(fifo->free == fifo->size){
//         return -1;
//     }
//     data = fifo->buf[fifo->next_rd];
//     fifo->next_rd++;
//     if(fifo->next_rd == fifo->size)
//         fifo->next_rd = 0;
//     fifo->free++;
//     return data;
// }

// int fifo8_status(struct FIFO8 *fifo){
//     return fifo->size - fifo->free;
// }

void init_fifo32(FIFO32 *fifo, int size, int *buf, TASK *task){
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->next_wt = 0;
    fifo->next_rd = 0;
    fifo->task = task;
    return;
}

int fifo32_put(FIFO32 *fifo, int data){
    if(fifo->free == 0){
        fifo->flags |= FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->next_wt] = data;
    fifo->next_wt++;
    if(fifo->next_wt == fifo->size)
        fifo->next_wt = 0;
    fifo->free--;

    if(fifo->task != NULL)
        if(fifo->task->flags != TASK_RUN)
            task_run(fifo->task, -1, 0);    /* 不设置优先级（沿用原来的优先级，和优先队列） */

    return 0;
}

int fifo32_get(FIFO32 *fifo){
    int data;
    if(fifo->free == fifo->size){
        return -1;
    }
    data = fifo->buf[fifo->next_rd];
    fifo->next_rd++;
    if(fifo->next_rd == fifo->size)
        fifo->next_rd = 0;
    fifo->free++;
    return data;
}

int fifo32_status(FIFO32 *fifo){
    return fifo->size - fifo->free;
}


