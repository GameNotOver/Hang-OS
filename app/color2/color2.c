#include "../_header_/apifunc.h"

unsigned char rgb2pal(int r, int g, int b, int x, int y);

void HariMain(){
    int win;
    int x, y;
    char buf[144 * 164];
    win = api_openwin_buf(buf, 144, 164, "color2");
    for(y = 0; y < 128; y++){
        for(x = 0; x < 128; x++){
             buf[(x + 8) + (y + 28) * 144] = rgb2pal(x*2, y*2, 0, x, y);
        }
    }
    api_refreshwin(win, 8, 28, 136, 156);
    api_getkey(1);
    api_end();
}

unsigned char rgb2pal(int r, int g, int b, int x, int y){
    static int table[4] = {3, 1, 0, 2};
    int i;
    y &= 1; /* 判断奇偶 */
    x &= 1;
    i = table[x + y * 2];   /* 用来生成中间色的常量 */
    r = (r * 21) / 256;     /* r为0~20 */
    g = (g * 21) / 256;
    b = (b * 21) / 256;
    r = (r + i) / 4;        /* r为0~5 */
    g = (g + i) / 4;
    b = (b + i) / 4;
    return 16 + r + g * 6 + b *36;
}
