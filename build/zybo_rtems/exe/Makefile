##
## RTEMS static link makefile
##

# DEBUG_OPTS      = -g -O0
DEBUG_OPTS      = -O2

##
## define build products 
##
CFE_CORE_TARGET  = cfe-core.slf
CFE_CORE_IMAGE   = cfe-core.elf
CFE_CORE_OBJ     = ../cfe/link/cfe-core.o 

#
# The Rtems Kernel image is kept in a central place. 
# It is supposed to be the latest working kernel
#
RTEMS_KERNEL_IMAGE = ../rtems/rki.elf

#
# The link script for an app
# 
RTEMS_LINK_SCRIPT  = linkcmds

##
## Architecture Definitions
## 
ARCH  = arm-rtems4.11

##
## Memory locations for the cFE Core and apps:
##

CFE_CORE_ADDRESS = 0x01000000

SCH_LAB_ADDRESS    = 0x03100000
SCH_LAB_ENTRYPT    = SCH_Lab_AppMain

#BUZZ_ADDRESS       = 0x03200000
#BUZZ_ENTRYPT       = TO_Lab_AppMain

PING_ADDRESS       = 0x03300000
PING_ENTRYPT       = PING_AppMain

PONG_ADDRESS       = 0x03400000
PONG_ENTRYPT       = PONG_AppMain

##
## Linker flags that are needed
##
LDFLAGS = -Map=map.txt
CFE_LD_FLAGS = -R$(RTEMS_KERNEL_IMAGE) -ePOSIX_Init -T$(RTEMS_LINK_SCRIPT) -Map=map.txt
APP_LD_FLAGS = -R$(RTEMS_KERNEL_IMAGE) -R$(CFE_CORE_IMAGE) -T$(RTEMS_LINK_SCRIPT) -Map=map.txt

####################################################
## Host Development System and Toolchain defintions
##
## Host OS utils
##
LS=ls
RM=rm -f
CP=cp
MV=mv
CD=cd
TAR=tar
CAT=cat
CHMOD=chmod
CHGRP=chgrp

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

ELF2STATICLOAD = ../../../static-loader/tools/elf2staticload/elf2staticload
###############################################################################################
## 
## Build Targets
##  

##
## The default "make" target is the subsystem object module.
## 
default:: cfe apps imfsfilesys

cfe::$(CFE_CORE_TARGET)

#libs: cfs_lib.slf 
libs:

apps: sch_lab.slf ping.slf pong.slf

sizes: cfe-core.size sch_lab.size ping.size pong.size

imfsfilesys: cfe apps
	cp cfe-core.slf ../rtems/rootfs/cf
	cp sch_lab.slf ../rtems/rootfs/cf
	cp ping.slf ../rtems/rootfs/cf
	cp pong.slf    ../rtems/rootfs/cf
	cp cfe_es_startup.scr ../rtems/rootfs/cf
	echo "test" > test.tbl
	cp *.tbl ../rtems/rootfs/cf

##
## The filesys target is the EEPROM file system image.
## This EEPROM File system gets loaded into one bank of EEPROM. 
##
eefilesys: cfe apps
	 geneepromfs -f -s 0x80000 --endian=big eefs-contents.txt ../rtems/eefs.img
##
## cFE Core Link Rule
## 
$(CFE_CORE_TARGET): $(CFE_CORE_OBJ) $(RTEMS_KERNEL_IMAGE)
	$(LINKER) $(CFE_LD_FLAGS) -Ttext $(CFE_CORE_ADDRESS) -o $(CFE_CORE_IMAGE) $(CFE_CORE_OBJ) 
	$(ELF2STATICLOAD) --object_name="cFE Core"  --entry_point_name="POSIX_Init"  $(CFE_CORE_IMAGE) $(CFE_CORE_TARGET)

cfe-core.size: $(CFE_CORE_TARGET)
	$(SIZE) $(CFE_CORE_IMAGE)
	@echo ------------------------------------------------------------------

##
## Library Link Rules
##
#cfs_lib.slf: $(CFE_CORE_TARGET) ../cfs_lib/cfs_lib.o
#	$(LINKER) $(APP_LD_FLAGS) -Ttext $(CFS_LIB_ADDRESS) -e$(CFS_LIB_ENTRYPT) -o cfs_lib.elf ../cfs_lib/cfs_lib.o 
#	$(ELF2STATICLOAD)  --object_name="CFS_LIB"  --entry_point_name="CFS_LibInit"  cfs_lib.elf cfs_lib.slf

#cfs_lib.size: cfs_lib.slf
#	$(SIZE) cfs_lib.elf
#	@echo ------------------------------------------------------------------

## 
## CFS Application link rules
## 
sch_lab.slf: $(CFE_CORE_TARGET) ../sch_lab/sch_lab.o
	$(LINKER) $(APP_LD_FLAGS) -Ttext $(SCH_LAB_ADDRESS) -e$(SCH_LAB_ENTRYPT) -o sch_lab.elf ../sch_lab/sch_lab.o
	$(ELF2STATICLOAD)  --object_name="SCH_LAB"   --entry_point_name="SCH_Lab_AppMain"  sch_lab.elf sch_lab.slf

sch_lab.size: sch_lab.slf
	$(SIZE) sch_lab.elf
	@echo ------------------------------------------------------------------

ping.slf: $(CFE_CORE_TARGET) ping.slf  ../ping/ping_app.o
	$(LINKER) $(APP_LD_FLAGS) -Ttext $(PING_ADDRESS) -e$(PING_ENTRYPT) -o ping.elf ../ping/ping_app.o
	$(ELF2STATICLOAD)  --object_name="PING"  --entry_point_name="PING_AppMain"  ping.elf ping.slf

ping.size: ping.slf
	$(SIZE) ping.elf
	@echo ------------------------------------------------------------------

pong.slf: $(CFE_CORE_TARGET) pong.slf  ../pong/pong_app.o
	$(LINKER) $(APP_LD_FLAGS) -Ttext $(PONG_ADDRESS) -e$(PONG_ENTRYPT) -o pong.elf ../pong/pong_app.o
	$(ELF2STATICLOAD)  --object_name="PONG"  --entry_point_name="PONG_AppMain"  pong.elf pong.slf

pong.size: pong.slf
	$(SIZE) pong.elf
	@echo ------------------------------------------------------------------

##
## Make clean rule
##
clean::
	-$(RM) *.slf
	-$(RM) *.elf
	-$(RM) *.o
	-$(RM) *.s3r
	-$(RM) *.img
	-$(RM) map.txt
