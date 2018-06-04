###############################################################################
# Makefile for the project BRTOS_MSP430
###############################################################################

ifdef UNET_DEVICE_TYPE  
DEFINES = -DUNET_DEVICE_TYPE=$(UNET_DEVICE_TYPE)
#$(info $$UNET_DEVICE_TYPE is [${UNET_DEVICE_TYPE}]) ## Debug includes

# Must be defined by DUNET_DEVICE_TYPE, see code below
APPLICATION =
ifeq ($(UNET_DEVICE_TYPE),PAN_COORDINATOR)
APPLICATION = _server
endif
ifeq ($(UNET_DEVICE_TYPE),ROUTER)
APPLICATION = _client
endif

#else # if not defined, compile both
#$(strip $(shell make UNET_DEVICE_TYPE=PAN_COORDINATOR))
#$(stip $(shell make UNET_DEVICE_TYPE=ROUTER))
#$(exit)

endif #UNET_DEVICE_TYPE
		  
############# system binaries & configurations ################
CC      	= msp430-elf-gcc
CPP			= msp430-elf-g++
LD      	= msp430-elf-ld
AR      	= msp430-elf-ar
AS      	= msp430-elf-gcc
GASP    	= msp430-elf-gasp
NM      	= msp430-elf-nm
OBJCOPY 	= msp430-elf-objcopy
SIZE		= msp430-elf-size
MAKETXT 	= srec_cat
UNIX2DOS	= unix2dos
RM      	= rm -f
MKDIR		= mkdir -p
COPY		= cp
MOVE		= mv
NULL		= /dev/null

## Compiller libraries
LIBRARY = /home/user/ti/msp430_gcc/include
LIBRARY2 = /home/user/ti/msp430_gcc/lib/gcc/msp430-elf/5.3.0/include
LIBRARY3 = /home/user/ti/msp430_gcc/lib/gcc/msp430-elf/5.3.0/include/ssp


############# project configuration ################
PROJECT_NAME = unet4cooja
PROJECT_DIR := $(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))
#$(info $$PROJECT_DIR is [${PROJECT_DIR}]) ## Debug includes

## Generate assembly code
GENERATE_ASSEMBLY = FALSE

OUTPUTS = $(PROJECT_DIR)/objects
BIN = $(PROJECT_DIR)/bin

## Select the platform that will be used
# if MCU is not selected, choose wismote as default
ifndef MCU
MCU = msp430f5437
PLATFORM = unkown
endif
ifeq ($(MCU),msp430f5437)
PLATFORM = wismote
endif
ifeq ($(MCU),msp430f2617)
PLATFORM = z1
endif
ifeq ($(MCU),msp430f1611)
PLATFORM = sky
endif

#$(info $$APPLICATION is [${APPLICATION}]) ## Debug includes
#pause


## Compiler target
TARGET = $(PROJECT_NAME)$(APPLICATION)

############## files ##################
## Include Directories
INCLUDES = 	-I"$(PROJECT_DIR)" \
			-I"$(PROJECT_DIR)/hal/drivers" \
			-I"$(PROJECT_DIR)/hal" \
			-I"$(PROJECT_DIR)/brtos" \
			-I"$(PROJECT_DIR)/brtos/includes" \
			-I"$(PROJECT_DIR)/uNET" \
			-I"$(PROJECT_DIR)/uNET/pcg-rand" \
			-I"$(PROJECT_DIR)/uNET/radio" \
			-I"$(PROJECT_DIR)/uNET/radio/cc2520" \
			-I"$(PROJECT_DIR)/config" \
			-I"$(PROJECT_DIR)/debug" \
			-I"$(PROJECT_DIR)/app" \
			-I"$(PROJECT_DIR)/app/terminal" \
			-I"$(PROJECT_DIR)/cooja" \
			-I"$(PROJECT_DIR)/lib" \
			-I"$(LIBRARY)" \
			-I"$(LIBRARY2)" \
			-I"$(LIBRARY3)"
			
#$(info $$INCLUDES is [${INCLUDES}]) ## Debug includes

## Souces files that will be compiled
SOURCES = app/main.c \
		  hal/drivers.c \
		  hal/drivers/mcu.c \
		  hal/drivers/leds.c \
		  hal/drivers/uart.c \
		  hal/drivers/spi.c \
		  hal/HAL.c \
		  cooja/cooja.c \
		  brtos/BRTOS.c \
		  brtos/mbox.c \
		  brtos/mutex.c \
		  brtos/OSInfo.c \
		  brtos/OSTime.c \
		  brtos/queue.c \
		  brtos/semaphore.c \
		  brtos/stimer.c \
		  uNET/ieee802154.c \
		  uNET/link.c \
		  uNET/node.c \
		  uNET/packet.c \
		  uNET/transport.c \
		  uNET/trickle.c \
		  uNET/unet_core.c \
		  uNET/unet_router.c \
		  uNET/pcg-rand/pcg_basic.c \
		  uNET/radio/cc2520/cc2520.c \
		  uNET/radio/radio_null.c \
		  app/tasks.c \
		  app/benchmark.c \
		  lib/stdio.c \
		  lib/random.c \
		  debug/debugdco.c

############## flags ##################
GFLAGS	 = -mlarge -mcode-region=either # general flags -Wall
CFLAGS   = -mmcu=$(MCU) $(GFLAGS) -gstabs+ -Os -Wall -Wunused -fno-builtin-fprintf -fno-builtin-printf -fno-builtin-putc $(INCLUDES) $(DEFINES) # "-g -> essa opção apresenta erros ao ler o arquivo .elf pelo mspsim"
ASFLAGS  = -mmcu=$(MCU) $(GFLAGS) -x assembler-with-cpp -w,-gstabs+
LDFLAGS  = -mmcu=$(MCU) $(GFLAGS) -Wl,-Map=$(TARGET).map -L$(LIBRARY)

#######################################

# the file which will include dependencies
DEPENDS = $(SOURCES:.c=.d)
# all the object files
OBJECTS = $(SOURCES:.c=.o)
# all assembly files
ifeq ($(GENERATE_ASSEMBLY),TRUE)
ASSEMBLYS = $(SOURCES:.c=.s)
else
ASSEMBLYS :=
endif
# create a output folder
##$(info creating outputs folder: [${MKDIR}] [${OUTPUTS}] 2> [${NULL}]) ## Debug includes

## Compile
all: $(TARGET).elf $(TARGET).hex $(TARGET).txt 
$(TARGET).elf: $(OBJECTS) $(ASSEMBLYS)
## Create .elf file
	echo "Linking $@"
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
## Create platform binary	
	$(COPY) $(TARGET).elf $(TARGET).$(PLATFORM)
	$(MOVE) $(TARGET).$(PLATFORM) $(BIN)
## Move binaries to specific folder
#	$(MKDIR) $(OUTPUTS) 2> $(NULL)
	$(MOVE) $(OBJECTS) $(OUTPUTS)
ifeq ($(GENERATE_ASSEMBLY),TRUE)
	$(MOVE) $(ASSEMBLYS) $(OUTPUTS)
endif
	$(MOVE) $(DEPENDS) $(OUTPUTS)
## Show code size
	echo
	echo "========================================================"
	echo "Project: $(TARGET)"
	echo "Platform: $(PLATFORM) $(MCU)"
	echo ">>>> Size of Firmware <<<<<"
	$(SIZE) $(TARGET).elf
	echo "========================================================"
	echo
%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@
%.txt: %.hex
	$(MAKETXT) -O $@ -TITXT $< -I
	$(MOVE) $(TARGET).txt $(BIN)
	$(MOVE) $(TARGET).hex $(BIN)
	$(MOVE) $(TARGET).elf $(BIN)
	$(MOVE) $(TARGET).map $(BIN)
%.o: %.c
	echo "Compiling $<"
	$(CC) $(INCLUDES) -c $(CFLAGS) -o $@ $<

ifeq ($(GENERATE_ASSEMBLY),TRUE)
%.s: %.c
	echo "Assembling $<"
	$(CC) $(INCLUDES) $(CFLAGS) -masm-hex -S -o $@ $<
endif
# rule for making assembler source listing, to see the code
%.lst: %.c
	$(CC) -c $(CFLAGS) -Wa,-anlhd $< > $@
# include the dependencies unless we're going to clean, then forget about them.
ifneq ($(MAKECMDGOALS), clean)
-include $(DEPENDS)
endif
# dependencies file
# includes also considered, since some of these are our own
# (otherwise use -MM instead of -M)
%.d: %.c
	echo "Generating dependencies $@ from $<"
	$(CC) -M ${CFLAGS} $< >$@
.SILENT:
.PHONY:	clean
clean:
	-$(RM) $(OBJECTS)
	-$(RM) $(TARGET)*.map
	-$(RM) $(TARGET)*.elf $(TARGET)*.hex $(TARGET)*.txt
	-$(RM) $(TARGET).lst
	-$(RM) $(TARGET).$(PLATFORM)
	-$(RM) $(SOURCES:.c=.lst)
	-$(RM) $(ASSEMBLYS)
	-$(RM) $(DEPENDS)
	-$(RM) $(OUTPUTS)/*

## Other dependencies
#-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

