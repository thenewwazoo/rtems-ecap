
struct eCAP_data {
    struct eCAP_regs* ecap_regs;
    unsigned long int num_intr;   // number of interrupts
    unsigned short int int_flags; // ECFLG storage
    rtems_id* intr_sem_id; // semaphore to track interrupts
};

void init_ecap(
        uint8_t ecap_module_num,
        rtems_interrupt_handler handler,
        struct eCAP_data* arg);
