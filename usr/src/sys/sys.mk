SYS-SRC=usr/src/sys
SYS-CC=clang
SYS-CFLAGS=-Iusr/src \
	  -std=c99 \
	  -target x86_64-elf \
	  -static \
	  -ffreestanding \
	  -fno-stack-protector \
	  -fno-pic \
	  -mno-red-zone \
	  -fshort-wchar \
	  -mcmodel=kernel
SYS-LD=ld.lld

SYS-TGT=boot/alix.sys
SYS-OBJ=$(SYS-SRC)/main.o $(SYS-SRC)/fb.o

sys: $(SYS-TGT)

$(SYS-TGT): $(SYS-OBJ) $(SYS-SRC)/link.ld
	$(SYS-LD) -T $(SYS-SRC)/link.ld $(SYS-OBJ) -o $(SYS-TGT)

$(SYS-SRC)/main.o: $(SYS-SRC)/main.c
	$(SYS-CC) $(SYS-CFLAGS) -c $(SYS-SRC)/main.c -o $(SYS-SRC)/main.o

$(SYS-SRC)/fb.o: $(SYS-SRC)/fb.c
	$(SYS-CC) $(SYS-CFLAGS) -c $(SYS-SRC)/fb.c -o $(SYS-SRC)/fb.o

sys-clean:
	rm -f $(SYS-TGT)

