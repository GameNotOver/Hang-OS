#include "../include/function.h"

TIMER *taskTimer;
TASKCTRL *taskCtrl;

TASK *task_init(MEMMAN *man){   /* 调用该函数以后，所有程序的运行都会被看作任务进行管理 */
    int i;
    TASK *task, *idle;
    SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *) ADR_GDT;
    taskCtrl = (TASKCTRL *) memman_alloc_4k(man, sizeof(TASKCTRL));
    for(i = 0; i < MAX_TASKLEVELS; i++){
        taskCtrl->tasks0[i].flags = TASK_FREE;
        taskCtrl->tasks0[i].selector = (TASK_GDT0 + i) * 8;
        set_segmdesc(gdt + (TASK_GDT0 + i), 103, (int) &taskCtrl->tasks0[i].tss, AR_TSS32);     /* sizeof(tss) = 104 B, 0~103 */
    }
    for(i = 0; i < MAX_TASKLEVELS; i++){
        taskCtrl->level[i].running = 0;
        taskCtrl->level[i].current = 0;
    }
    /* 调用该函数的程序也属于一个任务 */
    task = task_alloc();
    task->flags = TASK_RUN;
    task->priority = 2;         /* 0.02秒 , 默认主程序优先级 */
    task->level = 0;            /* 最高level */
    task_add(task);
    task_switch_preset();           /* level设置 */
    load_tr(task->selector);    /* 3 * 8 */
    taskTimer = timer_alloc();
    timer_settimer(taskTimer, task->priority);

    idle = task_alloc();
    idle->tss.eip = (int) &task_idle;
    idle->tss.esp = memman_alloc_4k(man, 64 * 1024) + 64 * 1024;
    idle->tss.es = 1 * 8;
    idle->tss.cs = 2 * 8;	/* GDT的2号 */
    idle->tss.ss = 1 * 8;
    idle->tss.ds = 1 * 8;
    idle->tss.fs = 1 * 8;
    idle->tss.gs = 1 * 8;
    task_run(idle, MAX_TASKLEVELS-1, 1);
    return task;
}

TASK *task_alloc(void){
    int i;
    TASK *task;
    for(i = 0; i < MAX_TASKS; i++){
        if(taskCtrl->tasks0[i].flags == TASK_FREE){
            task = &taskCtrl->tasks0[i];
            task->flags = TASK_ALLOC;
            task->tss.eflags = 0x00000202;  /* IF = 1; */
            task->tss.eax = 0;
            task->tss.ecx = 0;
            task->tss.edx = 0;
            task->tss.ebx = 0;
            // task->tss.esp = task_b_esp_top;
            task->tss.ebp = 0;
            task->tss.esi = 0;
            task->tss.edi = 0;
            task->tss.es = 0;
            task->tss.cs = 0;	
            task->tss.ss = 0;
            task->tss.ds = 0;
            task->tss.fs = 0;
            task->tss.gs = 0;
            task->tss.ldtr = 0;
            task->tss.iomap = 0x40000000;
            return task;
        }
    }
    return NULL;
}

void task_run(TASK *task, int level, int priority){
    if(level < 0)
        level = task->level;
    if(priority > 0)
        task->priority = priority;
    /* 如果当前任务正处于运行且需要修改优先队列，则先关闭任务 */
    if(task->flags == TASK_RUN && task->level != level)
        task_remove(task);
    /* 启动任务，并设置优先队列号 */
    if(task->flags != TASK_RUN){
        task->level = level;
        task_add(task);
    }
    taskCtrl->lv_change = 1;    /* 下次任务切换时检查level */
    return;
}

void task_switch(void){
    TASKLEVEL *task_level = &taskCtrl->level[taskCtrl->cur_lv];
    TASK *cur_task = task_level->tasks[task_level->current];
    TASK *new_task;
    task_level->current++;
    if(task_level->current == task_level->running)
        task_level->current = 0;
    if(taskCtrl->lv_change != 0){   /* 需要重新获取task_level */
        task_switch_preset();
        task_level = &taskCtrl->level[taskCtrl->cur_lv];
    }
    new_task = task_level->tasks[task_level->current];

    timer_settimer(taskTimer, new_task->priority);
    /* 当只有一个任务时，执行farjmp是原地跳转，CPU会认为这是错误命令，导致BUG，所以需要判断当前待运行的任务数 */
    if(new_task != cur_task)
        farjmp(0, task_level->tasks[task_level->current]->selector);
    return;
}

void task_sleep(TASK *task){
    TASK *cur_task;
    if(task->flags == TASK_RUN){
        cur_task = task_current();
        task_remove(task);
        if(task == cur_task){
            /* 如果让自己休眠，则需要切换任务 */
            task_switch_preset();
            cur_task = task_current();  /* 设定后获取当前任务值 */
            farjmp(0, cur_task->selector);
        }
    }
    return;
}

TASK *task_current(void){
    TASKLEVEL *task_level = &taskCtrl->level[taskCtrl->cur_lv];
    return task_level->tasks[task_level->current];
}

void task_add(TASK *task){
    TASKLEVEL *task_level = &taskCtrl->level[task->level];
    if(task_level->running < MAX_TASKS_LV){
        task_level->tasks[task_level->running] = task;
        task_level->running++;
        task->flags = TASK_RUN;
    }

    return;
}

void task_remove(TASK *task){
    int i;
    TASKLEVEL *task_level = &taskCtrl->level[taskCtrl->cur_lv];
    /* 寻找task所在的位置 */
    for(i = 0; i < task_level->running; i++)
        if(task_level->tasks[i] == task)
            break;
    task_level->running--;

    if(i < task_level->current)
        task_level->current--;
    if(task_level->current >= task_level->running)  /* 如果current的值出现异常，则修正 */
        task_level->current = 0;
    
    task->flags = TASK_ALLOC;

    for(; i < task_level->running; i++)
        task_level->tasks[i] = task_level->tasks[i + 1];
    
    return;    
}

void task_switch_preset(void){
    int i;
    for(i = 0; i < MAX_TASKLEVELS; i++)
        if(taskCtrl->level[i].running > 0)
            break;
    taskCtrl->cur_lv = i;
    taskCtrl->lv_change = 0;
    return;
}

void task_idle(void){
    for(;;)
        io_hlt;
}
