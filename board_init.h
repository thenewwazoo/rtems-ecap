/*
 * Stub for board initialization
 */

/* this file depends upon
 *
 *  <rtems/irq-extension.h>
 *  "hw/ecap_defn.h"
 *
 */

struct eCAP_data {
    struct eCAP_regs* ecap_regs;
    unsigned long int num_intr;   // number of interrupts
    unsigned short int int_flags; // ECFLG storage
};

void init_board();
void init_ecap0(rtems_interrupt_handler handler, struct eCAP_data* arg);
