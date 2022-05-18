#include "apifunc.h"
#include <stdio.h>
void HariMain(){
    char s[12];
    int win;
    int timer = 0;
    int sec = 0, min = 0, hour = 0;

    win = api_openwin(150, 50, "noodle");
    timer = api_alloctimer();
    api_boxfilwin(win, 28, 27, 115, 41, COL8_FFFFFF);
    api_putstrwin(win, 28, 27, COL8_000000, s);
    api_inittimer(timer, 128);
    for(;;){

        sprintf(s, "%02d:%02d:%02d", hour, min, sec);
        api_boxfilwin(win, 28, 27, 115, 41, COL8_FFFFFF);
        api_putstrwin(win, 28, 27, COL8_000000, s);

        api_settimer(timer, 1);   /* 1秒 */

        if(api_getkey(1) != 128){
            break;
        }

        sec++;
        
        if(sec == 60){
            sec = 0;
            min++;
            if(min == 60){
                min = 0;
                hour++;
            }
        }
    }
    api_end();
}