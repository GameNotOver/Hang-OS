#include "../_header_/apifunc.h"

void HariMain(){
    int fh;
    char c;
    char cmdline[30], *p;
    api_cmdline(cmdline, 30);
    for(p = cmdline; *p != ' ' && *p != 0; p++);
    for(; *p == ' '; p++);
    fh = api_fopen(p);
    if(fh != 0){    
        for(;;){
            if(api_fread(&c, 1, fh) == 0)
                break;
            api_putchar(c);
        }
    }else{
        api_putstr("\nFile not found!");
    }
    api_end();
}
