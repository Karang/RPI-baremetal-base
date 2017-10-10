CC_BIN=../gcc-arm-none-eabi-5_4-2016q3/bin
CC=$(CC_BIN)/arm-none-eabi-gcc
OBJ_CPY=$(CC_BIN)/arm-none-eabi-objcopy

CPU=-mtune=cortex-a7 -march=armv7-a -mfloat-abi=hard -mfpu=neon-vfpv4
CFLAGS=-O4 -Wall -Wextra -Wno-unused-parameter -Wno-format
INCLUDES=-Iincludes/
LIBS=

C_FILES = $(wildcard src/*.c)
OBJ_FILES = $(addprefix build/,$(notdir $(C_FILES:.c=.o)))

all: kernel7.img

clean:
	rm -f kernel7.img build/*

build/boot.o: src_asm/boot.S
	$(CC) $(CPU) -fpic -ffreestanding -c src_asm/boot.S -o build/boot.o

build/%.o: src/%.c
	$(CC) $(CPU) $(CFLAGS) $(INCLUDES) -fpic -std=gnu99 -o $@ -c $<

build/kernel.elf: build/boot.o $(OBJ_FILES)
	$(CC) $(CPU) -Wl,-T,linker.ld -o build/kernel.elf -O4 -nostartfiles build/boot.o $(OBJ_FILES) $(LIBS)

kernel7.img: build/kernel.elf
	$(OBJ_CPY) build/kernel.elf -O binary kernel7.img

