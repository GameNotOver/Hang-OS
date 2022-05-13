
int stringlength(char *str){
    int count = 0;
    while(*(str++) != 0x00)
        count++;
    return count;
}
