
#include <assert.h>
#include <bsp.h>
#include <pthread.h>
#include <rtems/stackchk.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw/bbb_mmap.h"
#include "hw/ecap_defn.h"
#include "hw/gpio_defn.h"
#include "hw/pwmss_defn.h"
#include "board_init.h"
#include "debug.h"


/* forward declarations for tasks */
rtems_task ecap_task(rtems_task_argument arg);

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

    /* start the tasks we've created */

    /***********************/
    /* ECAP0 consumer task */

    ecap0_sem_name = rtems_build_name('E', 'C', '0', 'S');
    ret = rtems_semaphore_create(
            ecap0_sem_name,
            0,
            RTEMS_DEFAULT_ATTRIBUTES,
            1,
            &ecap0_sem);
    assert(ret == RTEMS_SUCCESSFUL);
    ecap0_data.intr_sem_name = ecap0_sem_name;

    /* start the ecap0 module and register the interrupt handler */
    init_ecap(0, ecap_handler, &ecap0_data);

    ret = rtems_task_start( ecap_task_id, ecap_task, (rtems_task_argument)&ecap0_data);
    assert(ret == RTEMS_SUCCESSFUL);

    /* delete the startup task now that we're running */
    ret = rtems_task_delete(RTEMS_SELF);
    assert(ret == RTEMS_SUCCESSFUL);

}

rtems_task Init(rtems_task_argument arg)
{
    printf( "init entered\n" );
    clocks_init_L3();
    clocks_init_L4();

    gpio_init();

    start_tasks();

    printf( "goodbye\n" );
    exit( 0 );
}


/* task definitions */

rtems_task ecap_task(rtems_task_argument arg)
{
    struct eCAP_data* ecap_data = (struct eCAP_data*)arg;
    rtems_status_code ret;
    rtems_id ecap_sem;

    /* Get a handle by name for the interrupt semaphore */
    ret = rtems_semaphore_ident(ecap_data->intr_sem_name, 0, &ecap_sem);
    if (!rtems_is_status_successful(ret)) {
        printf("Failed to find semaphore id %d, returned error code %s\n",
                (int)ecap_data->intr_sem_name,
                rtems_status_text(ret));
        perror("ecap task exiting\n");
        return;
    }

    while (1)
    {
        ret = rtems_semaphore_obtain(ecap_sem, RTEMS_DEFAULT_OPTIONS, 0);

        printf(" ecap0_data->int_flags:\t0x%08x\n", ecap_data->int_flags);
        printf(" that was interrupt number %lu\n", ecap_data->num_intr);

        print_ecap(ecap_data);
        rtems_stack_checker_report_usage();
    }

}

/* interrupt handlers */

static void ecap_handler(void* arg)
{
    struct eCAP_data* ecap_data = (struct eCAP_data*)arg;
    rtems_id ecap_sem;
    rtems_semaphore_ident(ecap_data->intr_sem_name, 0, &ecap_sem);

    ecap_data->int_flags = ecap_data->ecap_regs->ECFLG;
    ecap_data->num_intr++;
    ecap_data->ecap_regs->ECCLR = 0xFF;
    rtems_semaphore_release(ecap_sem);

}

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
