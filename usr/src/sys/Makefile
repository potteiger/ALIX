.POSIX:
.SUFFIXES: .o .c .S

CC=clang
CFLAGS=-I../ \
	-std=c99 \
	-target x86_64-elf \
	-static \
	-ffreestanding \
	-fno-stack-protector \
	-fno-pic \
	-mno-red-zone \
	-fshort-wchar \
	-mcmodel=kernel

LD=ld.lld
LDFLAGS=-T link.ld

SYS=alix.sys
OBJ_DEV=dev/fb.o dev/console.o dev/vt.o dev/uart.o
OBJ=main.o x64.o pmm.o $(OBJ_DEV)

all: $(SYS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.S.o:
	nasm -f elf64 $< -o $@

$(SYS): $(OBJ) link.ld
	$(LD) $(LDFLAGS) $(OBJ) -o $(SYS)

clean:
	rm -f $(SYS) $(OBJ)

