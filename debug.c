
/*
 * This file contains general-purpose utility functions for pretty-printing
 * things. Probably mostly registers.
 */

#include <bsp.h>
#include <inttypes.h>

#include "hw/ecap_defn.h"
#include "hw/gpio_defn.h"
#include "board_init.h"

void print_ecap(struct eCAP_data* ecap_data)
{
    printk("ECAP0 register values ={\n"
    "\tTSCTR:\t0x%08"PRIx32     "\tCTRPHS:\t0x%08"PRIx32"\n" 
    "\tCAP1:\t0x%08"PRIx32      "\tCAP2:\t0x%08"PRIx32       "\tCAP3:\t0x%08"PRIx32       "\tCAP4:\t0x%08"PRIx32"\n"
    "\tECCTL1:\t0x%04"PRIx16    "\tECCTL2:\t0x%04"PRIx16     "\tECEINT:\t0x%04"PRIx16"\n"
    "\tECFLG:\t0x%04"PRIx16     "\tECCLR:\t0x%04"PRIx16      "\tECFRC:\t0x%04"PRIx16"\n"
    "\tREVID:\t0x%08"PRIx32"\n"
    "}\n",
    ecap_data->ecap_regs->TSCTR,  ecap_data->ecap_regs->CTRPHS, 
    ecap_data->ecap_regs->CAP1,   ecap_data->ecap_regs->CAP2,   ecap_data->ecap_regs->CAP3,   ecap_data->ecap_regs->CAP4,
    ecap_data->ecap_regs->ECCTL1, ecap_data->ecap_regs->ECCTL2, ecap_data->ecap_regs->ECEINT,
    ecap_data->ecap_regs->ECFLG,  ecap_data->ecap_regs->ECCLR,  ecap_data->ecap_regs->ECFRC,
    ecap_data->ecap_regs->REVID);
}

void print_gpio(struct GPIO_regs* gpio_regs)
{
    printk("GPIOn register values = {\n"
    "\tGPIO_SYSCONFIG:\t0x%08"PRIx32"\n"
    "\tGPIO_CTRL:\t0x%08"PRIx32"\n"
    "\tGPIO_SYSSTATUS:\t0x%08"PRIx32"\n"
    "\tGPIO_OE:\t0x%08"PRIx32"\n"
    "\tGPIO_DATAOUT:\t0x%08"PRIx32"\tGPIO_DATAIN:\t0x%08"PRIx32"\n"
    "}\n",
        gpio_regs->GPIO_SYSCONFIG,
        gpio_regs->GPIO_CTRL,
        gpio_regs->GPIO_SYSSTATUS,
        gpio_regs->GPIO_OE,
        gpio_regs->GPIO_DATAOUT,    gpio_regs->GPIO_DATAIN
        );
}
