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
#include <uacpi/utilities.h>
#include <uacpi/internal/log.h>
#include <uacpi/internal/stdlib.h>

static inline uacpi_phys_addr uacpi_truncate_phys_addr_with_warn(uacpi_u64 large_addr)
{
    if (sizeof(uacpi_phys_addr) < 8 && large_addr > 0xFFFFFFFF) {
        uacpi_warn(
            "truncating a physical address 0x%"UACPI_PRIX64
            " outside of address space\n", UACPI_FMT64(large_addr)
        );
    }

    return (uacpi_phys_addr)large_addr;
}

#define UACPI_PTR_TO_VIRT_ADDR(ptr)   ((uacpi_virt_addr)(ptr))
#define UACPI_VIRT_ADDR_TO_PTR(vaddr) ((void*)(vaddr))

#define UACPI_PTR_ADD(ptr, value) ((void*)(((uacpi_u8*)(ptr)) + value))

/*
 * Target buffer must have a length of at least 8 bytes.
 */
void uacpi_eisa_id_to_string(uacpi_u32, uacpi_char *out_string);

enum uacpi_base {
    UACPI_BASE_AUTO,
    UACPI_BASE_OCT = 8,
    UACPI_BASE_DEC = 10,
    UACPI_BASE_HEX = 16,
};
uacpi_status uacpi_string_to_integer(
    const uacpi_char *str, uacpi_size max_chars, enum uacpi_base base,
    uacpi_u64 *out_value
);

uacpi_bool uacpi_is_valid_nameseg(uacpi_u8 *nameseg);

void uacpi_free_dynamic_string(const uacpi_char *str);
