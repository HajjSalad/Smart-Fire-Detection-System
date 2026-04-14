# ========================================================
# Makefile for STM32 Sensor Node
# ========================================================

# Target binary name
TARGET = STM32_Sensor_Node

# Build directory
BUILD_DIR = Build

# Source files directory
C_SOURCES = $(wildcard Src/*.c) \
			$(wildcard Src/comm/*.c) \
			$(wildcard Src/tasks/*.c) \
            $(wildcard Src/utils/*.c) \
			$(wildcard Src/sensors/*.c) \
            $(wildcard FreeRTOS/Source/*.c) \
            $(wildcard FreeRTOS/Source/portable/GCC/ARM_CM4F/*.c) \
            FreeRTOS/Source/portable/MemMang/heap_4.c

# C++ source files
CXX_SOURCES = $(wildcard Src/*.cpp) \
              $(wildcard Src/core/*.cpp) \
			  $(wildcard Src/utils/*.cpp)

# ASM sources
ASM_SOURCES = Startup/startup_stm32f446retx.s

# Options for float-abi:
# -mfloat-abi=soft     : No FPU, software emulation (slower, smaller code, more compatible)
# -mfloat-abi=softfp   : FPU used but software calling convention (compatible with soft)
# -mfloat-abi=hard     : FPU used with hardware calling convention (faster, larger code, less compatible)
# Third option used

# MCU definition
CPU = -mcpu=cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT_ABI = -mfloat-abi=hard
MCU = $(CPU) -mthumb $(FPU) $(FLOAT_ABI)

# C includes
C_INCLUDES = -IInc \
             -IInc/comm \
             -IInc/core \
             -IInc/tasks \
			 -IInc/utils \
		     -IInc/sensors \
             -IInc/CMSIS/Core/Include \
             -IInc/STM32F4xx/Include \
             -IFreeRTOS/Source/include \
             -IFreeRTOS/Source/portable/GCC/ARM_CM4F \
             -IFreeRTOS

# C defines
C_DEFS = -DSTM32F446xx \
         -DUSE_FULL_ASSERT \

# C flags
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) -O2 -g3 -Wall -fdata-sections -ffunction-sections

CXXFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) -O2 -g3 -Wall -fdata-sections -ffunction-sections \
           -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics

# Linker script
LDSCRIPT = STM32F446RETX_FLASH.ld

# Libraries
LIBS = -lc -lm -lnosys -lstdc++
LIBDIR =

# Linker flags
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -specs=nano.specs \
          -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections \
          -Wl,--no-warn-mismatch -u _printf_float

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

# Object files (all go into Build/)
OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(CXX_SOURCES:.cpp=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(ASM_SOURCES:.s=.o)))

all: $(BUILD_DIR)/$(TARGET).elf
	@echo "Build complete."
	$(SZ) $(BUILD_DIR)/$(TARGET).elf

# Create Build directory
$(BUILD_DIR):
	mkdir -p $@

# Compile C sources
$(BUILD_DIR)/%.o: Src/%.c | $(BUILD_DIR)
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ sources - Src/
$(BUILD_DIR)/%.o: Src/%.cpp | $(BUILD_DIR)
	@echo "Compiling (C++) $< ..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C sources - Src/tasks/
$(BUILD_DIR)/%.o: Src/tasks/%.c | $(BUILD_DIR)
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C sources - Src/comm/
$(BUILD_DIR)/%.o: Src/comm/%.c | $(BUILD_DIR)
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C sources - Src/sensors/
$(BUILD_DIR)/%.o: Src/sensors/%.c | $(BUILD_DIR)
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C sources - Src/utils/
$(BUILD_DIR)/%.o: Src/utils/%.c | $(BUILD_DIR)
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ sources - Src/utils/
$(BUILD_DIR)/%.o: Src/utils/%.cpp | $(BUILD_DIR)
	@echo "Compiling (C++) $< ..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile FreeRTOS sources
$(BUILD_DIR)/%.o: FreeRTOS/Source/%.c | $(BUILD_DIR)
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile FreeRTOS port
$(BUILD_DIR)/%.o: FreeRTOS/Source/portable/GCC/ARM_CM4F/%.c | $(BUILD_DIR)
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile FreeRTOS heap
$(BUILD_DIR)/%.o: FreeRTOS/Source/portable/MemMang/%.c | $(BUILD_DIR)
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble startup
$(BUILD_DIR)/%.o: Startup/%.s | $(BUILD_DIR)
	@echo "Assembling $< ..."
	$(AS) $(CFLAGS) -c $< -o $@

# Link to create ELF
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	@echo "Linking $(TARGET).elf ..."
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

# Clean up build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Flash target using OpenOCD
flash: all
	@echo "Flashing $(TARGET).elf to STM32..."
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

erase:
	@echo "Erasing STM32 flash memory..."
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "init; halt; stm32f4x mass_erase 0; exit"