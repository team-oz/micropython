#include "gpio.h"

inline void gpio_set_latch(int pin, unsigned level) {
    if (level)
        __asm__("outh %0" : : "r" (pin));
    else
        __asm__("outl %0" : : "r" (pin));
}

inline void gpio_set_level(int pin, unsigned level) {
    if (level)
        __asm__("drvh %0" : : "r" (pin));
    else
        __asm__("drvl %0" : : "r" (pin));
}

inline unsigned gpio_get_level(int pin)
{
    unsigned value;
    
    __asm__("testp %0 wc" : : "r" (pin));
    __asm__("wrc %0" : "=r" (value));
    return value;
}

inline void gpio_set_direction(int pin, unsigned direction)
{
    if (direction) {
        __asm__("dirh %0" : : "r" (pin));
    } else {
        __asm__("dirl %0" : : "r" (pin));
    }
}

inline void gpio_pinclear(int pin)
{
        __asm__("dirl %0" : : "r" (pin));
        gpio_wrpin(pin, 0);
}

inline void gpio_toggle(int pin)
{
    __asm__("drvnot %0" : : "r" (pin));
}

inline void gpio_wrpin(int pin, unsigned wrval)
{
    __asm__("wrpin %0, %1" : : "r" (wrval), "r" (pin));
}

inline void gpio_wxpin(int pin, unsigned xval)
{
    __asm__("wxpin %0, %1" : : "r" (xval), "r" (pin));
}

inline void gpio_wypin(int pin, unsigned yval)
{
    __asm__("wypin %0, %1" : : "r" (yval), "r" (pin));
}

inline unsigned gpio_rdpin(int pin)
{
    unsigned value;

    __asm__("rdpin %0, %1" : "=r" (value) : "r" (pin));
    return value;
}

inline unsigned gpio_rqpin(int pin)
{
    unsigned value;

    __asm__("rqpin %0, %1" : "=r" (value) : "r" (pin));
    return value;
}

inline void gpio_akpin(int pin)
{
    __asm__("akpin %0" : : "r" (pin));
}

