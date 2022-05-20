[FORMAT "WCOFF"]                    ; 生成对象文件模式
[INSTRSET "i486p"]                    ; 生成使用486兼容指令集
[BITS 32]                           ; 生成32位模式机器语言
[FILE "window.nas"]                 ; 源文件名信息

		GLOBAL	_api_openwin, _api_openwin_buf, _api_putstrwin, _api_boxfilwin, _api_refreshwin, _api_closewin

[SECTION .text]

_api_openwin:    ; int api_openwin(int xsiz, int ysiz, char *title); char *buf , int col_inv
        PUSH    EDI
        PUSH    ESI
        PUSH    EBX
        MOV     EDX, 5
        ; MOV     EBX, [ESP+16]           ; buf
        MOV     ESI, [ESP+16]           ; xsiz
        MOV     EDI, [ESP+20]           ; ysiz
        ; MOV     EAX, [ESP+28]           ; col_inv
        MOV     ECX, [ESP+24]           ; title
        INT     0x40
        POP     EBX
        POP     ESI
        POP     EDI
        RET

_api_openwin_buf:	; int api_openwin_buf(char *buf, int xsiz, int ysiz, char *title);
        PUSH    EDI
        PUSH    ESI
        PUSH    EBX
        MOV     EDX, 0xff
        MOV     EBX, [ESP+16]           ; buf
        MOV     ESI, [ESP+20]           ; xsiz
        MOV     EDI, [ESP+24]           ; ysiz
        ; MOV     EAX, [ESP+28]           ; col_inv
        MOV     ECX, [ESP+28]           ; title
        INT     0x40
        POP     EBX
        POP     ESI
        POP     EDI
        RET		


_api_putstrwin:	; void api_putstrwin(int win, int x, int y, int col, char *str);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX, 6
		MOV		EBX, [ESP+20]	; win
		MOV		ESI, [ESP+24]	; x
		MOV		EDI, [ESP+28]	; y
		MOV		EAX, [ESP+32]	; col
		; MOV	ECX, [ESP+36]	; len
		MOV		EBP, [ESP+36]	; str
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET

_api_boxfilwin:	; void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX, 7
		MOV		EBX, [ESP+20]	; win
		MOV		EAX, [ESP+24]	; x0
		MOV		ECX, [ESP+28]	; y0
		MOV		ESI, [ESP+32]	; x1
		MOV		EDI, [ESP+36]	; y1
		MOV		EBP, [ESP+40]	; col
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET

_api_refreshwin:			; void api_refreshwin(int win, int x0, int y0, int x1, int y1);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX, 12
		MOV		EBX, [ESP+16]		; win
		MOV		EAX, [ESP+20]		; x0
		MOV		ECX, [ESP+24]		; y0
		MOV		ESI, [ESP+28]		; x1
		MOV		EDI, [ESP+32]		; y1
		INT		0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET

_api_closewin:		; void api_closewin(int win);
		PUSH	EBX
		MOV		EDX, 14
		MOV		EBX, [ESP+8]		; win
		INT		0x40
		POP		EBX
		RET
