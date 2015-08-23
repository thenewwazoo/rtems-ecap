
#include <bsp.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "hw/bbb_mmap.h"
#include "hw/gpio_defn.h"

#include "bit_manip.h"
#include "ecap.h"
#include "gpio.h"

#include "debug.h"

void init_gpio_per(
        uint32_t cm_per_clk_reg_offset,
        struct GPIO_regs* gpio_regs
        );


void gpio_init()
{
    printf("gpio modules init\n");

    init_gpio_per(
            CM_PER_GPIO1_CLKCTRL_OFFSET,
            (struct GPIO_regs*)GPIO1_MMIO_BASE
            );
/*
    init_gpio_per(
            CM_PER_GPIO2_CLKCTRL_OFFSET,
            (struct GPIO_regs*)GPIO2_MMIO_BASE
            );

    init_gpio_per(
            CM_PER_GPIO3_CLKCTRL_OFFSET,
            (struct GPIO_regs*)GPIO3_MMIO_BASE
            );
*/
    printf("gpio modules initialized\n");
}

/* Initialize a CM peripheral GPIO device.
 * On the AM335x, that's GPIO1:3. Initializing GPIO 0 is left as
 *  an exercise for the reader.
 */
void init_gpio_per(
        uint32_t cm_per_clk_reg_offset,
        struct GPIO_regs* gpio_regs
        )
{

    /* Enable the GPIO module in CM_PER_GPIOn_CLKCTRL (8.1.12.1.31) */
    printf("enabling clock for gpio module\n");

    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + cm_per_clk_reg_offset),
            MODULEMODE_ENABLE << MODULEMODE);

    set_bit_and_wait((void*)(CM_PER_MMIO_BASE + cm_per_clk_reg_offset),
            FCLK_EN << OPTFCLKEN_GPIOn_GDBCLK);
    RTEMS_COMPILER_MEMORY_BARRIER();

    while ((mmio_read(CM_PER_MMIO_BASE + cm_per_clk_reg_offset)
                & (0x3 << IDLEST))
            != (IDLEST_FUNC << IDLEST));
    
    while ((mmio_read(CM_PER_MMIO_BASE + CM_PER_L4LS_CLKSTCTRL_OFFSET)
                & (CLKACTIVITY_ACT << CLKACTIVITY_GPIO_1_GDBCLK))
            != (CLKACTIVITY_ACT << CLKACTIVITY_GPIO_1_GDBCLK));
    RTEMS_COMPILER_MEMORY_BARRIER();

    printf("enabling gpio module\n");

    /* Make sure the module is not disabled (literally, turn off disable) */
    /* and set the gating ratio to 1 */
    gpio_regs->GPIO_CTRL = ((DISABLEMODULE_EN << DISABLEMODULE) | (GATINGRATIO_DIV0 << GATINGRATIO));

    /* Put the peripheral into smart idle mode */
    gpio_regs->GPIO_SYSCONFIG |= (IDLEMODE_SMART << IDLEMODE);
    RTEMS_COMPILER_MEMORY_BARRIER();

    /* Do a soft reset of the module, and wait until it finishes */
    printk("resetting GPIO module\n");
    gpio_regs->GPIO_SYSCONFIG |= (SOFTRESET_RESET << SOFTRESET);
    RTEMS_COMPILER_MEMORY_BARRIER();
    while (!(gpio_regs->GPIO_SYSSTATUS & (RESETDONE_COMPLETED << RESETDONE)));

    printf("gpio module initialized: current register values follow\n");
    print_gpio(gpio_regs);
}

/** Mux a pin with a specific setup necessary for GPIO use. 
 *
 * Unfortunately, this requires foreknowledge of which register you need
 * to twiddle. You can find how to do this in the "Need to find the 
 * control register offset" note in bbb_mmap.h
 */
void gpio_pin_setup(uint32_t soc_control_conf_reg_offset)
{
    mmio_write(SOC_CONTROL_REGS + soc_control_conf_reg_offset,
            (SLEWCTRL_SLOW << CONTROL_CONF_SLEWCTRL) |
            (RXACTIVE_EN << CONTROL_CONF_RXACTIVE) |
            (PUTYPESEL_PULLDOWN << CONTROL_CONF_PUTYPESEL) |
            (PUDEN_EN << CONTROL_CONF_PUDEN) |
            CONTROL_CONF_MUXMODE(7));
}

void mux_pin(
        uint32_t soc_control_conf_reg_offset,
        uint8_t mux_mode)
{
    mmio_write(SOC_CONTROL_REGS + soc_control_conf_reg_offset, 
            (RXACTIVE_EN << CONTROL_CONF_RXACTIVE) | CONTROL_CONF_MUXMODE(mux_mode)
           );
}

inline struct GPIO_regs* gpio_get_regs(gpio_module module)
{
    switch (module)
    {
        case 0 :
            return (struct GPIO_regs*)GPIO0_MMIO_BASE;
        case 1 :
            return (struct GPIO_regs*)GPIO1_MMIO_BASE;
        case 2 :
            return (struct GPIO_regs*)GPIO2_MMIO_BASE;
        case 3 :
            return (struct GPIO_regs*)GPIO3_MMIO_BASE;
    }
    return (struct GPIO_regs*)NULL; /* diafs TODO: return something useful */
}

void gpio_print_debug(gpio_module module)
{
    print_gpio(gpio_get_regs(module));
}

void gpio_setdirection(
        gpio_module module, 
        gpio_pin pin,
        bool is_input)
{
    struct GPIO_regs* gpio_regs = gpio_get_regs(module);
    if (gpio_regs == NULL) { return; }

    if (is_input) {
        gpio_regs->GPIO_OE |= (1<<pin);
    } else {
        gpio_regs->GPIO_OE &= ~(1<<pin);
    }
}

void gpio_out(
        gpio_module module,
        gpio_pin pin,
        bool value)
{
    struct GPIO_regs* gpio_regs = gpio_get_regs(module);

    if (value) {
        gpio_regs->GPIO_SETDATAOUT = (1<<pin);
    } else {
        gpio_regs->GPIO_CLEARDATAOUT = (1<<pin);
    }
}


