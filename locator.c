#include <assert.h>
#include <bsp.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw/ecap_defn.h"

#include "ecap.h"
#include "locator.h"
#include "events.h"
#include "detector.h"

static void ecap_handler(void* arg);

rtems_task locator_task(rtems_task_argument arg)
{
    struct locator_spec* spec = (struct locator_spec*)arg;
    struct eCAP_data* ecap_data = spec->ecap_data;

    Detector *d = (Detector*)malloc(sizeof(Detector));
    static uint8_t dist_map[3] = {1, 1, 2};

    rtems_status_code ret;
    rtems_event_set evt_got;

    printf("Locator task starting up\n");

    ecap_data->notify = rtems_task_self();
    ecap_data->num_intr = 0;
    ecap_data->ecap_module = spec->ecap_module;

    /* start the ecap0 module and register the interrupt handler        */
    /* this function populates ecap_data->ecap_regs and ->ecap_event_id */
    init_ecap(ecap_handler, ecap_data);

    /* initialize our detector. the specific data we're encoding here   *
     *  should come from $elsewhere (e.g. nonvolatile storage)          */
    d->fw_teeth_tips = 3;
    d->fw_teeth_posns = 4;
    d->tooth_dist = dist_map;
    d->tooth_prob = (float*)malloc(sizeof(float)*d->fw_teeth_tips);
    make_uniform_dist(d->tooth_prob, d->fw_teeth_tips);
    d->error_rate = 0.7;
    d->sync_threshold = 0.9;

    printf("Locator task listening for events...\n");
    while (1)
    {
        uint32_t latest_timer = 0;
        uint8_t old_tooth;
        float predicted_accel;
        uint8_t predicted_dist;

        ret = rtems_event_receive(
                ecap_data->ecap_event_id,
                RTEMS_DEFAULT_OPTIONS,
                RTEMS_NO_TIMEOUT,
                &evt_got);
        assert(ret == RTEMS_SUCCESSFUL);

        /* We're going to naively assume we never miss an interrupt here, and
         *  therefore never get more than one CEVTn flag at a time. If we do
         *  miss an interrupt, this will explode horribly.
         *
         * For a proven strategy to handle missed interrupts, see erl_poc's
         *  rotation-based lowest-energy strategy. */

        detector_move(d->tooth_prob, d->fw_teeth_tips, d->error_rate);
        detector_locate(d, latest_timer);

        old_tooth = d->current_tooth;
        d->confidence = max_probability(
                            d->tooth_prob,
                            d->fw_teeth_tips,
                           &d->current_tooth
                            );
        predicted_dist = find_tooth_dist(
                d->tooth_dist,
                d->fw_teeth_tips,
                old_tooth,
                d->current_tooth);
        calc_accel(&predicted_accel,
                d->fw_teeth_tips,
                d->previous_timer,
                d->tooth_dist[old_tooth],
                latest_timer,
                predicted_dist);

        if (d->confidence > d->sync_threshold && predicted_accel < d->max_accel)
            d->has_sync = true;
        else
            d->has_sync = false;

    }

}

static void ecap_handler(void* arg)
{
    struct eCAP_data* ecap_data = (struct eCAP_data*)arg;
    
    ecap_data->int_flags = ecap_data->ecap_regs->ECFLG;
    ecap_data->num_intr++;
    ecap_data->ecap_regs->ECCLR = 0xFF;

    rtems_event_send(ecap_data->notify, ecap_data->ecap_event_id);
}
