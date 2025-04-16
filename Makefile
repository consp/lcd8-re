
######################################
# target
######################################
TARGET = LCD8H-firmware

######################################
# building variables
######################################
# debug build?
DEBUG = 1
MONITOR = 1
# optimization
ifeq ($(DEBUG), 1)
OPT = -O1
else
OPT = -O3   
endif

# PLATFORM=LCD8
PLATFORM=SIM

CHIP = sim
# CHIP=gd32f303
# CHIP = at32f415
# CHIP=AT32F435


LVGL_VERSION = 8
# LVGL_VERSION = 9


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
src/controls.c \
src/comm.c \
src/crc.c \
src/gui.c \
src/main.c

ifeq ($(CHIP),at32f415)
MCU_PATH=at32f415
else ifeq ($(CHIP),at32f435)
MCU_PATH=at32f435
else ifeq ($(CHIP),gd32f303)
MCU_PATH=gd32f303
endif

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


# platform sources
ifeq ($(PLATFORM),LCD8)
C_SOURCES +=\
src/hal/lcd8h/$(MCU_PATH)/sysclock.c \
src/hal/lcd8h/${MCU_PATH}/lcd.c \
src/hal/lcd8h/${MCU_PATH}/uart.c \
src/hal/lcd8h/${MCU_PATH}/delay.c \
src/hal/lcd8h/${MCU_PATH}/clock.c \
src/hal/lcd8h/${MCU_PATH}/eeprom.c \
src/hal/lcd8h/${MCU_PATH}/cntl.c
else ifeq (${PLATFORM},SIM)
# lcd driver is replaced by gtk, large numers need fake item
C_SOURCES += \
	src/hal/sim/sysclock.c \
	src/hal/sim/uart.c \
	src/hal/sim/lcd.c \
	src/hal/sim/delay.c \
	src/hal/sim/clock.c \
	src/hal/sim/eeprom.c \
	src/hal/sim/cntl.c \
	thirdparty/lv_drivers/gtkdrv/gtkdrv.c
endif

ifeq ($(CHIP),at32f415)
C_SOURCES += \
src/startup/at32f415/system_at32f415.c \
src/drivers/at32f415/at32f415_crm.c \
src/drivers/at32f415/at32f415_crc.c \
src/drivers/at32f415/at32f415_dma.c \
src/drivers/at32f415/at32f415_tmr.c \
src/drivers/at32f415/at32f415_ertc.c \
src/drivers/at32f415/at32f415_pwc.c \
src/drivers/at32f415/at32f415_adc.c \
src/drivers/at32f415/at32f415_usart.c \
src/drivers/at32f415/at32f415_gpio.c \
src/drivers/at32f415/at32f415_i2c.c \
src/drivers/at32f415/at32f415_misc.c 
else ifeq ($(CHIP),gd32f303)
C_SOURCES += \
src/startup/gd32f303/system_gd32f30x.c \
src/drivers/gd32f303/gd32f30x_adc.c \
src/drivers/gd32f303/gd32f30x_dma.c \
src/drivers/gd32f303/gd32f30x_gpio.c \
src/drivers/gd32f303/gd32f30x_pmu.c \
src/drivers/gd32f303/gd32f30x_rcu.c \
src/drivers/gd32f303/gd32f30x_misc.c \
src/drivers/gd32f303/gd32f30x_timer.c \
src/drivers/gd32f303/gd32f30x_usart.c
else # sim
# do nothing for now
endif


# ASM sources
ifeq ($(CHIP),at32f415)
ASM_SOURCES =  \
src/startup/at32f415/startup_at32f415.S
else ifeq ($(CHIP),gd32f303)
ASM_SOURCES =  \
src/startup/gd32f303/startup_gd32f30x.S
else
ARM_SOURCES = 
endif

LVGL_PATH ?= $(shell pwd)/thirdparty/lvgl

# append files
C_SOURCES += $(shell find $(LVGL_PATH)/src -type f -name '*.c')
ifneq ($(PLATFORM),SIM)
ASM_SOURCES += $(shell find $(LVGL_PATH)/src -type f -name '*.S') 
endif


#######################################
# binaries
#######################################
ifneq ($(PLATFORM),SIM)
PREFIX = arm-none-eabi-
GCC_PATH = /opt/arm/bin/
else
PREFIX = 
endif
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
ifeq ($(DEBUG),1)
LTO=
else
LTO= -flto
endif


# mcu
ifneq ($(PLATFORM),SIM)
MCU = $(CPU) -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))'
endif

# macros for gcc
# AS defines
AS_DEFS = -DLV_VER=$(LVGL_VERSION) -DLV_CONF_INCLUDE_SIMPLE -DLV_LVGL_H_INCLUDE_SIMPLE

# C defines
C_DEFS =  \
	-DLV_VER=$(LVGL_VERSION) \
	-DLV_CONF_INCLUDE_SIMPLE \
	-DPLATFORM_$(PLATFORM) \
	-DLV_LVGL_H_INCLUDE_SIMPLE

ifeq ($(MONITOR),1)
C_DEFS += -DMONITOR=1
endif

ifeq ($(CHIP),at32f415)
C_DEFS += -DAT32F415RCT7 \
		  -DAT32F415 \
		  -DARM_MATH_CM4 '-D__packed=__attribute__((__packed__))' \
		  -DLV_MEM_SIZE_KB=13U
else ifeq ($(CHIP),at32f435)
C_DEFS += -DAT32F435RMT7 \
		  -DAT32F435 \
		  -DARM_MATH_CM4 '-D__packed=__attribute__((__packed__))'\
		  -DLV_MEM_SIZE_KB=96U
else ifeq ($(CHIP),gd32f303)
C_DEFS += -DGD32F303 \
		  -DGD32F30X_XD \
		  -DARM_MATH_CM4 '-D__packed=__attribute__((__packed__))' \
		  -DLV_MEM_SIZE_KB=32U
else
C_DEFS += \
		  -DUSE_GTK=1 \
		  -DLV_MEM_SIZE_KB=1024U
endif

# AS includes
AS_INCLUDES = \
-Iinc/ \
-Iinc/drivers/$(CHIP)/ \
-Iinc/cmsis/core \
-Iinc/cmsis/device \
-I$(LVGL_PATH)/src

# C includes
C_INCLUDES =  \
-Iinc/ \
-Iinc/drivers/$(CHIP) \
-I$(LVGL_PATH)/src

ifeq ($(PLATFORM),LCD8)
C_INCLUDES += \
	-Iinc/cmsis/core \
	-Iinc/cmsis/device \
	-Iinc/hal/lcd8/$(CHIP)
else 
C_INCLUDES += 	\
	-Iinc/hal/sim \
	-Ithirdparty/lv_drivers/gtkdrv/
endif



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections $(LTO) -Wa,-Iinc/

ifeq ($(PLATFORM),SIM)
CFLAGS += `pkg-config --cflags gtk+-3.0` $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall $(LTO)
else
CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -fno-ident -fno-asynchronous-unwind-tables $(LTO)
endif


ifeq ($(DEBUG), 1)
CFLAGS += -g3 -DDEBUG_UART_PRINT=1 -DDEBUG=1
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
#
ifeq (${CHIP},at32f415)
LDSCRIPT = src/startup/at32f415/AT32F415xC_FLASH.ld
else ifeq ($(CHIP),gd32f303)
LDSCRIPT = src/startup/gd32f303/gd32f30x.ld
endif

# libraries
# LIBS = -lc -lm -lnosys -larm_cortexM3l_math
ifeq ($(PLATFORM),SIM)
LIBS = \
	-lm \
	`pkg-config --libs gtk+-3.0`
else
LIBS = 
endif

ifneq ($(PLATFORM),SIM)
LDFLAGS = $(MCU) -specs=nano.specs -specs=nosys.specs --specs=rdimon.specs -T$(LDSCRIPT) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage $(CFLAGS)
else
LDFLAGS = $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage $(CFLAGS)
endif

# default action: build all
ifeq ($(PLATFORM),SIM)
all: $(BUILD_DIR)/$(TARGET)
else
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
endif	

#######################################
# build the application
#######################################
# list of objects

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))
# DOBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.pre)))
# SOBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.S)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

# $(BUILD_DIR)/%.pre: %.c Makefile | $(BUILD_DIR)
# 	$(CC) -E $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/%.S: %.c Makefile | $(BUILD_DIR)
	$(AS) -E $(ASFLAGS) -o $@ $<

ifeq ($(PLATFORM),SIM)
$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
else
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
endif
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	$(MAKE) --no-print-directory post-build
	
$(BUILD_DIR):
	mkdir -p $@	
	
preprocessor: $(DOBJECTS)

assembler: $(SOBJECTS)

flash: $(BUILD_DIR)/$(TARGET).bin
	pyocd load -u 69747484 build/LCD8H-firmware.bin --target stm32f103rc

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
