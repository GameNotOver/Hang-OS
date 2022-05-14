#include "../include/function.h"

int cons_newline(int cursor_y, SHEET *sheet){
    int x, y;
    
    if(cursor_y < 28 + 112){
        cursor_y += 16;
    }else{	/* 滚动 */
        /* 除第一行往上移一行 */
        for(y = 28; y < 28 + 112; y++)
            for(x = 8; x < 8 + 240; x++)
                sheet->buf[x + sheet->bxsize * y] = sheet->buf[x + sheet->bxsize * (y + 16)];
        /* 最后一行涂黑 */
        for(y = 28 + 112; y < 28 + 128; y++)	
            for(x = 8; x < 8 + 240; x++)
                sheet->buf[x + sheet->bxsize * y] = COL8_000000;
        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
    }
    return cursor_y;
}