/* teh stimulator */

#include <assert.h>
#include <bsp.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "hw/gpio_defn.h"
#include "gpio.h"
#include "stimulator.h"

void strobe_pin(struct stim_spec* stim_data)
{
    rtems_status_code ret;

    gpio_out(stim_data->module, stim_data->pin, true);

    ret = rtems_task_wake_after(stim_data->interval / 10);
    if (!rtems_is_status_successful(ret)) { printf("Failed to keep high!\n"); }

    gpio_out(stim_data->module, stim_data->pin, false);
}

rtems_task stimulator(rtems_task_argument arg)
{
    struct stim_spec* stim_data = (struct stim_spec*)arg;
    uint8_t tooth_count; /* how many teeth have passed in the current map bin? */
    uint8_t tooth_idx = 0; /* where in the map are we? */

    /* we assume that our GPIO pin is already configured */

    rtems_id tooth_period;
    rtems_status_code ret;

    ret = rtems_rate_monotonic_create(
            rtems_build_name('S', 'T', 'M', '1'),
            &tooth_period);
    assert(ret == RTEMS_SUCCESSFUL);

    while (1)
    {
        if (rtems_rate_monotonic_period(tooth_period, stim_data->interval) == RTEMS_TIMEOUT)
            break; /* something went wrong and now we have to leave. goodbye! */

        /* ding! fries are done. */
        tooth_count++;

        if (tooth_count == stim_data->map[tooth_idx])
        {
            tooth_idx = (tooth_idx + 1) % stim_data->num_teeth;
            tooth_count = 0;
            strobe_pin(stim_data);
        }
    }

    printf("oops stimulator blowed up\n");

    ret = rtems_rate_monotonic_delete(tooth_period);
    assert(ret == RTEMS_SUCCESSFUL);

    ret = rtems_task_delete(rtems_task_self()); /* should not return */
    assert(ret == RTEMS_SUCCESSFUL);

}
