[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "hello.nas"]

        GLOBAL _HariMain

[SECTION .text]

_HariMain:
        MOV     ECX, msg
        MOV     EDX, 1
putloop:
        MOV     AL, [ECX]            ; AL指明每次读取1个字节
        CMP     AL, 0
        JE      fin
        INT     0x40                    ; 通过中断调用asm_cons_putchar
        ADD     ECX, 1
        JMP     putloop
fin:
        MOV     EDX, 4
        INT     0x40

[SECTION .data]
msg:
        DB      "nas file put str by char",0xa, 0              ; 前面字符的ascii码加上0作结尾标志