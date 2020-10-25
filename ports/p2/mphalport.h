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

#endif // MICROPY_INCLUDED_MPHALPORT_H
