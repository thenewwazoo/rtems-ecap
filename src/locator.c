#include <assert.h>
#include <bsp.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw/bbb_mmap.h"
#include "hw/ecap_defn.h"
#include "hw/gpio_defn.h"

#include "gpio.h"
#include "ecap.h"
#include "locator.h"
#include "events.h"

#include "markov-localizer/c99_fp/detector.h"
#include "markov-localizer/c99_fp/test_data.h"


static void ecap_handler(void* arg);


Detector d;


rtems_task locator_task(rtems_task_argument arg)
{
    struct locator_spec* spec = (struct locator_spec*)arg;
    struct eCAP_data* ecap_data = spec->ecap_data;

    uint8_t tooth_dists[] = TEST_TOOTH_MAP;
    uint8_t num_tooth_tips = sizeof(tooth_dists)/sizeof(tooth_dists[0]);
    float tooth_prob[num_tooth_tips];

    /* initialize our detector. the specific data we're encoding here   *
     *  should come from $elsewhere (e.g. nonvolatile storage)          */
    detector_init(
            &d,
            tooth_dists,
            num_tooth_tips,
            count_tooth_posns(num_tooth_tips, tooth_dists),
            tooth_prob,
            TEST_SAMPLE_RATE,
            TEST_MAX_ACCEL,
            TEST_ERROR_RATE);

    rtems_status_code ret;
    rtems_event_set evt_got;

    printf("Locator task starting up\n");

    ecap_data->notify = rtems_task_self();
    ecap_data->num_intr = 0;
    ecap_data->ecap_module = spec->ecap_module;

    /* start the ecap0 module and register the interrupt handler        */
    /* this function populates ecap_data->ecap_regs and ->ecap_event_id */
    init_ecap(ecap_handler, ecap_data);

    setup_pin( /* mcasp0_aclkr shares P9_42 with ecap0 */
            CONTROL_CONF_MCASP0_ACLKR,
            0, /*ignored*/
            RXACTIVE_EN, /* set to input (high-z) mode */
            0, /*ignored*/
            PUDEN_DIS, /* disable pullup/down */
            0  /*ignored*/);

    printf("Locator task listening for events...\n");
    while (1)
    {
        ret = rtems_event_receive(
                ecap_data->ecap_event_id,
                RTEMS_DEFAULT_OPTIONS,
                RTEMS_NO_TIMEOUT,
                &evt_got);
        assert(ret == RTEMS_SUCCESSFUL);

        detector_interrupt(ecap_read_timer_value(ecap_data, false), &d);
        if (d.has_sync)
            printf("+");
        else
            printf(".");
        printf(
                " t=%d v=%.2f a=%.2f c=%.2f p=%d\n",
                d->previous_timer,
                d->velocity,
                d->last_acceleration,
                d->confidence,
                d->tooth_posn
              );
    }

}

static void ecap_handler(void* arg)
{
    struct eCAP_data* ecap_data = (struct eCAP_data*)arg;

    /* TODO add a mutex for int_flags? */
    /* maybe OR the value instead of copying? */
    ecap_data->int_flags = ecap_data->ecap_regs->ECFLG;
    ecap_data->num_intr++;
    ecap_data->ecap_regs->ECCLR = 0xFF; /* probably chould clear individual bits too */

    rtems_event_send(ecap_data->notify, ecap_data->ecap_event_id);
}
