FILES = ./build/shell.o 
INCLUDES = -I../stdlib/src
FLAGS = -g -ffreestanding -fno-builtin

all: ${FILES}
	i686-elf-gcc -g -T ./linker.ld -o ./shell.elf -ffreestanding -O0 -nostdlib -fpic -g ${FILES} ../stdlib/stdlib.elf

./build/shell.o : ./src/shell.c
	i686-elf-gcc ${INCLUDES} -I./ $(FLAGS) -std=gnu99 -c ./src/shell.c -o ./build/shell.o

clean:
	rm -rf ${FILES}
	rm -rf ./shell.elf
