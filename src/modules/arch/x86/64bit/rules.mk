M:=\
    $(DIR_SRC)/modules/arch/x86/64bit/64bit.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/64bit/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
