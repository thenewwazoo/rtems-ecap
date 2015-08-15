
#include <assert.h>
#include <bsp.h>
#include <inttypes.h>
#include <rtems/stackchk.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw/bbb_mmap.h"
#include "hw/ecap_defn.h"
#include "hw/gpio_defn.h"

#include "ecap.h"
#include "gpio.h"
#include "system_clocks.h"

#include "debug.h"

/* forward declarations for tasks */
rtems_task ecap_task(rtems_task_argument arg);
rtems_task strober(rtems_task_argument arg);

/* forward declarations for interrupt handlers */
static void ecap_handler(void* arg);

/* function definitions */

void start_tasks()
{

    /* things in which to put other things */
    rtems_id   ecap_task_id;
    rtems_name ecap_task_name;
    struct eCAP_data ecap0_data;
    rtems_id ecap0_sem;
    rtems_name ecap0_sem_name;

    rtems_id   strober_task_id;
    rtems_name strober_task_name;

    rtems_status_code ret;

    /* create the tasks we'll later start */
    ecap_task_name = rtems_build_name('C', 'A', 'P', '0');
    ret = rtems_task_create(
            ecap_task_name,
            1,
            RTEMS_MINIMUM_STACK_SIZE,
            RTEMS_DEFAULT_MODES,
            RTEMS_DEFAULT_ATTRIBUTES,
            &ecap_task_id
            );
    assert(ret == RTEMS_SUCCESSFUL);

    strober_task_name = rtems_build_name('S', 'T', 'R', 'B');
    ret = rtems_task_create(
            strober_task_name,
            1,
            RTEMS_MINIMUM_STACK_SIZE,
            RTEMS_DEFAULT_MODES,
            RTEMS_DEFAULT_ATTRIBUTES,
            &strober_task_id
            );
    assert(ret == RTEMS_SUCCESSFUL);

    /* start the tasks we've created */

    /***********************/
    /* ECAP0 consumer task */
    /***********************/

    ecap0_sem_name = rtems_build_name('S', 'E', 'M', '0');
    ret = rtems_semaphore_create(
            ecap0_sem_name,
            0,
            RTEMS_DEFAULT_ATTRIBUTES,
            1,
            &ecap0_sem);
    assert(ret == RTEMS_SUCCESSFUL);
    ecap0_data.intr_sem_id = &ecap0_sem;

    /* start the ecap0 module and register the interrupt handler */
    init_ecap(0, ecap_handler, &ecap0_data);

    ret = rtems_task_start( ecap_task_id, ecap_task, (rtems_task_argument)&ecap0_data);
    assert(ret == RTEMS_SUCCESSFUL);

    /***************************/
    /* ECAP0 pin strobing task */
    /***************************/

    /* Configure gpio1_17, which lives on control_conf_gpmc_a1 */
    mux_pin(CONTROL_CONF_GPMC_A1_OFFSET, CONTROL_CONF_MUXMODE(7));

    /* set gpio1_17 to be an output */
    gpio_setdirection(1, 17, false);

    ret = rtems_task_start( strober_task_id, strober, (rtems_task_argument)NULL);
    assert(ret == RTEMS_SUCCESSFUL);

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


/* task definitions */

rtems_task ecap_task(rtems_task_argument arg)
{
    struct eCAP_data* ecap_data = (struct eCAP_data*)arg;
    rtems_status_code ret;

    while (1)
    {
        ret = rtems_semaphore_obtain(*(ecap_data->intr_sem_id), RTEMS_DEFAULT_OPTIONS, 0);
        assert(ret == RTEMS_SUCCESSFUL);
        uint32_t timer = ecap_data->ecap_regs->TSCTR;
        uint32_t delay;
        switch ((ecap_data->int_flags >> 1) & 15)
        {
            case 1:
                delay = timer - ecap_data->ecap_regs->CAP1;
                break;
            case 2:
                delay = timer - ecap_data->ecap_regs->CAP2;
                break;
            case 4:
                delay = timer - ecap_data->ecap_regs->CAP3;
                break;
            case 8:
                delay = timer - ecap_data->ecap_regs->CAP4;
                break;
            default:
                continue;
        }
        printf("\rflags:\t0x%08x\tnum: %lu\tdelay: 0x%08"PRIx32"\n", 
                ecap_data->int_flags, 
                ecap_data->num_intr, 
                delay);
    }

}

rtems_task strober(rtems_task_argument arg)
{
    rtems_status_code ret;

    printf("Pin strobe task starting on gpio1_17.\n");
    while (1)
    {
        ret = rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 1);
        if (!rtems_is_status_successful(ret)) { printf("strober failed to sleep!\n"); }
        gpio_out((gpio_module)1, (gpio_pin)17, true);
        ret = rtems_task_wake_after(rtems_clock_get_ticks_per_second() / 10);
        if (!rtems_is_status_successful(ret)) { printf("Failed to keep high!\n"); }
        gpio_out((gpio_module)1, (gpio_pin)17, false);
    }
}

/* interrupt handlers */

static void ecap_handler(void* arg)
{
    struct eCAP_data* ecap_data = (struct eCAP_data*)arg;

    ecap_data->int_flags = ecap_data->ecap_regs->ECFLG;
    ecap_data->num_intr++;
    ecap_data->ecap_regs->ECCLR = 0xFF;
    rtems_semaphore_release(*(ecap_data->intr_sem_id));

}

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
