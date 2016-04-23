
#include <assert.h>
#include <bsp.h>
#include <inttypes.h>
#include <rtems/cpuuse.h>
#include <rtems/stackchk.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw/bbb_mmap.h"
#include "hw/ecap_defn.h"
#include "hw/gpio_defn.h"

#include "ecap.h"
#include "gpio.h"
#include "locator.h"
#include "system_clocks.h"

#include "debug.h"

rtems_task display(rtems_task_argument arg);

void start_tasks()
{

    /* things in which to put other things */
    rtems_id locator_task_id;
    struct eCAP_data* ecap0_data = (struct eCAP_data*)malloc(sizeof(struct eCAP_data));
    struct locator_spec* l1_spec = (struct locator_spec*)malloc(sizeof(struct locator_spec));

    rtems_status_code ret; /* scratch space */

    /* create the tasks we'll later start */
    ret = rtems_task_create(
            rtems_build_name('C', 'A', 'P', '0'),
            1,
            RTEMS_MINIMUM_STACK_SIZE,
            RTEMS_DEFAULT_MODES,
            RTEMS_DEFAULT_ATTRIBUTES,
            &locator_task_id
            );
    assert(ret == RTEMS_SUCCESSFUL);

    /***********************/
    /* ECAP0 consumer task */
    /***********************/

    l1_spec->ecap_module = 0;
    l1_spec->ecap_data = ecap0_data;

    ret = rtems_task_start(
            locator_task_id,
            locator_task,
            (rtems_task_argument)l1_spec);
    assert(ret == RTEMS_SUCCESSFUL);

    /* done starting tasks, now time to leave */

    /* delete the init task now that we're running */
    ret = rtems_task_delete(RTEMS_SELF);
    assert(ret == RTEMS_SUCCESSFUL);
}

rtems_task Init(rtems_task_argument arg)
{
    printf("Init function entered!\n" );
    printf("ticks per second is %"PRIu32"\n", rtems_clock_get_ticks_per_second());
    clocks_init_L3();
    clocks_init_L4();

    start_tasks();

    exit( 0 ); /* We never get here, since we delete the init task */
}


#define CONFIGURE_MICROSECONDS_PER_TICK 1000
#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
