void api_end(void);

void HariMain(void){
    *((int *) 0x00102600) = 0;
    api_end();
}
