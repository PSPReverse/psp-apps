CROSS_COMPILE=arm-none-eabi-
CFLAGS=-Os -I../Lib/include -std=gnu99 -fomit-frame-pointer -nostartfiles -ffreestanding -Wextra -Werror
VPATH=../Lib/src

OBJS = main.o string.o svc.o

all : inject-svc-6.elf inject-svc-6.raw

clean:
	rm -f _start.o $(OBJS) inject-svc-6.elf inject-svc-6.raw psp-svc-inject.h psp-svc-inject.o psp-svc-inject.bin psp-svc-inject.elf

%.o: %.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

psp-svc-inject.h: psp-svc-inject.bin
	./bin2c $^ $@ psp_svc_inject

psp-svc-inject.bin: psp-svc-inject.elf
	$(CROSS_COMPILE)objcopy -O binary $^ $@

psp-svc-inject.elf : psp-svc-inject.ld psp-svc-inject_head.o psp-svc-inject.o
	$(CROSS_COMPILE)ld -T $^ -o $@

psp-svc-inject.o: psp-svc-inject.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -march=armv7-a -mthumb -fno-toplevel-reorder -o $@ $^

psp-svc-inject_head.o: psp-svc-inject.s
	$(CROSS_COMPILE)as -march=armv7-a -mcpu=cortex-a8 -o $@ $^

_start.o: ../Lib/_start.S
	$(CROSS_COMPILE)as -march=armv7-a -mcpu=cortex-a8 -o $@ $^

main.o: main.c psp-svc-inject.h
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ main.c

inject-svc-6.elf : ../build/linker.ld _start.o $(OBJS)
	$(CROSS_COMPILE)ld -T $^ -o $@

inject-svc-6.raw: inject-svc-6.elf
	$(CROSS_COMPILE)objcopy -O binary $^ $@


