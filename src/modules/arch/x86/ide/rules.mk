M:=\
    $(DIR_SRC)/modules/arch/x86/ide/ide.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/ide/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
