.POSIX:
TARGETS=boot sys
all: $(TARGETS)
include usr/src/boot/boot.mk
include usr/src/sys/sys.mk
clean: boot-clean sys-clean
run: $(TARGETS)
	qemu-system-x86_64 -no-reboot -bios boot/OVMFX64.fd \
	-hdb fat:rw:boot -no-shutdown -monitor stdio #-d int

