#include "../_header_/apifunc.h"

void HariMain(){
    int i;
    int timer;
    timer = api_alloctimer();
    api_inittimer(timer, 128);
    for(i = 20000000; i >= 20000; i-= i / 100){
        /* 人类可以听到的范围：20KHz ~ 20Hz */
        /* i以i%的速度递减 */
        api_beep(i);
        api_settimer(timer, 1); /* 0.01s */
        if(api_getkey(1) != 128)
            break;
    }

    api_beep(0);
    api_end();
}
