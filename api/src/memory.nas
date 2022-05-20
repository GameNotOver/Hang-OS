[FORMAT "WCOFF"]                    ; 生成对象文件模式
[INSTRSET "i486p"]                    ; 生成使用486兼容指令集
[BITS 32]                           ; 生成32位模式机器语言
[FILE "memory.nas"]                 ; 源文件名信息
        GLOBAL  _api_initmalloc, _api_malloc, _api_free
[SECTION .text]

_api_initmalloc:	; void api_initmalloc(void);
		PUSH	EBX
		MOV		EDX, 8
		MOV		EBX, [CS:0x0020]	; malloc内存空间的地址
		MOV		EAX, EBX
		ADD		EAX, 32*1024		; 加上32KB
		MOV		ECX, [CS:0X0000]	; 数据段大小
		SUB		ECX, EAX
		INT		0X40
		POP		EBX
		RET

_api_malloc:		; char *api_malloc(int size);
		PUSH	EBX
		MOV		EDX, 9
		MOV		EBX, [CS:0x0020]
		MOV		ECX, [ESP+8]		; size
		INT		0x40
		POP		EBX
		RET

_api_free:			; void api_free(char *addr, int size);
		PUSH	EBX
		MOV		EDX, 10
		MOV		EBX, [CS:0x0020]
		MOV		EAX, [ESP+8]		; addr
		MOV		ECX, [ESP+12]		; addr
		INT		0x40
		POP		EBX
		RET
