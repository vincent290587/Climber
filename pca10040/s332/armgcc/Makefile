PROJECT_NAME     := ble_app_lns_c_pca10040_s132
TARGETS          := nrf52832_xxaa
SOFTDEVICE       := components/softdevice/s332/hex/ANT_s332_nrf52832_6.1.1.hex

include Makefile.local

PROJ_DIR := ../../..

$(OUTPUT_DIRECTORY)/nrf52832_xxaa.out: \
  LINKER_SCRIPT  := ble_app_lns_c_gcc_nrf52.ld


# Source files common to all targets
SRC_FILES += \
  $(PROJ_DIR)/rf/ant.c \
  $(PROJ_DIR)/rf/ble_api6.c \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52.S \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52.c \
  $(SDK_ROOT)/components/ant/ant_channel_config/ant_channel_config.c \
  $(SDK_ROOT)/components/ant/ant_key_manager/ant_key_manager.c \
  $(SDK_ROOT)/components/ant/ant_state_indicator/ant_state_indicator.c \
  $(SDK_ROOT)/components/ant/ant_search_config/ant_search_config.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_common/pages/ant_common_page_80.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_common/pages/ant_common_page_81.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ant.c \
  $(wildcard $(PROJ_DIR)/libraries/ant_profiles/ant_common/pages/*.c) \
  $(wildcard $(PROJ_DIR)/libraries/ant_profiles/ant_fec/*.c) \
  $(wildcard $(PROJ_DIR)/libraries/ant_profiles/ant_fec/pages/*.c) \
  $(wildcard $(PROJ_DIR)/libraries/ant_profiles/ant_fec/utils/*.c) \

# Include folders common to all targets
INC_FOLDERS += \
  ../config \
  $(SDK_ROOT)/components/softdevice/s332/headers \
  $(SDK_ROOT)/components/softdevice/s332/headers/nrf52 \
  $(PROJ_DIR)/libraries/ant_profiles/ant_common/pages \
  $(PROJ_DIR)/libraries/ant_profiles/ant_fec \
  $(PROJ_DIR)/libraries/ant_profiles/ant_fec/pages \
  $(PROJ_DIR)/libraries/ant_profiles/ant_fec/utils \
  
include $(PROJ_DIR)/Makefile.files

include $(PROJ_DIR)/ble.files

# Libraries common to all targets
LIB_FILES += -lm -lstdc++

# Optimization flags
OPT = -Og -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
#CFLAGS += -DUSE_SVIEW=1 -DNRF_LOG_ENABLED=0 -DUSE_RTT=0
CFLAGS += -DUSE_SVIEW=0 -DNRF_LOG_ENABLED=1 -DUSE_RTT=1
#CFLAGS += -DUSE_SVIEW=0 -DUSE_JSCOPE -DNRF_LOG_ENABLED=0 -DUSE_RTT=1
CFLAGS += -DDEBUG -DDEBUG_NRF -DDEBUG_NRF_USER
CFLAGS += -DUSE_APP_CONFIG
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
CFLAGS += -DBOARD_CUSTOM -DCUSTOM_V0
CFLAGS += -D__HEAP_SIZE=2048
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DANT_STACK_SUPPORT_REQD
CFLAGS += -DNRF_SD_BLE_API_VERSION=6
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF52_PAN_74
CFLAGS += -DS332
CFLAGS += -mcpu=cortex-m4 -DARM_MATH_CM4
CFLAGS += -Wall $(OPT)
CFLAGS += -DFPU_INTERRUPT_MODE
CFLAGS += -DNRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED=1
CFLAGS += -DFLOAT_ABI_HARD -fsingle-precision-constant
CFLAGS += -Wdouble-promotion -Wfloat-conversion
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums -nostdlib -fno-exceptions

ifeq ($(MMD), 1)
CFLAGS += -DMMD
CFLAGS += -DCONFIG_JLINK_MONITOR_ENABLED
CFLAGS += -DCONFIG_WATCHDOG_ENABLED
SRC_FILES += $(PROJ_DIR)/MMD/JLINK_MONITOR.c
SRC_FILES += $(PROJ_DIR)/MMD/JLINK_MONITOR_ISR_SES.s
INC_FOLDERS += $(PROJ_DIR)/MMD
$(info Building for monitor debug mode.)
endif

# C++ flags common to all targets
CXXFLAGS += $(OPT) -std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti

# Assembler flags common to all targets
ASMFLAGS += -g3 -mcpu=cortex-m4
ASMFLAGS += -DARM_MATH_CM4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DNRF52
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DBOARD_CUSTOM
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52832_XXAA
ASMFLAGS += -DNRF52_PAN_74
ASMFLAGS += -D__HEAP_SIZE=2048
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DANT_STACK_SUPPORT_REQD
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=6
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DS332

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys

.PHONY: $(TARGETS) default all clean help flash flashl flash_softdevice

# Default target - first one defined
default: nrf52832_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	nrf52832_xxaa

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc


include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

# Flash the program
flash: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	@echo Flashing: $<
	nrfjprog --program $< -f nrf52 --sectorerase
	nrfjprog --reset -f nrf52
	
# Flash the program
flashl:
	@echo Flashing: nrf52832_xxaa.hex
	nrfjprog --program $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex -f nrf52 --sectorerase
	nrfjprog --reset -f nrf52

# Flash softdevice
flash_softdevice:
	@echo Flashing
	nrfjprog --program $(SDK_ROOT)/$(SOFTDEVICE) -f nrf52 --sectorerase 
#	nrfjprog --reset -f nrf52

dfu:
	nrfutil --verbose pkg generate --hw-version 52 --sd-req 0xba --application-version 1 --application $(OUTPUT_DIRECTORY)/$(TARGETS).hex --key-file $(SDK_ROOT)/../vault/priv.pem SpinMeter.zip
	nrfutil pkg display SpinMeter.zip
	nrfutil dfu ant -pkg SpinMeter.zip -p $(COM_PORT)


SDK_CONFIG_FILE := ../config/sdk_config.h
SDK_CONFIG_FILE_ANT := ../config/sdk_config_ant.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)
	
sdk_config_ant:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE_ANT)

erase:
	nrfjprog --eraseall -f nrf52
