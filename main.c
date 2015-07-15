
#include <stdio.h>
#include <stdlib.h>

#include "board_init.h"

void *POSIX_Init(void *argument)
{
  printf( "init entered\n" );
  board_init();
  printf( "goodbye\n" );
  exit( 0 );
}

/* configuration information */

#include <bsp.h>

/* NOTICE: the clock driver is explicitly disabled */
#define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_POSIX_INIT_THREAD_TABLE
#define CONFIGURE_MAXIMUM_POSIX_THREADS 1

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
/* end of file */
