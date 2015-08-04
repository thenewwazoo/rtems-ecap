
#include <bsp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "hw/ecap_defn.h"
#include "hw/pwmss_defn.h"
#include "hw/bbb_mmap.h"
#include "board_init.h"


/* forward declarations */
rtems_task ecap_task(rtems_task_argument ignored);
static void ecap0_handler(void* arg);

rtems_id ecap_task_id;
rtems_name ecap_task_name;

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
    struct PWMSS_regs* pwmss_regs = (struct PWMSS_regs*)PWMSS0_MMIO_BASE;

    printf("ecap task starting\n");

    printf("initializing eCAP 0\n");
    init_ecap0(ecap0_handler, &ecap0_data);

    printf("PWMSS register values = {\n"
           "\tIDVER:\t\t0x%08"PRIx32"\n"
           "\tSYSCONFIG:\t0x%08"PRIx32"\n"
           "\tCLKCONFIG:\t0x%08"PRIx32"\n"
           "\tCLKSTATUS:\t0x%08"PRIx32"\n}"
           "\n",
           pwmss_regs->IDVER, 
           pwmss_regs->SYSCONFIG, 
           pwmss_regs->CLKCONFIG, 
           pwmss_regs->CLKSTATUS
          );
    printf("CONTROL_CONF_ECAP_EVT_CAP:\t0x%08"PRIu32"\n", mmio_read(SOC_CONTROL_REGS+CONTROL_CONF_ECAP_EVT_CAP));

    while (1)
    {
        printf("ECAP0 register values ={\n"
            "\tTSCTR:\t0x%08"PRIx32     "\tCTRPHS:\t0x%08"PRIx32"\n" 
            "\tCAP1:\t0x%08"PRIx32      "\tCAP2:\t0x%08"PRIx32       "\tCAP3:\t0x%08"PRIx32       "\tCAP4:\t0x%08"PRIx32"\n"
            "\tECCTL1:\t0x%04"PRIx16    "\tECCTL2:\t0x%04"PRIx16     "\tECEINT:\t0x%04"PRIx16"\n"
            "\tECFLG:\t0x%04"PRIx16     "\tECCLR:\t0x%04"PRIx16      "\tECFRC:\t0x%04"PRIx16"\n"
            "\tREVID:\t0x%08"PRIx32"\n"
            "}\n",
            ecap0_data.ecap_regs->TSCTR,  ecap0_data.ecap_regs->CTRPHS, 
            ecap0_data.ecap_regs->CAP1,   ecap0_data.ecap_regs->CAP2,   ecap0_data.ecap_regs->CAP3,   ecap0_data.ecap_regs->CAP4,
            ecap0_data.ecap_regs->ECCTL1, ecap0_data.ecap_regs->ECCTL2, ecap0_data.ecap_regs->ECEINT,
            ecap0_data.ecap_regs->ECFLG,  ecap0_data.ecap_regs->ECCLR,  ecap0_data.ecap_regs->ECFRC,
            ecap0_data.ecap_regs->REVID);
        sleep(5);
        printf("forcing an interrupt\n");
        ecap0_data.ecap_regs->ECFRC = (EC_FORCE << CEVT1);
        printf("shoulda got something\n");
        printf("ecap0_data->int_flags:\t0x%08x\n", ecap0_data.int_flags);
        printf(" that was interrupt number %lu\n", ecap0_data.num_intr);
    }

}


/* interrupt handlers */

    static struct eCAP_data* ecap_data;

static void ecap0_handler(void* arg)
{
    ecap_data = (struct eCAP_data*)arg;

    ecap_data->int_flags = ecap_data->ecap_regs->ECFLG;
    ecap_data->num_intr++;
    ecap_data->ecap_regs->ECCLR = 0xFF;
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_TASKS 2

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
