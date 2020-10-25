/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2019 Damien P. George
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

#include "py/runtime.h"
#include "py/mphal.h"
#include "lib/oofatfs/ff.h"
#include "extmod/vfs_fat.h"

#include "sdcard.h"
#include "gpio.h"

// TODO: This ported code needs further cleanup and is likely temporary.

// We may ultimately use the SPI support built into MicroPython in
// extmod/machine_spi.c to allow different pins to be used that don't 
// conflict with the Flash chip.
// We may also wish to use a smart pin mode to regulate the timing
// at different frequencies. TBD. (rogloh) 

#define _dirh(x) gpio_set_direction(x, 1)
#define _dirl(x) gpio_set_direction(x, 0)
#define drvl_(x) gpio_set_level(x, 0)
#define drvh_(x) gpio_set_level(x, 1)
#define getpin(x) gpio_get_level(x)
#define pin_read getpin
#define _pinl drvl_
#define _pinh drvh_
#define _pinf(x) do { \
          gpio_set_direction(x, 0); \
          gpio_set_latch(x, 0); \
        } while (0);

#define getcycles mp_hal_ticks_us


#define PIN_SS   60
#define PIN_MOSI 59
#define PIN_MISO 58
#define PIN_CLK  61
#define PIN_SD_TEST1 (58) /* if high, assume card present */
#define PIN_SD_TEST2 (60) /* if pullup on this pin, assume card present */

#if MICROPY_HW_ENABLE_SDCARD

#define PYB_SDMMC_FLAG_SD       (0x01)
#define PYB_SDMMC_FLAG_SDHC     (0x02)
#define PYB_SDMMC_FLAG_ACTIVE   (0x04)

static uint8_t pyb_sdmmc_flags;
static uint8_t isSDHC;

static int spi_read()
{
    int i, r;
    r = 0;
    for (i = 0; i < 8; i++) {
        drvl_(PIN_CLK);
        drvh_(PIN_CLK);
        mp_hal_delay_us(1);
        r = (r << 1) | getpin(PIN_MISO);
    }
    return r;
}

// timeout is 100ms
#define TIMEOUT 100000

static int spi_chktimeout(uint32_t endtime) {
    int32_t now = (int32_t) getcycles();
    if (now - (int32_t)endtime > 0) {
        return 1;
    }
    return 0;
}

static int spi_readresp(void)
{
    int r;
    uint32_t endtime = getcycles() + TIMEOUT;
    
    while (1) {
        r = spi_read();
        if (r != 0xff) break;
        if (spi_chktimeout(endtime)) {
            return -1;
        }
    }
    return r;
}

static void spi_send(int outv)
{
    int i;
    for (i = 0; i < 8; i++) {
        drvl_(PIN_CLK);
        if (outv & 0x80) {
            drvh_(PIN_MOSI);
        } else {
            drvl_(PIN_MOSI);
        }
        outv <<= 1;
        drvh_(PIN_CLK);
    }
    drvh_(PIN_MOSI);
}

//
//  Wait until card stops returning busy
//
int spi_busy(void)
{
    int r;
    uint32_t endtime = getcycles() + 10*TIMEOUT;

    while (1)
    {
        r = spi_read();
        if (r) break;
        if (spi_chktimeout(endtime)) {
            return -1;
        }
    }

    return r;
}

// calculate crc for commands
// TODO: A table lookup is faster than bitwise
static int crc7(int crc, int val)
{
    int i;
    for (i = 0; i < 8; i++) {
        crc = crc << 1;
        if ( (crc^val) & 0x80 ) {
            crc ^= 0x09;
        }
        val = val<<1;
    }
    return crc & 0x7f;
}

static int crc_send(int crc, int byte)
{
    crc = crc7(crc, byte);
    spi_send(byte);
    return crc;
}

static int spi_cmd(int index, int arg)
{
    int crc = 0;
    drvl_(PIN_SS);
    (void)spi_read();
    crc = crc_send(crc, 0x40+index);
    crc = crc_send(crc, (arg >> 24) & 0xff);
    crc = crc_send(crc, (arg >> 16) & 0xff);
    crc = crc_send(crc, (arg >> 8) & 0xff);
    crc = crc_send(crc, arg & 0xff);

    // send the CRC
    crc = (crc<<1) | 1;
    spi_send(crc);
    return spi_readresp();
}

static void spi_endcmd(void)
{
    drvh_(PIN_SS);
}

static int sdcard_wait_for_status(void)
{
    int tries = 0;
    int i;
    while (tries++ < 100) {
        spi_cmd(13, 0);
        i = spi_readresp();
        spi_endcmd();
        if (i == 0) {
            return 0;
        }
    }
    return -1;
}

/* main code */
void sdcard_init(void) {
    // Set SD/MMC to no mode and inactive
    pyb_sdmmc_flags = 0;

    // configure SD GPIO
    
    _dirl(PIN_MISO);
    _dirh(PIN_CLK);
    // _pinh(PIN_SS); - (rogloh) original SS init position
}

bool sdcard_is_present(void) {
    int i;

    if (pyb_sdmmc_flags & PYB_SDMMC_FLAG_ACTIVE) {
        return true;
    }
    i = pin_read(PIN_SD_TEST1);
    if (i == 0) return 1;

    // check for a pull-up on pin 60
    _pinl(PIN_SD_TEST2);
    //_waitx(200); // wait > 1us
    mp_hal_delay_us(2);
    _pinf(PIN_SD_TEST2); // does a fltl
    //_waitx(1000); // wait > 5 us
    mp_hal_delay_us(6);
    i = pin_read(PIN_SD_TEST2);
    return i;
}

bool sdcard_power_on(void) {
    int i;
    int x;

    if (pyb_sdmmc_flags & PYB_SDMMC_FLAG_ACTIVE) {
        return true;
    }
    _pinh(PIN_MOSI);
    _pinh(PIN_SS); // (rogloh) new SS init position to allow flash access until enabled
    for (i = 0; i < 600; i++) {
        (void) spi_read();
    }

    i = spi_cmd(0, 0);
    spi_endcmd();
    if (i < 0) {
        return false;
    }
    i = spi_cmd(8, 0x1aa);
    spi_endcmd();
    if (i < 0) {
        return false;
    }
    while (1) {
        spi_cmd(55, 0);
        i = spi_cmd(41, 0x40000000);
        if (i == 0) break;
        if (i < 0) {
            return false;
        }
    }
    i = spi_cmd(58, 0);
    x = spi_read();
    spi_endcmd();
    isSDHC = (x >> 6) & 1;
    if (i) {
//        printf("could not init card\n");
        return false;
    } else {
//        printf("cmd58 returned 0x%x\n", x);
    }
    i = sdcard_wait_for_status();
    if (i < 0) {
        //printf("timeout waiting for card\n");
        return false;
    }
    pyb_sdmmc_flags |= PYB_SDMMC_FLAG_ACTIVE;
    return true;
}

void sdcard_power_off(void) {
    pyb_sdmmc_flags &= ~PYB_SDMMC_FLAG_ACTIVE;
    _pinl(PIN_MOSI);
    _dirl(PIN_MOSI);
    _pinf(PIN_SS); // (rogloh) float the SS pin to allow flash access
 }

uint64_t sdcard_get_capacity_in_bytes(void) {
    uint64_t c_size;
    uint8_t csd[16]; // 16 bytes
    int i;
    if (!(pyb_sdmmc_flags & PYB_SDMMC_FLAG_ACTIVE)) {
        return 0;
    }
    i = spi_cmd(9, 0);
    i = spi_readresp();
    if (i < 0) {
        //iprintf("csd read timed out\n");
        spi_endcmd();
        return 0;
    }
    for (i = 0; i < 16; i++) {
        csd[i] = spi_read();
    }
    (void)spi_read();
    (void)spi_read();
    spi_endcmd();

    c_size = (((uint32_t)csd[7] & 0x3f) << 16) | ((uint32_t)csd[8] << 8) | csd[9];
    c_size = (c_size+1) * (512ULL * 1024ULL);
    return c_size;
}


// read one block
static int sdcard_read_one_block(uint8_t *dest, uint32_t offset)
{
    int r, i;
    if (!isSDHC) {
        offset = offset << 9; // multiply by 512 for non SDHC cards
    }
    spi_cmd(17, offset);
    r = spi_readresp();
    if (r < 0) return r;
    for (i = 0; i < 512; i++) {
        *dest++ = spi_read();
    }
    spi_read(); spi_read();
    spi_endcmd();
    return 0;
}

// read one block
static int sdcard_write_one_block(const uint8_t *src, uint32_t offset)
{
    int r, i;
    if (!isSDHC) {
        offset = offset << 9; // multiply by 512 for non SDHC cards
    }
    spi_cmd(24, offset);
    spi_send(0xfe);
    for (i = 0; i < 512; i++) {
        spi_send(*src++);
    }
    spi_read(); spi_read();
    r = spi_readresp();
    if ((r & 0x1f) != 5) {
        return -42;
    }
    r = spi_busy();
    spi_endcmd();
    if (r < 0) return r;
    return 0;
}

mp_uint_t sdcard_read_blocks(uint8_t *dest, uint32_t start_block, uint32_t num_blocks) {
    uint32_t i;
    int r;

    // check that SD card is initialised
    if (!(pyb_sdmmc_flags & PYB_SDMMC_FLAG_ACTIVE)) {
        return -1;
    }
    //printf("sdcard_read_blocks: start=%lu num=%lu\n", start_block, num_blocks);
    for (i = 0; i < num_blocks; i++) {
        r = sdcard_read_one_block(dest, start_block);
        if (r) return r;
        dest += 512;
        start_block++;
    }
    return 0;
}

mp_uint_t sdcard_write_blocks(const uint8_t *src, uint32_t start_block, uint32_t num_blocks) {
    uint32_t i;
    int r;
    // check that SD card is initialised
    if (!(pyb_sdmmc_flags & PYB_SDMMC_FLAG_ACTIVE)) {
        return -1;
    }
    for (i = 0; i < num_blocks; i++) {
        r = sdcard_write_one_block(src, start_block);
        if (r) return r;
        src += 512;
        start_block++;
    }
    return 0;
}

/******************************************************************************/
// MicroPython bindings
//
// Expose the SD card or MMC as an object with the block protocol.

// There are singleton SDCard/MMCard objects
#if MICROPY_HW_ENABLE_SDCARD
const mp_obj_base_t pyb_sdcard_obj = {&pyb_sdcard_type};
#endif
#if MICROPY_HW_ENABLE_MMCARD
const mp_obj_base_t pyb_mmcard_obj = {&pyb_mmcard_type};
#endif

#if MICROPY_HW_ENABLE_SDCARD
STATIC mp_obj_t pyb_sdcard_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    #if MICROPY_HW_ENABLE_MMCARD
    if (pyb_sdmmc_flags & PYB_SDMMC_FLAG_MMC) {
        mp_raise_ValueError("peripheral used by MMCard");
    }
    #endif

    pyb_sdmmc_flags |= PYB_SDMMC_FLAG_SD;

    // return singleton object
    return MP_OBJ_FROM_PTR(&pyb_sdcard_obj);
}
#endif

#if MICROPY_HW_ENABLE_MMCARD
STATIC mp_obj_t pyb_mmcard_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    #if MICROPY_HW_ENABLE_SDCARD
    if (pyb_sdmmc_flags & PYB_SDMMC_FLAG_SD) {
        mp_raise_ValueError("peripheral used by SDCard");
    }
    #endif

    pyb_sdmmc_flags |= PYB_SDMMC_FLAG_MMC;

    // return singleton object
    return MP_OBJ_FROM_PTR(&pyb_mmcard_obj);
}
#endif

STATIC mp_obj_t sd_present(mp_obj_t self) {
    return mp_obj_new_bool(sdcard_is_present());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sd_present_obj, sd_present);

STATIC mp_obj_t sd_power(mp_obj_t self, mp_obj_t state) {
    bool result;
    if (mp_obj_is_true(state)) {
        result = sdcard_power_on();
    } else {
        sdcard_power_off();
        result = true;
    }
    return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(sd_power_obj, sd_power);

STATIC mp_obj_t sd_info(mp_obj_t self) {
    if (!(pyb_sdmmc_flags & PYB_SDMMC_FLAG_ACTIVE)) {
        return mp_const_none;
    }
#ifdef FIXME    
    uint32_t card_type;
    uint32_t log_block_nbr;
    uint32_t log_block_size;
    {
        card_type = sdmmc_handle.sd.SdCard.CardType;
        log_block_nbr = sdmmc_handle.sd.SdCard.LogBlockNbr;
        log_block_size = sdmmc_handle.sd.SdCard.LogBlockSize;
    }
    // cardinfo.SD_csd and cardinfo.SD_cid have lots of info but we don't use them
    mp_obj_t tuple[3] = {
        mp_obj_new_int_from_ull((uint64_t)log_block_nbr * (uint64_t)log_block_size),
        mp_obj_new_int_from_uint(log_block_size),
        mp_obj_new_int(card_type),
    };
    return mp_obj_new_tuple(3, tuple);
#else
    printf("*** warning: sd_info ignored\n");
    return mp_const_none;
#endif
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(sd_info_obj, sd_info);

// now obsolete, kept for backwards compatibility
STATIC mp_obj_t sd_read(mp_obj_t self, mp_obj_t block_num) {
    uint8_t *dest = m_new(uint8_t, SDCARD_BLOCK_SIZE);
    mp_uint_t ret = sdcard_read_blocks(dest, mp_obj_get_int(block_num), 1);

    if (ret != 0) {
        m_del(uint8_t, dest, SDCARD_BLOCK_SIZE);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception, "sdcard_read_blocks failed [%u]", ret));
    }

    return mp_obj_new_bytearray_by_ref(SDCARD_BLOCK_SIZE, dest);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(sd_read_obj, sd_read);

// now obsolete, kept for backwards compatibility
STATIC mp_obj_t sd_write(mp_obj_t self, mp_obj_t block_num, mp_obj_t data) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);
    if (bufinfo.len % SDCARD_BLOCK_SIZE != 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "writes must be a multiple of %d bytes", SDCARD_BLOCK_SIZE));
    }

    mp_uint_t ret = sdcard_write_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / SDCARD_BLOCK_SIZE);

    if (ret != 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception, "sdcard_write_blocks failed [%u]", ret));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(sd_write_obj, sd_write);

STATIC mp_obj_t pyb_sdcard_readblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_WRITE);
    mp_uint_t ret = sdcard_read_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / SDCARD_BLOCK_SIZE);
    return mp_obj_new_bool(ret == 0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_sdcard_readblocks_obj, pyb_sdcard_readblocks);

STATIC mp_obj_t pyb_sdcard_writeblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    mp_uint_t ret = sdcard_write_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / SDCARD_BLOCK_SIZE);
    return mp_obj_new_bool(ret == 0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_sdcard_writeblocks_obj, pyb_sdcard_writeblocks);

STATIC mp_obj_t pyb_sdcard_ioctl(mp_obj_t self, mp_obj_t cmd_in, mp_obj_t arg_in) {
    mp_int_t cmd = mp_obj_get_int(cmd_in);
    switch (cmd) {
        case MP_BLOCKDEV_IOCTL_INIT:
            if (!sdcard_power_on()) {
                return MP_OBJ_NEW_SMALL_INT(-1); // error
            }
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case MP_BLOCKDEV_IOCTL_DEINIT:
            sdcard_power_off();
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case MP_BLOCKDEV_IOCTL_SYNC:
            // nothing to do
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT:
            return MP_OBJ_NEW_SMALL_INT(sdcard_get_capacity_in_bytes() / SDCARD_BLOCK_SIZE);

        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE:
            return MP_OBJ_NEW_SMALL_INT(SDCARD_BLOCK_SIZE);

        default: // unknown command
            return MP_OBJ_NEW_SMALL_INT(-1); // error
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_sdcard_ioctl_obj, pyb_sdcard_ioctl);

STATIC const mp_rom_map_elem_t pyb_sdcard_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_present), MP_ROM_PTR(&sd_present_obj) },
    { MP_ROM_QSTR(MP_QSTR_power), MP_ROM_PTR(&sd_power_obj) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&sd_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&sd_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&sd_write_obj) },
    // block device protocol
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&pyb_sdcard_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&pyb_sdcard_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&pyb_sdcard_ioctl_obj) },
};

STATIC MP_DEFINE_CONST_DICT(pyb_sdcard_locals_dict, pyb_sdcard_locals_dict_table);

#if MICROPY_HW_ENABLE_SDCARD
const mp_obj_type_t pyb_sdcard_type = {
    { &mp_type_type },
    .name = MP_QSTR_SDCard,
    .make_new = pyb_sdcard_make_new,
    .locals_dict = (mp_obj_dict_t*)&pyb_sdcard_locals_dict,
};
#endif

#if MICROPY_HW_ENABLE_MMCARD
const mp_obj_type_t pyb_mmcard_type = {
    { &mp_type_type },
    .name = MP_QSTR_MMCard,
    .make_new = pyb_mmcard_make_new,
    .locals_dict = (mp_obj_dict_t*)&pyb_sdcard_locals_dict,
};
#endif

void sdcard_init_vfs(fs_user_mount_t *vfs, int part) {
    pyb_sdmmc_flags = (pyb_sdmmc_flags & PYB_SDMMC_FLAG_ACTIVE) | PYB_SDMMC_FLAG_SD; // force SD mode
    vfs->base.type = &mp_fat_vfs_type;
    vfs->blockdev.flags |= MP_BLOCKDEV_FLAG_NATIVE | MP_BLOCKDEV_FLAG_HAVE_IOCTL;
    vfs->fatfs.drv = vfs;
    vfs->fatfs.part = part;
    vfs->blockdev.readblocks[0] = MP_OBJ_FROM_PTR(&pyb_sdcard_readblocks_obj);
    vfs->blockdev.readblocks[1] = MP_OBJ_FROM_PTR(&pyb_sdcard_obj);
    vfs->blockdev.readblocks[2] = MP_OBJ_FROM_PTR(sdcard_read_blocks); // native version
    vfs->blockdev.writeblocks[0] = MP_OBJ_FROM_PTR(&pyb_sdcard_writeblocks_obj);
    vfs->blockdev.writeblocks[1] = MP_OBJ_FROM_PTR(&pyb_sdcard_obj);
    vfs->blockdev.writeblocks[2] = MP_OBJ_FROM_PTR(sdcard_write_blocks); // native version
    vfs->blockdev.u.ioctl[0] = MP_OBJ_FROM_PTR(&pyb_sdcard_ioctl_obj);
    vfs->blockdev.u.ioctl[1] = MP_OBJ_FROM_PTR(&pyb_sdcard_obj);
}

#if 0
// disk utilities
uint32_t get_fattime(void)
{
    uint64_t seconds = getcycles() / 160000000;
    uint32_t dosdate;
    uint32_t month, day, year;
    uint32_t hh, mm, ss;

    ss = (uint32_t)seconds;
    year = 2019;
    month = 6;
    day = ss / 86400;
    if (day > 30) day = 31;
    ss = ss % 86400;
    hh = ss / 3600;
    ss = ss % 3600;
    mm = ss / 60;
    ss = ss % 60;

    dosdate = ((year-1980) << 25) | ((month+1)<<31) | ((day+1)<<16);
    dosdate |= (hh << 11) | (mm << 5) | (ss >> 1);
    return dosdate;
}
#endif
#endif // MICROPY_HW_ENABLE_SDCARD || MICROPY_HW_ENABLE_MMCARD
