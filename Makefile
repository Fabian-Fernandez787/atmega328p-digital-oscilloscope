# Hardware Configuration
MCU          = atmega328p
F_CPU        = 16000000UL
BAUD         = 115200
PORT         = COM5

# Tools
CC           = avr-gcc
OBJCOPY      = avr-objcopy
AVRDUDE      = avrdude

# Compiler Flags
CFLAGS       = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Iinclude

# Source files
SRC          = src/main.c
TARGET       = build/oscilloscope

all: $(TARGET).hex

# Step 1: Compile C into ELF binary
$(TARGET).elf: $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET).elf

# Step 2: Convert ELF to HEX machine code
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex

# Step 3: Flash HEX to ATmega328P via Arduino Bootloader
flash: $(TARGET).hex
	$(AVRDUDE) -c arduino -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$(TARGET).hex:i

# Clean up build artifacts
clean:
	rm -rf build