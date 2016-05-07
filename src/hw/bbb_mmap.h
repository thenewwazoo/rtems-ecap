
/* Need to find the control register offset to mux a pin?
 *  1) Look at the BBB SRM and find the P8 or P9 pin you want to drive
 *  2) Get the processor pin number from the PROC column
 *  3) Look in the AM335x datasheet Table 4-1 and find your pin in the
 *      "ZCZ ball number" column
 *  4) Note the pin name
 *  5) Look in the AM335x TRM Table 9-10 for your pin name (in lowercase)
 */

#define SOC_CONTROL_REGS                        (0x44E10000)
#define CONTROL_IOPAD_OFFSET                    (0x800)
#define CONTROL_CONF_GPMC_A1_OFFSET             (0x844)
#define CONTROL_CONF_ECAP0_IN_PWM0_OUT_OFFSET   (0x964)
#define CONTROL_CONF_PWMSS_CTRL                 (0x664)
#define CONTROL_CONF_ECAP_EVT_CAP               (0xFD4)

/* Generic conf_<module>_<pin> bit definitions (9.3.1.51) */
#define CONTROL_CONF_MUXMODE(n)   (n)
#define CONTROL_CONF_MMODE          0   /* aka CONTROL_CONF_MUXMODE(n) */
#define CONTROL_CONF_PUDEN          3
#define CONTROL_CONF_PUTYPESEL      4
#define CONTROL_CONF_RXACTIVE       5
#define CONTROL_CONF_SLEWCTRL       6

/* PUDEN values - pull-up/-down disable */
#define PUDEN_EN    0
#define PUDEN_DIS   1

/* PUTYPESEL values */
#define PUTYPESEL_PULLDOWN  0
#define PUTYPESEL_PULLUP    1

/* RXACTIVE values */
#define RXACTIVE_DIS    0
#define RXACTIVE_EN     1

/* SLEWCTRL values */
#define SLEWCTRL_FAST   0
#define SLEWCTRL_SLOW   1

/* CONTROL_CONF_PWMSS_CTRL bit fields */
#define PWMSS0_TBCLKEN                  0
#define PWMSS1_TBCLKEN                  1
#define PWMSS2_TBCLKEN                  2

/* CONTROL_CONF_PWMSS_CTRL bit values */
#define TBCLKEN_DISABLE                 0
#define TBCLKEN_ENABLE                  1

/* CONTROL_CONF_ECAP_EVT_CAP bit fields */
#define ECAP0_EVTCAP                    0x00
#define ECAP1_EVTCAP                    0x08
#define ECAP2_EVTCAP                    0x16

#define PWMSS0_MMIO_BASE    (0x48300000)
#define PWMSS0_MMIO_END     (0x483000FF)
#define ECAP0_REGS_BASE     (0x48300100)
#define ECAP0_REGS_OFFSET   (0x100) /* offset from PWMSS_MMIO_BASE */
#define ECAP0_REGS_END      (0x4830017F)

/* L4_WKUP peripheral stuff */
#define L4_WKUP_MMIO_BASE   (0x44C00000)

/* CM_WKUP (clock module wakeup) stuff */
#define CM_WKUP_MMIO_BASE               (0x44E00400)
#define CM_WKUP_GPIO0_CLKCTRL_OFFSET    (0x08)

/* Peripheral module control register offsets */
#define CM_PER_MMIO_BASE                    (0x44E00000)
#define CM_PER_EPWMSS0_CLKCTRL_OFFSET       (0xD4)
#define CM_PER_L4LS_CLKSTCTRL_OFFSET        (0x00)
#define CM_PER_L3S_CLKSTCTRL_OFFSET         (0x04)
#define CM_PER_L3_CLKSTCTRL_OFFSET          (0x0C)
#define CM_PER_L4LS_CLKCTRL_OFFSET          (0x60)
#define CM_PER_GPIO1_CLKCTRL_OFFSET         (0xAC)
#define CM_PER_GPIO2_CLKCTRL_OFFSET         (0xB0)
#define CM_PER_GPIO3_CLKCTRL_OFFSET         (0xB4)
#define CM_PER_L3_INSTR_CLKCTRL_OFFSET      (0xDC)
#define CM_PER_L3_CLKCTRL_OFFSET            (0xE0)
#define CM_PER_OCPWP_L3_CLKSTCTRL_OFFSET    (0x12C)
#define CM_PER_OCPWP_CLKCTRL_OFFSET         (0x130)

/* GPIO stuff */
#define GPIO0_MMIO_BASE     (0x44E07000)
#define GPIO1_MMIO_BASE     (0x4804C000)
#define GPIO2_MMIO_BASE     (0x481AC000)
#define GPIO3_MMIO_BASE     (0x481AE000)

#define IDLEST      16
#define MODULEMODE  0

#define IDLEST_FUNC     0
#define IDLEST_TRANS    1
#define IDLEST_IDLE     2
#define IDLEST_DISABLE  3

#define MODULEMODE_DISABLED 0
#define MODULEMODE_ENABLE   2

#define OPTFCLKEN_GPIOn_GDBCLK     18
#define FLCK_DIS                    0
#define FCLK_EN                     1

/* Ugh. I really don't want to have to do a clocks_defn.h */
/* following from 8.1.12.1.3 */
#define CLKTRCTRL                   0

#define CLKACTIVITY_EMIF_GCLK       2
#define CLKACTIVITY_MMC_FCLK        3
#define CLKACTIVITY_L3_GLCK         4
#define CLKACTIVITY_CPTS_RFT_GCLK   6
#define CLKACTIVITY_MCASP_GCLK      7

#define CLKACTIVITY_OCPWP_L3_GCLK   4
#define CLKACTIVITY_OCPWP_L4_GCLK   5

#define CLKACTIVITY_L3S_GCLK        3 /* of CM_PER_L3S_CLKSTCTRL */
#define CLKACTIVITY_L3_GCLK         4 /* of CM_PER_L3_CLKSTCTRL */
#define CLKACTIVITY_L4LS_GCLK       8 /* of CM_PER_L4LS_CLKSTCTRL */

#define CLKACTIVITY_GPIO_1_GDBCLK   19 /* of CM_PER_L4LS_CLKSTCTRL */
#define CLKACTIVITY_GPIO_2_GDBCLK   20 /* of CM_PER_L4LS_CLKSTCTRL */
#define CLKACTIVITY_GPIO_3_GDBCLK   21 /* of CM_PER_L4LS_CLKSTCTRL */

#define CLKTRCTRL_NO_SLEEP  0
#define CLKTRCTRL_SW_SLEEP  1
#define CLKTRCTRL_SW_WKUP   2

#define CLKACTIVITY_ACT     1
#define CLKACTIVITY_INACT   0
