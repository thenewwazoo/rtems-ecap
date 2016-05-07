include $(RTEMS_MAKEFILE_PATH)/Makefile.inc

include $(RTEMS_CUSTOM)
include $(RTEMS_ROOT)/share/rtems4.11/make/directory.cfg

SUBDIRS=lib/markov-localizer/c99_fp src
