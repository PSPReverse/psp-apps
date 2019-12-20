CROSS_COMPILE=arm-none-eabi-
CFLAGS=-O2 -g -I../Lib/include -std=gnu99 -fomit-frame-pointer -nostartfiles -ffreestanding -Wextra -Werror
VPATH=../Lib/src

OBJS = main.o string.o svc.o log.o tm.o x86mem.o

all : bin-loader.elf bin-loader.raw

clean:
	rm -f _start.o $(OBJS)

%.o: %.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

_start.o: ../Lib/_start.S
	$(CROSS_COMPILE)as -march=armv7-a -mcpu=cortex-a8 -o $@ $^

main.o: main.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ main.c

bin-loader.elf : ../build/linker.ld _start.o $(OBJS)
	$(CROSS_COMPILE)ld -T $^ -o $@

bin-loader.raw: bin-loader.elf
	$(CROSS_COMPILE)objcopy -O binary $^ $@


