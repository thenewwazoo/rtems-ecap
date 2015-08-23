
struct locator_spec {
    char locator_id;
    uint8_t ecap_module;
    struct eCAP_data* ecap_data;
    uint32_t timeval;
};

rtems_task locator_task(rtems_task_argument arg);

