
######################################
# target
######################################
TARGET = LCD8H-firmware


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O2


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
src/startup/system_at32f415.c \
src/drivers/at32f415_crm.c \
src/drivers/at32f415_dma.c \
src/drivers/at32f415_tmr.c \
src/drivers/at32f415_adc.c \
src/drivers/at32f415_usart.c \
src/drivers/at32f415_gpio.c \
src/drivers/at32f415_i2c.c \
src/drivers/at32f415_misc.c \
src/at32f415_clock.c \
src/delay.c \
src/eeprom.c \
src/lcd.c \
src/controls.c \
src/main.c
# src/system_stm32f1xx_Bootloader.c
#Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c \
# ASM sources
ASM_SOURCES =  \
src/startup/startup_at32f415.s

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
GCC_PATH = /opt/arm/bin/
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu

# float-abi
LTO=

# mcu
MCU = $(CPU) -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))'

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DAT32F415RCT7 \
-DARM_MATH_CM4 '-D__packed=__attribute__((__packed__))'

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-Iinc/ \
-Iinc/drivers/ \
-Iinc/cmsis/core \
-Iinc/cmsis/device



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections $(LTO) 

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections $(LTO)

ifeq ($(DEBUG), 1)
CFLAGS += -g3
endif


# Generate dependency information
# CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
# LDSCRIPT = STM32F103C6Tx_FLASH_Bootloader.ld
# LDSCRIPT = STM32F103C6Tx_FLASH.ld

LDSCRIPT = src/startup/AT32F415xB_FLASH.ld

# libraries
# LIBS = -lc -lm -lnosys -larm_cortexM3l_math
LIBS =  -lm
# LIBDIR = -LDrivers/CMSIS
LDFLAGS = $(MCU) -specs=nano.specs -specs=nosys.specs --specs=rdimon.specs  -T$(LDSCRIPT) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage $(CFLAGS)

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
		

#######################################
# build the application
#######################################
# list of objects

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
DOBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.pre)))
SOBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.S)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.pre: %.c Makefile | $(BUILD_DIR)
	$(CC) -E $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/%.S: %.c Makefile | $(BUILD_DIR)
	$(AS) -S $(CFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	$(MAKE) --no-print-directory post-build
	
$(BUILD_DIR):
	mkdir $@	

preprocessor: $(DOBJECTS)

assembler: $(SOBJECTS)

flash: $(BUILD_DIR)/$(TARGET).bin
	pyocd load build/LCD8H-firmware.bin --target stm32f103rc
	
#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

#######################################
# post build
#######################################
post-build:
	-@echo 'code size in flash:'
	stat "build/$(TARGET).bin"

# *** EOF ***
