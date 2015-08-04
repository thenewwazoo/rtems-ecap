
#include <bsp.h>
#include <inttypes.h>
#include <pthread.h>
#include <rtems/stackchk.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw/ecap_defn.h"
#include "hw/pwmss_defn.h"
#include "hw/bbb_mmap.h"
#include "board_init.h"


/* things that will do things later */
rtems_task ecap_task(rtems_task_argument ignored);
static void ecap0_handler(void* arg);

/* things in which to put other things */
rtems_id ecap_task_id;
rtems_name ecap_task_name;

/* things that do things */
void start_tasks()
{
    rtems_status_code ret;

    /* eventually, we'll create our threads here */

    ecap_task_name = rtems_build_name('C', 'A', 'P', '0');
    ret = rtems_task_create(
            ecap_task_name,
            1,
            RTEMS_MINIMUM_STACK_SIZE,
            RTEMS_DEFAULT_MODES,
            RTEMS_DEFAULT_ATTRIBUTES,
            &ecap_task_id
            );
    if (ret != RTEMS_SUCCESSFUL) { 
        perror("failed to create ecap task!\n");
        exit(1);
    }

    ret = rtems_task_start(
            ecap_task_id,
            ecap_task,
            (rtems_task_argument)NULL
            );
    if (ret != RTEMS_SUCCESSFUL) { 
        perror("failed to start ecap task!\n");
        exit(1);
    }
    
    ret = rtems_task_delete(RTEMS_SELF);
    if (ret != RTEMS_SUCCESSFUL) { 
        perror("failed to end main task!\n");
        exit(1);
    }
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
    /*struct PWMSS_regs* pwmss_regs = (struct PWMSS_regs*)PWMSS0_MMIO_BASE;*/

    printf("ecap task starting\n");

    printf("initializing eCAP 0\n");
    init_ecap0(ecap0_handler, &ecap0_data);

    uint8_t which = 0;
    while (ecap0_data.num_intr < 9)
    {
        printf("sleeping 1...");sleep(1);printf("\n");
        printf("forcing an interrupt: CAP%1x\n", which+1);
        ecap0_data.ecap_regs->ECFRC = (EC_FORCE << (which+1));
        printf("sleeping 1...");sleep(1);printf("\n");
        which = (which + 1) % 4;
        printf("ecap0_data->int_flags:\t0x%08x\n", ecap0_data.int_flags);
        printf(" that was interrupt number %lu\n", ecap0_data.num_intr);

    }

    rtems_stack_checker_report_usage();
}


/* interrupt handlers */

    static struct eCAP_data* ecap_data;

static void ecap0_handler(void* arg)
{
    ecap_data = (struct eCAP_data*)arg;

    printk("eCAP0 got interrupt!\n");

    printk("ECAP0 register values ={\n"
        "\tTSCTR:\t0x%08"PRIx32     "\tCTRPHS:\t0x%08"PRIx32"\n" 
        "\tCAP1:\t0x%08"PRIx32      "\tCAP2:\t0x%08"PRIx32       "\tCAP3:\t0x%08"PRIx32       "\tCAP4:\t0x%08"PRIx32"\n"
        "\tECCTL1:\t0x%04"PRIx16    "\tECCTL2:\t0x%04"PRIx16     "\tECEINT:\t0x%04"PRIx16"\n"
        "\tECFLG:\t0x%04"PRIx16     "\tECCLR:\t0x%04"PRIx16      "\tECFRC:\t0x%04"PRIx16"\n"
        "\tREVID:\t0x%08"PRIx32"\n"
        "}\n",
        ecap_data->ecap_regs->TSCTR,  ecap_data->ecap_regs->CTRPHS, 
        ecap_data->ecap_regs->CAP1,   ecap_data->ecap_regs->CAP2,   ecap_data->ecap_regs->CAP3,   ecap_data->ecap_regs->CAP4,
        ecap_data->ecap_regs->ECCTL1, ecap_data->ecap_regs->ECCTL2, ecap_data->ecap_regs->ECEINT,
        ecap_data->ecap_regs->ECFLG,  ecap_data->ecap_regs->ECCLR,  ecap_data->ecap_regs->ECFRC,
        ecap_data->ecap_regs->REVID);

    ecap_data->int_flags = ecap_data->ecap_regs->ECFLG;
    ecap_data->num_intr++;
    printk("flg, intr: 0x%04"PRIx16" 0x%04"PRIx16"\n", ecap_data->ecap_regs->ECFLG, ecap_data->num_intr);
    ecap_data->ecap_regs->ECCLR = 0xFF;

}

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
