SYSCC=clang
SYSCFLAGS=-I./ \
	  -target x86_64-pc-none-elf \
	  -ffreestanding \
	  -fno-stack-protector \
	  -mno-red-zone \
	  -nostdinc
SYSTGT=sys/bone.elf
sys: $(SYSTGT)
sys/bone.elf: sys/main.c
	$(SYSCC) $(SYSCFLAGS) sys/main.c -o sys/bone.elf
sys-clean:
	rm -f $(SYSTGT)

