M:=\
    $(DIR_SRC)/modules/system/system.ko \

O:=\
    $(DIR_SRC)/modules/system/main.o \
    $(DIR_SRC)/modules/system/backend.o \
    $(DIR_SRC)/modules/system/protocol.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
