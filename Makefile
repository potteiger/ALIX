.POSIX:
TARGETS=boot sys
all: $(TARGETS)
include boot/boot.mk
include sys/sys.mk
clean: boot-clean sys-clean
run: $(TARGETS)
	mv sys/bone.elf boot/bone.elf
	qemu-system-x86_64 -bios boot/OVMFX64.fd -hdb fat:rw:boot -serial stdio

