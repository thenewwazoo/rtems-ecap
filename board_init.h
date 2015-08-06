/*
 * Stub for board initialization
 */

/* this file depends upon
 *
 *  <stdbool.h>
 *  <rtems/irq-extension.h>
 *  "hw/ecap_defn.h"
 *  "hw/gpio_defn.h"
 *
 */

struct eCAP_data {
    struct eCAP_regs* ecap_regs;
    unsigned long int num_intr;   // number of interrupts
    unsigned short int int_flags; // ECFLG storage
    rtems_id* intr_sem; // semaphore to track interrupts
};

void init_board();
void init_ecap0(
        rtems_interrupt_handler handler, 
        struct eCAP_data* arg,
        rtems_id* sem);

struct GPIO_regs* gpio_get_regs(gpio_module module);
void gpio_out(gpio_module module, gpio_pin pin, bool value);
