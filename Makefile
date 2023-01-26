.POSIX:
TARGETS=boot sys
all: $(TARGETS)
include usr/src/boot/boot.mk
include usr/src/sys/sys.mk
clean: boot-clean sys-clean
run: $(TARGETS)
	qemu-system-x86_64 -bios boot/OVMFX64.fd -hdb fat:rw:boot -serial stdio

