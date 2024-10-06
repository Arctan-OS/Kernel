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
#include <uacpi/acpi.h>
#include <uacpi/io.h>

uacpi_size uacpi_round_up_bits_to_bytes(uacpi_size bit_length);

void uacpi_read_buffer_field(
    const uacpi_buffer_field *field, void *dst
);
void uacpi_write_buffer_field(
    uacpi_buffer_field *field, const void *src, uacpi_size size
);

uacpi_status uacpi_read_field_unit(
    uacpi_field_unit *field, void *dst, uacpi_size size
);
uacpi_status uacpi_write_field_unit(
    uacpi_field_unit *field, const void *src, uacpi_size size
);
