TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
GOLIB	 = $(TOOLPATH)golib00.exe
DEL      = del

SRCPATH	 = ./src/

LINKOBJS = alloca cmdline file graph memory os string timer window lang

NEED 	= $(foreach obj, $(LINKOBJS), $(obj).obj )	

default :
	$(MAKE) apifunc.lib

%.obj: $(SRCPATH)%.nas Makefile
	$(NASK) $(SRCPATH)$*.nas $*.obj $*.lst

apifunc.lib: $(NEED) Makefile
	$(GOLIB) $(NEED) out:apifunc.lib

clean :
	-$(DEL) *.lst
	-$(DEL) *.obj