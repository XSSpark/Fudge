ARCH:=x86
LOADER:=mboot
PREFIX:=

AS:=$(PREFIX)$(AS)
CC:=$(PREFIX)$(CC)
LD:=$(PREFIX)$(LD)

ASFLAGS_x86:=-32
CFLAGS_x86:=-m32
LDFLAGS_x86:=-melf_i386

ASFLAGS_arm:=-mfpu=softfpa
CFLAGS_arm:=
LDFLAGS_ARM:=

ASFLAGS:=$(ASFLAGS_$(ARCH))
CFLAGS:=-Wall -Werror -ffreestanding -nostdlib -std=c89 -pedantic -O2 $(CFLAGS_$(ARCH))
LDFLAGS:=$(LDFLAGS_$(ARCH))
ARFLAGS:=rs

KERNEL_NAME:=fudge
KERNEL:=$(KERNEL_NAME)

RAMDISK_NAME:=initrd
RAMDISK_TYPE:=tar
RAMDISK:=$(RAMDISK_NAME).$(RAMDISK_TYPE)

INSTALL_PATH:=/boot

LIBS:=
LIBS_OBJECTS:=
MODULES:=
MODULES_OBJECTS:=
PACKAGES:=
PACKAGES_OBJECTS:=

.PHONY: all clean kernel libs modules packages

all: libs modules packages kernel ramdisk

.s.o:
	$(AS) $(ASFLAGS) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

include libs/rules.mk
include modules/rules.mk
include packages/rules.mk

clean:
	rm -rf $(LIBS) $(LIBS_OBJECTS)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(PACKAGES) $(PACKAGES_OBJECTS)
	rm -rf $(KERNEL)
	rm -rf $(RAMDISK)

image/bin: $(PACKAGES)
	mkdir -p $@
	cp $(PACKAGES) $@

image/boot:
	mkdir -p $@

image/boot/fudge: image/boot $(KERNEL)
	cp $(KERNEL) $@

image/boot/mod: image/boot $(MODULES)
	mkdir -p $@
	cp $(MODULES) $@

image/config: system/config
	cp -r $< $@

image/home: system/home
	cp -r $< $@

image/share: system/share
	cp -r $< $@

image/system:
	mkdir -p $@

image/temp:
	mkdir -p $@

install:
	install -m 644 $(KERNEL) $(INSTALL_PATH)
	install -m 644 $(RAMDISK) $(INSTALL_PATH)

kernel: $(LIBKERNEL) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -Tlibs/$(ARCH)/$(LOADER)/linker.ld -o $(KERNEL) $^

libs: $(LIBS)

modules: $(MODULES)

packages: $(PACKAGES)

ramdisk: $(RAMDISK_NAME).$(RAMDISK_TYPE)

$(RAMDISK_NAME).tar: image/bin image/boot image/boot/fudge image/boot/mod image/config image/home image/share image/system image/temp
	tar -cf $(RAMDISK) image
	rm -rf image

$(RAMDISK_NAME).cpio: image/bin image/boot image/boot/fudge image/boot/mod image/config image/home image/share image/system image/temp
	find image -depth | cpio -o > $(RAMDISK)
	rm -rf image
