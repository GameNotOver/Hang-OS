[FORMAT "WCOFF"]                    ; 生成对象文件模式
[INSTRSET "i486p"]                    ; 生成使用486兼容指令集
[BITS 32]                           ; 生成32位模式机器语言
[FILE "file.nas"]                 ; 源文件名信息

        GLOBAL  _api_cmdline

[SECTION .text]

_api_cmdline:   ; int api_cmdline(char *buf, int maxsize);
        PUSH    EBX
        MOV     EDX, 26
        MOV     ECX, [ESP+12]       ; maxsize
        MOV     EBX, [ESP+8]        ; buf
        INT     0x40
        POP     EBX
        RET

