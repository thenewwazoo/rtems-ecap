#include <assert.h>
#include <bsp.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

#include "hw/ecap_defn.h"

#include "ecap.h"
#include "locator.h"
#include "events.h"

static void ecap_handler(void* arg);

rtems_task locator_task(rtems_task_argument arg)
{
    struct locator_spec* spec = (struct locator_spec*)arg;
    struct eCAP_data* ecap_data = spec->ecap_data;
    rtems_status_code ret;
    rtems_event_set evt_got;

    printf("Locator task starting up\n");

    ecap_data->notify = rtems_task_self();
    ecap_data->num_intr = 0;
    ecap_data->ecap_module = spec->ecap_module;

    /* start the ecap0 module and register the interrupt handler        */
    /* this function populates ecap_data->ecap_regs and ->ecap_event_id */
    init_ecap(ecap_handler, ecap_data);

    printf("Locator task listening for events...\n");
    while (1)
    {
        ret = rtems_event_receive(
                ecap_data->ecap_event_id,
                RTEMS_DEFAULT_OPTIONS,
                RTEMS_NO_TIMEOUT,
                &evt_got);
        assert(ret == RTEMS_SUCCESSFUL);

        switch (ecap_data->int_flags & 0x1E) /* (CEVT4 | CEVT3 | CEVT2 | CEVT1) */
        {
            case 0x02:
                spec->timeval = ecap_data->ecap_regs->CAP1;
                break;
            case 0x04:
                spec->timeval = ecap_data->ecap_regs->CAP2;
                break;
            case 0x08:
                spec->timeval = ecap_data->ecap_regs->CAP3;
                break;
            case 0x10:
                spec->timeval = ecap_data->ecap_regs->CAP4;
                break;
            default: /* CTROVF, so no timestamp to send */
                continue;
        }
    }

}

/* We're going to naively assume we never miss an interrupt here, and
 *  therefore never get more than one CEVTn flag at a time. If we do
 *  miss an interrupt, this will explode horribly.
 *
 * For a proven strategy to handle missed interrupts, see erl_poc's
 *  rotation-based lowest-energy strategy. */
static void ecap_handler(void* arg)
{
    struct eCAP_data* ecap_data = (struct eCAP_data*)arg;
    
    ecap_data->int_flags = ecap_data->ecap_regs->ECFLG;
    ecap_data->num_intr++;
    ecap_data->ecap_regs->ECCLR = 0xFF;

    rtems_event_send(ecap_data->notify, ecap_data->ecap_event_id);
}
