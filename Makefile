#--------------------------------------Makefile-------------------------------------

CFILES = $(wildcard ./kernel/*.c)
OFILES = $(CFILES:./kernel/%.c=./build/%.o)

CLIBFILE = $(wildcard ./lib/*.c)
OLIBFILE = $(CLIBFILE:./lib/%.c=./build/%.o)

CUARTFILE = $(wildcard ./uart/*.c)
OUARTFILE = $(CUARTFILE:./uart/%.c=./build/%.o)

CDMAFILE = $(wildcard ./dma/*.c)
ODMAFILE = $(CDMAFILE:./dma/%.c=./build/%.o)

GCCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib

ifeq ($(pi),4)
	GCCFLAGS += -DRPI4
else
	GCCFLAGS += -DRPI3
endif

all: clean kernel8.img run0
# uart1: clean uart1_build kernel8.img run1
# uart0: clean uart0_build kernel8.img run0
compile: clean kernel8.img


#./build/uart.o: ./uart/uart1.c
#	aarch64-none-elf-gcc $(GCCFLAGS) -c ./uart/uart1.c -o ./build/uart.o

# uart1_build: ./uart/uart1.c
# 	aarch64-none-elf-gcc $(GCCFLAGS) -c ./uart/uart1.c -o ./build/uart.o

# uart0_build: ./uart/uart0.c
#	aarch64-none-elf-gcc $(GCCFLAGS) -c ./uart/uart0.c -o ./build/uart.o

./build/boot.o: ./kernel/boot.S
	aarch64-none-elf-gcc $(GCCFLAGS) -c $^ -o $@

./build/%.o: ./kernel/%.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@
./build/%.o: ./lib/%.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@
./build/%.o: ./uart/%.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@
./build/%.o: ./dma/%.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

kernel8.img: ./build/boot.o $(OUARTFILE) $(OFILES) $(OLIBFILE) $(ODMAFILE)
	aarch64-none-elf-ld -nostdlib $^ -T ./kernel/link.ld -o ./build/kernel8.elf
	aarch64-none-elf-objcopy -O binary ./build/kernel8.elf kernel8.img

clean:
ifeq ($(OS),Windows_NT)
	del .\build\kernel8.elf .\build\*.o *.img
else
	rm -f ./build/kernel8.elf ./build/*.o *.img
endif

# Run emulation with QEMU
run1: 
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio

run0: 
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
