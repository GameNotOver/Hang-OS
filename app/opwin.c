#define COL8_000000		0 		// BLACK			
#define COL8_FF0000		1 		// LIGHT_RED		
#define COL8_00FF00		2 		// LIGHT_GREEN	
#define COL8_FFFF00		3 		// LIGHT_YELLOW	
#define COL8_0000FF		4 		// LIGHT_BLUE1	
#define COL8_FF00FF		5 		// LIGHT_PURPLE	
#define COL8_00FFFF		6 		// LIGHT_BLUE2	
#define COL8_FFFFFF		7 		// WHITE			
#define COL8_C6C6C6		8 		// LIGHT_GRAY		
#define COL8_840000		9 		// DARK_RED		
#define COL8_008400		10		// DARK_GREEN		
#define COL8_848400		11		// DARK_YELLOW	
#define COL8_000084		12		// DARK_BLUE1		
#define COL8_840084		13		// DARK_PURPLE	
#define COL8_008484		14		// DARK_BLUE2		
#define COL8_848484		15		// DARK_GRAY
#define COL8_TRSPAR		99		// TRANSPARENT

int api_openwin(int xsiz, int ysiz, char *title);
int api_openwin_buf(char *buf, int xsiz, int ysiz, char *title);
void api_end(void);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_putstrwin(int win, int x, int y, int col, char *str);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);

int rand(void);

void HariMain(void){
    int win, win2;
    char *win2_buf;
    int x, y, i;
    
    api_initmalloc();
    win = api_openwin(150, 100, "hello");
    api_boxfilwin(win, 6, 26, 143, 93, COL8_000000);
    for(i = 0; i < 70; i++){
        x = (rand() % 137) + 6;
        y = (rand() % 67) + 26;
        api_point(win + 1, x, y, COL8_FFFF00);
    }
    api_refreshwin(win, 6, 26, 144, 94);
    api_end();
}
// win2_buf = api_malloc(150 * 50);
// win2 = api_openwin_buf(win2_buf, 150, 50, "buf win");
// api_putstrwin(win, 28, 28, COL8_000000, "hello world");
// api_boxfilwin(win2, 8, 36, 141, 43, COL8_0000FF);
// api_putstrwin(win2, 28, 28, COL8_000000, "hello buf");
