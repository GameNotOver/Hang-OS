[FORMAT "WCOFF"]
[BITS 32]
[FILE "hlt.nas"]
        GLOBAL _HariMain
[SECTION .text]
        ; CLI 
_HariMain: 
fin:
        HLT
        JMP     fin