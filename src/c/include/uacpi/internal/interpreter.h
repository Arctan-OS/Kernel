/**
 *
 * MIT License
 *
 * Copyright (c) 2022-2024 Daniil Tatianin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * */
#pragma once

#include <uacpi/types.h>
#include <uacpi/status.h>
#include <uacpi/internal/namespace.h>

enum uacpi_table_load_cause {
    UACPI_TABLE_LOAD_CAUSE_LOAD_OP,
    UACPI_TABLE_LOAD_CAUSE_LOAD_TABLE_OP,
    UACPI_TABLE_LOAD_CAUSE_INIT,
    UACPI_TABLE_LOAD_CAUSE_HOST,
};

uacpi_status uacpi_execute_table(void*, enum uacpi_table_load_cause cause);
uacpi_status uacpi_osi(uacpi_handle handle, uacpi_object *retval);

uacpi_status uacpi_execute_control_method(
    uacpi_namespace_node *scope, uacpi_control_method *method,
    const uacpi_args *args, uacpi_object **ret
);
