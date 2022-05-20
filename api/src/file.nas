[FORMAT "WCOFF"]                    ; 生成对象文件模式
[INSTRSET "i486p"]                    ; 生成使用486兼容指令集
[BITS 32]                           ; 生成32位模式机器语言
[FILE "file.nas"]                 ; 源文件名信息

        GLOBAL  _api_fopen,  _api_fclose, _api_fseek, _api_fsize, _api_fread

[SECTION .text]

_api_fopen:     ; int api_fopen(char *fname);
        PUSH    EBX
        MOV     EDX, 21
        MOV     EBX, [ESP+8]        ; fname
        INT     0x40
        POP     EBX
        RET

_api_fclose:     ; void api_fclose(int fhandle);
        MOV     EDX, 22
        MOV     EAX, [ESP+4]        ; fhandle
        INT     0x40
        RET

_api_fseek:     ; int api_fseek(int fhandle, int offset, int mode);
        PUSH    EBX
        MOV     EDX, 23
        MOV     EAX, [ESP+8]        ; fhandle
        MOV     ECX, [ESP+16]       ; mode
        MOV     EBX, [ESP+12]       ; offset
        INT     0x40
        POP     EBX
        RET

_api_fsize:     ; int api_fsize(int fhandle, int mode);
        MOV     EDX, 24
        MOV     EAX, [ESP+4]        ; fhandle
        MOV     ECX, [ESP+8]        ; mode
        INT     0x40
        RET

_api_fread:     ; int api_fread(char *buf, int maxsize, int fhandle);
        PUSH    EBX
        MOV     EDX, 25
        MOV     EAX, [ESP+16]       ; fhandle
        MOV     ECX, [ESP+12]       ; maxsize
        MOV     EBX, [ESP+8]        ; buf
        INT     0x40
        POP     EBX
        RET
