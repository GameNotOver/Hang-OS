define \n 


endef
TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
GOLIB	 = $(TOOLPATH)golib00.exe
COPY     = copy
DEL      = del

SRCPATH  = ./src/
RESPATH  = ./resource/
HEDPATH  = ./include/
APPPATH	 = ./app/
APIPATH  = ./api/

OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj \
	int.obj fifo.obj keyboard.obj mouse.obj memory.obj math.obj sheet.obj string.obj \
	data.obj timer.obj multitask.obj console.obj file.obj windows.obj

APPS = beepdown c_c c_s color color2 opwin sec walk prime type iroha chklang bball invader

COPY_APPS		= $(foreach app, $(APPS), copy from:$(APPPATH)$(app)/$(app).hrb to:@: \${\n})	

# 默认动作

default :
	$(MAKE) img

# 一般规则

%.gas : $(SRCPATH)%.c Makefile
	$(CC1) -o $*.gas $(SRCPATH)$*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

# os app
# 文件生成规则

ipl10.bin : $(SRCPATH)ipl10.nas Makefile
	$(NASK) $(SRCPATH)ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : $(SRCPATH)asmhead.nas Makefile
	$(NASK) $(SRCPATH)asmhead.nas asmhead.bin asmhead.lst

naskfunc.obj : $(SRCPATH)naskfunc.nas Makefile
	$(NASK) $(SRCPATH)naskfunc.nas naskfunc.obj naskfunc.lst

hankaku.bin : $(RESPATH)hankaku.txt Makefile
	$(MAKEFONT) $(RESPATH)hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
	$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys

haribote.img : ipl10.bin haribote.sys $(foreach app, $(APPS), $(APPPATH)$(app))  Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
wbinimg src:ipl10.bin len:512 from:0 to:0 \
copy from:haribote.sys to:@: \
copy from:$(RESPATH)gb2312.txt to:@: \
copy from:$(RESPATH)songti.fnt to:@: \
$(COPY_APPS) imgout:haribote.img

# 命令

img :
	$(MAKE) haribote.img

run :
	$(MAKE) apps
	$(MAKE) img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

apps :
	$(foreach app, $(APPS), $(MAKE) -C $(APPPATH)$(app) $(\n))

appclean :
	$(foreach app, $(APPS), $(MAKE) -C $(APPPATH)$(app) clean $(\n))

apis :
	$(MAKE)	-C $(APIPATH)

apiclean :
	$(MAKE)	-C $(APIPATH) clean

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
