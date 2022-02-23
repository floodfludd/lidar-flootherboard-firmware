# Change these variables to match your system
#
# Install location of the MSP432 SDK
SIMPLELINK_MSP432_SDK_INSTALL_DIR = $(HOME)/ti/simplelink_msp432p4_sdk_3_40_01_02
# Install location of arm-none-eabi-gcc toolchain
GCC_ARM_COMPILER = /usr
# Install location of XDC tools
XDC_INSTALL_DIR = $(HOME)/ti/xdctools_3_60_02_34_core
# Path to sysconfig CLI tool
SYSCONFIG_TOOL = $(HOME)/ti/sysconfig_1.7.0/sysconfig_cli.sh

KERNEL_BUILD := $(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/kernel/tirtos/builds/MSP_EXP432P401R/release

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

PLATFORM = ti.platforms.msp432:MSP432P401R
XDCTARGET = gnu.targets.arm.M4F
XDCPATH = $(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/source;$(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/kernel/tirtos/packages;

OBJECTS = cli.obj main.obj radar.obj sim7000.obj storage.obj transmission.obj
DEPS = ../cli.h ../common.h ../radar.h ../sim7000.h ../storage.h \
       ../transmission.h
# Seperate target for ti drivers config, since it requires syscfg
GENERATED_OBJECTS = ti_drivers_config.o

DEVNULL = /dev/null

NAME = flood_msp432_firmware

CFLAGS = -I../.. \
    -I. \
    "-I$(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/source/ti/posix/gcc" \
    -mcpu=cortex-m4 \
    -march=armv7e-m \
    -mthumb \
    -std=c99 \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -ffunction-sections \
    -fdata-sections \
    -gstrict-dwarf \
    -Wall \
    -D __MSP432P401R__ \
    "-I$(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" \
    "-I$(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = -Wl,-T,MSP_EXP432P401R_TIRTOS.lds \
    "-Wl,-Map,$(NAME).map" \
    -nostartfiles \
    "-L$(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/source" \
    -l:ti/display/lib/display.am4fg \
    -l:ti/grlib/lib/gcc/m4f/grlib.a \
    -l:third_party/spiffs/lib/gcc/m4f/spiffs.a \
    -l:ti/drivers/lib/drivers_msp432p401x.am4fg \
    -l:third_party/fatfs/lib/gcc/m4f/fatfs.a \
    "-L$(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    -l:ti/dpl/lib/dpl_msp432p401x.am4fg \
    "-Wl,-T,$(NAME)/linker.cmd" \
    -l:ti/devices/msp432p4xx/driverlib/gcc/msp432p4xx_driverlib.a \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -static \
    -Wl,--gc-sections \
    "-L$(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard" \
    -lgcc \
    -lc \
    -lm \
    -u _printf_float \
    --specs=nano.specs

# Debug builds are the default target
debug: CFLAGS += -g
# Add rdimon lib for semihosting
debug: LFLAGS += -lrdimon
# Set BUILDTYPE to debug (for the .cfg file)
debug: BUILDTYPE = debug
# Do not create a bin file, it won't work with semihosting enabled.
debug: $(NAME).out

release: CFLAGS += -O2
release: LFLAGS += -lnosys
# Set BUILDTYPE to release (for the .cfg file)
release: BUILDTYPE = release
release: $(PROG).bin
	@echo "Release build complete"

$(PROG).bin: $(PROG).out
	@echo creating .bin file...
	@ $(OBJCPY) -O binary ${<} ${@}

%/compiler.opt: %/linker.cmd;

%/linker.cmd: ../$(NAME).cfg
	@ echo Running Configuro...
	@ $(XDC_INSTALL_DIR)/xs --xdcpath="$(XDCPATH)" xdc.tools.configuro -c "$(GCC_ARM_COMPILER)" -t $(XDCTARGET) -p $(PLATFORM) --compileOptions "$(CFLAGS)" -o ./$(NAME) $<

.INTERMEDIATE: syscfg
ti_drivers_config.c ti_drivers_config.h: syscfg
	@ echo generation complete

syscfg: ../$(NAME).syscfg
	@ echo Generating configuration files...
	@ $(SYSCONFIG_TOOL) --product $(SIMPLELINK_MSP432_SDK_INSTALL_DIR)/.metadata/product.json --output $(@D) $<

%.obj: ../%.c ti_drivers_config.h $(NAME)/compiler.opt $(DEPS)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(NAME)/compiler.opt -o $@

ti_drivers_config.o: ti_drivers_config.c ti_drivers_config.h $(NAME)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(NAME)/compiler.opt -o $@

$(NAME).out: $(OBJECTS) $(GENERATED_OBJECTS) $(NAME)/linker.cmd
	@ echo linking $@
	@ $(LNK)  $(OBJECTS) $(GENERATED_OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(GENERATED_OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) -r $(NAME) > $(DEVNULL) 2>&1
	@ $(RM) syscfg_c.rov.xs
	@ $(RM) ti_drivers_config.c ti_drivers_config.h > $(DEVNULL) 2>&1
