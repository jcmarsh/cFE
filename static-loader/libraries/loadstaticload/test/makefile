##############################################################################
## GNU Makefile for building UTF unit tests

#
# Supported MAKEFILE targets:
#   clean - deletes object files, executables, output files, and gcov files
#   all   - makes utf_test_runner.exe
#   run   - runs utf_test_runner.exe
#   gcov  - prints a GCOV coverage report (make all, make run, make gcov)
#
# If you are using the source level debugger you will want to disable GCOV.  To disable GCOV you 
# can override the ENABLE_GCOV variable on the command line by setting it to FALSE.  For  
# example "make ENABLE_GCOV=FALSE".
#

#
# VPATH specifies the search paths for source files outside of the current directory.  Note that
# all object files will be created in the current directory even if the source file is not in the 
# current directory.
#
VPATH := .
VPATH += ..

#
# INCLUDES specifies the search paths for include files outside of the current directory.  
# Note that the -I is required. 
#
INCLUDES := -I.
INCLUDES += -I..
INCLUDES += -I../../inc
INCLUDES += -I../../../inc

#
# FLIGHT_OBJS specifies flight software object files.
#
FLIGHT_OBJS := loadstaticloadfile.o
FLIGHT_OBJS += LzmaDec.o

#
# UT_OBJS specifies unit test object files.
#
UT_OBJS := loadstaticloadfile_testcase.o
UT_OBJS += utassert.o
UT_OBJS += uttest.o
UT_OBJS += utlist.o
UT_OBJS += uttools.o

###############################################################################

COMPILER=gcc
LINKER=gcc

#
# Compiler and Linker Options
#
ENABLE_GCOV = TRUE
ifeq ($(ENABLE_GCOV), TRUE)
GCOV_COPT = -fprofile-arcs -ftest-coverage -pg -p
GCOV_LOPT = -pg -p -fprofile-arcs -ftest-coverage -lgcov
endif

WARNINGS = -Wall -W -ansi -Werror -Wstrict-prototypes -Wundef
DEBUGGER = -g

COPT = $(WARNINGS) $(DEBUGGER) $(GCOV_COPT) -D_ix86_ 

LOPT = $(GCOV_LOPT)

###############################################################################
## Rule to make the specified TARGET
##
%.exe: %.o
	$(LINKER) $(LOPT) $^ -o $*.exe

###############################################################################
##  "C" COMPILER RULE
##
%.o: %.c
	$(COMPILER) -c $(COPT) $(INCLUDES) $<

##############################################################################
##

all:loadstaticloadfile_testrunner.exe

loadstaticloadfile_testrunner.exe: loadstaticloadfile_testrunner.o $(UT_OBJS) $(FLIGHT_OBJS)

clean ::
	rm -f *.o *.exe *.out *.gcda *.gcno *.gcov gmon.out

run ::
	loadstaticloadfile_testrunner.exe

gcov ::
	@echo
	@gcov $(FLIGHT_OBJS:.o=.gcda) | sed 'N;s/\n/ /' | \
		sed -n '/File/p' | sed '/ads/d'  | \
		sed 's/ Lines executed:/ /; s/File/gcov:/; s/of//'
	@rm -f *.gcda *.gcno
	@echo

# end of file
