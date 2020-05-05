CROSS_COMPILE=arm-none-eabi-
CFLAGS=-O2 -DIN_PSP -g -I../include -I../Lib/include -std=gnu99 -fomit-frame-pointer -nostartfiles -nostdlib -ffreestanding -Wextra -Werror -march=armv7-a -mthumb
VPATH=../Lib/src
LIBGCC=$(shell $(CROSS_COMPILE)gcc -print-libgcc-file-name)
LDFLAGS=$(LIBGCC)


OBJS = main.o thumb-interwork.o utils.o string.o log.o tm.o uart.o pdu-transp-uart.o pdu-transp-spi-flash.o pdu-transp-spi-em100.o

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
	$(CROSS_COMPILE)ld -Map=psp-serial-stub.map -T $^ -o $@ $(LDFLAGS)

psp-serial-stub.raw: psp-serial-stub.elf
	$(CROSS_COMPILE)objcopy -O binary $^ $@


