CROSS_COMPILE=arm-none-eabi-
CFLAGS=-Os -DIN_PSP -g -I../include -I../Lib/include -std=gnu99 -fomit-frame-pointer -nostartfiles -nostdlib -ffreestanding -Wextra -Werror -march=armv7-a -marm
VPATH=../Lib/src
LIBGCC=$(shell $(CROSS_COMPILE)gcc -print-libgcc-file-name)
LDFLAGS=$(LIBGCC)

ifeq ($(TARGET), PRIME-X370)
CFLAGS += -DTARGET=1
else ifeq ($(TARGET), H11DSU-IN)
CFLAGS += -DTARGET=2
endif

OBJS = main.o misc.o string.o uart.o x86-map.o smn-map.o platform.o

all : hello_world.elf hello_world.raw

clean:
	rm -f _svc-start.o $(OBJS) hello_world.raw hello_world.map hello_world.elf

%.o: %.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

_svc-start.o: ../Lib/_svc-start.S
	$(CROSS_COMPILE)as -march=armv7ve -o $@ $^

main.o: main.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ main.c

hello_world.elf : ../build/svc-linker.ld _svc-start.o $(OBJS)
	$(CROSS_COMPILE)ld -Map=hello_world.map -T $^ -o $@ $(LDFLAGS)

hello_world.raw: hello_world.elf
	$(CROSS_COMPILE)objcopy -O binary $^ $@

