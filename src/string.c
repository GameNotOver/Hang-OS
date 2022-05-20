
int stringlength(char *str){
    int count = 0;
    while(*(str++) != 0x00)
        count++;
    return count;
}

int strcmp_len(char *str_1, char *str_2, int len){
    int i = 0;
    while(i < len){
        if(str_1[i] > str_2[i])
            return 1;
        if(str_1[i] < str_2[i])
            return -1;
        i++;
    }
    return 0;
}

void stringcat(char *dist, char *src){
    int i = 0, j;
    while(dist[i] != 0)
        i++;
    for(j = 0; src[j] != 0; j++, i++)
        dist[i] = src[j] ;
    dist[i] = 0;
}

// void strcpy_len(char *dist, char *src, int len){
//     int i;
//     while(i < len){

//     }
// }
