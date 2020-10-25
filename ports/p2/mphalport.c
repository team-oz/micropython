#include <unistd.h>
#include "py/mpconfig.h"

/*
 * Core UART functions to implement for a port
 */

void uart_init(void)
{
    __asm__("push   r0");
    __asm__("flth   #62");
    __asm__("wrpin  ##%0000_0000_000_0000000000000_01_11110_0, #62 'set smartpin async tx serial");
    __asm__("rdlong r0, ##_p2bitcycles");
    __asm__("shl    r0, #16");
    __asm__("or     r0, #7 ' 8 data bits");
    __asm__("wxpin  r0, #62");
    __asm__("drvh   #62");
    __asm__("pop    r0");
}

void uart_tx(int val)
{
    __asm__("putpoll");
    __asm__("            rqpin   inb, #62   wc 'transmitting? (C high == yes)  Needed to initiate tx");
    __asm__("            testp   #62        wz 'buffer free? (IN high == yes)");
    __asm__("if_nc_or_z  wypin   %0, #62       'write new byte to Y buffer" : : "r" (val));
#if 0
    __asm__("if_c_and_nz jmp ##$-12");
    __asm__("            jmp lr");
#else
    __asm__("if_nc_or_z  jmp lr                'return");
#endif
    __asm__("            jmp #putpoll          'wait while Smartpin is both full (nz) and transmitting (c)");

}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) 
{
    while (len--)
        uart_tx(*str++);
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"

int uart_rx(void)
{

#if 0
    int ch;
    __asm__("call  #\\__RXCHAR" : "=r" (ch));
    return ch;
#else
    __asm__("push lr");
//    __asm__("calld lr, #getrx");
    __asm__("calld lr, #__RXCHAR");
    __asm__("pop lr");
#endif

}

// Receive single character
int mp_hal_stdin_rx_chr(void) 
{
    int data = 0;
    while ((data = uart_rx()) < 0); // poll until data is present
    return (unsigned char)data;
}

mp_uint_t mp_hal_ticks_ms(void) 
{
    // TODO: apply extended asm syntax with register constraints
    // mp_uint_t ticks;
    __asm__("push r1");
    __asm__("push r2");
    
    __asm__("getct r0 wc");
    __asm__("getct r1");

    __asm__("rdlong r2, #$14"); // system clock freq at address $14
    __asm__("qdiv r2, ##1000"); // determine clocks per millisecond
    __asm__("getqx r2");
    __asm__("setq r0");
    __asm__("qdiv r1, r2"); // divide system counter by (clocks per msec)
    __asm__("getqx r0");  // result returned in r0

    __asm__("pop r2");
    __asm__("pop r1");
}


mp_uint_t mp_hal_ticks_us(void) 
{
    // TODO: apply extended asm syntax with register constraints
    // mp_uint_t ticks;
    __asm__("push r1");
    __asm__("push r2");

    __asm__("getct r0 wc");
    __asm__("getct r1");

    __asm__("rdlong r2, #$14"); // system clock freq at address $14
    __asm__("qdiv r2, ##1000000"); // determine clocks per millisecond
    __asm__("getqx r2");
    __asm__("setq r0");
    __asm__("qdiv r1, r2"); // divide system counter by (clocks per usec)
    __asm__("getqx r0");  // result returned in r0

    __asm__("pop r2");
    __asm__("pop r1");
}
#pragma GCC diagnostic pop

mp_uint_t mp_hal_ticks_cpu(void)
{
    mp_uint_t ticks;
    __asm__("getct %0" : "=r" (ticks) );
    return ticks;
}

void mp_hal_delay_ms(mp_uint_t ms)
{
    uint32_t temp;
    
    if (ms)
    {
    __asm__("rdlong %0, #$14" : "=r" (temp));
    __asm__("augs #1000");
    __asm__("qdiv %0, #(1000 & $1ff)" : : "r" (temp));
    __asm__("getqx %0" : "=r" (temp));
    __asm__("qmul %0, %1" : : "r" (ms),  "r" (temp));
    __asm__("getqx %0" : "=r" (ms));
    __asm__("getct %0" : "=r" (temp));
    __asm__("addct1 %0, %2" : "=r" (ms) : "0" (ms), "r" (temp));
    __asm__("waitct1" : : "r" (ms), "r" (temp));
    }
}

void mp_hal_delay_us(mp_uint_t us)
{
    uint32_t temp;
    
    if (us)
    {
    __asm__("rdlong %0, #$14" : "=r" (temp));
    __asm__("augs #1000000");
    __asm__("qdiv %0, #(1000000 & $1ff)" : : "r" (temp));
    __asm__("getqx %0" : "=r" (temp));
    __asm__("qmul %0, %1" : : "r" (us),  "r" (temp));
    __asm__("getqx %0" : "=r" (us));
    __asm__("getct %0" : "=r" (temp));
    __asm__("addct1 %0, %2" : "=r" (us) : "0" (us), "r" (temp));
    __asm__("waitct1" : : "r" (us), "r" (temp));
    }
}
