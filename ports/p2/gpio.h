#ifndef MICROPY_INCLUDED_GPIO_H
#define MICROPY_INCLUDED_GPIO_H

// GPIO pin mode types
#define GPIO_MODE_INPUT (0)
#define GPIO_MODE_INPUT_OUTPUT (1)
#define GPIO_MODE_INPUT_OUTPUT_OD (2)

void gpio_set_latch(unsigned int pin, unsigned int level);
void gpio_set_level(unsigned int pin, unsigned int level);
unsigned int gpio_get_level(unsigned int pin);
void gpio_set_direction(unsigned int pin, unsigned int direction);
void gpio_pinclear(unsigned int pin);
void gpio_toggle(unsigned int pin);
void gpio_wrpin(unsigned int pin, unsigned int wrval);
void gpio_wxpin(unsigned int pin, unsigned int xval);
void gpio_wypin(unsigned int pin, unsigned int yval);
void gpio_akpin(unsigned int pin);
unsigned int gpio_rdpin(unsigned int pin);
unsigned int gpio_rqpin(unsigned int pin);

#endif // MICROPY_INCLUDED_GPIO_H

