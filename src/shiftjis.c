#include "../include/function.h"

void supportShiftJis(char *vram, int xsize, int x, int y, char c, unsigned char *s){
    extern char hankaku[4096];
	TASK *task = task_current();
	// char *nihong = (char *) *((int *) 0x0fc8);
	char *nihong = (char *) *((int *) 0x0fc8);
	char *font;
	unsigned int k, t;

    if(task->langmode == 1){	/* SHIFT-JIS */
		for (; *s != 0x00; s++) {
			if(task->langbyte1 == 0){
				if((0x81 <= *s && *s <= 0x9f) || (0xe0 <= *s && *s <= 0xfc))
					task->langbyte1 = *s;
				else
					putfont8(vram, xsize, x, y, c, nihong + *s * 16);
			}else{
				if(0x81 <= task->langbyte1 && task->langbyte1 <= 0x9f)
					k = (task->langbyte1 - 0x81) * 2;
				else
					k = (task->langbyte1 - 0xe0) * 2 + 62;

				if(0x40 <= *s && *s <= 0x7e){
					t = *s - 0x40;
				}else if(0x80 <= *s && *s <= 0x9e){
					t = *s - 0x80 + 63;
				}else{
					t = *s - 0x9f;
					k++;
				}
				task->langbyte1 = 0;
				font = nihong + 256 * 16 + (k * 94 + t) * 32;
				putfont8(vram, xsize, x - 8, y, c, font);		/* 左半部分 */
				putfont8(vram, xsize, x    , y, c, font + 16);	/* 右半部分 */
			}
			x += 8;
		}
	}
	if(task->langmode == 2){	/* EUC */
		for(; *s != 0x00; s++){
			if(task->langbyte1 == 0){
				if(0x81 <= *s && *s <= 0xfe){
					task->langbyte1 = *s;
				}else{
					putfont8(vram, xsize, x, y, c, nihong + *s * 16);
				}
			}else{
				k = task->langbyte1 - 0xa1;
				t = *s - 0xa1;
				task->langbyte1 = 0;
				font = nihong + 256 * 16 + (k * 94 + t) * 32;
				putfont8(vram, xsize, x - 8, y, c, font);
				putfont8(vram, xsize, x, y, c, font + 16);
			}
			x += 8;
		}
	}
}