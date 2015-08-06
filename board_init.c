
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include <rtems.h>
#include <bsp.h>

#include <rtems/irq-extension.h>

#include "hw/bbb_mmap.h"
#include "hw/ecap_defn.h"
#include "hw/gpio_defn.h"
#include "hw/pwmss_defn.h"

#include "board_init.h"
#include "debug.h"

/* 
 * forward declarations 
 */

void init_ecap(
        uint32_t cm_per_clk_reg_offset,
        rtems_vector_number ecap_irq,
        struct PWMSS_regs* pwmss_regs,
        struct eCAP_regs* ecap_regs,
        rtems_interrupt_handler vector_handler,
        char* handler_info,
        struct eCAP_data* handler_arg);

void mux_pin(
        uint32_t soc_control_conf_reg_offset,
        uint8_t mux_mode);

void init_gpio1();
void init_gpio3();
void init_gpio_per(
        uint32_t cm_per_clk_reg_offset,
        struct GPIO_regs* gpio_regs
        );

void enable_gpio(gpio_module module, gpio_pin pin, bool is_output);
/*
 * init_board()
 * General-purpose hardware initialization routines; stuff that probably
 *  isn't module-specific.
 *
 */
void init_board()
{
    printf("board init\n");
    init_gpio1();

    /* Mux GPMC_A1 pin to mode 7 to enable GPIO1[17] */
    mux_pin(CONTROL_CONF_GPMC_A1_OFFSET, 7);
    enable_gpio(1, 17, false);

    printf("board initialized\n");
}

/* init_ecap0
 * Initialize the first eCAP module. This module will trigger on
 *  every rising edge, and will store four captures.
 *
 */
void init_ecap0(
        rtems_interrupt_handler handler,
        struct eCAP_data* arg,
        rtems_id* sem)
{
    /* Set up some pointers used by the interrupt handler */
    arg->ecap_regs = (struct eCAP_regs*)ECAP0_REGS_BASE;
    arg->num_intr = 0;
    arg->intr_sem = sem;

    init_ecap( 
            CM_PER_EPWMSS0_CLKCTRL_OFFSET,
            ECAP0_INT,
            (struct PWMSS_regs*)PWMSS0_MMIO_BASE,
            (struct eCAP_regs*)ECAP0_REGS_BASE,
            handler,
            "eCAP0",
            arg
            );

    /* Mux the ECAP0_IN_APWM_OUT pin by setting the RXACTIVE bit and mode 0 
     * in register conf_ecap0_in_pwm0_out (9.3.1.51) to enable external eCAP
     * triggering
     */
    mux_pin(CONTROL_CONF_ECAP0_IN_PWM0_OUT_OFFSET, 0);

    printf("ecap0 initialized\n");
}

void init_ecap(
        uint32_t cm_per_clk_reg_offset,
        rtems_vector_number ecap_irq,
        struct PWMSS_regs* pwmss_regs,
        struct eCAP_regs* ecap_regs,
        rtems_interrupt_handler vector_handler,
        char* handler_info,
        struct eCAP_data* handler_arg)
{
    uint32_t util_val; /* used for clarity in register operations */

    /* Disable interrupts while we start up the eCAP module */
    rtems_interrupt_level irqlvl;
    rtems_interrupt_disable(irqlvl);

    /* Enable the eCAP module in CM_PER_EPWMSS0_CLKCTRL (8.1.12.1.36) */
    util_val = mmio_read(CM_PER_MMIO_BASE + cm_per_clk_reg_offset);
    util_val |= (MODULEMODE_ENABLE << MODULEMODE);
    mmio_write(CM_PER_MMIO_BASE + cm_per_clk_reg_offset, util_val);
    util_val = mmio_read(CM_PER_MMIO_BASE + cm_per_clk_reg_offset);
    printf("CM_PER_EPWMSS0_CLKCTRL:\t0x%08"PRIx32"\n", util_val);
    RTEMS_COMPILER_MEMORY_BARRIER();

    /* Configure the eCAP 0 module to 
     *   trigger on rising pulses with no timer prescale
     *   interrupt when the timer wraps
     *   run in capture mode
     *   wrap the timer
     *   disable input and output synchronization
     */
    ecap_regs->ECEINT = 0x0;
    RTEMS_COMPILER_MEMORY_BARRIER();
    /* I probably want EC_ABS_MODE for the CTRRSTn fields */
    ecap_regs->ECCTL1 = (EC_RISING << CAP1POL)     | (EC_RISING << CAP2POL)
                      | (EC_RISING << CAP3POL)     | (EC_RISING << CAP4POL)
                      | (EC_DELTA_MODE << CTRRST1) | (EC_DELTA_MODE << CTRRST2)
                      | (EC_DELTA_MODE << CTRRST3) | (EC_DELTA_MODE << CTRRST4)
                      | (EC_ENABLE << CAPLDEN)     | (EC_DIV1 << PRESCALE);
    /*ecap_regs->ECCTL1 = 0x01aa;*/
    ecap_regs->ECCTL2 = (EC_CONTINUOUS << CONT_ONESHT)
                      | (EC_EVENT4 << STOP_WRAP)
                      | (EC_DISABLE << SYNCI_EN)
                      | (EC_SYNCO_DIS << SYNCO_SEL)
                      | (EC_CAP_MODE << CAP_APWM);

    /* Per the docs, clear all the interrupt flags and timer registers, and
     * then enable the interrupts (15.3.4.1.9) */
    ecap_regs->ECCLR  = BIT(CTR_EQ_CMP) | BIT(CTR_EQ_PRD) | BIT(CTROVF) 
                      | BIT(CEVT4) | BIT(CEVT3) | BIT(CEVT2) | BIT(CEVT1)
                      | BIT(INT);
    ecap_regs->ECEINT = BIT(CTROVF) | BIT(CEVT4) | BIT(CEVT3) | BIT(CEVT2) | BIT(CEVT1);
    RTEMS_COMPILER_MEMORY_BARRIER();

    /* Enable the eCAP 0 timer (it will start when the clock is enabled */
    ecap_regs->ECCTL2 |= (EC_RUN << TSCTRSTOP);
    RTEMS_COMPILER_MEMORY_BARRIER();

    /* Set the "smart" idle mode in register SYSCONFIG (15.1.3.2) */
    util_val = (IDLEMODE_SMART << SYSCONFIG_IDLEMODE);
    pwmss_regs->SYSCONFIG = util_val;
    RTEMS_COMPILER_MEMORY_BARRIER();

    /* Enable the clock in pwmss_ctrl (9.3.1.32) */
    util_val = pwmss_regs->CLKCONFIG;
    util_val |= (CLKCONFIG_CLK_EN << eCAPCLK_EN);
    pwmss_regs->CLKCONFIG = util_val;
    RTEMS_COMPILER_MEMORY_BARRIER();

    util_val = pwmss_regs->CLKSTATUS;
    if ( !(util_val & (1<<eCAP_CLK_EN_ACK)) ) 
    { 
        perror("Failed to start eCAP counter!\n"); 
    }

    util_val = mmio_read(CM_PER_MMIO_BASE + CM_PER_L4LS_CLKSTCTRL_OFFSET);
    printf("CM_PER_L4LS_CKLSTCTRL:\t0x%08"PRIx32"\n", util_val);
    util_val = mmio_read(CM_PER_MMIO_BASE + CM_PER_L4LS_CLKCTRL_OFFSET);
    printf("CM_PER_L4LS_CLKCTRL:\t0x%08"PRIx32"\n", util_val);

    /* Install our eCAP interrupt handler */
    rtems_status_code have_isr = rtems_interrupt_handler_install(
            ecap_irq,
            handler_info,
            RTEMS_INTERRUPT_UNIQUE,
            vector_handler,
            (void*)handler_arg);

    if (have_isr != RTEMS_SUCCESSFUL) { printf("Failed to install eCAP0 ISR!\n"); }

    /* Re-enable interrupts, preserving cpsr */
    rtems_interrupt_enable(irqlvl);

}

void init_gpio1()
{
    init_gpio_per(
            CM_PER_GPIO1_CLKCTRL_OFFSET,
            (struct GPIO_regs*)GPIO1_MMIO_BASE
            );
}

void init_gpio3()
{
    init_gpio_per(
            CM_PER_GPIO3_CLKCTRL_OFFSET,
            (struct GPIO_regs*)GPIO3_MMIO_BASE
            );
}

/* Initialize a CM peripheral GPIO device
 *  On the AM335x, that's GPIO1:3
 */
void init_gpio_per(
        uint32_t cm_per_clk_reg_offset,
        struct GPIO_regs* gpio_regs
        )
{
    uint32_t util_val = 0;

    /* Enable the GPIO module in CM_PER_GPIOn_CLKCTRL (8.1.12.1.31) */
    printf("enabling clock for gpio\n");
    util_val = mmio_read(CM_PER_MMIO_BASE + cm_per_clk_reg_offset);
    util_val |= (FCLK_EN << OPTFCLKEN_GPIOn_GDBCLK) | (MODULEMODE_ENABLE << MODULEMODE);
    mmio_write(CM_PER_MMIO_BASE + cm_per_clk_reg_offset, util_val);
    RTEMS_COMPILER_MEMORY_BARRIER();

    /* Make sure the module is not disabled */
    printf("enabling gpio module\n");
    gpio_regs->GPIO_CTRL = 0;
    /* mmio_clear(gpio_regs->GPIO_CTRL, DISABLEMODULE); */
    RTEMS_COMPILER_MEMORY_BARRIER();

    /* Put the peripheral into smart idle mode */
    gpio_regs->GPIO_SYSCONFIG |= (IDLEMODE_SMART << IDLEMODE);
    RTEMS_COMPILER_MEMORY_BARRIER();

    /* Do a soft reset of the module, and wait until it finishes */
    /*
    print_gpio(gpio_regs);
    printk("resetting GPIO module\n");
    gpio_regs->GPIO_SYSCONFIG |= (SOFTRESET_RESET << SOFTRESET);
    RTEMS_COMPILER_MEMORY_BARRIER();
    while (!gpio_regs->GPIO_SYSSTATUS & (RESETDONE_COMPLETED << RESETDONE));
    */

    print_gpio(gpio_regs);

}


void mux_pin(
        uint32_t soc_control_conf_reg_offset,
        uint8_t mux_mode)
{
    write16(SOC_CONTROL_REGS + soc_control_conf_reg_offset, 
            CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(mux_mode)
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

void enable_gpio(
        gpio_module module, 
        gpio_pin pin,
        bool is_output)
{
    struct GPIO_regs* gpio_regs = gpio_get_regs(module);
    if (gpio_regs == NULL) { return; }

    if (is_output)
    {
        gpio_regs->GPIO_OE &= ~(1<<pin);
    } else {
        gpio_regs->GPIO_OE |= (1<<pin);
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
