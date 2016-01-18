/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/nlr.h"
#if MICROPY_PY_SOFTIRQ
#include <py/softirq.h>

#define STACK_DEPTH (5)

typedef struct _stack_elem_t {
    mp_obj_t irqfunc;
    mp_obj_t arg;
} stack_elem_t;

short sp = -1;
stack_elem_t stack[STACK_DEPTH];


void exec_softirq(void){
    stack_elem_t softirq;
    mp_uint_t irqstate;
    bool busy = false;
    
    if (sp >= 0 && busy == false) {
        irqstate = disable_irq();
        softirq = stack[sp--];
        busy = true;
        enable_irq(irqstate);
        mp_call_function_1(softirq.irqfunc, softirq.arg);
        busy = false;
    }
}

STATIC mp_obj_t mp_softirq_setsoftirq(mp_obj_t function, mp_obj_t arg) {
    
    mp_uint_t irqstate = disable_irq();
    if (sp < STACK_DEPTH - 1) {
        sp++;
        stack[sp].irqfunc = function;
        stack[sp].arg = arg;
        enable_irq(irqstate);
    } else {
        enable_irq(irqstate);
        //TODO Do something sensible if the stack gets full
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mp_softirq_setsoftirq_obj, mp_softirq_setsoftirq);

STATIC const mp_rom_map_elem_t mp_module_softirq_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_softirq) },

    { MP_ROM_QSTR(MP_QSTR_setsoftirq), MP_ROM_PTR(&mp_softirq_setsoftirq_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_softirq_globals, mp_module_softirq_globals_table);


const mp_obj_module_t mp_module_softirq = {
    .base = { &mp_type_module },
    .name = MP_QSTR_softirq,
    .globals = (mp_obj_dict_t*)&mp_module_softirq_globals,
};

#endif //MICROPY_PY_SOFTIRQ
