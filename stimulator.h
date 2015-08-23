
rtems_task stimulator(rtems_task_argument arg);

struct stim_spec {
    gpio_module module;
    gpio_pin pin;
    rtems_interval interval;
    uint8_t num_teeth;
    uint8_t* map;
};
