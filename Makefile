OBJS=main.o isr.o boot.o dtable.o flush.o page.o vga.o string.o common.o mtwist.o mm.o interrupt.o timer.o mboot.o sched.o process.o syscall.o systable.o

CC=gcc
LD=ld
CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -m32 -fomit-frame-pointer -Wall -Wextra -g
ASFLAGS=$(CFLAGS)
LDFLAGS=-Tlink.ld -melf_i386

KFILE=kern.bin

all: $(KFILE)

$(KFILE): $(OBJS) link.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $(KFILE)

clean:
	rm -f *.o $(KFILE)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

%.o: %.S
	$(CC) $(CFLAGS) -D__ASSEMBLY__ -c $<

