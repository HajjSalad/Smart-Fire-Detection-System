
TARGET = stm32_sensor_node
MCU = cortex-m4
CPU = -mcpu=$(MCU) -mthumb -mfloat-abi=soft -mfpu=fpv4-sp-d16

CFLAGS = $(CPU) -Wall -g -O2 -ffunction-sections -fdata-sections \
         -I/opt/homebrew/arm-none-eabi/arm-none-eabi/include \
         -I/Users/abdirahmanhajj/STM32_Workspace/STM32Cube_FW_F4/Drivers/CMSIS/Include \
         -I/Users/abdirahmanhajj/STM32_Workspace/STM32Cube_FW_F4/Drivers/CMSIS/Device/ST/STM32F4xx/Include

CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions  # No runtime type info (RTTI) or exceptions for embedded

LDFLAGS = -T STM32F446RETX_FLASH.ld --specs=nosys.specs -Wl,--gc-sections -lstdc++

# Directories 
INCDIR = Inc \
         /Users/abdirahmanhajj/STM32_Workspace/STM32Cube_FW_F4/Drivers/CMSIS/Include \
         /Users/abdirahmanhajj/STM32_Workspace/STM32Cube_FW_F4/Drivers/CMSIS/Device/ST/STM32F4xx
SRCDIR = Src
OBJDIR = Build

# Toolchain
CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size
GDB = arm-none-eabi-gdb
OPENOCD = openocd

# Source files
CSRCS = $(wildcard $(SRCDIR)/*.c)
CPPSRCS = $(wildcard $(SRCDIR)/*.cpp)
ASRCS = startup/startup_stm32f446retx.s

OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(CSRCS)) \
       $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(CPPSRCS)) \
       $(patsubst startup/%.s, $(OBJDIR)/%.o, $(ASRCS))

# Rules
all: $(TARGET).elf $(TARGET).bin

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Compile C files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(foreach d, $(INCDIR), -I$d) -c $< -o $@

# Compile C++ files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(foreach d, $(INCDIR), -I$d) -c $< -o $@

# Assemble startup file
$(OBJDIR)/%.o: startup/%.s | $(OBJDIR)
	$(AS) $< -o $@

# Link everything with the C++ compiler
$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
	$(SIZE) $@

# Convert ELF to binary
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	$(OPENOCD) -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(TARGET).bin verify reset exit"

debug: $(TARGET).elf
	$(OPENOCD) -f interface/stlink.cfg -f target/stm32f4x.cfg -c "init; reset halt"

clean:
	rm -rf $(OBJDIR) $(TARGET).elf $(TARGET).bin
