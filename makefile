# This makefile should go in the root of a repository (except lib-common)

# Reference: http://www.atmel.com/webdoc/avrlibcreferencemanual/group__demo__project_1demo_project_compile.html


# Parameters that might need to be changed, depending on the repository
#-------------------------------------------------------------------------------
# Libraries from lib-common to link
LIB = -L./lib-common/lib -ladc -lcan -lconversions -ldac -lpex -lqueue -lspi -ltimer -luart -lutilities
# Program name
PROG = eps
#-------------------------------------------------------------------------------


# AVR-GCC compiler
CC = avr-gcc
# Compiler flags
CFLAGS = -Wall -std=gnu99 -g -mmcu=atmega32m1 -Os -mcall-prologues
# Includes (header files)
INCLUDES = -I./lib-common/include/
# Programmer
PGMR = stk500
# Microcontroller
MCU = m32m1
# Build directory
BUILD = build
# Manual tests directory
MANUAL_TESTS = $(dir $(wildcard manual_tests/*/.))


# Detect operating system - based on https://gist.github.com/sighingnow/deee806603ec9274fd47

# One of these flags will be set to true based on the operating system
WINDOWS := false
MAC_OS := false
LINUX := false

ifeq ($(OS),Windows_NT)
	WINDOWS := true
else
	# Unix - get the operating system
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		MAC_OS := true
	endif
	ifeq ($(UNAME_S),Linux)
		LINUX := true
	endif
endif

# PORT - Computer port that the programmer is connected to
# Try to automatically detect the port
ifeq ($(WINDOWS), true)
	# higher number
	PORT = $(shell powershell "[System.IO.Ports.SerialPort]::getportnames() | sort | select -First 2 | select -Last 1")
endif
ifeq ($(MAC_OS), true)
	# lower number
	PORT = $(shell find /dev -name 'tty.usbmodem[0-9]*' | sort | head -n1)
endif
ifeq ($(LINUX), true)
	# lower number
	# TODO - test this
	PORT = $(shell find /dev -name 'ttyS[0-9]*' | sort | head -n1)
endif

# If automatic port detection fails,
# uncomment one of these lines and change it to set the port manually
# PORT = COM3						# Windows
# PORT = /dev/tty.usbmodem00208212	# macOS
# PORT = /dev/ttyS3					# Linux


# Special commands
.PHONY: all clean debug help lib-common manual_tests read-eeprom upload

# Get all .c files in src folder
SRC = $(wildcard ./src/*.c)
# All .c files in src get compiled to to .o files in build
OBJ = $(SRC:./src/%.c=./build/%.o)
DEP = $(OBJ:.o=.d)

all: $(PROG)

# Make main program
$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o ./build/$@.elf $(OBJ) $(LIB)
	avr-objcopy -j .text -j .data -O ihex ./build/$@.elf ./build/$@.hex

# .o files depend on .c files
./build/%.o: ./src/%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

-include $(DEP)

./build/%.d: ./src/%.c | $(BUILD)
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

# Create the build directory if it doesn't exist
$(BUILD):
	mkdir $(BUILD)

# Remove all files in the build directory
clean:
	rm -f $(BUILD)/*

# Print debug information
debug:
	@echo ------------
	@echo $(SRC)
	@echo ------------
	@echo $(OBJ)
	@echo ------------

# Help shows available commands
help:
	@echo "usage: make [all | clean | debug | help | lib-common | manual_tests | read-eeprom | upload]"
	@echo "Running make without any arguments is equivalent to running make all."
	@echo "all            build the main program (src directory)"
	@echo "clean          clear the build directory and all subdirectories"
	@echo "debug          display debugging information"
	@echo "help           display this help message"
	@echo "lib-common     fetch and build the latest version of lib-common"
	@echo "manual_tests   build all manual test programs (manual_tests directory)"
	@echo "read-eeprom    read and display the contents of the microcontroller's EEPROM"
	@echo "upload         upload the main program to a board"

lib-common:
	@echo "Fetching latest version of lib-common..."
	git submodule update --remote
	@echo "Building lib-common..."
	make -C lib-common clean
	make -C lib-common

manual_tests:
	@for dir in $(MANUAL_TESTS) ; do \
		cd $$dir ; \
		make clean ; \
		make ; \
		cd ../.. ; \
	done

# Create a file called eeprom.bin, which contains a raw binary copy of the micro's EEPROM memory.
# View the contents of the binary file in hex
read-eeprom:
	@echo "Reading EEPROM to binary file eeprom.bin..."
	avrdude -p m32m1 -c stk500 -P $(PORT) -U eeprom:r:eeprom.bin:r
	@echo "Displaying eeprom.bin in hex..."
ifeq ($(WINDOWS), true)
	powershell Format-Hex eeprom.bin
else
	hexdump eeprom.bin
endif

# Upload program to board
upload: $(PROG)
	avrdude -c $(PGMR) -p $(MCU) -P $(PORT) -U flash:w:./build/$^.hex
