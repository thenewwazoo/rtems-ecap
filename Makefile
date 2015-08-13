#  RTEMS_MAKEFILE_PATH is typically set in an environment variable

PGM=${ARCH}/beaglebone.exe

# optional managers required
MANAGERS=all

# C source names
CSRCS = main.c board_init.c debug.c
COBJS = $(CSRCS:%.c=${ARCH}/%.o)

CFLAGS_OPTIMIZE_V = -g

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

OBJS= $(COBJS) $(CXXOBJS) $(ASOBJS)

all:    ${ARCH} $(PGM)

$(PGM): $(OBJS)
	$(make-exe)
