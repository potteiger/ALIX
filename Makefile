.POSIX:

TARGETS=boot/alix.sys boot/EFI/BOOT/BOOTX64.EFI

all: $(TARGETS)

boot/EFI/BOOT/BOOTX64.EFI:
	cd usr/src/boot && $(MAKE)
	mv usr/src/boot/BOOTX64.EFI boot/EFI/BOOT/BOOTX64.EFI

boot/alix.sys:
	cd usr/src/sys && $(MAKE)
	mv usr/src/sys/alix.sys boot/alix.sys

clean:
	cd usr/src/boot && $(MAKE) clean
	cd usr/src/sys && $(MAKE) clean
	rm -f $(TARGETS)

qemu-int: $(TARGETS)
	qemu-system-x86_64 -no-reboot -bios boot/OVMFX64.fd \
	-hdb fat:rw:boot -no-shutdown -d int
qemu-monitor: $(TARGETS)
	qemu-system-x86_64 -no-reboot -bios boot/OVMFX64.fd \
	-hdb fat:rw:boot -no-shutdown -monitor stdio
qemu-serial: $(TARGETS)
	qemu-system-x86_64 -no-reboot -bios boot/OVMFX64.fd \
	-hdb fat:rw:boot -no-shutdown -serial stdio

