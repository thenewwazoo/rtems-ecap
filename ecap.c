
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

#include "ecap.h"
#include "events.h"
#include "gpio.h"

#include "debug.h"
/* 
 * forward declarations 
 */


/* init_ecap
 * Initialize an eCAP module. This module will trigger on
 *  every rising edge, and will store four captures.
 *
 */
void init_ecap(
        rtems_interrupt_handler handler,
        struct eCAP_data* arg)
{

    uint32_t cm_per_clk_reg_offset;
    uint8_t  ecap_intr_num;
    char     ecap_intr_name[6];
    uint8_t  ecap_pin_mux_mode;
    uint32_t ecap_pin_reg_offset;
    struct eCAP_regs*  ecap_regs;
    struct PWMSS_regs* pwmss_regs;
    uint8_t  pwmss_tbclken_shift;
    rtems_event_set ecap_event_id;

    uint32_t util_val; /* used for clarity in register operations */

    switch (arg->ecap_module)
    {
        case (0):
            cm_per_clk_reg_offset = CM_PER_EPWMSS0_CLKCTRL_OFFSET;
            ecap_intr_num = ECAP0_INT;
            strncpy(ecap_intr_name, "eCAP0", 6);
            ecap_pin_mux_mode   = CONTROL_CONF_MUXMODE(0);
            ecap_pin_reg_offset = CONTROL_CONF_ECAP0_IN_PWM0_OUT_OFFSET;
            ecap_regs  = (struct eCAP_regs*)ECAP0_REGS_BASE;
            pwmss_regs = (struct PWMSS_regs*)PWMSS0_MMIO_BASE;
            pwmss_tbclken_shift = PWMSS0_TBCLKEN;
            ecap_event_id = ECAP_0_EVENT;
            break;
        default:
            return;
    }

    /* Set up some generic data used by the ecap task and interrupt handler */
    arg->num_intr  = 0;
    arg->ecap_regs = ecap_regs;
    arg->ecap_event_id = ecap_event_id;

    /* Disable interrupts while we start up the eCAP module */
    rtems_interrupt_level irqlvl;
    rtems_interrupt_disable(irqlvl);

    mmio_write(SOC_CONTROL_REGS + CONTROL_CONF_PWMSS_CTRL,
            (TBCLKEN_ENABLE << pwmss_tbclken_shift));

    /* Enable the eCAP module in CM_PER_EPWMSS0_CLKCTRL (8.1.12.1.36) */
    util_val = mmio_read(CM_PER_MMIO_BASE + cm_per_clk_reg_offset);
    util_val |= (MODULEMODE_ENABLE << MODULEMODE);
    mmio_write(CM_PER_MMIO_BASE + cm_per_clk_reg_offset, util_val);
    /* the following read _must_ happen or the ECCTL1 doesn't get set */
    /* no, I don't know why. */
    util_val = mmio_read(CM_PER_MMIO_BASE + cm_per_clk_reg_offset);
    RTEMS_COMPILER_MEMORY_BARRIER();

    /* Configure the eCAP module to 
     *   trigger on rising pulses with no timer prescale
     *   interrupt when the timer wraps
     *   run in capture mode
     *   wrap the timer
     *   disable input and output synchronization
     */
    ecap_regs->ECEINT = 0x0;
    RTEMS_COMPILER_MEMORY_BARRIER();
    ecap_regs->ECCTL1 = (EC_RISING << CAP1POL)     | (EC_RISING << CAP2POL)
                      | (EC_RISING << CAP3POL)     | (EC_RISING << CAP4POL)
                      | (EC_ABS_MODE << CTRRST1)   | (EC_ABS_MODE << CTRRST2)
                      | (EC_ABS_MODE << CTRRST3)   | (EC_ABS_MODE << CTRRST4)
                      | (EC_ENABLE << CAPLDEN)     | (EC_DIV1 << PRESCALE);
    RTEMS_COMPILER_MEMORY_BARRIER();
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

    rtems_status_code have_isr = rtems_interrupt_handler_install(
            ecap_intr_num,
            ecap_intr_name,
            RTEMS_INTERRUPT_UNIQUE,
            handler,
            (void*)arg);

    if (have_isr != RTEMS_SUCCESSFUL) { printf("Failed to install eCAP0 ISR!\n"); }

    /* Re-enable interrupts, preserving cpsr */
    rtems_interrupt_enable(irqlvl);

    /* Mux the ecap pin by setting the RXACTIVE bit and mode  
     * in pin register (9.3.1.51) to enable external eCAP
     * triggering
     */
    mux_pin(ecap_pin_reg_offset, ecap_pin_mux_mode);

}

