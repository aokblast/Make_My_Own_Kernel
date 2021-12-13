all: system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system kernel.bin

system: head.o main.o printk.o
	ld -b elf64-x86-64 -z muldefs -o  system head.o main.o printk.o -T kernel.lds 

head.o: head.S
	gcc -E head.S > head.s
	as --64 -o head.o head.s

main.o: main.c
	gcc -static -mcmodel=large -fno-builtin -m64 -c main.c -fno-stack-protector

printk.o: printk.c lib.h
	gcc -static -mcmodel=large -fno-builtin -m64 -c printk.c -fno-stack-protector

clean:
	rm -rf *.o *.s~ *.s *.S~ *.c~ *.h~ system Makefile~ Kernel.lds~ kernel.bin