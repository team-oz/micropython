#include "gpio.h"

inline void gpio_set_latch(unsigned int pin, unsigned int level) {
    if (level)
        __asm__("outh %0" : : "r" (pin));
    else
        __asm__("outl %0" : : "r" (pin));
}

inline void gpio_set_level(unsigned int pin, unsigned int level) {
    if (level)
        __asm__("drvh %0" : : "r" (pin));
    else
        __asm__("drvl %0" : : "r" (pin));
}

inline unsigned int gpio_get_level(unsigned int pin)
{
    unsigned int value;
    
    __asm__("testp %0 wc" : : "r" (pin));
    __asm__("wrc %0" : "=r" (value));
    return value;
}

inline void gpio_set_direction(unsigned int pin, unsigned int direction)
{
    if (direction) {
        __asm__("dirh %0" : : "r" (pin));
    } else {
        __asm__("dirl %0" : : "r" (pin));
    }
}

inline void gpio_pinclear(unsigned int pin)
{
        __asm__("dirl %0" : : "r" (pin));
        gpio_wrpin(pin, 0);
}

inline void gpio_toggle(unsigned int pin)
{
    __asm__("drvnot %0" : : "r" (pin));
}

inline void gpio_wrpin(unsigned int pin, unsigned int wrval)
{
    __asm__("wrpin %0, %1" : : "r" (wrval), "r" (pin));
}

inline void gpio_wxpin(unsigned int pin, unsigned int xval)
{
    __asm__("wxpin %0, %1" : : "r" (xval), "r" (pin));
}

inline void gpio_wypin(unsigned int pin, unsigned int yval)
{
    __asm__("wypin %0, %1" : : "r" (yval), "r" (pin));
}

inline unsigned int gpio_rdpin(unsigned int pin)
{
    unsigned int value;

    __asm__("rdpin %0, %1" : "=r" (value) : "r" (pin));
    return value;
}

inline unsigned int gpio_rqpin(unsigned int pin)
{
    unsigned int value;

    __asm__("rqpin %0, %1" : "=r" (value) : "r" (pin));
    return value;
}

inline void gpio_akpin(unsigned int pin)
{
    __asm__("akpin %0" : : "r" (pin));
}

