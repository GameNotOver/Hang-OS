[FORMAT "WCOFF"]                    ; 生成对象文件模式
[INSTRSET "i486p"]                    ; 生成使用486兼容指令集
[BITS 32]                           ; 生成32位模式机器语言
[FILE "os.nas"]                 ; 源文件名信息
        GLOBAL  _api_getkey, _api_beep, _api_end
[SECTION .text]

_api_end:       ; void api_end(void);
        MOV     EDX, 4
        INT     0x40

_api_getkey:		; int api_getkey(int mode);
		MOV		EDX, 15
		MOV		EAX, [ESP+4]		; mode
		INT		0x40
		RET

_api_beep:			; void api_beep(int tone);
		MOV		EDX, 20
		MOV		EAX, [ESP+4]		; tone
		int		0x40
		RET
