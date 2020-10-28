#ifndef MICROPY_INCLUDED_MPHALPORT_H
#define MICROPY_INCLUDED_MPHALPORT_H

extern void uart_init(void);

extern mp_uint_t mp_hal_ticks_ms(void);
extern mp_uint_t mp_hal_ticks_us(void);
extern mp_uint_t mp_hal_ticks_cpu(void);

extern void mp_hal_delay_ms(mp_uint_t ms);
extern void mp_hal_delay_us(mp_uint_t us);

static inline void mp_hal_set_interrupt_char(char c) {}
static inline uint64_t mp_hal_time_ns(void) { return 0; }

#define mp_hal_delay_us_fast(us) mp_hal_delay_us(us)

#include "py/obj.h"
#include "gpio.h"
#define MP_HAL_PIN_FMT "%u"
#define mp_hal_pin_obj_t mp_uint_t
mp_hal_pin_obj_t machine_pin_get_id(mp_obj_t pin_in);
#define mp_hal_get_pin_obj(o) machine_pin_get_id(o)
#define mp_hal_pin_name(p) (p)
static inline void mp_hal_pin_input(mp_hal_pin_obj_t pin) {
    //gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}
static inline void mp_hal_pin_output(mp_hal_pin_obj_t pin) {
    //gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT);
}
static inline void mp_hal_pin_open_drain(mp_hal_pin_obj_t pin) {
    //gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT_OD);
}
static inline void mp_hal_pin_od_low(mp_hal_pin_obj_t pin) {
    gpio_set_level(pin, 0);
}
static inline void mp_hal_pin_od_high(mp_hal_pin_obj_t pin) {
    gpio_set_level(pin, 1);
}
static inline int mp_hal_pin_read(mp_hal_pin_obj_t pin) {
    return gpio_get_level(pin);
}
static inline void mp_hal_pin_write(mp_hal_pin_obj_t pin, int v) {
    gpio_set_level(pin, v);
}


#endif // MICROPY_INCLUDED_MPHALPORT_H
