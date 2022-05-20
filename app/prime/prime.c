#include <stdio.h>
#include "../_header_/apifunc.h"

#define MAX     10000

void HariMain(){
    char flag[MAX];
    char s[5];
    int i, j;
    for(i = 0; i < MAX; i++)
        flag[i] = 0;
    for(i = 2; i < MAX; i++){
        if(flag[i] == 0){   /* 没有标记为素数 */
            sprintf(s, "%d ", i);
            api_putstr(s);
            for(j = i * 2; j < MAX; j += i)
                flag[j] = 1;
        }
    }
    api_end();
}