
#include <stdio.h>

#include <rtems.h>
#include <bsp.h>

#include <rtems/irq-extension.h>
#include <rtems/malloc.h>

#include "hw/bbb_mmap.h"
#include "hw/ecap_defn.h"
#include "hw/pwmss_defn.h"

#include "board_init.h"

/* 
 * forward declarations 
 */

void init_ecap(
        uint32_t soc_control_conf_reg_offset,
        uint32_t cm_per_clk_reg_offset,
        rtems_vector_number ecap_irq,
        struct PWMSS_regs* pwmss_regs,
        struct eCAP_regs* ecap_regs,
        rtems_interrupt_handler vector_handler,
        struct eCAP_data* handler_arg);

/*
 * init_board()
 * General-purpose hardware initialization routines; stuff that probably
 *  isn't module-specific.
 *
 */
void init_board()
{
    printf("board initialized\n");
}

/* init_ecap0
 * Initialize the first eCAP module. This module will trigger on
 *  every rising edge, and will store four captures.
 *
 */
void init_ecap0(rtems_interrupt_handler handler, struct eCAP_data* arg)
{
    init_ecap( 
            CONTROL_CONF_ECAP0_IN_PWM0_OUT_OFFSET,
            CM_PER_EPWMSS0_CLKCTRL_OFFSET,
            ECAP0_INT,
            (struct PWMSS_regs*)PWMSS0_MMIO_BASE,
            (struct eCAP_regs*)ECAP0_REGS_BASE,
            handler,
            arg
            );
    arg->ecap_regs = (struct eCAP_regs*)ECAP0_REGS_BASE;
    arg->num_intr = 0;
    printf("ecap0 initialized\n");
}

void init_ecap(
        uint32_t soc_control_conf_reg_offset,
        uint32_t cm_per_clk_reg_offset,
        rtems_vector_number ecap_irq,
        struct PWMSS_regs* pwmss_regs,
        struct eCAP_regs* ecap_regs,
        rtems_interrupt_handler vector_handler,
        struct eCAP_data* handler_arg)
{
    unsigned int mux_mode = 0; /* mux mode for the pin */
    uint16_t util_val; /* used for clarity in register operations */

    /* Disable interrupts while we start up the eCAP module */
    /*
    rtems_interrupt_level irqlvl;
    rtems_interrupt_disable(irqlvl);
    */

    /* Enable the eCAP module in CM_PER_EPWMSS0_CLKCTRL (8.1.12.1.36) */
    util_val = read16(CM_PER_MMIO_BASE + cm_per_clk_reg_offset);
    util_val |= (MODULEMODE_ENABLE << MODULEMODE);
    write16(CM_PER_MMIO_BASE + cm_per_clk_reg_offset, util_val);

    /* Mux the ECAP0_IN_APWM_OUT pin by setting the RXACTIVE bit and mode 0 
     * in register conf_ecap0_in_pwm0_out (9.3.1.51) to enable external eCAP
     * triggering
     */
    write16(SOC_CONTROL_REGS + soc_control_conf_reg_offset, 
            CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(mux_mode)
           );

    /* Configure the eCAP 0 module to 
     *   trigger on rising pulses with no timer prescale
     *   interrupt when the timer wraps
     *   run in capture mode
     *   wrap the timer
     *   disable input and output synchronization
     */
    ecap_regs->ECEINT = 0x0;
    ecap_regs->ECCTL1 = (EC_RISING << CAP1POL)   | (EC_RISING << CAP2POL)
                      | (EC_RISING << CAP3POL)   | (EC_RISING << CAP4POL)
                      | (EC_ABS_MODE << CTRRST1) | (EC_ABS_MODE << CTRRST2) 
                      | (EC_ABS_MODE << CTRRST3) | (EC_ABS_MODE << CTRRST4)
                      | (EC_ENABLE << CAPLDEN)   | (EC_DIV1 << PRESCALE);
    ecap_regs->ECCTL2 = (EC_CAP_MODE << CAP_APWM)
                      | (EC_CONTINUOUS << CONT_ONESHT)
                      | (EC_EVENT4 << STOP_WRAP)
                      | (EC_SYNCO_DIS << SYNCO_SEL)
                      | (EC_DISABLE << SYNCI_EN);

    /* Per the docs, clear all the interrupt flags and timer registers, and
     * then enable the interrupts (15.3.4.1.9) */
    ecap_regs->ECCLR  = BIT(CTR_EQ_CMP) | BIT(CTR_EQ_PRD) | BIT(CTROVF) 
        | BIT(CEVT4) | BIT(CEVT3) | BIT(CEVT2) | BIT(CEVT1) | BIT(INT);
    ecap_regs->ECEINT = BIT(CTROVF) | BIT(CEVT4) | BIT(CEVT3) | BIT(CEVT2) | BIT(CEVT1);

    /* Enable the eCAP 0 timer (it will start when the clock is enabled */
    ecap_regs->ECCTL2 |= (EC_RUN << TSCTRSTOP);

    /* Set the "smart" idle mode in register SYSCONFIG (15.1.3.2) */
    util_val = (IDLEMODE_SMART << SYSCONFIG_IDLEMODE);
    pwmss_regs->SYSCONFIG = util_val;

    /* Enable the clock in pwmss_ctrl (9.3.1.32) */
    util_val = pwmss_regs->CLKCONFIG;
    util_val |= (CLKCONFIG_CLK_EN << eCAPCLK_EN);
    pwmss_regs->CLKCONFIG = util_val;

    util_val = pwmss_regs->CLKSTATUS;
    if ( !(util_val & (1<<eCAP_CLK_EN_ACK)) ) 
    { 
        printf("Failed to start eCAP counter!\n"); 
    }

    /* Install our eCAP interrupt handler */
    rtems_status_code have_isr = rtems_interrupt_handler_install(
            ecap_irq,
            "eCAP0",
            RTEMS_INTERRUPT_UNIQUE,
            vector_handler,
            (void*)handler_arg);

    if (have_isr != RTEMS_SUCCESSFUL) { printf("Failed to install eCAP0 ISR!\n"); }

    /* Re-enable interrupts, preserving cpsr */
    /*
    rtems_interrupt_enable(irqlvl);
    */

}
