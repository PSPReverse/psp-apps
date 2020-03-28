CROSS_COMPILE=arm-none-eabi-
CFLAGS=-O0 -DIN_PSP -g -I../include -I../Lib/include -std=gnu99 -fomit-frame-pointer -nostartfiles -ffreestanding -Wextra -Werror -march=armv7ve
VPATH=../Lib/src

OBJS = main.o string.o log.o tm.o uart.o

all : psp-serial-stub.elf psp-serial-stub.raw

clean:
	rm -f _svc-start.o $(OBJS)

%.o: %.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

_svc-start.o: ../Lib/_svc-start.S
	$(CROSS_COMPILE)as -march=armv7ve -o $@ $^

main.o: main.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ main.c

psp-serial-stub.elf : ../build/svc-linker.ld _svc-start.o $(OBJS)
	$(CROSS_COMPILE)ld -T $^ -o $@

psp-serial-stub.raw: psp-serial-stub.elf
	$(CROSS_COMPILE)objcopy -O binary $^ $@

