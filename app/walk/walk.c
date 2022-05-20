#include "../_header_/apifunc.h"

void HariMain(){
    int win;
    int i;
    int x, y;

    win = api_openwin(160, 100, "Walk");
    api_boxfilwin(win, 4, 24, 155, 95, COL8_000000);
    x = 76; y = 56;

    api_putstrwin(win, x, y, COL8_FFFF00, "*");
    for(;;){
        i = api_getkey(1);
        api_putstrwin(win, x, y, COL8_000000, "*");

        if( i == 'd' && x < 148) { x += 8; }
        if( i == 'a' && x > 4  ) { x -= 8; }
        if( i == 's' && y < 80 ) { y += 8; }
        if( i == 'w' && y > 24 ) { y -= 8; }

        if(i == 0x0a)
            break;
        api_putstrwin(win, x, y, COL8_FFFF00, "*");
    }
    api_closewin(win);
    api_end();
}
