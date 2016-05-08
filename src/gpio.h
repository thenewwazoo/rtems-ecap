/* Depends upon
 * <inttypes.h>
 * "hw/gpio_defn.h" */

void mux_pin(
        uint32_t soc_control_conf_reg_offset,
        uint8_t mux_mode);
void setup_pin(
        uint32_t soc_control_conf_reg_offset,
        uint8_t slewctrl,
        uint8_t rxactive,
        uint8_t putypesel,
        uint8_t puden,
        uint8_t mux_mode);

void gpio_init();
void gpio_pin_setup(uint32_t soc_control_conf_reg_offset);
void gpio_setdirection(
        gpio_module module,
        gpio_pin pin,
        bool is_input);
void gpio_out(gpio_module module, gpio_pin pin, bool value);
void gpio_print_debug(gpio_module module);

