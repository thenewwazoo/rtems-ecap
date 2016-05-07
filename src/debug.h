/* expects:
 *  board_init.h
 */

/*
 * This file contains general-purpose utility functions for pretty-printing
 * things. Probably mostly registers.
 */

void print_ecap(struct eCAP_data* ecap_data);
void print_gpio(struct GPIO_regs* gpio_regs);
