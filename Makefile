#  RTEMS_MAKEFILE_PATH is typically set in an environment variable

PGM=${ARCH}/beaglebone.exe

# optional managers required
MANAGERS=all

# C source names
CSRCS = main.c debug.c ecap.c gpio.c system_clocks.c
COBJS = $(CSRCS:%.c=${ARCH}/%.o)

CFLAGS_OPTIMIZE_V = -g -O3

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

OBJS= $(COBJS) $(CXXOBJS) $(ASOBJS)

all:    ${ARCH} $(PGM)

$(PGM): $(OBJS)
	$(make-exe)
