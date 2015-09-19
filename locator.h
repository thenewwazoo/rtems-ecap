
struct locator_spec {
    uint8_t ecap_module;
    struct eCAP_data* ecap_data;
};

rtems_task locator_task(rtems_task_argument arg);

