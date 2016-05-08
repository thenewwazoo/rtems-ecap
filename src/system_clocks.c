#include <bsp.h>
#include <inttypes.h>
#include <stdio.h>

#include "hw/bbb_mmap.h"
#include "hw/gpio_defn.h"

#include "bit_manip.h"
#include "system_clocks.h"

/* Initialize the L3 and L4 peripheral clocks needed for GPIO */
void clocks_init_L3()
{
    /* L3 clocks */
    printf("Initializing L3 clocks...\n");

    /* First we set some bits, and wait for our change to stick */
    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + CM_PER_L3_CLKCTRL_OFFSET),
            MODULEMODE_ENABLE << MODULEMODE);

    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + CM_PER_L3_INSTR_CLKCTRL_OFFSET),
            MODULEMODE_ENABLE << MODULEMODE);

    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + CM_PER_L3_CLKSTCTRL_OFFSET),
            CLKTRCTRL_SW_WKUP << CLKTRCTRL);

    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + CM_PER_L3S_CLKSTCTRL_OFFSET),
            CLKTRCTRL_SW_WKUP << CLKTRCTRL);

    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + CM_PER_OCPWP_CLKCTRL_OFFSET),
            MODULEMODE_ENABLE << MODULEMODE);

    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + CM_PER_OCPWP_L3_CLKSTCTRL_OFFSET),
            CLKTRCTRL_SW_WKUP << CLKTRCTRL);

    /* Now we wait for some proof that the clocks have started */
    while ((mmio_read(CM_PER_MMIO_BASE + CM_PER_L3_CLKCTRL_OFFSET)
                & (0x3 << IDLEST))
            != (IDLEST_FUNC << IDLEST));

    while ((mmio_read(CM_PER_MMIO_BASE + CM_PER_L3_INSTR_CLKCTRL_OFFSET)
                & (0x3 << IDLEST))
            != (IDLEST_FUNC << IDLEST));

    while ((mmio_read(CM_PER_MMIO_BASE + CM_PER_L3_CLKSTCTRL_OFFSET)
                & (CLKACTIVITY_ACT << CLKACTIVITY_L3_GCLK))
            != (CLKACTIVITY_ACT << CLKACTIVITY_L3_GCLK));

    while ((mmio_read(CM_PER_MMIO_BASE + CM_PER_OCPWP_CLKCTRL_OFFSET)
                & (0x3 << IDLEST))
            != (IDLEST_FUNC << IDLEST));

    while ((mmio_read(CM_PER_MMIO_BASE + CM_PER_OCPWP_L3_CLKSTCTRL_OFFSET)
                & (CLKACTIVITY_ACT << CLKACTIVITY_OCPWP_L3_GCLK))
            != (CLKACTIVITY_ACT << CLKACTIVITY_OCPWP_L3_GCLK));

    while ((mmio_read(CM_PER_MMIO_BASE + CM_PER_L3S_CLKSTCTRL_OFFSET)
                & (CLKACTIVITY_ACT << CLKACTIVITY_L3S_GCLK))
            != (CLKACTIVITY_ACT << CLKACTIVITY_L3S_GCLK));
}

void clocks_init_L4()
{
    printf("Initializing L4 clocks...\n");
    /* L4 clocks */

    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + CM_PER_L4LS_CLKCTRL_OFFSET),
            MODULEMODE_ENABLE << MODULEMODE);

    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + CM_PER_L4LS_CLKSTCTRL_OFFSET),
            CLKTRCTRL_SW_WKUP << CLKTRCTRL);

    while ((mmio_read(CM_PER_MMIO_BASE + CM_PER_L4LS_CLKCTRL_OFFSET)
                & (0x3 << IDLEST))
            != (IDLEST_FUNC << IDLEST));

    while ((mmio_read(CM_PER_MMIO_BASE + CM_PER_L4LS_CLKSTCTRL_OFFSET)
                & (CLKACTIVITY_ACT << CLKACTIVITY_L4LS_GCLK))
            != (CLKACTIVITY_ACT << CLKACTIVITY_L4LS_GCLK));
}
