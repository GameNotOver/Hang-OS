[FORMAT "WCOFF"]                    ; 生成对象文件模式
[INSTRSET "i486p"]                    ; 生成使用486兼容指令集
[BITS 32]                           ; 生成32位模式机器语言
[FILE "lang.nas"]                 ; 源文件名信息

        GLOBAL  _api_getlang

[SECTION .text]

_api_getlang:   ; int api_getlang(void);
        MOV     EDX, 27
        INT     0x40
        RET
