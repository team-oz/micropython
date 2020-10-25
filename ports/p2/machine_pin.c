/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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
#include "mphalport.h"
#include "modmachine.h"
#include "extmod/virtpin.h"
#include "gpio.h"

#include "smartpindefs.h"
// Used to implement a range of drive/pull capabilities
#define GPIO_PULL_DOWN_FAST  (0)
#define GPIO_PULL_DOWN_1K5   (1)
#define GPIO_PULL_DOWN_15K   (2)
#define GPIO_PULL_DOWN_150K  (3)
#define GPIO_PULL_DOWN_1MA   (4)
#define GPIO_PULL_DOWN_100UA (5)
#define GPIO_PULL_DOWN_10UA  (6)
#define GPIO_PULL_DOWN_FLOAT (7)
#define GPIO_PULL_UP_FAST    (8)
#define GPIO_PULL_UP_1K5     (9)
#define GPIO_PULL_UP_15K    (10)
#define GPIO_PULL_UP_150K   (11)
#define GPIO_PULL_UP_1MA    (12)
#define GPIO_PULL_UP_100UA  (13)
#define GPIO_PULL_UP_10UA   (14)
#define GPIO_PULL_UP_FLOAT  (15)

// default pull up/down
#define GPIO_PULL_UP   (GPIO_PULL_UP_1K5)
#define GPIO_PULL_DOWN (GPIO_PULL_DOWN_1K5) 

// GPIO pin mode types
#define GPIO_MODE_INPUT (0)
#define GPIO_MODE_INPUT_OUTPUT (1)
#define GPIO_MODE_INPUT_OUTPUT_OD (2)

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    mp_uint_t id;
    mp_uint_t mode; // gpio mode (in/out/od)
    mp_uint_t r; // smartpin wrpin
    mp_uint_t x; // smartpin wxpin
    mp_uint_t y; // smartpin wypin
} machine_pin_obj_t;

STATIC machine_pin_obj_t machine_pin_obj[] = {
    {{&machine_pin_type}, 0},
    {{&machine_pin_type}, 1},
    {{&machine_pin_type}, 2},
    {{&machine_pin_type}, 3},
    {{&machine_pin_type}, 4},
    {{&machine_pin_type}, 5},
    {{&machine_pin_type}, 6},
    {{&machine_pin_type}, 7},
    {{&machine_pin_type}, 8},
    {{&machine_pin_type}, 9},
    {{&machine_pin_type},10},
    {{&machine_pin_type},11},
    {{&machine_pin_type},12},
    {{&machine_pin_type},13},
    {{&machine_pin_type},14},
    {{&machine_pin_type},15},
    {{&machine_pin_type},16},
    {{&machine_pin_type},17},
    {{&machine_pin_type},18},
    {{&machine_pin_type},19},
    {{&machine_pin_type},20},
    {{&machine_pin_type},21},
    {{&machine_pin_type},22},
    {{&machine_pin_type},23},
    {{&machine_pin_type},24},
    {{&machine_pin_type},25},
    {{&machine_pin_type},26},
    {{&machine_pin_type},27},
    {{&machine_pin_type},28},
    {{&machine_pin_type},29},
    {{&machine_pin_type},30},
    {{&machine_pin_type},31},
    {{&machine_pin_type},32},
    {{&machine_pin_type},33},
    {{&machine_pin_type},34},
    {{&machine_pin_type},35},
    {{&machine_pin_type},36},
    {{&machine_pin_type},37},
    {{&machine_pin_type},38},
    {{&machine_pin_type},39},
    {{&machine_pin_type},40},
    {{&machine_pin_type},41},
    {{&machine_pin_type},42},
    {{&machine_pin_type},43},
    {{&machine_pin_type},44},
    {{&machine_pin_type},45},
    {{&machine_pin_type},46},
    {{&machine_pin_type},47},
    {{&machine_pin_type},48},
    {{&machine_pin_type},49},
    {{&machine_pin_type},50},
    {{&machine_pin_type},51},
    {{&machine_pin_type},52},
    {{&machine_pin_type},53},
    {{&machine_pin_type},54},
    {{&machine_pin_type},55},
    {{&machine_pin_type},56},
    {{&machine_pin_type},57},
    {{&machine_pin_type},58},
    {{&machine_pin_type},59},
    {{&machine_pin_type},60},
    {{&machine_pin_type},61},
    {{&machine_pin_type},62},
    {{&machine_pin_type},63},
};

void machine_pins_init(void) {
    
}

void machine_pins_deinit(void) {
}


#if 0
gpio_num_t machine_pin_get_id(mp_obj_t pin_in) {
    if (mp_obj_get_type(pin_in) != &machine_pin_type) {
        mp_raise_ValueError(MP_ERROR_TEXT("expecting a pin"));
    }
    machine_pin_obj_t *self = pin_in;
    return self->id;
}
#endif

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "Pin(%u)Mode(%u) wx=%u wy=%u wr=%u", self->id, (self->mode), self->x, self->y, self->r);
}

// pin.init(mode, pull=None, *, value)
STATIC mp_obj_t machine_pin_obj_init_helper(machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_pull, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_SMALL_INT(-1)}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };
    mp_int_t pin_io_mode;

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    if (args[ARG_mode].u_obj == mp_const_none) {
        return mp_const_none;
    }
    // get the mode (in/out/opendrain)
    pin_io_mode = mp_obj_get_int(args[ARG_mode].u_obj);
    if (pin_io_mode < 0 || pin_io_mode > GPIO_MODE_INPUT_OUTPUT_OD) {
        mp_raise_ValueError(MP_ERROR_TEXT("Expecting a valid pin mode"));
    }
    // configure the pin for simple gpio in case it was being a smart pin
    gpio_pinclear(self->id);
    self->r = 0;
    self->x = 0;
    self->y = 0;
    self->mode = GPIO_MODE_INPUT; // the default for now

    // set initial output bit value if specified (do this before configuring mode pull up/down)
    if (args[ARG_value].u_obj != MP_OBJ_NULL) {
        printf("value parameter specified for pin %u\n", (uint32_t)self->id);
        gpio_set_latch(self->id, mp_obj_is_true(args[ARG_value].u_obj));
    }

    // configure optional pull up or pull down
    if (args[ARG_pull].u_obj != MP_OBJ_NEW_SMALL_INT(-1)) {
        printf("pullup parameter specified\n");
        // a pullup or pulldown argument was specified
        int pullmode = -1;
        if (args[ARG_pull].u_obj != mp_const_none) {
            pullmode = mp_obj_get_int(args[ARG_pull].u_obj);
            printf("pullup parameter specified as %d\n", (uint32_t)pullmode);
            if (pullmode < 0 || pullmode > GPIO_PULL_UP_FLOAT) {
                mp_raise_ValueError(MP_ERROR_TEXT("Expecting a valid pin pullup/pulldown value"));
            }
            // configure based on pin mode (in/output/od)
            switch(pin_io_mode) {
            case GPIO_MODE_INPUT:
                printf("setting input %u\n", (uint32_t)pullmode);
                if (pullmode > GPIO_PULL_DOWN_FLOAT) { // a pullup was specified
                    gpio_set_latch(self->id, 1); // setup output high when enabled
                    self->r = (mp_uint_t)(pullmode-GPIO_PULL_UP_FAST) << 11;
                }
                else { // a pulldown was specified
                    gpio_set_latch(self->id, 0); // setup output low when enabled
                    self->r = (mp_uint_t)(pullmode) << 8;
                }
                pin_io_mode = GPIO_MODE_INPUT_OUTPUT; // override to drive output
                break;
            case GPIO_MODE_INPUT_OUTPUT:
                printf("setting input/output %u\n", (uint32_t)pullmode);
                if (pullmode > GPIO_PULL_DOWN_FLOAT) { // a pullup was specified
                    self->r = (mp_uint_t)(pullmode-GPIO_PULL_UP_FAST) << 11;
                }
                else { // a pulldown was specified
                    self->r = (mp_uint_t)(pullmode) << 8;
                }
                break;
            case GPIO_MODE_INPUT_OUTPUT_OD: // open drain output mode
                printf("setting input/output OD %u\n", (uint32_t)pullmode);
                if (pullmode > GPIO_PULL_DOWN_FLOAT) { // a pullup was specified
                    self->r = (mp_uint_t)(pullmode-GPIO_PULL_UP_FAST) << 11;
                }
                else { // otherwise just float the high level
                    self->r = P_HIGH_FLOAT;
                }
                break;
            }
        }
        else  { // pullup/pulldown option was listed as None
            printf("pullup parameter specified as None\n");
            if (pin_io_mode == GPIO_MODE_INPUT_OUTPUT_OD) {
                self->r = P_HIGH_FLOAT; // default as as full open drain without pullup
            }
        }
    }
    else { // no pullup/pulldown argument supplied
        printf("No pullup parameter specified \n");
        if (pin_io_mode == GPIO_MODE_INPUT_OUTPUT_OD) {
            self->r = P_HIGH_FLOAT; // default as as full open drain without pullup
        }
    }
    if (self->r) { // we had pullup/pulldown requirements
        printf("Writing WRPIN %u to %u\n", (uint32_t)self->id, (uint32_t)self->r);
        gpio_wrpin(self->id, self->r);
    }
    // enable pin as input/output
    printf("Setting direction to %u\n", (uint32_t)pin_io_mode);
    gpio_set_direction(self->id, pin_io_mode); 
    self->mode = pin_io_mode; 

    return mp_const_none;
}

// constructor(id, ...)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get the wanted pin object
    int wanted_pin = mp_obj_get_int(args[0]);
    machine_pin_obj_t *self = NULL;
    if (0 <= wanted_pin && wanted_pin < MP_ARRAY_SIZE(machine_pin_obj)) {
        self = (machine_pin_obj_t *)&machine_pin_obj[wanted_pin];
    }
    if (self == NULL || self->base.type == NULL) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid pin"));
    }

   // printf("make new: number of args=%d n_kw=%d\n", n_args, n_kw);
    if (n_args > 1 || n_kw > 0) {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

// fast method for getting/setting pin value
STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;
    if (n_args == 0) {
        // get pin
        return MP_OBJ_NEW_SMALL_INT(gpio_get_level(self->id));
    } else {
        // set pin
        gpio_set_latch(self->id, mp_obj_is_true(args[0]));
        return mp_const_none;
    }
}

// pin.init(mode, pull)
STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

// pin.value([value])
STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

// pin.pinread()
STATIC mp_obj_t machine_pin_read(mp_obj_t self_in) {
    machine_pin_obj_t *self = self_in;
    // get pin
    return MP_OBJ_NEW_SMALL_INT(gpio_get_level(self->id));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_read_obj,  machine_pin_read);

// pin.rdpin()
STATIC mp_obj_t machine_pin_rdpin(mp_obj_t self_in) {
    machine_pin_obj_t *self = self_in;
    // get pin
    return MP_OBJ_NEW_SMALL_INT(gpio_rdpin(self->id));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_rdpin_obj,  machine_pin_rdpin);

// pin.rqpin()
STATIC mp_obj_t machine_pin_rqpin(mp_obj_t self_in) {
    machine_pin_obj_t *self = self_in;
    // get pin
    return MP_OBJ_NEW_SMALL_INT(gpio_rqpin(self->id));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_rqpin_obj,  machine_pin_rqpin);

// pin.off()
STATIC mp_obj_t machine_pin_off(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    gpio_set_level(self->id, 0);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_off_obj, machine_pin_off);

// pin.on()
STATIC mp_obj_t machine_pin_on(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    gpio_set_level(self->id, 1);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_on_obj, machine_pin_on);

// pin.float()
STATIC mp_obj_t machine_pin_float(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    gpio_set_direction(self->id, 0);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_float_obj, machine_pin_float);

// pin.toggle()
STATIC mp_obj_t machine_pin_toggle(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    gpio_toggle(self->id);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_toggle_obj, machine_pin_toggle);

// pin.pinclear()
static mp_obj_t machine_pin_clear(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    gpio_pinclear(self->id);
    self->r = 0;
    self->mode = GPIO_MODE_INPUT; // convert to input pin by default
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_clear_obj, machine_pin_clear);

// pin.pinstart()
mp_obj_t machine_pin_start(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = args[0];
    self->y = (n_args > 3) ? mp_obj_get_int_truncated(args[3]) : 0;
    self->x = (n_args > 2) ? mp_obj_get_int_truncated(args[2]) : 0;
    self->r = mp_obj_get_int_truncated(args[1]);
    gpio_set_direction(self->id, 0); // dir=0
    gpio_wxpin(self->id, self->x);
    gpio_wypin(self->id, self->y);
    gpio_wrpin(self->id, self->r);
    gpio_set_direction(self->id, 1); // dir=1
    self->mode = GPIO_MODE_INPUT_OUTPUT; // treat as input output pin
    printf("SmartPin(%u)Mode(%u) wx=%08x wy=%08x wr=%08x\n", (uint32_t)self->id, (uint32_t)(self->mode), (uint32_t)self->x, (uint32_t)self->y, (uint32_t)self->r);
    return mp_const_none; 
}   
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_start_obj, 2, 4, machine_pin_start);

// pin.wxpin()
mp_obj_t machine_pin_wxpin(size_t n_args, const mp_obj_t *args) {
   machine_pin_obj_t *self = args[0];
    // readback
    if (n_args == 1) {
        return mp_obj_new_int_from_uint(self->x);
    }
    self->x = mp_obj_get_int_truncated(args[1]);
    gpio_wxpin(self->id, self->x);
    return mp_const_none; 
}   
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_wxpin_obj, 1, 2, machine_pin_wxpin);

// pin.wypin()
mp_obj_t machine_pin_wypin(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = args[0];
    // readback
    if (n_args == 1) {
        return mp_obj_new_int_from_uint(self->y);
    }
    self->y = mp_obj_get_int_truncated(args[1]);
    gpio_wypin(self->id, self->y);
    return mp_const_none; 
}   
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_wypin_obj, 1, 2, machine_pin_wypin);

// pin.wrpin()
mp_obj_t machine_pin_wrpin(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = args[0];
    // readback
    if (n_args == 1) {
        return mp_obj_new_int_from_uint(self->r);
    }
    self->r = mp_obj_get_int_truncated(args[1]);
    gpio_wypin(self->id, self->r);
    return mp_const_none; 
}   
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_wrpin_obj, 1, 2, machine_pin_wrpin);

// pin.off()
STATIC mp_obj_t machine_pin_ack(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    gpio_akpin(self->id);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_ack_obj, machine_pin_ack);

#if 0
STATIC void machine_pin_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (destination[0] != MP_OBJ_NULL) {
        return;
    }
    if (attribute == MP_QSTR_x) {
        destination[0] = mp_obj_new_int_from_uint(self->x);
    }
    else if (attribute == MP_QSTR_y) {
        destination[0] = mp_obj_new_int_from_uint(self->y);
    }
    else if (attribute == MP_QSTR_r) {
        destination[0] = mp_obj_new_int_from_uint(self->r);
    }
}
#endif

STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinr), MP_ROM_PTR(&machine_pin_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinread), MP_ROM_PTR(&machine_pin_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&machine_pin_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinlow), MP_ROM_PTR(&machine_pin_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinl), MP_ROM_PTR(&machine_pin_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&machine_pin_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinh), MP_ROM_PTR(&machine_pin_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinhigh), MP_ROM_PTR(&machine_pin_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_wrpin), MP_ROM_PTR(&machine_pin_wrpin_obj) },
    { MP_ROM_QSTR(MP_QSTR_wxpin), MP_ROM_PTR(&machine_pin_wxpin_obj) },
    { MP_ROM_QSTR(MP_QSTR_wypin), MP_ROM_PTR(&machine_pin_wypin_obj) },
    { MP_ROM_QSTR(MP_QSTR_akpin), MP_ROM_PTR(&machine_pin_ack_obj) },
    { MP_ROM_QSTR(MP_QSTR_rdpin), MP_ROM_PTR(&machine_pin_rdpin_obj) },
    { MP_ROM_QSTR(MP_QSTR_rqpin), MP_ROM_PTR(&machine_pin_rqpin_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinstart), MP_ROM_PTR(&machine_pin_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinclear), MP_ROM_PTR(&machine_pin_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_pintoggle), MP_ROM_PTR(&machine_pin_toggle_obj) },
    { MP_ROM_QSTR(MP_QSTR_pint), MP_ROM_PTR(&machine_pin_toggle_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinfloat), MP_ROM_PTR(&machine_pin_float_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinf), MP_ROM_PTR(&machine_pin_float_obj) },
    // class constants
    // - pin modes
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPIO_MODE_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPIO_MODE_INPUT_OUTPUT) },
    { MP_ROM_QSTR(MP_QSTR_OPEN_DRAIN), MP_ROM_INT(GPIO_MODE_INPUT_OUTPUT_OD) },
    // - pullup/pulldown constants
    { MP_ROM_QSTR(MP_QSTR_PULL_UP), MP_ROM_INT(GPIO_PULL_UP) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_1K5), MP_ROM_INT(GPIO_PULL_UP_1K5) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_15K), MP_ROM_INT(GPIO_PULL_UP_15K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_150K), MP_ROM_INT(GPIO_PULL_UP_150K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_1MA), MP_ROM_INT(GPIO_PULL_UP_1MA) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_100UA), MP_ROM_INT(GPIO_PULL_UP_100UA) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_10UA), MP_ROM_INT(GPIO_PULL_UP_10UA) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_FLOAT), MP_ROM_INT(GPIO_PULL_UP_FLOAT) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(GPIO_PULL_DOWN) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN_1K5), MP_ROM_INT(GPIO_PULL_DOWN_1K5) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN_15K), MP_ROM_INT(GPIO_PULL_DOWN_15K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN_150K), MP_ROM_INT(GPIO_PULL_DOWN_150K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN_1MA), MP_ROM_INT(GPIO_PULL_DOWN_1MA) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN_100UA), MP_ROM_INT(GPIO_PULL_DOWN_100UA) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN_10UA), MP_ROM_INT(GPIO_PULL_DOWN_10UA) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN_FLOAT), MP_ROM_INT(GPIO_PULL_DOWN_FLOAT) },
    // - smart pin constants
    { MP_ROM_QSTR(MP_QSTR_P_TRUE_A), MP_ROM_INT(P_TRUE_A) },
    { MP_ROM_QSTR(MP_QSTR_P_INVERT_A), MP_ROM_INT(P_INVERT_A) },
    { MP_ROM_QSTR(MP_QSTR_P_LOCAL_A), MP_ROM_INT(P_LOCAL_A) },
    { MP_ROM_QSTR(MP_QSTR_P_PLUS1_A), MP_ROM_INT(P_PLUS1_A) },
    { MP_ROM_QSTR(MP_QSTR_P_PLUS2_A), MP_ROM_INT(P_PLUS2_A) },
    { MP_ROM_QSTR(MP_QSTR_P_PLUS3_A), MP_ROM_INT(P_PLUS3_A) },
    { MP_ROM_QSTR(MP_QSTR_P_OUTBIT_A), MP_ROM_INT(P_OUTBIT_A) },
    { MP_ROM_QSTR(MP_QSTR_P_MINUS3_A), MP_ROM_INT(P_MINUS3_A) },
    { MP_ROM_QSTR(MP_QSTR_P_MINUS2_A), MP_ROM_INT(P_MINUS2_A) },
    { MP_ROM_QSTR(MP_QSTR_P_MINUS1_A), MP_ROM_INT(P_MINUS1_A) },
    { MP_ROM_QSTR(MP_QSTR_P_TRUE_B), MP_ROM_INT(P_TRUE_B) },
    { MP_ROM_QSTR(MP_QSTR_P_INVERT_B), MP_ROM_INT(P_INVERT_B) },
    { MP_ROM_QSTR(MP_QSTR_P_LOCAL_B), MP_ROM_INT(P_LOCAL_B) },
    { MP_ROM_QSTR(MP_QSTR_P_PLUS1_B), MP_ROM_INT(P_PLUS1_B) },
    { MP_ROM_QSTR(MP_QSTR_P_PLUS2_B), MP_ROM_INT(P_PLUS2_B) },
    { MP_ROM_QSTR(MP_QSTR_P_PLUS3_B), MP_ROM_INT(P_PLUS3_B) },
    { MP_ROM_QSTR(MP_QSTR_P_OUTBIT_B), MP_ROM_INT(P_OUTBIT_B) },
    { MP_ROM_QSTR(MP_QSTR_P_MINUS3_B), MP_ROM_INT(P_MINUS3_B) },
    { MP_ROM_QSTR(MP_QSTR_P_MINUS2_B), MP_ROM_INT(P_MINUS2_B) },
    { MP_ROM_QSTR(MP_QSTR_P_MINUS1_B), MP_ROM_INT(P_MINUS1_B) },
    { MP_ROM_QSTR(MP_QSTR_P_PASS_AB), MP_ROM_INT(P_PASS_AB) },
    { MP_ROM_QSTR(MP_QSTR_P_AND_AB), MP_ROM_INT(P_AND_AB) },
    { MP_ROM_QSTR(MP_QSTR_P_OR_AB), MP_ROM_INT(P_OR_AB) },
    { MP_ROM_QSTR(MP_QSTR_P_XOR_AB), MP_ROM_INT(P_XOR_AB) },
    { MP_ROM_QSTR(MP_QSTR_P_FILT0_AB), MP_ROM_INT(P_FILT0_AB) },
    { MP_ROM_QSTR(MP_QSTR_P_FILT1_AB), MP_ROM_INT(P_FILT1_AB) },
    { MP_ROM_QSTR(MP_QSTR_P_FILT2_AB), MP_ROM_INT(P_FILT2_AB) },
    { MP_ROM_QSTR(MP_QSTR_P_FILT3_AB), MP_ROM_INT(P_FILT3_AB) },
    { MP_ROM_QSTR(MP_QSTR_P_LOGIC_A), MP_ROM_INT(P_LOGIC_A) },
    { MP_ROM_QSTR(MP_QSTR_P_LOGIC_A_FB), MP_ROM_INT(P_LOGIC_A_FB) },
    { MP_ROM_QSTR(MP_QSTR_P_LOGIC_B_FB), MP_ROM_INT(P_LOGIC_B_FB) },
    { MP_ROM_QSTR(MP_QSTR_P_SCHMITT_A), MP_ROM_INT(P_SCHMITT_A) },
    { MP_ROM_QSTR(MP_QSTR_P_SCHMITT_A_FB), MP_ROM_INT(P_SCHMITT_A_FB) },
    { MP_ROM_QSTR(MP_QSTR_P_SCHMITT_B_FB), MP_ROM_INT(P_SCHMITT_B_FB) },
    { MP_ROM_QSTR(MP_QSTR_P_COMPARE_AB), MP_ROM_INT(P_COMPARE_AB) },
    { MP_ROM_QSTR(MP_QSTR_P_COMPARE_AB_FB), MP_ROM_INT(P_COMPARE_AB_FB) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_GIO), MP_ROM_INT(P_ADC_GIO) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_VIO), MP_ROM_INT(P_ADC_VIO) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_FLOAT), MP_ROM_INT(P_ADC_FLOAT) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_1X), MP_ROM_INT(P_ADC_1X) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_3X), MP_ROM_INT(P_ADC_3X) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_10X), MP_ROM_INT(P_ADC_10X) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_30X), MP_ROM_INT(P_ADC_30X) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_100X), MP_ROM_INT(P_ADC_100X) },
    { MP_ROM_QSTR(MP_QSTR_P_DAC_990R_3V), MP_ROM_INT(P_DAC_990R_3V) },
    { MP_ROM_QSTR(MP_QSTR_P_DAC_600R_2V), MP_ROM_INT(P_DAC_600R_2V) },
    { MP_ROM_QSTR(MP_QSTR_P_DAC_124R_3V), MP_ROM_INT(P_DAC_124R_3V) },
    { MP_ROM_QSTR(MP_QSTR_P_DAC_75R_2V), MP_ROM_INT(P_DAC_75R_2V) },
    { MP_ROM_QSTR(MP_QSTR_P_LEVEL_A), MP_ROM_INT(P_LEVEL_A) },
    { MP_ROM_QSTR(MP_QSTR_P_LEVEL_A_FBN), MP_ROM_INT(P_LEVEL_A_FBN) },
    { MP_ROM_QSTR(MP_QSTR_P_LEVEL_B_FBP), MP_ROM_INT(P_LEVEL_B_FBP) },
    { MP_ROM_QSTR(MP_QSTR_P_LEVEL_B_FBN), MP_ROM_INT(P_LEVEL_B_FBN) },
    { MP_ROM_QSTR(MP_QSTR_P_ASYNC_IO), MP_ROM_INT(P_ASYNC_IO) },
    { MP_ROM_QSTR(MP_QSTR_P_SYNC_IO), MP_ROM_INT(P_SYNC_IO) },
    { MP_ROM_QSTR(MP_QSTR_P_TRUE_IN), MP_ROM_INT(P_TRUE_IN) },
    { MP_ROM_QSTR(MP_QSTR_P_INVERT_IN), MP_ROM_INT(P_INVERT_IN) },
    { MP_ROM_QSTR(MP_QSTR_P_TRUE_OUTPUT), MP_ROM_INT(P_TRUE_OUTPUT) },
    { MP_ROM_QSTR(MP_QSTR_P_INVERT_OUTPUT), MP_ROM_INT(P_INVERT_OUTPUT) },
    { MP_ROM_QSTR(MP_QSTR_P_HIGH_FAST), MP_ROM_INT(P_HIGH_FAST) },
    { MP_ROM_QSTR(MP_QSTR_P_HIGH_1K5), MP_ROM_INT(P_HIGH_1K5) },
    { MP_ROM_QSTR(MP_QSTR_P_HIGH_15K), MP_ROM_INT(P_HIGH_15K) },
    { MP_ROM_QSTR(MP_QSTR_P_HIGH_150K), MP_ROM_INT(P_HIGH_150K) },
    { MP_ROM_QSTR(MP_QSTR_P_HIGH_1MA), MP_ROM_INT(P_HIGH_1MA) },
    { MP_ROM_QSTR(MP_QSTR_P_HIGH_100UA), MP_ROM_INT(P_HIGH_100UA) },
    { MP_ROM_QSTR(MP_QSTR_P_HIGH_10UA), MP_ROM_INT(P_HIGH_10UA) },
    { MP_ROM_QSTR(MP_QSTR_P_HIGH_FLOAT), MP_ROM_INT(P_HIGH_FLOAT) },
    { MP_ROM_QSTR(MP_QSTR_P_LOW_FAST), MP_ROM_INT(P_LOW_FAST) },
    { MP_ROM_QSTR(MP_QSTR_P_LOW_1K5), MP_ROM_INT(P_LOW_1K5) },
    { MP_ROM_QSTR(MP_QSTR_P_LOW_15K), MP_ROM_INT(P_LOW_15K) },
    { MP_ROM_QSTR(MP_QSTR_P_LOW_150K), MP_ROM_INT(P_LOW_150K) },
    { MP_ROM_QSTR(MP_QSTR_P_LOW_1MA), MP_ROM_INT(P_LOW_1MA) },
    { MP_ROM_QSTR(MP_QSTR_P_LOW_100UA), MP_ROM_INT(P_LOW_100UA) },
    { MP_ROM_QSTR(MP_QSTR_P_LOW_10UA), MP_ROM_INT(P_LOW_10UA) },
    { MP_ROM_QSTR(MP_QSTR_P_LOW_FLOAT), MP_ROM_INT(P_LOW_FLOAT) },
    { MP_ROM_QSTR(MP_QSTR_P_TT_00), MP_ROM_INT(P_TT_00) },
    { MP_ROM_QSTR(MP_QSTR_P_TT_01), MP_ROM_INT(P_TT_01) },
    { MP_ROM_QSTR(MP_QSTR_P_TT_10), MP_ROM_INT(P_TT_10) },
    { MP_ROM_QSTR(MP_QSTR_P_TT_11), MP_ROM_INT(P_TT_11) },
    { MP_ROM_QSTR(MP_QSTR_P_OE), MP_ROM_INT(P_OE) },
    { MP_ROM_QSTR(MP_QSTR_P_CHANNEL), MP_ROM_INT(P_CHANNEL) },
    { MP_ROM_QSTR(MP_QSTR_P_BITDAC), MP_ROM_INT(P_BITDAC) },
    { MP_ROM_QSTR(MP_QSTR_P_NORMAL), MP_ROM_INT(P_NORMAL) },
    { MP_ROM_QSTR(MP_QSTR_P_REPOSITORY), MP_ROM_INT(P_REPOSITORY) },
    { MP_ROM_QSTR(MP_QSTR_P_DAC_NOISE), MP_ROM_INT(P_DAC_NOISE) },
    { MP_ROM_QSTR(MP_QSTR_P_DAC_DITHER_RND), MP_ROM_INT(P_DAC_DITHER_RND) },
    { MP_ROM_QSTR(MP_QSTR_P_DAC_DITHER_PWM), MP_ROM_INT(P_DAC_DITHER_PWM) },
    { MP_ROM_QSTR(MP_QSTR_P_PULSE), MP_ROM_INT(P_PULSE) },
    { MP_ROM_QSTR(MP_QSTR_P_TRANSITION), MP_ROM_INT(P_TRANSITION) },
    { MP_ROM_QSTR(MP_QSTR_P_NCO_FREQ), MP_ROM_INT(P_NCO_FREQ) },
    { MP_ROM_QSTR(MP_QSTR_P_NCO_DUTY), MP_ROM_INT(P_NCO_DUTY) },
    { MP_ROM_QSTR(MP_QSTR_P_PWM_TRIANGLE), MP_ROM_INT(P_PWM_TRIANGLE) },
    { MP_ROM_QSTR(MP_QSTR_P_PWM_SAWTOOTH), MP_ROM_INT(P_PWM_SAWTOOTH) },
    { MP_ROM_QSTR(MP_QSTR_P_PWM_SMPS), MP_ROM_INT(P_PWM_SMPS) },
    { MP_ROM_QSTR(MP_QSTR_P_QUADRATURE), MP_ROM_INT(P_QUADRATURE) },
    { MP_ROM_QSTR(MP_QSTR_P_REG_UP), MP_ROM_INT(P_REG_UP) },
    { MP_ROM_QSTR(MP_QSTR_P_REG_UP_DOWN), MP_ROM_INT(P_REG_UP_DOWN) },
    { MP_ROM_QSTR(MP_QSTR_P_COUNT_RISES), MP_ROM_INT(P_COUNT_RISES) },
    { MP_ROM_QSTR(MP_QSTR_P_COUNT_HIGHS), MP_ROM_INT(P_COUNT_HIGHS) },
    { MP_ROM_QSTR(MP_QSTR_P_STATE_TICKS), MP_ROM_INT(P_STATE_TICKS) },
    { MP_ROM_QSTR(MP_QSTR_P_HIGH_TICKS), MP_ROM_INT(P_HIGH_TICKS) },
    { MP_ROM_QSTR(MP_QSTR_P_EVENTS_TICKS), MP_ROM_INT(P_EVENTS_TICKS) },
    { MP_ROM_QSTR(MP_QSTR_P_PERIODS_TICKS), MP_ROM_INT(P_PERIODS_TICKS) },
    { MP_ROM_QSTR(MP_QSTR_P_PERIODS_HIGHS), MP_ROM_INT(P_PERIODS_HIGHS) },
    { MP_ROM_QSTR(MP_QSTR_P_COUNTER_TICKS), MP_ROM_INT(P_COUNTER_TICKS) },
    { MP_ROM_QSTR(MP_QSTR_P_COUNTER_HIGHS), MP_ROM_INT(P_COUNTER_HIGHS) },
    { MP_ROM_QSTR(MP_QSTR_P_COUNTER_PERIODS), MP_ROM_INT(P_COUNTER_PERIODS) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC), MP_ROM_INT(P_ADC) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_EXT), MP_ROM_INT(P_ADC_EXT) },
    { MP_ROM_QSTR(MP_QSTR_P_ADC_SCOPE), MP_ROM_INT(P_ADC_SCOPE) },
    { MP_ROM_QSTR(MP_QSTR_P_USB_PAIR), MP_ROM_INT(P_USB_PAIR) },
    { MP_ROM_QSTR(MP_QSTR_P_SYNC_TX), MP_ROM_INT(P_SYNC_TX) },
    { MP_ROM_QSTR(MP_QSTR_P_SYNC_RX), MP_ROM_INT(P_SYNC_RX) },
    { MP_ROM_QSTR(MP_QSTR_P_ASYNC_TX), MP_ROM_INT(P_ASYNC_TX) },
    { MP_ROM_QSTR(MP_QSTR_P_ASYNC_RX), MP_ROM_INT(P_ASYNC_RX) },
};

STATIC mp_uint_t pin_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    (void)errcode;
    machine_pin_obj_t *self = self_in;

    switch (request) {
        case MP_PIN_READ: {
            return gpio_get_level(self->id);
        }
        case MP_PIN_WRITE: {
            gpio_set_level(self->id, arg);
            return 0;
        }
    }
    return -1;
}

STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

STATIC const mp_pin_p_t pin_pin_p = {
    .ioctl = pin_ioctl,
};

const mp_obj_type_t machine_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .print = machine_pin_print,
    .make_new = mp_pin_make_new,
//    .attr = machine_pin_attr,
    .call = machine_pin_call,
    .protocol = &pin_pin_p,
    .locals_dict = (mp_obj_t)&machine_pin_locals_dict,
};

