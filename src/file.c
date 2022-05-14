#include "../include/function.h"

void file_readfat(int *fat, unsigned char *img){    /* 将映像中的FAT解压缩 */
    int i, j = 0;
    for(i = 0; i < 2880; i += 2){
        /* 例：03 40 00 -> 003 004 */
        /* 例：13 40 01 -> 013 014 */
        fat[i + 0] = (img[j + 0]        | img[j + 1] << 8) & 0xfff; 
        fat[i + 1] = (img[j + 2] << 4   | img[j + 1] >> 4) & 0xfff;
        j += 3;
    }
    return;
}

//(char *) (finfo[x].clustno * 512 + 0x003e00 + ADR_DISKIMG); img = 0x003e00 + ADR_DISKIMG
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img){
    int i;
    for(;;){
        if(size <= 512){
            for(i = 0; i < size; i++)
                buf[i] = img[clustno * 512 + i];
            break;
        }
        for(i = 0; i < 512; i++)
            buf[i] = img[clustno * 512 + i];
        size -= 512;
        buf += 512;
        clustno = fat[clustno];
    }
    return;
}