B:=\
    $(DIR_SRC)/sha/sha1 \

O:=\
    $(DIR_SRC)/sha/sha1.o \

D:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

include $(DIR_MK)/bin.mk
