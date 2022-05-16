[INSTRSET "i486p"]
[BITS 32]
        MOV     ECX, msg
        MOV     EDX, 1
putloop:
        MOV     AL, [CS:ECX]            ; AL指明每次读取1个字节
        CMP     AL, 0
        JE      fin
        INT     0x40                    ; 通过中断调用asm_cons_putchar
        ADD     ECX, 1
        JMP     putloop
fin:
        MOV     EDX, 4
        INT     0x40
msg:
        DB      "hello,yuhangwu", 0              ; 前面字符的ascii码加上0作结尾标志