[FORMAT "WCOFF"]                    ; 生成对象文件模式
[INSTRSET "i486p"]                    ; 生成使用486兼容指令集
[BITS 32]                           ; 生成32位模式机器语言
[FILE "timer.nas"]                 ; 源文件名信息

		GLOBAL	_api_alloctimer, _api_inittimer, _api_settimer, _api_freetimer

[SECTION .text]

_api_alloctimer:	; int api_alloctimer(void);
		MOV		EDX, 16
		INT		0x40
		RET

_api_inittimer:		; void api_inittimer(int timer, int data);
		PUSH	EBX
		MOV		EDX, 17
		MOV		EBX, [ESP+8]		; timer
		MOV		EAX, [ESP+12]		; data
		INT		0x40
		POP		EBX
		RET

_api_settimer:		; void api_settimer(int timer, int time);
		PUSH	EBX
		MOV		EDX, 18
		MOV		EBX, [ESP+8]		; timer
		MOV		EAX, [ESP+12]		; time
		INT		0x40
		POP		EBX
		RET

_api_freetimer:		; void api_freetimer(int timer);
		PUSH	EBX
		MOV		EDX, 19
		MOV		EBX, [ESP+8]		; timer
		INT		0x40
		POP		EBX
		RET
