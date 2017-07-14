###############################################################################
## compiler-opts.mak - compiler definitions and options for building the cFE 
##
## Target: ARM RTEMS 4.11
##
## Modifications:
##
###############################################################################

##
## Warning Level Configuration
##
## WARNINGS=-Wall -ansi -pedantic -Wstrict-prototypes
WARNINGS	= -Wall

SYSINCS =

##
## RTEMS Configuration
##
RTEMS_VER=4.11
RTEMS_BASE=/home/jcmarsh/research/$(RTEMS_VER)
RTEMS_BSP=arm-rtems$(RTEMS_VER)/xilinx_zynq_zybo
RTEMS_BSP_DIR=$(RTEMS_BASE)/$(RTEMS_BSP)

##
## Target Defines for the OS, Hardware Arch, etc..
##
TARGET_DEFS=-D$(OS) -DBUILD=$(BUILD) -D_REENTRANT -D_EMBED_ -D_GNU_SOURCE
TARGET_DEFS+=--pipe -B$(RTEMS_BSP_DIR)/lib/ -specs bsp_specs -qrtems
TARGET_DEFS+=-mlong-calls -std=gnu99 -O0 -g
#TARGET_DEFS = -D_RTEMS_OS_ -DOS_HWARCH=$(HWARCH) -D$(OS) -D_EMBED_

##
## Endian Defines
##
ENDIAN_DEFS=-D_EL -DENDIAN=_EL -DSOFTWARE_LTTILE_BIT_ORDER

##
## Compiler Architecture Switches ( double check arch switch -m52xx, m523x etc.. )
##
ARCH_OPTS=-march=armv7-a -mthumb -mfpu=neon -mfloat-abi=hard -mtune=cortex-a9

##
## Application specific compiler switches
##
ifeq ($(BUILD_TYPE),CFE_APP)
   APP_COPTS =
   APP_ASOPTS   =
else
   APP_COPTS =
   APP_ASOPTS   =
endif

##
## Extra Cflags for Assembly listings, etc.
##
LIST_OPTS    = -Wa,-a=$*.lis

##
## gcc options for dependancy generation
## 
COPTS_D = $(APP_COPTS) $(ENDIAN_DEFS) $(TARGET_DEFS) $(ARCH_OPTS) $(SYSINCS) $(WARNINGS)

## 
## General gcc options that apply to compiling and dependency generation.
##
COPTS=$(LIST_OPTS) $(COPTS_D)

##
## Extra defines and switches for assembly code
##
ASOPTS = $(APP_ASOPTS) -P -xassembler-with-cpp 

##---------------------------------------------------------
## Application file extention type
## This is the defined application extention.
## Known extentions: Mac OS X: .bundle, Linux: .so, RTEMS:
##   .s3r, vxWorks: .o etc.. 
##---------------------------------------------------------
APP_EXT = o

####################################################
## Host Development System and Toolchain defintions
##
## Host OS utils
##
RM=rm -f
CP=cp

##
## Compiler tools
##
CROSS_COMPILE=arm-rtems$(RTEMS_VER)-
COMPILER=$(CROSS_COMPILE)gcc
ASSEMBLER=$(CROSS_COMPILE)as
LINKER=$(CROSS_COMPILE)ld
AR=$(CROSS_COMPILE)ar
NM=$(CROSS_COMPILE)nm
SIZE=$(CROSS_COMPILE)size
OBJCOPY=$(CROSS_COMPILE)objcopy
OBJDUMP=$(CROSS_COMPILE)objdump
TABLE_BIN=elf2cfetbl
