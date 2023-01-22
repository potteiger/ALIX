BOOTCC=clang
BOOTCFLAGS=-I./ \
       -target x86_64-unknown-windows-cygnus \
       -ffreestanding \
       -fno-stack-protector \
       -fshort-wchar \
       -mno-red-zone
BOOTLD=lld-link
BOOTLDFLAGS=-subsystem:efi_application \
	-nodefaultlib \
	-dll
BOOT=boot/EFI/BOOT/BOOTX64.EFI

boot: $(BOOT)
$(BOOT): boot/bootx64.o
	mkdir -p boot/EFI/BOOT
	$(BOOTLD) $(BOOTLDFLAGS) -entry:bootx64 boot/bootx64.o -out:$(BOOT)
boot/bootx64.o: boot/bootx64.c
	$(BOOTCC) $(BOOTCFLAGS) -c boot/bootx64.c -o boot/bootx64.o
boot-clean:
	rm -f boot/bone.elf
	rm -f $(BOOT)
	rm -f boot/*.o boot/*.lib
	rm -rf boot/EFI

