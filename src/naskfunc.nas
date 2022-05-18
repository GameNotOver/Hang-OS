; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的模式	
[INSTRSET "i486p"]				; 使用到486为止的指令
[BITS 32]						; 制作32位模式用的机器
[FILE "naskfunc.nas"]			; 源程序文件名

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_asm_inthandler0c, _asm_inthandler0d, _asm_inthandler20 
		GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
		GLOBAL	_load_cr0, _store_cr0
		GLOBAL 	_memtest_sub
		GLOBAL 	_load_tr
		GLOBAL 	_farjmp, _farcall
		GLOBAL 	_asm_cons_putchar
		GLOBAL 	_asm_os_api
		GLOBAL 	_start_app, _asm_end_app
		EXTERN 	_inthandler0c, _inthandler0d, _inthandler20, _inthandler21, _inthandler27, _inthandler2c
		EXTERN	 _cons_putchar, _os_api

[SECTION .text]

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_io_cli:	; void io_cli(void);
		CLI
		RET

_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS という意味
		POP		EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS という意味
		RET

_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

; _asm_inthandler20:
; 		PUSH	ES
; 		PUSH	DS
; 		PUSHAD
; 		MOV		AX, SS
; 		CMP		AX, 1*8
; 		JNE		.from_app
; 		MOV		EAX, ESP
; 		PUSH	EAX
; 		MOV		AX, SS
; 		MOV		DS, AX
; 		MOV		ES, AX
; 		CALL	_inthandler20
; 		POP		EAX
; 		POPAD
; 		POP		DS
; 		POP		ES
; 		IRETD	
; .from_app:
; ; 当应用程序活动时发生中断
; 		MOV		EAX, 1*8
; 		MOV		DS, AX						; 先仅将DS设定为操作系统用
; 		MOV		ECX, [0xfe4]				; 操作系统用ESP
; 		ADD		ECX, -8
; 		MOV		[ECX+4], SS
; 		MOV		[ECX], ESP
; 		MOV		SS, AX
; 		MOV		ES, AX
; 		MOV		ESP, ECX
; 		CALL	_inthandler20
; 		POP		ECX
; 		POP		EAX
; 		MOV		SS, AX						; 将SS设回应用程序用
; 		MOV		ESP, ECX					; 将ESP设回应用程序用
; 		POPAD
; 		POP		DS
; 		POP		ES
; 		IRETD

 _asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH	EAX
		MOV		AX, SS
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD


_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX, ESP
		PUSH	EAX
		MOV		AX, SS
		MOV		DS, AX
		MOV		ES, AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

; 栈异常处理
_asm_inthandler0c:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0c
		CMP		EAX, 0
		JNE		_asm_end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP, 4						; 在INT 0x0c中需要这句
		IRETD
		

_asm_inthandler0d:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP		EAX, 0
		JNE		_asm_end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP, 4						; 在INT 0x0d中需要这句
		IRETD


_load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

_store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV 	CR0,EAX
		RET

_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end);
		PUSH	EDI						; 由于要使用EBX，ESI，EDI，所以执行本段程序前需要先把寄存器中的原值保存
		PUSH	ESI	
		PUSH	EBX	

		MOV		ESI, 0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI, 0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX, [ESP+12+4]			; i = start;
mts_loop:
		MOV		EBX, EAX	
		ADD		EBX, 0xffc				; p = i + 0xffc;
		MOV 	EDX, [EBX]				; old = *p;
		MOV		[EBX], ESI				; *p = pat0;
		XOR		DWORD [EBX], 0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI, [EBX]				; if(*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX], 0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI, [EBX]				; if(*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX], EDX				; *p = old;
		ADD		EAX, 0x1000				; i += 0x1000
		CMP		EAX, [ESP+12+8]			; if(i <= end) goto mts_loop;
		JBE		mts_loop

		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX], EDX				; *p = old;

		POP		EBX						; break;
		POP		ESI
		POP		EDI
		RET
_load_tr:		; void load_tr(int tr);
		LTR 	[ESP+4]
		RET
_farjmp:		; void farjmp(int eip, int cs);
		JMP		FAR [ESP+4]				; eip, cs
		RET
_farcall:		; void farcall(int eip, int cs)
		CALL	FAR [ESP+4]				; eip, cs
		RET

_asm_cons_putchar:
		STI								; 对于INT指令，CPU会自动调用CLI指令，在本程序中不需要该操作，所以开中断
		PUSHAD							; 保存寄存器当前值
		PUSH	1						; cons_putchar函数的第三个参数
		AND		EAX, 0xff				; 将AH和EAX的高位置零， 将EAX置为已存入字符编码的状态
		PUSH	EAX						; cons_putchar函数的第二个参数
		PUSH	DWORD [0x0fec]			; 读取内存并push该值（cons的地址），cons_putchar函数的第一个参数
		CALL	_cons_putchar
		ADD		ESP, 12					; 将栈中数据丢弃
		POPAD							; 恢复各个寄存器的值
		IRETD							; 既然用了far-CALL，那么也应该用far-RET，即RETF指令，
										;  进一步通过INT指令调用的程序会被视作中断处理，用RETF
										;  是无法返回的需要使用IRETD指令。
; _asm_os_api:
; 		; 为了方便起见，开头就禁止中断
; 		PUSH	DS						
; 		PUSH	ES						
; 		PUSHAD							;  顺序：EAX->ECX->EDX->EBX->ESP->EBP->ESI->EDI
; 		MOV		EAX, 1*8
; 		MOV		DS, AX					; 先仅将DS设定为操作系统用
; 		MOV		ECX, [0xfe4]			; 操作系统的ESP
; 		ADD		ECX, -40
; 		MOV 	[ECX+32], ESP			; 保存应用程序的ESP
; 		MOV		[ECX+36], SS			; 保存应用程序的SS

; ; 将PUSHAD后的值复制到系统栈

; 		MOV		EDX, [ESP]				; 复制传递给os_api
; 		MOV		EBX, [ESP+4]			; 复制传递给os_api
; 		MOV 	[ECX], EDX
; 		MOV		[ECX+4], EBX

; 		MOV		EDX, [ESP+8]			; 复制传递给os_api
; 		MOV		EBX, [ESP+12]			; 复制传递给os_api
; 		MOV 	[ECX+8], EDX
; 		MOV		[ECX+12], EBX

; 		MOV		EDX, [ESP+16]			; 复制传递给os_api
; 		MOV 	EBX, [ESP+20]			; 复制传递给os_api
; 		MOV 	[ECX+16], EDX
; 		MOV		[ECX+20], EBX

; 		MOV		EDX, [ESP+24]			; 复制传递给os_api
; 		MOV 	EBX, [ESP+28]			; 复制传递给os_api
; 		MOV 	[ECX+24], EDX
; 		MOV		[ECX+28], EBX

; 		MOV		ES, AX					; 将剩余的段寄存器也设为操作系统使用
; 		MOV		SS, AX
; 		MOV		ESP, ECX
; 		STI								; 开中断

; 		CALL	_os_api					; 传入os_api中的8个参数存于ECX+Offset

; 		MOV		ECX, [ESP+32]			; 取出应用程序的ESP
; 		MOV		EAX, [ESP+36]			; 取出应用程序的SS
; 		CLI								; 关中断
; 		MOV		SS, AX
; 		MOV		ESP, ECX	
; 		POPAD
; 		POP		ES
; 		POP		DS
; 		IRETD							; 这个程序会自动执行STI

_asm_os_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD							; 用于保存的PUSH
		PUSHAD							; 用于向os_api传值的push
		MOV		AX, SS
		MOV		DS, AX					; 将操作系统用段地址存入DS和ES
		MOV		ES, AX					
		CALL 	_os_api
		CMP		EAX, 0					; 当EAX不为0时程序结束
		JNE		_asm_end_app
		ADD		ESP, 32
		POPAD
		POP		ES
		POP		DS
		IRETD

_asm_end_app:
; EAX为tss.esp0的地址
		MOV		ESP, [EAX]
		MOV		DWORD [EAX+4], 0
		POPAD
		RET								; 返回cmd_app


_start_app:		; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
		PUSHAD							; 将32位寄存器全部保护起来
		MOV		EAX, [ESP+36]			; 应用程序用EIP
		MOV		ECX, [ESP+40]			; 应用程序用CS
		MOV		EDX, [ESP+44]			; 应用程序用ESP
		MOV		EBX, [ESP+48]			; 应用程序用DS/SS
		MOV		EBP, [ESP+52]			; tss.esp0的地址
		MOV		[EBP], ESP				; 保存操作系统用ESP
		MOV		[EBP+4], SS				; 保存操作系统用SS
		; CLI								; 切换过程中关中断
		MOV		ES, BX
		; MOV		SS, BX
		MOV		DS, BX
		MOV		FS, BX
		MOV		GS, BX
		; MOV		ESP, EDX
; 下面调整栈，以免用RETF跳转到应用程序
		OR		ECX, 3					; 将应用程序用段号和3进行OR运算
		OR		EBX, 3					; 将应用程序用段号和3进行OR运算
		PUSH	EBX						; 应用程序的SS
		PUSH	EDX						; 应用程序的ESP
		PUSH	ECX						; 应用程序的CS
		PUSH	EAX						; 应用程序的EIP
		RETF
;应用程序结束后不会回到这里

; 		STI								; 切换完成开中断
; 		PUSH	ECX						; 用于far-CALL的PUSH(cs)
; 		PUSH	EAX						; 用于far-CALL的PUSH(eip)
; 		CALL	FAR [ESP]				; 调用应用程序

; ; 应用程序结束后返回此处

; 		MOV		EAX, 1*8				; 操作系统用DS/SS	
; 		CLI								; 再次进行切换，关中断
; 		MOV		ES, AX
; 		MOV		SS, AX
; 		MOV		DS, AX
; 		MOV		FS, AX
; 		MOV		GS, AX
; 		MOV		ESP, [0xfe4]
; 		STI								; 切换完成，开中断
; 		POPAD							; 恢复之前保存的寄存器的值
; 		RET



