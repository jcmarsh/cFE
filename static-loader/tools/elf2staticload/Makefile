##############################################################################
## GNU Makefile for building UTF unit tests

#
# Supported MAKEFILE targets:
#   clean - deletes object files, executables, output files, and gcov files
#   all   - makes utf_test_runner.exe
#

#
# VPATH specifies the search paths for source files outside of the current directory.  Note that
# all object files will be created in the current directory even if the source file is not in the 
# current directory.
#
VPATH := .

#
# INCLUDES specifies the search paths for include files outside of the current directory.  
# Note that the -I is required. 
#
INCLUDES := -I.
INCLUDES += -I../../inc
#
# FLIGHT_OBJS specifies flight software object files.
#
FLIGHT_OBJS := cmdlineopt.o
FLIGHT_OBJS += elffile.o
FLIGHT_OBJS += LzFind.o
FLIGHT_OBJS += LzmaEnc.o

###############################################################################

COMPILER=gcc
LINKER=gcc

#
# Compiler and Linker Options
#

WARNINGS = -Wall -W -ansi -Wstrict-prototypes -Wundef
# WARNINGS = -Wall -W -ansi -Werror -Wstrict-prototypes -Wundef
DEBUGGER = -g

COPT = $(WARNINGS) $(DEBUGGER) -D_ix86_ -m32 

LOPT = -m32

###############################################################################
## Rule to make the specified TARGET
##
%: %.o
	$(LINKER) $(LOPT) $^ -o $*

###############################################################################
##  "C" COMPILER RULE
##
%.o: %.c
	$(COMPILER) -c $(COPT) $(INCLUDES) $<

##############################################################################
##

all:elf2staticload

elf2staticload: elf2staticload.o $(FLIGHT_OBJS)

clean ::
	rm -f *.o *.exe

# end of file
