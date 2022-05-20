[FORMAT "WCOFF"]                    ; 生成对象文件模式
[INSTRSET "i486p"]                    ; 生成使用486兼容指令集
[BITS 32]                           ; 生成32位模式机器语言
[FILE "string.nas"]                 ; 源文件名信息

        GLOBAL  _api_putchar, _api_putstr

[SECTION .text]

_api_putchar:   ; void api_putchar(char c);
        MOV     EDX, 1
        MOV     AL, [ESP+4]             ;参数c
        INT     0x40
        RET

_api_putstr:    ; void api_putstr(char *str)
        PUSH    EBX
        MOV     EDX, 2
        MOV     EBX, [ESP+8]            ; 参数str
        INT     0x40
        POP     EBX
        RET
