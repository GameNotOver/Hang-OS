void api_putstr(char *str);
void api_end(void);

void HariMain(void){
    
    char str[10] = "hang";
    api_putstr(str);
    api_end();
}