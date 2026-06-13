CC=clang
AS=nasm
LD=ld

CFLAGS=-ffreestanding -nostdlib -nodefaultlibs -fno-stack-protector \
	-Xclang -triple=i686-elf

all: kernel.elf mkgrub

qemu: kernel.elf mkgrub
	qemu-system-i386 -cdrom kfs1.iso # -d int

bootstrap.o: bootstrap.S
	$(AS) -felf32 -o $@ $<

interrupt.o: interrupt.S
	$(AS) -felf32 -o $@ $<

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c -o $@ $<

print.o: print.c
	$(CC) $(CFLAGS) -c -o $@ $<

kernel.elf: bootstrap.o interrupt.o kernel.o print.o
	$(LD) -T script.lds -melf_i386 -o $@ bootstrap.o interrupt.o kernel.o print.o

mkgrub:
	mkdir -p iso/boot/grub
	cp grub.cfg iso/boot/grub/grub.cfg
	cp kernel.elf iso/boot/kernel.elf
	grub2-mkrescue -o kfs1.iso ./iso

clean:
	rm -f bootstrap.o interrupt.o kernel.o print.o kfs1.iso kernel.elf
	rm -rf ./iso
