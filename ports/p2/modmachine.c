/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

#include "modmachine.h"
#include "py/gc.h"
#include "py/runtime.h"
#include "py/objstr.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "extmod/machine_mem.h"
#include "sdcard.h"
#include "extmod/machine_signal.h"
#if 0
#include "extmod/machine_pulse.h"
#include "extmod/machine_i2c.h"
#endif
#include "lib/utils/pyexec.h"
#if 0
#include "lib/oofatfs/ff.h"
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "gccollect.h"
#include "irq.h"
#include "powerctrl.h"
#include "pybthread.h"
#include "rng.h"
#include "storage.h"
#include "pin.h"
#include "timer.h"
#include "usb.h"
#include "rtc.h"
#include "i2c.h"
#include "spi.h"
#include "uart.h"
#include "wdt.h"

// machine.info([dump_alloc_table])
// Print out lots of information about the board.
STATIC mp_obj_t machine_info(size_t n_args, const mp_obj_t *args) {
    // get and print unique id; 96 bits
    {
        byte *id = (byte *)MP_HAL_UNIQUE_ID_ADDRESS;
        printf("ID=%02x%02x%02x%02x:%02x%02x%02x%02x:%02x%02x%02x%02x\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8], id[9], id[10], id[11]);
    }

    // get and print clock speeds
    // SYSCLK=168MHz, HCLK=168MHz, PCLK1=42MHz, PCLK2=84MHz
    {
        #if defined(STM32F0)
        printf("S=%u\nH=%u\nP1=%u\n",
            (unsigned int)HAL_RCC_GetSysClockFreq(),
            (unsigned int)HAL_RCC_GetHCLKFreq(),
            (unsigned int)HAL_RCC_GetPCLK1Freq());
        #else
        printf("S=%u\nH=%u\nP1=%u\nP2=%u\n",
            (unsigned int)HAL_RCC_GetSysClockFreq(),
            (unsigned int)HAL_RCC_GetHCLKFreq(),
            (unsigned int)HAL_RCC_GetPCLK1Freq(),
            (unsigned int)HAL_RCC_GetPCLK2Freq());
        #endif
    }

    // to print info about memory
    {
        printf("_etext=%p\n", &_etext);
        printf("_sidata=%p\n", &_sidata);
        printf("_sdata=%p\n", &_sdata);
        printf("_edata=%p\n", &_edata);
        printf("_sbss=%p\n", &_sbss);
        printf("_ebss=%p\n", &_ebss);
        printf("_sstack=%p\n", &_sstack);
        printf("_estack=%p\n", &_estack);
        printf("_ram_start=%p\n", &_ram_start);
        printf("_heap_start=%p\n", &_heap_start);
        printf("_heap_end=%p\n", &_heap_end);
        printf("_ram_end=%p\n", &_ram_end);
    }

    // qstr info
    {
        size_t n_pool, n_qstr, n_str_data_bytes, n_total_bytes;
        qstr_pool_info(&n_pool, &n_qstr, &n_str_data_bytes, &n_total_bytes);
        printf("qstr:\n  n_pool=%u\n  n_qstr=%u\n  n_str_data_bytes=%u\n  n_total_bytes=%u\n", n_pool, n_qstr, n_str_data_bytes, n_total_bytes);
    }

    // GC info
    {
        gc_info_t info;
        gc_info(&info);
        printf("GC:\n");
        printf("  %u total\n", info.total);
        printf("  %u : %u\n", info.used, info.free);
        printf("  1=%u 2=%u m=%u\n", info.num_1block, info.num_2block, info.max_block);
    }

    // free space on flash
    {
        #if MICROPY_VFS_FAT
        for (mp_vfs_mount_t *vfs = MP_STATE_VM(vfs_mount_table); vfs != NULL; vfs = vfs->next) {
            if (strncmp("/flash", vfs->str, vfs->len) == 0) {
                // assumes that it's a FatFs filesystem
                fs_user_mount_t *vfs_fat = MP_OBJ_TO_PTR(vfs->obj);
                DWORD nclst;
                f_getfree(&vfs_fat->fatfs, &nclst);
                printf("LFS free: %u bytes\n", (uint)(nclst * vfs_fat->fatfs.csize * 512));
                break;
            }
        }
        #endif
    }

    #if MICROPY_PY_THREAD
    pyb_thread_dump();
    #endif

    if (n_args == 1) {
        // arg given means dump gc allocation table
        gc_dump_alloc_table();
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_info_obj, 0, 1, machine_info);

#endif

static int id_get()
{ 
    int cogid;
    __asm__("cogid %0" : "=r" (cogid) ); 
    return cogid;
}

// Returns the COGID
STATIC mp_obj_t machine_cogid(void) {
    return mp_obj_new_int(id_get());
}
MP_DEFINE_CONST_FUN_OBJ_0(machine_cogid_obj, machine_cogid);

// RESET the P2 
STATIC mp_obj_t machine_reset(void) {
    __asm__("hubset ##$10000000"); 
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(machine_reset_obj, machine_reset);

STATIC mp_obj_t machine_soft_reset(void) {
    pyexec_system_exit = PYEXEC_FORCED_EXIT;
    mp_raise_type(&mp_type_SystemExit);
}
MP_DEFINE_CONST_FUN_OBJ_0(machine_soft_reset_obj, machine_soft_reset);

STATIC void atn(int mask) {
    __asm__("cogatn %0": : "r" (mask));
}

// COG ATN notification
STATIC mp_obj_t machine_cogatn(mp_obj_t cogmask) {
    mp_int_t mask = mp_obj_get_int(cogmask);
    atn(mask);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_cogatn_obj, machine_cogatn);

// RNG read from P2 HW
unsigned int p2_random(void) {
    unsigned int num;
    __asm__("getrnd %0" : "=r" (num) ); 
    return num;
}

// Return a 24-bit hardware generated random number.
STATIC mp_obj_t machine_rng_get(void) {
    return mp_obj_new_int(p2_random() >> 8);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_rng_obj, machine_rng_get);

// get or set the P2 frequency
STATIC mp_obj_t machine_freq(size_t n_args, const mp_obj_t *args) {
    if (n_args == 0) {
        // get
        return mp_obj_new_int(*(int32_t *)0x14);
    } else {
        // set
        printf("Not implemented yet\n");
#if 0
        mp_int_t sysclk = mp_obj_get_int(args[0]);
        if (n_args > 1) {
            mp_int_t clkmode = mp_obj_get_int(args[1]);
        }
        int ret = powerctrl_set_sysclk(sysclk, ahb, apb1, apb2);
        if (ret == -MP_EINVAL) {
            mp_raise_ValueError(MP_ERROR_TEXT("invalid freq"));
        } else if (ret < 0) {
            void NORETURN __fatal_error(const char *msg);
            __fatal_error("can't change freq");
        }
#endif
        return mp_const_none;
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_freq_obj, 0, 2, machine_freq);


STATIC const mp_rom_map_elem_t machine_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_ROM_QSTR(MP_QSTR_umachine) },
    { MP_ROM_QSTR(MP_QSTR_cogid),               MP_ROM_PTR(&machine_cogid_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset),               MP_ROM_PTR(&machine_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_soft_reset),          MP_ROM_PTR(&machine_soft_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_cogatn),              MP_ROM_PTR(&machine_cogatn_obj) },
    { MP_ROM_QSTR(MP_QSTR_freq),                MP_ROM_PTR(&machine_freq_obj) },
    #if MICROPY_HW_ENABLE_RNG
    { MP_ROM_QSTR(MP_QSTR_rng),                 MP_ROM_PTR(&machine_rng_obj) },
    #endif

 // { MP_ROM_QSTR(MP_QSTR_time_pulse_us),       MP_ROM_PTR(&machine_time_pulse_us_obj) },

    { MP_ROM_QSTR(MP_QSTR_mem8),                MP_ROM_PTR(&machine_mem8_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem16),               MP_ROM_PTR(&machine_mem16_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem32),               MP_ROM_PTR(&machine_mem32_obj) },

    { MP_ROM_QSTR(MP_QSTR_Cpu),                 MP_ROM_PTR(&machine_cpu_type) },
    { MP_ROM_QSTR(MP_QSTR_Pin),                 MP_ROM_PTR(&machine_pin_type) },
    { MP_ROM_QSTR(MP_QSTR_Signal),              MP_ROM_PTR(&machine_signal_type) },

//  { MP_ROM_QSTR(MP_QSTR_RTC),                 MP_ROM_PTR(&pyb_rtc_type) },
//  { MP_ROM_QSTR(MP_QSTR_ADC),                 MP_ROM_PTR(&machine_adc_type) },
    #if MICROPY_PY_MACHINE_I2C
    { MP_ROM_QSTR(MP_QSTR_I2C),                 MP_ROM_PTR(&machine_i2c_type) },
    #endif
//  { MP_ROM_QSTR(MP_QSTR_SPI),                 MP_ROM_PTR(&machine_hard_spi_type) },
//  { MP_ROM_QSTR(MP_QSTR_UART),                MP_ROM_PTR(&pyb_uart_type) },
//  { MP_ROM_QSTR(MP_QSTR_WDT),                 MP_ROM_PTR(&pyb_wdt_type) },
//  { MP_ROM_QSTR(MP_QSTR_Timer),               MP_ROM_PTR(&machine_timer_type) },
#if MICROPY_HW_ENABLE_SDCARD
    { MP_ROM_QSTR(MP_QSTR_SDCard),              MP_ROM_PTR(&pyb_sdcard_type) },
#endif

};

STATIC MP_DEFINE_CONST_DICT(machine_module_globals, machine_module_globals_table);

const mp_obj_module_t mp_module_machine = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&machine_module_globals,
};

