SYS-SRC=usr/src/sys
SYS-CC=clang
SYS-CFLAGS=-Iusr/src/include \
	  -target x86_64-elf \
	  -static \
	  -ffreestanding \
	  -fno-stack-protector \
	  -fno-pic \
	  -mno-red-zone \
	  -nostdinc \
	  -mabi=sysv \
	  -mcmodel=kernel

SYS-TGT=boot/bonex64.sys

sys: $(SYS-TGT)

$(SYS-TGT): $(SYS-SRC)/main.c
	$(SYS-CC) $(SYS-CFLAGS) $(SYS-SRC)/main.c -o $(SYS-TGT)

sys-clean:
	rm -f $(SYS-TGT)

