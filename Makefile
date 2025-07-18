
######################################
# target
######################################
TARGET = LCD8H-firmware

######################################
# building variables
######################################
# build as debug, if disabled all others are as well
DEBUG=0
PROFILE=0
# log to console/uart
LVGL_LOG=1
# print monitoring info on display and/or logs
MONITOR=1
# use libasan in sim build (memory error detection)
ASAN=0


## Select platform, SIM uses DRM and requires linux/X11/Wayland/Gtk
# PLATFORM=LCD8
# PLATFORM=SIM
PLATFORM=custom

## Select chip If you replace the chip on the board (e.g. at32f415 to at32f435) you can have more memory etc.
# CHIP = gd32f303 ## DOES NOT WORK
# CHIP = at32f415
# CHIP=at32f435
CHIP=stm32h743

## Select lvgl version, note: lvgl 9 requires >= 64kb of memory
# LVGL_VERSION = 8
LVGL_VERSION = 9


#######################################
# Selectors needed for other parts
#######################################
ifeq ($(PLATFORM),SIM)
	CHIP = sim
endif

# optimization
ifeq ($(DEBUG),1)
	OPT = -O2
else
	OPT = -Ofast
endif


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
			 src/fonts/$(LVGL_VERSION).x/ANDALEMO_72.c \
			 src/fonts/$(LVGL_VERSION).x/ANDALEMO_32.c \
			 src/fonts/$(LVGL_VERSION).x/ANDALEMO_28.c \
			 src/fonts/$(LVGL_VERSION).x/ANDALEMO_16.c \
			 src/fonts/$(LVGL_VERSION).x/ANDALEMO_12.c \
			 src/fonts/$(LVGL_VERSION).x/PLEX_72.c \
			 src/fonts/$(LVGL_VERSION).x/PLEX_32.c \
			 src/fonts/$(LVGL_VERSION).x/PLEX_28.c \
			 src/fonts/$(LVGL_VERSION).x/PLEX_16.c \
			 src/fonts/$(LVGL_VERSION).x/PLEX_12.c \
			 src/fonts/$(LVGL_VERSION).x/FRY_32.c \
			 src/fonts/large_text_1bpp.c \
			 src/img/display/lvgl$(LVGL_VERSION)/battery_black.c \
			 src/img/display/lvgl$(LVGL_VERSION)/icon_clock.c \
			 src/img/display/lvgl$(LVGL_VERSION)/icon_brake.c \
			 src/img/display/lvgl$(LVGL_VERSION)/icon_temperature.c \
			 src/img/display/lvgl$(LVGL_VERSION)/icon_engine.c \
			 src/img/display/lvgl$(LVGL_VERSION)/icon_journey.c \
			 src/img/display/lvgl$(LVGL_VERSION)/icon_headlight.c \
			 src/img/display/lvgl$(LVGL_VERSION)/icon_headlight_auto.c \
			 src/img/display/lvgl$(LVGL_VERSION)/icon_energy.c \
			 src/img/display/lvgl$(LVGL_VERSION)/icon_trip.c


# platform sources
ifeq ($(PLATFORM),LCD8)
	C_SOURCES += \
				 src/hal/lcd8h/$(MCU_PATH)/sysclock.c \
				 src/hal/lcd8h/${MCU_PATH}/lcd.c \
				 src/hal/lcd8h/${MCU_PATH}/uart.c \
				 src/hal/lcd8h/${MCU_PATH}/delay.c \
				 src/hal/lcd8h/${MCU_PATH}/clock.c \
				 src/hal/lcd8h/${MCU_PATH}/eeprom.c \
				 src/hal/lcd8h/${MCU_PATH}/cntl.c
else ifeq ($(PLATFORM),custom)
	C_SOURCES += \
				 src/hal/custom/$(MCU_PATH)/sysclock.c \
				 src/hal/custom/${MCU_PATH}/lcd.c \
				 src/hal/custom/${MCU_PATH}/uart.c \
				 src/hal/custom/${MCU_PATH}/delay.c \
				 src/hal/custom/${MCU_PATH}/clock.c \
				 src/hal/custom/${MCU_PATH}/eeprom.c \
				 src/hal/custom/${MCU_PATH}/cntl.c
else ifeq (${PLATFORM},SIM)
	# lcd driver is replaced by gtk, large numers need fake item
	C_SOURCES += \
				 src/hal/sim/sysclock.c \
				 src/hal/sim/uart.c \
				 src/hal/sim/lcd.c \
				 src/hal/sim/delay.c \
				 src/hal/sim/clock.c \
				 src/hal/sim/eeprom.c \
				 src/hal/sim/cntl.c
endif

ifeq ($(PLATFORM)$(LVGL_VERSION),SIM8)
	C_SOURCES += \
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
else ifeq ($(CHIP),at32f435)
	C_SOURCES += \
				 src/startup/at32f435/system_at32f435_437.c \
				 src/drivers/at32f435/at32f435_437_crm.c \
				 src/drivers/at32f435/at32f435_437_crc.c \
				 src/drivers/at32f435/at32f435_437_dma.c \
				 src/drivers/at32f435/at32f435_437_flash.c \
				 src/drivers/at32f435/at32f435_437_tmr.c \
				 src/drivers/at32f435/at32f435_437_ertc.c \
				 src/drivers/at32f435/at32f435_437_pwc.c \
				 src/drivers/at32f435/at32f435_437_adc.c \
				 src/drivers/at32f435/at32f435_437_usart.c \
				 src/drivers/at32f435/at32f435_437_gpio.c \
				 src/drivers/at32f435/at32f435_437_i2c.c \
				 src/drivers/at32f435/at32f435_437_misc.c \
				 src/drivers/at32f435/at32f435_437_wdt.c \
				 src/hal/lcd8h/at32f435/at32f435_437_int.c
else ifeq ($(CHIP),stm32h743)
		#
	C_SOURCES += \
				src/hal/custom/stm32h743/system_stm32h7xx.c \
	src/hal/custom/stm32h743/syscalls.c \
	src/hal/custom/stm32h743/sysmem.c \
				src/hal/custom/stm32h743/stm32h7xx_it.c \
				src/drivers/stm32h743/stm32h7xx_hal_pcd.c \
				src/drivers/stm32h743/stm32h7xx_hal_pcd_ex.c \
				src/drivers/stm32h743/stm32h7xx_ll_usb.c \
				src/drivers/stm32h743/stm32h7xx_hal_rcc.c \
				src/drivers/stm32h743/stm32h7xx_hal_rcc_ex.c \
				src/drivers/stm32h743/stm32h7xx_hal_flash.c \
				src/drivers/stm32h743/stm32h7xx_hal_flash_ex.c \
				src/drivers/stm32h743/stm32h7xx_hal_gpio.c \
				src/drivers/stm32h743/stm32h7xx_hal_hsem.c \
				src/drivers/stm32h743/stm32h7xx_hal_dma.c \
				src/drivers/stm32h743/stm32h7xx_hal_dma_ex.c \
				src/drivers/stm32h743/stm32h7xx_hal_mdma.c \
				src/drivers/stm32h743/stm32h7xx_hal_pwr.c \
				src/drivers/stm32h743/stm32h7xx_hal_pwr_ex.c \
				src/drivers/stm32h743/stm32h7xx_hal_cortex.c \
				src/drivers/stm32h743/stm32h7xx_hal.c \
				src/drivers/stm32h743/stm32h7xx_hal_i2c.c \
				src/drivers/stm32h743/stm32h7xx_hal_i2c_ex.c \
				src/drivers/stm32h743/stm32h7xx_hal_exti.c \
				src/drivers/stm32h743/stm32h7xx_hal_adc.c \
				src/drivers/stm32h743/stm32h7xx_hal_adc_ex.c \
				src/drivers/stm32h743/stm32h7xx_hal_crc.c \
				src/drivers/stm32h743/stm32h7xx_hal_crc_ex.c \
				src/drivers/stm32h743/stm32h7xx_hal_dma2d.c \
				src/drivers/stm32h743/stm32h7xx_hal_fdcan.c \
				src/drivers/stm32h743/stm32h7xx_ll_fmc.c \
				src/drivers/stm32h743/stm32h7xx_hal_nor.c \
				src/drivers/stm32h743/stm32h7xx_hal_sram.c \
				src/drivers/stm32h743/stm32h7xx_hal_nand.c \
				src/drivers/stm32h743/stm32h7xx_hal_sdram.c \
				src/drivers/stm32h743/stm32h7xx_hal_uart.c \
				src/drivers/stm32h743/stm32h7xx_hal_uart_ex.c \
				src/drivers/stm32h743/stm32h7xx_hal_rtc.c \
				src/drivers/stm32h743/stm32h7xx_hal_rtc_ex.c \
				src/drivers/stm32h743/stm32h7xx_hal_tim.c \
				src/drivers/stm32h743/stm32h7xx_hal_tim_ex.c
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
else ifeq ($(CHIP),at32f435)
	ASM_SOURCES = \
				  src/startup/at32f435/startup_at32f435_437.S
else ifeq ($(CHIP),stm32h743)
	ASM_SOURCES = \
				  src/startup/stm32h743/startup_stm32h743xx.S
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
ifeq ($(CHIP),stm32h743)
CPU = -mcpu=cortex-m7
else
CPU = -mcpu=cortex-m4
endif

# fpu

# float-abi
ifeq ($(DEBUG),1)
	# LTO= -flto
	LTO = 
else
	LTO= -flto
endif


# mcu
ifneq ($(PLATFORM),SIM)
	MCU = $(CPU) -mthumb '-D__weak=__attribute__((weak))'
endif

ifeq ($(CHIP),at32f435)
	MCU += -mfloat-abi=hard -mfpu=fpv4-sp-d16
	# MCU += -mfloat-abi=soft
else ifeq ($(CHIP),stm32h743)
	MCU += -mfloat-abi=hard -mfpu=fpv5-d16
else ifneq ($(PLATFORM),SIM)
	MCU += -mfloat-abi=soft 
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
	C_DEFS += \
			  -DMONITOR=1
	# -DSEGGER_RTT
	# # add segger rtt files
	# C_SOURCES += \
		# 	src/rtt/SEGGER_RTT.c \
		# 	src/rtt/SEGGER_RTT_printf.c \
		# 	src/rtt/SEGGER_RTT_Syscalls_GCC.c 
	# ASM_SOURCES += src/rtt/SEGGER_RTT_ASM_ARMv7M.S
	# AS_INCLUDES += -Iinc/rtt
	# C_INCLUDES += -Iinc/rtt
endif

ifeq ($(MONITOR)$(DEBUG),11)
ifeq ($(CHIP),stm32h743)
	C_DEFS += \
			  -DSWO
else
	C_DEFS += \
			  -DSEMIHOSTING 
endif
else ifeq ($(LVGL_LOG),1)
ifeq ($(CHIP),stm32h743)
	C_DEFS += \
			  -DSWO
else
	C_DEFS += \
			  -DSEMIHOSTING
endif
endif

ifeq ($(CHIP),at32f415)
	C_DEFS += -DAT32F415RCT7 \
			  -DAT32F415 \
			  -DARM_MATH_CM4 '-D__packed=__attribute__((__packed__))' \
			  -DLV_MEM_SIZE_KB=13U
	else ifeq ($(CHIP),at32f435)
	C_DEFS += -DAT32F435RMT7 \
			  -DAT32F435 \
			  -DARM_MATH_CM4 '-D__packed=__attribute__((__packed__))' \
			  -DLV_MEM_SIZE_KB=48U
else ifeq ($(CHIP),gd32f303)
C_DEFS += -DGD32F303 \
		  -DGD32F30X_XD \
		  -DARM_MATH_CM4 '-D__packed=__attribute__((__packed__))' \
		  -DLV_MEM_SIZE_KB=32U
else ifeq ($(CHIP),stm32h743)
C_DEFS += -DSTM32H743 \
		  -DARM_MATH_CM7 '-D__packed=__attribute__((__packed__))' \
		  -DLV_MEM_SIZE_KB=64u
else ifeq ($(PLATFORM),SIM)
C_DEFS += -DLV_MEM_SIZE_KB=16384U
else
$(error invalid platform or chipset: "$(PLATFORM)" "$(CHIP)")
endif

ifeq ($(LVGL_LOG),1)
C_DEFS += -DLVGL_LOG=1
endif

# AS includes
AS_INCLUDES += \
-Iinc/ \
-Iinc/drivers/$(CHIP)/ \
-Iinc/cmsis/core \
-Iinc/cmsis/device \
-I$(LVGL_PATH)/src

# C includes
C_INCLUDES +=  \
-Iinc/ \
-Iinc/drivers/$(CHIP) \
-I$(LVGL_PATH)/src

ifeq ($(PLATFORM),SIM)
C_INCLUDES += 	\
	-Iinc/hal/sim 
else 
C_INCLUDES += \
	-Iinc/cmsis/core \
	-Iinc/cmsis/device \
	-Iinc/hal/$(PLATFORM)/$(CHIP)
endif

ifeq ($(PLATFORM)$(LVGL_VERSION),SIM8)
C_SOURCES += -Ithirdparty/lv_drivers/gtkdrv/
endif


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS)  $(OPT) -Wall -fdata-sections -ffunction-sections $(LTO) -Wa,-Iinc/
GTK_CFLAGS=$(shell pkg-config --cflags gtk+-3.0)
SDL2_CFLAGS=$(shell pkg-config --cflags sdl2)
ifeq ($(PROFILE),1)
	CFLAGS += -DPROFILE=1
endif
ifeq ($(PLATFORM)$(LVGL_VERSION),SIM9)
CFLAGS += $(SDL2_CFLAGS) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall $(LTO) 
endif
ifeq ($(PLATFORM)$(LVGL_VERSION),SIM8)
CFLAGS += $(GTK_CFLAGS) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall $(LTO) 
endif
ifeq ($(PLATFORM),LCD8)
CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -fno-ident -fno-asynchronous-unwind-tables $(LTO)
endif
ifeq ($(PLATFORM),custom)
CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -fno-ident -fno-asynchronous-unwind-tables -DSTM32H743xx $(LTO)
endif


ifeq ($(DEBUG), 1)
CFLAGS += -g3 -DDEBUG=1
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
else ifeq ($(CHIP),at32f435)
LDSCRIPT = src/startup/at32f435/AT32F435xM_FLASH.ld
# LDSCRIPT = src/startup/at32f435/test.ld
else ifeq ($(CHIP),stm32h743)
LDSCRIPT = src/startup/stm32h743/STM32H743XX_FLASH.ld
endif

# libraries
# -larm_cortexM3l_math
ifeq ($(PLATFORM),SIM)
LIBS = \
	-lm 
else
LIBS = -lc -lm # -lnosys # -lrdimon
endif

ifeq ($(PLATFORM)$(LVGL_VERSION),SIM8)
LIBS += `pkg-config --libs gtk+-3.0` 
else ifeq ($(PLATFORM)$(LVGL_VERSION),SIM9)
LIBS += `pkg-config --libs sdl2` 
endif

ifeq ($(ASAN),1)
LIBS_ASAN = -lasan
CFLAGS_ASAN = -fsanitize=address -fno-omit-frame-pointer -fno-common
CFLAGS += $(CFLAGS_ASAN)
endif
#  
ifneq ($(PLATFORM),SIM)
LDFLAGS += $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBS_ASAN) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage $(CFLAGS)
else
LDFLAGS += $(LIBS_ASAN) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage $(CFLAGS)
endif

# default action: build all
ifeq ($(PLATFORM),SIM)
all: $(BUILD_DIR)/$(TARGET)
else
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
endif	


ifeq ($(CHIP),at32f415)
PYOCD_TARGET = _at32f415rct7
else ifeq ($(CHIP),at32f435)
PYOCD_TARGET = _at32f437rmt7 
else ifeq ($(CHIP),stm32h743)
PYOCD_TARGET = stm32h74x
else 
PYOCD_TARGET = stm32f103rc
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
ifeq ($(CHIP),at32f435)
	/opt/SEGGER/JLink/JLinkExe < jlink-at32f435.cfg
else ifeq ($(CHIP),stm32h743)
	/opt/SEGGER/JLink/JLinkExe < jlink-stm32h743.cfg
else
	pyocd load -u 69747484 build/LCD8H-firmware.bin --target $(PYOCD_TARGET) 
endif

gdb: $(BUILD_DIR)/$(TARGET).elf
	pyocd gdbserver -u 69747484 --target $(PYOCD_TARGET) &
	$(GCC_PATH)/$(PREFIX)gdb --command=gdb
	

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
