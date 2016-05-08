#  RTEMS_MAKEFILE_PATH is typically set in an environment variable

PGM=${ARCH}/beaglebone.exe

# optional managers required
MANAGERS=all

# C source names
CSRCS = main.c \
	debug.c \
	ecap.c \
	gpio.c \
	system_clocks.c \
	locator.c
COBJS = $(CSRCS:%.c=${ARCH}/%.o)

XCPPFLAGS += -I../lib/
CFLAGS_OPTIMIZE_V += -g -O3
CFLAGS_OPTIMIZE_V += -mthumb -march=armv7-a -mfpu=neon -mfloat-abi=hard -mtune=cortex-a8

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

OBJS= $(COBJS) $(CXXOBJS) $(ASOBJS)

LD_LIBS += $(LIBMARKOV)
LIBMARKOV = ../lib/markov-localizer/c99_fp/${ARCH}/libmarkov.a

all:    $(LIBMARKOV) ${ARCH} $(PGM)

$(PGM): $(OBJS)
	$(make-exe)
