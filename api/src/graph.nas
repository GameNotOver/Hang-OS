[FORMAT "WCOFF"]                    ; 生成对象文件模式
[INSTRSET "i486p"]                    ; 生成使用486兼容指令集
[BITS 32]                           ; 生成32位模式机器语言
[FILE "graph.nas"]                 ; 源文件名信息

        GLOBAL  _api_point,  _api_line

[SECTION .text]

_api_point:			; void api_point(int win, int x, int y, int col);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX, 11
		MOV		EBX, [ESP+16]		; win
		MOV		ESI, [ESP+20]		; x
		MOV		EDI, [ESP+24]		; y
		MOV		EAX, [ESP+28]		; col
		INT		0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET

_api_line:		; void api_line(int win, int x0, int y0, int x1, int y1, int col);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX, 13
		MOV		EBX, [ESP+20]		; win
		MOV		EAX, [ESP+24]		; x0
		MOV		ECX, [ESP+28]		; y0
		MOV		ESI, [ESP+32]		; x1
		MOV		EDI, [ESP+36]		; y1
		MOV		EBP, [ESP+40]		; col
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET
