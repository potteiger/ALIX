.POSIX:
.SUFFIXES: .o .c .S

CC=clang
CFLAGS=-I../ \
	-std=c99 \
	-target x86_64-unknown-windows-cygnus \
	-ffreestanding \
	-fno-stack-protector \
	-fshort-wchar \
	-mno-red-zone
LD=lld-link
LDFLAGS=-subsystem:efi_application \
	-nodefaultlib

BOOT=BOOTX64.EFI
OBJ=boot.o load.o er.o

all: $(BOOT)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.S.o:
	nasm -f win64 $< -o $@

$(BOOT): $(OBJ)
	$(LD) $(LDFLAGS) -entry:boot $(OBJ) -out:$(BOOT)

clean:
	rm -f $(BOOT) $(OBJ)

