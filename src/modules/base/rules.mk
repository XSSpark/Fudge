M:=\
    $(DIR_SRC)/modules/base/base.ko \

N:=\
    $(DIR_SRC)/modules/base/base.ko.map \

O:=\
    $(DIR_SRC)/modules/base/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
