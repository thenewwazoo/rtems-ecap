
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
#include "stimulator.h"

#include "debug.h"

rtems_task display(rtems_task_argument arg);

void start_tasks()
{

    /* things in which to put other things */
    rtems_id locator_task_id;
    struct eCAP_data* ecap0_data = (struct eCAP_data*)malloc(sizeof(struct eCAP_data));
    struct locator_spec* l1_spec = (struct locator_spec*)malloc(sizeof(struct locator_spec));

    rtems_id strober_task_id;
    uint8_t map[] = { 1, 1, 2 };
    struct stim_spec* stim_data = (struct stim_spec*)malloc(sizeof(struct stim_spec));

    rtems_id display_task_id;

    rtems_status_code ret;

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

    ret = rtems_task_create(
            rtems_build_name('S', 'T', 'R', 'B'),
            2,
            RTEMS_MINIMUM_STACK_SIZE,
            RTEMS_DEFAULT_MODES,
            RTEMS_DEFAULT_ATTRIBUTES,
            &strober_task_id
            );
    assert(ret == RTEMS_SUCCESSFUL);

    ret = rtems_task_create(
            rtems_build_name('D', 'I', 'S', 'P'),
            3,
            RTEMS_MINIMUM_STACK_SIZE,
            RTEMS_DEFAULT_MODES,
            RTEMS_DEFAULT_ATTRIBUTES,
            &display_task_id);
    assert(ret == RTEMS_SUCCESSFUL);

    /* start the tasks we've created */

    /***********************/
    /* ECAP0 consumer task */
    /***********************/

    l1_spec->locator_id = '1';
    l1_spec->ecap_module = 0;
    l1_spec->ecap_data = ecap0_data;
    l1_spec->timeval = 0;

    ret = rtems_task_start(
            locator_task_id,
            locator_task,
            (rtems_task_argument)l1_spec);
    assert(ret == RTEMS_SUCCESSFUL);

    /***************************/
    /* ECAP0 pin strobing task */
    /***************************/

    stim_data->module = (gpio_module)1;
    stim_data->pin = (gpio_pin)17;
    stim_data->interval = rtems_clock_get_ticks_per_second() / 60;
    stim_data->num_teeth = 1;
    stim_data->map = map;

    /* Configure gpio1_17, which lives on control_conf_gpmc_a1 */
    mux_pin(CONTROL_CONF_GPMC_A1_OFFSET, CONTROL_CONF_MUXMODE(7));

    /* set gpio1_17 to be an output */
    gpio_setdirection(stim_data->module, stim_data->pin, false);

    ret = rtems_task_start(
            strober_task_id,
            stimulator,
            (rtems_task_argument)stim_data);
    assert(ret == RTEMS_SUCCESSFUL);

    /****************/
    /* Display task */
    /****************/

    ret = rtems_task_start(
            display_task_id,
            display,
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

    gpio_init();

    start_tasks();

    exit( 0 ); /* We never get here, since we delete the init task */
}

rtems_task display(rtems_task_argument arg)
{
    struct locator_spec* loc_spec = (struct locator_spec*)arg;
    struct eCAP_data* ecap_data = loc_spec->ecap_data;
    uint32_t num_intr = 0;
    uint32_t intr_delta;
    uint32_t time_value = 0;
    uint32_t time_delta;
    uint32_t calc_tps;

    printf("display starting...\n");
    while (1)
    {
        /* sleep */
        rtems_task_wake_after(rtems_clock_get_ticks_per_second());
        intr_delta = ecap_data->num_intr - num_intr;
        num_intr = ecap_data->num_intr;
        time_delta = loc_spec->timeval - time_value;
        time_value = loc_spec->timeval;
        calc_tps = 100000000lu / (time_delta / intr_delta);

        /* print stuff */
        printf("\033[2J");
        printf("intr_delta: %"PRIu32", calc_tps: %"PRIu32"\n", intr_delta, calc_tps);
        printf("num_intr: %"PRIu32", ecap_data->num_intr: %"PRIu32"\n", num_intr, ecap_data->num_intr);
        printf("timeval: %08"PRIx32", time_delta: %"PRIu32"\n", loc_spec->timeval, time_delta);
        rtems_cpu_usage_report();
        rtems_stack_checker_report_usage();
    }
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
