##
## RTEMS RKI makefile
##

##
## paths for the RTEMS tools and RTEMS BSP
##   my paths are the same since I install the tools and
##   bsp in the same directory
##
RTEMS_TOOL_BASE := /home/jcmarsh/research/rtems/
RTEMS_BSP_BASE := /home/jcmarsh/research/rtems/src/b-arm/

VPATH := ./
VPATH += ../../../rki
VPATH += ../../../static-loader/libraries/loadstaticload

INCLUDE_PATH := -I./
INCLUDE_PATH += -I../../../osal/src/os/inc
INCLUDE_PATH += -I../../../rki/inc
INCLUDE_PATH += -I../../../static-loader/inc

##
## Architecture Definitions
##

##
## Select your BSP here 
##
BSP            := xilinx_zynq_zybo
ARCH           := arm-rtems4.11
PREFIX         = $(RTEMS_TOOL_BASE)
RTEMS_PREFIX   = $(RTEMS_BSP_BASE)
RTEMS_ARCH_LIB = $(RTEMS_PREFIX)/$(ARCH)/$(BSP)/lib

##
## define build products
##
EXE_TARGET       = rki.elf
BINARY_TARGET    = rki.bin
TAR_IMAGE        = tarfile.o
LINKSCRIPT       = linkcmds

##
## Linker flags that are needed
##
LDFLAGS ?= -march=armv7-a -mthumb -mfpu=neon -mfloat-abi=hard -mtune=cortex-a7 
LDFLAGS += -pipe -B$(RTEMS_ARCH_LIB) -specs bsp_specs -qrtems $(WARNINGS) 
LDFLAGS += -qnolinkcmds -Wl,-T$(LINKSCRIPT)
LDFLAGS += -Wl,-relax

##
## Compiler Architecture Switches
##
# ARCH_OPTS := -mcpu=v8 -D__SPARC__  
ARCH_OPTS ?= -march=armv7-a -mthumb -mfpu=neon -mfloat-abi=hard -mtune=cortex-a7 -D__ARM__  
ARCH_OPTS += -pipe -B$(RTEMS_ARCH_LIB) -specs bsp_specs -qrtems
#ARCH_OPTS += -mlongcall # if needed, should be -mlong-calls

WARNINGS	= -Wall
DEBUG_OPTS	 = -g -O2


##
## Objects to build
##
OBJS = init.o local_shell_cmds.o filesys.o deps.o

##
## Optional objects in src directory
##
OBJS += ramdisk.o

##
## RTEMS examples / demos
##
OBJS += task_cmd.o
OBJS += hello_cmd.o
OBJS += dhrystone_cmd.o
OBJS += whetstone_cmd.o
OBJS += benchmarks.o

##
## Optional modules
##
##
## Static loader with decompression support
##
OBJS += loadstaticloadfile.o
OBJS += LzmaDec.o

##
## Libraries to link in
##
LIBS = -lm 

##
## Optional libs depending on the features needed
##
# LIBS += -lftpd
# LIBS += -ltelnetd
# LIBS += -lnfs

##
## Extra Cflags for Assembly listings, etc.
##
LIST_OPTS    = -Wa,-a=$*.lis

##
## General gcc options that apply to compiling and dependency generation.
##
COPTS=$(LIST_OPTS) $(ARCH_OPTS) $(WARNINGS) $(INCLUDE_PATH)  -I.

##
## Extra defines and switches for assembly code
##
ASOPTS = -P -xassembler-with-cpp

####################################################
## Host Development System and Toolchain defintions
##
## Host OS utils
##
RM=rm -f
CP=cp
MV=mv
CD=cd
TAR=tar
CAT=cat
MKDIR=mkdir
LS=ls

##
## Compiler tools
##
COMPILER   = $(ARCH)-gcc
ASSEMBLER  = $(ARCH)-gcc
LINKER	   = $(ARCH)-ld
AR	   = $(ARCH)-ar
NM         = $(ARCH)-nm
OBJCOPY    = $(ARCH)-objcopy
SIZE       = $(ARCH)-size

###############################################################################################
##
## Build Targets
##

##
## The default "make" target is the subsystem object module.
##
default::$(EXE_TARGET)

##
## Compiler rule
##
.c.o:
	$(COMPILER)  $(COPTS) $(DEBUG_OPTS)  -c -o $@ $<

##
## Assembly Code Rule
##
.s.o:
	$(COMPILER) $(ASOPTS) $(COPTS) $(DEBUG_OPTS)  -c -o $@ $<

##
## Build Tar image
##
$(TAR_IMAGE)::
	$(CD) rootfs; $(TAR) cf ../tarfile $(shell $(LS) rootfs)
	$(LINKER) -r --noinhibit-exec -o $(TAR_IMAGE) -b binary tarfile

##
## Build EEFS Image
##
#eefs.img.o: eefs.img
#	$(LINKER) -r --noinhibit-exec -o eefs.img.o -b binary eefs.img

## 
## Dummy rule to generate a 512KB eefs.img file
## This solves the "chicken vs. egg" issue of the kernel needing to link in an EEFS, but
## the apps in the EEFS image need the kernel..
## 
#eefs.img:
#	dd if=/dev/zero of=eefs.img bs=1k count=512

##
## Link Rule to make the final executable image
## add symtab.o for symbol table
$(EXE_TARGET): $(OBJS)  $(TAR_IMAGE) 
	$(COMPILER) $(DEBUG_FLAGS) $(LDFLAGS) -o $(EXE_TARGET)  $(OBJS) $(TAR_IMAGE) $(LIBS)
	$(OBJCOPY) -O binary --strip-all $(EXE_TARGET) $(BINARY_TARGET)
	$(SIZE) $(EXE_TARGET)

##
## Make clean rule
##
clean::
	-$(RM) $(OBJS) $(EXE_TARGET) $(BINARY_TARGET) 
	-$(RM) $(TAR_IMAGE) tarfile
	-$(RM) *.lis
	-$(RM) *.img
