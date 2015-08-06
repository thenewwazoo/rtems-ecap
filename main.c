
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


/* things that will do things later */
rtems_task ecap_task(rtems_task_argument ignored);
static void ecap0_handler(void* arg);
rtems_task ecap0_generator(rtems_task_argument arg);
rtems_task strober(rtems_task_argument arg);


/* things that do things */
void start_tasks()
{

    /* things in which to put other things */
    rtems_id   ecap_task_id;
    rtems_name ecap_task_name;

    rtems_id   ecap_gen_task_id;
    rtems_name ecap_gen_task_name;

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

    ecap_gen_task_name = rtems_build_name('E', 'C', '0', 'G');
    ret = rtems_task_create(
            ecap_gen_task_name,
            1,
            RTEMS_MINIMUM_STACK_SIZE,
            RTEMS_DEFAULT_MODES,
            RTEMS_DEFAULT_ATTRIBUTES,
            &ecap_gen_task_id
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
    ret = rtems_task_start( ecap_task_id, ecap_task, (rtems_task_argument)NULL);
    assert(ret == RTEMS_SUCCESSFUL);
    
    ret = rtems_task_start( ecap_gen_task_id, ecap0_generator, (rtems_task_argument)NULL);
    assert(ret == RTEMS_SUCCESSFUL);

    ret = rtems_task_start( strober_task_id, strober, (rtems_task_argument)NULL);
    assert(ret == RTEMS_SUCCESSFUL);

    /* delete the startup task now the we're running */
    ret = rtems_task_delete(RTEMS_SELF);
    assert(ret == RTEMS_SUCCESSFUL);

}

rtems_task Init(rtems_task_argument arg)
{
    printf( "init entered\n" );
    init_board();

    start_tasks();

    printf( "goodbye\n" );
    exit( 0 );
}


/* task definitions */

rtems_task ecap_task(rtems_task_argument arg)
{
    struct eCAP_data ecap0_data;
    rtems_id ecap0_sem;
    rtems_name ecap0_sem_name;
    rtems_status_code ret;

    ecap0_sem_name = rtems_build_name('E', 'C', '0', 'S');
    ret = rtems_semaphore_create(
            ecap0_sem_name,
            0,
            RTEMS_DEFAULT_ATTRIBUTES,
            1,
            &ecap0_sem);
    assert(ret == RTEMS_SUCCESSFUL);

    printf("ecap task starting\n");
    printf("initializing eCAP0\n");
    init_ecap0(ecap0_handler, &ecap0_data, &ecap0_sem);

    while (1)
    {
        printf("nom");
        ret = rtems_semaphore_obtain(ecap0_sem, RTEMS_DEFAULT_OPTIONS, 0);
        printf(".\n");

        printf(" ecap0_data->int_flags:\t0x%08x\n", ecap0_data.int_flags);
        printf(" that was interrupt number %lu\n", ecap0_data.num_intr);

        print_ecap(&ecap0_data);
        rtems_stack_checker_report_usage();
    }

}

rtems_task strober(rtems_task_argument arg)
{
    rtems_status_code ret;

    printf("Strober bout ta strobe every 2 seconds\n");
    while (1)
    {
        ret = rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 2);
        printf("stro");
        gpio_out((gpio_module)1, (gpio_pin)17, true);
        printf("b!\n");
    }
}

rtems_task ecap0_generator(rtems_task_argument arg)
{
    struct eCAP_regs* ecap_regs = (struct eCAP_regs*)ECAP0_REGS_BASE;
    rtems_status_code ret;

    printf("Interruptor beginning; interrupts every 3 seconds\n");
    while (1) {
        ret = rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 3);
        printf("moo");
        ecap_regs->ECFRC = (EC_FORCE << CEVT1);
        RTEMS_COMPILER_MEMORY_BARRIER();
        printf("!\n");
    }
}


/* interrupt handlers */


static void ecap0_handler(void* arg)
{
    struct eCAP_data* ecap_data = (struct eCAP_data*)arg;

    ecap_data->int_flags = ecap_data->ecap_regs->ECFLG;
    ecap_data->num_intr++;
    ecap_data->ecap_regs->ECCLR = 0xFF;
    printk("fly");
    rtems_semaphore_release(*(ecap_data->intr_sem));
    printk("?\n");

}

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
