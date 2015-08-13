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
    rtems_name intr_sem_name; // semaphore to track interrupts
};

void mux_pin(
        uint32_t soc_control_conf_reg_offset,
        uint8_t mux_mode);

void clocks_init_L3();
void clocks_init_L4();

void gpio_init();
void gpio_pin_setup(uint32_t soc_control_conf_reg_offset);
void gpio_setdirection(
        gpio_module module, 
        gpio_pin pin,
        bool is_input);
void gpio_out(gpio_module module, gpio_pin pin, bool value);
void gpio_print_debug(gpio_module module);

void init_ecap(
        uint8_t ecap_module_num,
        rtems_interrupt_handler handler,
        struct eCAP_data* arg);

/* Simply loop until the bit reads as set */
static inline void wait_for_bit(void* reg, uint32_t val)
{
    while (!(mmio_read(reg) & val));
}

/* Set a bit, and wait for that bit to read as set */
static inline void set_bit_and_wait(void* reg, uint32_t val)
{
    uint32_t util_val;

    /* Set the necessary values, and then wait to make sure they stick */
    util_val = mmio_read(reg);
    util_val |= val;
    mmio_write(reg, util_val);
    wait_for_bit(reg, val);
}


