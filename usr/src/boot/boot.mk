BOOT-SRC=usr/src/boot
BOOT-CC=clang
BOOT-CFLAGS=-Iusr/src/include \
       -target x86_64-unknown-windows-cygnus \
       -ffreestanding \
       -fno-stack-protector \
       -fshort-wchar \
       -mno-red-zone
BOOT-LD=lld-link
BOOT-LDFLAGS=-subsystem:efi_application \
	-nodefaultlib
BOOT=boot/EFI/BOOT/BOOTX64.EFI

boot: $(BOOT)

$(BOOT): $(BOOT-SRC)/x64/boot.o $(BOOT-SRC)/x64/boots.o
	$(BOOT-LD) $(BOOT-LDFLAGS) -entry:boots $(BOOT-SRC)/x64/boot.o \
		$(BOOT-SRC)/x64/boots.o -out:$(BOOT)

$(BOOT-SRC)/x64/boot.o: $(BOOT-SRC)/x64/boot.c
	$(BOOT-CC) $(BOOT-CFLAGS) -c $(BOOT-SRC)/x64/boot.c -o \
		$(BOOT-SRC)/x64/boot.o

$(BOOT-SRC)/x64/boots.o: $(BOOT-SRC)/x64/boot.s
	nasm -f win64 $(BOOT-SRC)/x64/boot.s -o $(BOOT-SRC)/x64/boots.o

boot-clean:
	rm -f boot/EFI/BOOT/*
	rm -f $(BOOT-SRC)/x64/*.o $(BOOT-SRC)/x64/*.lib

