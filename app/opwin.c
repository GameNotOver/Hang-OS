#include "apifunc.h"

int rand(void);

void HariMain(void){
    int win;
    int x, y, i;
    
    api_initmalloc();
    win = api_openwin(160, 100, "hello");
    api_boxfilwin(win, 6, 26, 153, 93, COL8_000000);
    for(i = 0; i < 70; i++){
        x = (rand() % 137) + 6;
        y = (rand() % 67) + 26;
        api_point(win + 1, x, y, COL8_FFFF00);
    }
    for(i = 0; i < 8; i++){
        api_line(win + 1, 8, 26, 77, i * 9 + 26, i + 1);
        api_line(win + 1, 88, 26, i * 9 + 88, 89, 8 - i);
    }

    api_refreshwin(win, 6, 26, 154, 94);

    for(;;)
        if(api_getkey(1) == 0x0a)
            break;
    api_closewin(win);
    api_end();
}
// win2_buf = api_malloc(150 * 50);
// win2 = api_openwin_buf(win2_buf, 150, 50, "buf win");
// api_putstrwin(win, 28, 28, COL8_000000, "hello world");
// api_boxfilwin(win2, 8, 36, 141, 43, COL8_0000FF);
// api_putstrwin(win2, 28, 28, COL8_000000, "hello buf");
