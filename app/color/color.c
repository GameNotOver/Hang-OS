#include <stdio.h>
#include "../_header_/apifunc.h"

void HariMain(){
    int win;
    int r, g, b;
    int x, y;
    char *buf;
    api_initmalloc();
    buf = api_malloc(144 * 164);
    win = api_openwin_buf(buf, 144, 164, "color");
    for(y = 0; y < 128; y++){
        for(x = 0; x < 128; x++){
            r = x * 2;
            g = y * 2;
            b = 0;

            buf[(x + 8) + (y + 28) * 144] = 16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36;
        }
    }
    api_refreshwin(win, 8, 28, 136, 156);
    api_getkey(1);
    api_end();
}
