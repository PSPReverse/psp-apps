CROSS_COMPILE=arm-none-eabi-
CFLAGS=-O2 -DIN_PSP -g -I../../include -I../../Lib/include -std=gnu99 -fomit-frame-pointer -nostartfiles -ffreestanding -Wextra -Werror -march=armv7ve -mthumb -mthumb-interwork
VPATH=../Lib/src

OBJS = main.o

all : cm-hello-world.elf cm-hello-world.raw

clean:
	rm -f $(OBJS) cm-hello-world.elf cm-hello-world.raw

%.o: %.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

_cm-start.o: ../../Lib/_cm-start.S
	$(CROSS_COMPILE)as -march=armv7ve -o $@ $^

cm-hello-world.elf : ../../build/cm-linker.ld _cm-start.o $(OBJS)
	$(CROSS_COMPILE)ld -Map=cm-hello-world.map -T $^ -o $@

cm-hello-world.raw: cm-hello-world.elf
	$(CROSS_COMPILE)objcopy -O binary $^ $@


