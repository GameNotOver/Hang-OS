[BITS 32]
        MOV     AL, 'h'
        INT     0x40                    ; 通过中断调用asm_cons_putchar
        MOV     AL, 'e'
        INT     0x40 
        MOV     AL, 'l'
        INT     0x40 
        MOV     AL, 'l'
        INT     0x40
        MOV     AL, 'o'
        INT     0x40
        MOV     AL, ' '
        INT     0x40
        MOV     AL, 'w'
        INT     0x40
        MOV     AL, 'u'
        INT     0x40
        RETF
; fin:
;         HLT
;         JMP     fin