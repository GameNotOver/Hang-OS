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

FILEINFO  *file_search(char *fname, FILEINFO *finfo, int max){
	int x, y;
	char fileName[12];

    for(x = 0; x < 12; x++)
        fileName[x] = ' ';
    fileName[11] = 0;
	for(x = 0, y = 0; y < 11 && fname[x] != 0; x++){
		if(fname[x] == '.' && y <= 8){
			y = 8;
		}else{
			fileName[y] = fname[x];
			if('a' <= fileName[y] && fileName[y] <= 'z')
				fileName[y] -= 0x20;
			y++;
		}
	}
	if(x == 13)	/* 文件名加后缀不得超过12 最长为8+1+3=12，如ABCDEFGH.HRB */
		return NULL;
	/* 寻找文件 */
	for(x = 0; x < max; ){
		if(finfo[x].name[0] == 0x00)
			break;
		if((finfo[x].type & 0x18) == 0){
			for(y = 0; y < 11; y++){
				if(finfo[x].name[y] != fileName[y])
					goto type_next_file;
			}
			return finfo + x; /* 找到文件 */
		}
	type_next_file:
		x++;
	}

	return NULL;
}
