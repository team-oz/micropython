#ifndef MICROPY_INCLUDED_GPIO_H
#define MICROPY_INCLUDED_GPIO_H

void gpio_set_latch(int pin, unsigned level);
void gpio_set_level(int pin, unsigned level);
unsigned gpio_get_level(int pin);
void gpio_set_direction(int pin, unsigned direction);
void gpio_pinclear(int pin);
void gpio_toggle(int pin);
void gpio_wrpin(int pin, unsigned wrval);
void gpio_wxpin(int pin, unsigned xval);
void gpio_wypin(int pin, unsigned yval);
void gpio_akpin(int pin);
unsigned gpio_rdpin(int pin);
unsigned gpio_rqpin(int pin);

#endif // MICROPY_INCLUDED_GPIO_H

