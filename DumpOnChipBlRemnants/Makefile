CROSS_COMPILE=arm-none-eabi-
CFLAGS=-Os -I../include -I../Lib/include -std=gnu99 -fomit-frame-pointer -nostartfiles -ffreestanding -Wextra -Werror
VPATH=../Lib/src

all : dump-on-chip-bl-remnants.elf dump-on-chip-bl-remnants.raw

clean:
	rm -f _start.o dump-on-chip-bl-remnants.elf dump-on-chip-bl-remnants.raw

%.o: %.S
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $^

dump-on-chip-bl-remnants.elf : ../build/linker.ld _start.o
	$(CROSS_COMPILE)ld -T $^ -o $@

dump-on-chip-bl-remnants.raw: dump-on-chip-bl-remnants.elf
	$(CROSS_COMPILE)objcopy -O binary $^ $@


