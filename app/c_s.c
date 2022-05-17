void api_putstr(char *str);
void api_end(void);

void HariMain(void){
    
    char *str = "c file put str by str";
    api_putstr(str);
    api_end();
}