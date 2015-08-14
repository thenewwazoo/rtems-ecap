#include <stdint.h>

typedef uint8_t gpio_module;
typedef uint8_t gpio_pin;

/* TODO add GPIO interrupt values here */

/* GPIO module registers */
/* Yeah, I know the GPIO_ is redundant, but it matches the TRM */
struct GPIO_regs {
    volatile uint32_t GPIO_REVISION;
    volatile uint16_t RSVD0[6];
    volatile uint32_t GPIO_SYSCONFIG;
    volatile uint16_t RSVD1[6];
    volatile uint32_t GPIO_EOI;
    volatile uint32_t GPIO_IRQSTATUS_RAW_0;
    volatile uint32_t GPIO_IRQSTATUS_RAW_1;
    volatile uint32_t GPIO_IRQSTATUS_0;
    volatile uint32_t GPIO_IRQSTATUS_1;
    volatile uint32_t GPIO_IRQSTATUS_SET_0;
    volatile uint32_t GPIO_IRQSTATUS_SET_1;
    volatile uint32_t GPIO_IRQSTATUS_CLR_0;
    volatile uint32_t GPIO_IRQSTATUS_CLR_1;
    volatile uint32_t GPIO_IRQWAKEN_0;
    volatile uint32_t GPIO_IRQWAKEN_1;
    volatile uint16_t RSVD2[100];
    volatile uint32_t GPIO_SYSSTATUS;
    volatile uint16_t RSVD3[12];
    volatile uint32_t GPIO_CTRL;
    volatile uint32_t GPIO_OE;
    volatile uint32_t GPIO_DATAIN;
    volatile uint32_t GPIO_DATAOUT;
    volatile uint32_t GPIO_LEVELDETECT0;
    volatile uint32_t GPIO_LEVELDETECT1;
    volatile uint32_t GPIO_RISINGDETECT;
    volatile uint32_t GPIO_FALLINGDETECT;
    volatile uint32_t GPIO_DEBOUNCEENABLE;
    volatile uint32_t GPIO_DEBOUNCINGTIME;
    volatile uint16_t RSVD4[28];
    volatile uint32_t GPIO_CLEARDATAOUT;
    volatile uint32_t GPIO_SETDATAOUT;
};

/* GPIO_SYSCONFIG fields */
#define AUTOIDLE    0
#define SOFTRESET   1
#define ENAWAKEUP   2
#define IDLEMODE    3

/* GPIO_SYSSTATUS fields */
#define RESETDONE   0

/* GPIO_CTRL fields */
#define DISABLEMODULE   0
#define GATINGRATIO     1

/* GPIO_DEBOUNCINGTIME */
#define DEBOUNCINGTIME    0

/* GPIO_SYSCONFIG *
 *  field bits    *
 ******************/
/* AUTOIDLE */
#define SYSCONFIG_FREERUNNING   0
#define SYSCONFIG_GATING        1
/* SOFTRESET */
#define SOFTRESET_NORMAL        0
#define SOFTRESET_RESET         1
/* ENAWAKEUP */
#define ENAWAKEUP_DIS           0
#define ENAWAKEUP_GPIO_TRANS    1
/* IDLEMODE */
#define IDLEMODE_FORCE          0
#define IDLEMODE_NOIDLE         1
#define IDLEMODE_SMART          2
#define IDLEMODE_SMART_WAKEUP   3 /* GPIO0 only */

/* GPIO_SYSSTATUS *
 *  field bits    *
 ******************/
/* RESETDONE */
#define RESETDONE_ONGOING       0
#define RESETDONE_COMPLETED     1

/* GPIO_CTRL  *
 * field bits *
 **************/
/* DISABLEMODULE */
#define DISABLEMODULE_EN        0
#define DISABLEMODULE_DIS       1
/* GATINGRATIO */
#define GATINGRATIO_DIV0        0
#define GATINGRADIO_DIV2        1
#define GATINGRATIO_DIV4        2
#define GATINGRATIO_DIV8        3

/* GPIO_DEBOUNCINGTIME *
 *  field bits       *
 *********************/
/* DEBOUNCETIME: Input debouncing value in 31 usec steps */
#define DEBOUNCETIME_us(x) (uint8_t)(((uint32_t)x/31)-1) /* request µs value */
