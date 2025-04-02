
######################################
# target
######################################
TARGET = LCD8H-firmware
SIM = lcd8-test

######################################
# building variables
######################################
# debug build?
DEBUG = 0
# optimization
ifeq ($(DEBUG), 1)
OPT = -Os   
else
OPT = -Ofast
endif

LVGL_VERSION = 8


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
src/drivers/at32f415_crc.c \
src/drivers/at32f415_dma.c \
src/drivers/at32f415_tmr.c \
src/drivers/at32f415_ertc.c \
src/drivers/at32f415_pwc.c \
src/drivers/at32f415_adc.c \
src/drivers/at32f415_usart.c \
src/drivers/at32f415_gpio.c \
src/drivers/at32f415_i2c.c \
src/drivers/at32f415_misc.c \
src/at32f415_clock.c \
src/delay.c \
src/eeprom.c \
src/uart.c \
src/lcd.c \
src/controls.c \
src/clock.c \
src/gui.c \
src/main.c

# fonts & images
C_SOURCES += \
src/fonts/8.x/ANDALEMO_72.c \
src/fonts/8.x/ANDALEMO_32.c \
src/fonts/8.x/ANDALEMO_28.c \
src/fonts/8.x/ANDALEMO_16.c \
src/fonts/8.x/ANDALEMO_12.c \
src/fonts/8.x/FRY_32.c \
src/fonts/large_text_1bpp.c \
src/img/battery_black.c \
src/img/icon_clock.c \
src/img/icon_brake.c \
src/img/icon_temperature.c \
src/img/icon_engine.c \
src/img/icon_journey.c \
src/img/icon_headlight.c \
src/img/icon_headlight_auto.c \
src/img/icon_trip.c
# src/img/icon_odo.c

# ASM sources
ASM_SOURCES =  \
src/startup/startup_at32f415.S

SIM_C_SOURCES = \
src/delay.c \
src/eeprom.c \
src/img.c \
src/controls.c \
src/gui.c \
src/main.c

LVGL_PATH ?= $(shell pwd)/thirdparty/lvgl

# append files
C_SOURCES += $(shell find $(LVGL_PATH)/src -type f -name '*.c')
SIM_C_SOURCES += $(shell find $(LVGL_PATH)/src -type f -name '*.c')
ASM_SOURCES += $(shell find $(LVGL_PATH)/src -type f -name '*.S') 


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

SIM_CC = gcc
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu

# float-abi
ifeq ($(DEBUG), 1)
LTO= -flto
else
LTO= -flto
endif

# mcu
MCU = $(CPU) -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))'

# macros for gcc
# AS defines
AS_DEFS = -DLV_VER=$(LVGL_VERSION) -DLV_CONF_INCLUDE_SIMPLE -DLV_LVGL_H_INCLUDE_SIMPLE

# C defines
C_DEFS =  \
-DAT32F415RCT7 \
-DARM_MATH_CM4 '-D__packed=__attribute__((__packed__))' \
-DLV_VER=$(LVGL_VERSION) \
-DLV_CONF_INCLUDE_SIMPLE \
-DLV_LVGL_H_INCLUDE_SIMPLE

# AS includes
AS_INCLUDES = \
-Iinc/ \
-Iinc/drivers/ \
-Iinc/cmsis/core \
-Iinc/cmsis/device \
-I$(LVGL_PATH)/src

# C includes
C_INCLUDES =  \
-Iinc/ \
-Iinc/drivers/ \
-Iinc/cmsis/core \
-Iinc/cmsis/device \
-I$(LVGL_PATH)/src



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections $(LTO) -Wa,-Iinc/

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -fno-ident -fno-asynchronous-unwind-tables $(LTO)
SIM_CFLAGS = $(C_INCLUDES) $(OPT) -Wall -DSIM

ifeq ($(DEBUG), 1)
CFLAGS += -g3 -DDEBUG_UART_PRINT=1 -DDEBUG=1
SIM_CFLAGS += -g3
else
CFLAGS += -Werror
endif


# Generate dependency information
# CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
# LDSCRIPT = STM32F103C6Tx_FLASH_Bootloader.ld
# LDSCRIPT = STM32F103C6Tx_FLASH.ld

LDSCRIPT = src/startup/AT32F415xC_FLASH.ld

# libraries
# LIBS = -lc -lm -lnosys -larm_cortexM3l_math
LIBS = 
SIM_LIBS = -lm -lX11
# LIBDIR = -LDrivers/CMSIS
LDFLAGS = $(MCU) -specs=nano.specs -specs=nosys.specs --specs=rdimon.specs -T$(LDSCRIPT) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage $(CFLAGS)
SIM_LDFLAGS = $(SIM_LIBS) -Wl,--print-memory-usage $(SIM_CFLAGS)

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
		

#######################################
# build the application
#######################################
# list of objects

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
SIM_OBJECTS = $(addprefix $(BUILD_DIR)/sim/,$(notdir $(SIM_C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.c $(sort $(dir $(SIM_C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))
# DOBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.pre)))
# SOBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.S)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/sim/%.o: %.c Makefile | $(BUILD_DIR)/sim
	$(SIM_CC) -c $(SIM_CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

# $(BUILD_DIR)/%.pre: %.c Makefile | $(BUILD_DIR)
# 	$(CC) -E $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/%.S: %.c Makefile | $(BUILD_DIR)
	$(AS) -E $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/sim/$(SIM): $(SIM_OBJECTS) Makefile
	$(SIM_CC) $(SIM_OBJECTS) $(SIM_LDFLAGS) -o $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	$(MAKE) --no-print-directory post-build
	
$(BUILD_DIR):
	mkdir -p $@	
	
$(BUILD_DIR)/sim: $(BUILD_DIR)
	mkdir -p $@	

preprocessor: $(DOBJECTS)

assembler: $(SOBJECTS)

flash: $(BUILD_DIR)/$(TARGET).bin
	pyocd load build/LCD8H-firmware.bin --target stm32f103rc

sim: $(BUILD_DIR)/sim/$(SIM) | $(BUILD_DIR)
	
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
