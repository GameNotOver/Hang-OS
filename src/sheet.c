#include "../include/function.h"

SHEETCTRL *sheetctrl_init(MEMMAN *man, unsigned char *vram, int xsize, int ysize){
    SHEETCTRL *ctrl;
    int i;
    ctrl = (SHEETCTRL *) memman_alloc_4k(man, sizeof(SHEETCTRL));
    if(ctrl == 0)
        goto err;
    
    ctrl->map = (unsigned char *) memman_alloc_4k(man, xsize * ysize);
    if(ctrl->map == 0){
        memman_free(man, (int) ctrl, sizeof(SHEETCTRL));
        goto err;
    }

    ctrl->vram = vram;
    ctrl->xsize = xsize;
    ctrl->ysize = ysize;
    ctrl->top = -1;         /* 一个sheet都没有 */
    for(i = 0; i < MAX_SHEETS; i++){
        ctrl->sheets0[i].flags = 0; /* 标记为未使用 */
        ctrl->sheets0[i].ctrl = ctrl;
    }
err:
    return ctrl;
}

SHEET *sheet_alloc(SHEETCTRL *ctrl){
    SHEET *sheet;
    int i;
    for(i = 0; i < MAX_SHEETS; i++){
        if(ctrl->sheets0[i].flags == 0){
            sheet = &ctrl->sheets0[i];
            sheet->flags = SHEET_USE;
            sheet->height = -1; /* 隐藏 */
            sheet->task = 0;    /* 不使用自动关闭功能 */
            return sheet;
        }
    }
    return 0;   /* 所有sheet均处于使用状态 */
}

void sheet_setbuf(SHEET *sheet, unsigned char *buf, int xsize, int ysize, int col_inv){
    sheet->buf = buf;
    sheet->bxsize = xsize;
    sheet->bysize = ysize;
    sheet->col_inv = col_inv;
    return;
}

void sheet_updown(SHEET *sheet, int height){
    int i;
    int old_height = sheet->height;     /* 存储原来的高度信息 */
    SHEETCTRL *ctrl = sheet->ctrl;

    /* 如果高度过低或过高，则进行修正 */
    height = min(height, ctrl->top + 1);
    height = max(height, -1);

    sheet->height = height;

    /* sheets[]重排序 */
    if(old_height > height){    /* 比以前低 */
        if(height >= 0){
            /* 把中间的往上提 */
            for(i = old_height; i > height; i--){
                ctrl->sheets[i] = ctrl->sheets[i-1];
                ctrl->sheets[i]->height = i;
            }
            ctrl->sheets[height] = sheet;
            sheet_refreshmap(ctrl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, height + 1);
            sheet_refreshsub(ctrl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, height + 1, old_height);    /* 重画当前图层 */
        }else{  /* 需要隐藏 */
            if(ctrl->top > old_height){     /* 不是最上层 */
                /* 把上面的降下来 */
                for(i = old_height; i < ctrl->top; i++){
                    ctrl->sheets[i] = ctrl->sheets[i+1];
                    ctrl->sheets[i]->height = i;
                }
            }
                                            /* 是最上层，则不需要处理*/
            ctrl->top--;    /* 由于显示中的图层减少一个，所以最上面的图层数减少 */
            sheet->height = -1;
            sheet_refreshmap(ctrl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, 0);
            sheet_refreshsub(ctrl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, 0, old_height - 1);    /* 重画当前图层 */
        }
    }else if (old_height < height){ /* 比以前高 */
        if(old_height >= 0){    /* 原本就是显示状态 */
            /* 把中间的往下扯 */
            for(i = old_height; i < height; i++){
                ctrl->sheets[i] = ctrl->sheets[i+1];
                ctrl->sheets[i]->height = i;
            }
            ctrl->sheets[height] = sheet;
        }else{  /* 由隐藏状态转为显示状态 */
            /* 将height以上的往上提 */
            for(i = ctrl->top; i >= height; i--){
                ctrl->sheets[i+1] = ctrl->sheets[i];
                ctrl->sheets[i+1]->height = i + 1;
            }
            ctrl->sheets[height] = sheet;
            ctrl->top++;    /* 由于该图层由隐藏变为显示，所以图层数增加 */
        }
        sheet_refreshmap(ctrl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, height);
        sheet_refreshsub(ctrl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, height, height);    /* 重画当前图层 */
    }
    return;
}

void sheet_refresh(SHEET *sheet, int bx0, int by0, int bx1, int by1){
    if(sheet->height >= 0){
        sheet_refreshsub(sheet->ctrl, sheet->vx0 + bx0, sheet->vy0 + by0, sheet->vx0 + bx1, sheet->vy0 + by1, sheet->height, sheet->height);
    }
    return;
}

void sheet_slide(SHEET *sheet, int vx0, int vy0){
    SHEETCTRL *ctrl = sheet->ctrl;
    int old_vx0, old_vy0;
    old_vx0 = sheet->vx0;
    old_vy0 = sheet->vy0;
    sheet->vx0 = vx0;
    sheet->vy0 = vy0;
    if(sheet->height >= 0){
        sheet_refreshmap(ctrl, old_vx0, old_vy0, old_vx0 + sheet->bxsize, old_vy0 + sheet->bysize, 0);
        sheet_refreshmap(ctrl, vx0, vy0, vx0 + sheet->bxsize, vy0 + sheet->bysize, sheet->height);
        sheet_refreshsub(ctrl, old_vx0, old_vy0, old_vx0 + sheet->bxsize, old_vy0 + sheet->bysize, 0, sheet->height - 1);
        sheet_refreshsub(ctrl, vx0, vy0, vx0 + sheet->bxsize, vy0 + sheet->bysize, sheet->height, sheet->height);
    }
    return;
}

void sheet_free(SHEET *sheet){
    if(sheet->height >= 0)
        sheet_updown(sheet, -1);
    sheet->flags = 0; /* 未使用状态 */
    return;
}

void sheet_refreshsub(SHEETCTRL *ctrl, int vx0, int vy0, int vx1, int vy1, int h0, int h1){
    int i;
    int bx, by;
    int bx0, bx1, by0, by1;
    int vx, vy;
    unsigned char *buf;
    unsigned char *vram = ctrl->vram;
    unsigned char *map = ctrl->map;
    unsigned char sid;
    SHEET *sheet;

    /* 如果refresh的范围超出了画面则修正 */
    vx0 = max(0, vx0);
    vy0 = max(0, vy0);
    vx1 = min(ctrl->xsize, vx1);
    vy1 = min(ctrl->ysize, vy1);

    for(i = h0; i <= h1; i++){
        sheet = ctrl->sheets[i];
        buf = sheet->buf;
        sid = sheet - ctrl->sheets0;

        /* 使用vx0 vy0 vx1 vy1 倒推 bx0 by0 bx1 by1 */
        bx0 = max(0, vx0 - sheet->vx0);
        by0 = max(0, vy0 - sheet->vy0);
        bx1 = min(sheet->bxsize, vx1 - sheet->vx0);
        by1 = min(sheet->bysize, vy1 - sheet->vy0);

        for(by = by0; by < by1; by++){
            vy = sheet->vy0 + by;
            for(bx = bx0; bx < bx1; bx++){
                vx = sheet->vx0 + bx;
                if(map[vy * ctrl->xsize + vx] == sid){
                    vram[vy * ctrl->xsize + vx] = buf[by * sheet->bxsize + bx];
                }
            }
        }
    }
    return;
}

void sheet_refreshmap(SHEETCTRL *ctrl, int vx0, int vy0, int vx1, int vy1, int h0){
    int i;
    int bx, by;
    int vx, vy;
    int bx0, by0, bx1, by1;
    unsigned char *buf, sid;
    unsigned char *map = ctrl->map;
    SHEET *sheet;

    vx0 = max(0, vx0);
    vy0 = max(0, vy0);
    vx1 = min(ctrl->xsize, vx1);
    vy1 = min(ctrl->ysize, vy1);

    for(i = h0; i <= ctrl->top; i++){
        sheet = ctrl->sheets[i];
        sid = sheet - ctrl->sheets0;    /* 将进行减法计算的地址作为图层id */
        buf = sheet->buf;

        bx0 = max(0, vx0 - sheet->vx0);
        by0 = max(0, vy0 - sheet->vy0);
        bx1 = min(sheet->bxsize, vx1 - sheet->vx0);
        by1 = min(sheet->bysize, vy1 - sheet->vy0);

        for(by = by0; by < by1; by++){
            vy = sheet->vy0 + by;
            for(bx = bx0; bx < bx1; bx++){
                vx = sheet->vx0 + bx;
                if(buf[by * sheet->bxsize + bx] != sheet->col_inv){
                    map[vy * ctrl->xsize + vx] = sid;
                }
            }
        }

    }
}
