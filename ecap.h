
struct eCAP_data {
    struct eCAP_regs* ecap_regs;
    unsigned long int num_intr;   /* number of interrupts   */
    unsigned short int int_flags; /* ECFLG storage          */
    rtems_id notify; /* Where to send events for interrupts */
    uint8_t ecap_module; /* which ecap module we're using   */
    rtems_event_set ecap_event_id; /* what event to send    */
};

void init_ecap(
        rtems_interrupt_handler handler,
        struct eCAP_data* arg);
