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

#ifdef UACPI_OVERRIDE_LIBC
#include "uacpi_libc.h"
#else
/*
 * The following libc functions are used internally by uACPI and have a default
 * (sub-optimal) implementation:
 * - memcpy
 * - memset
 * - memcmp
 * - strcmp
 * - memmove
 * - strnlen
 * - strlen
 * - snprintf
 * - vsnprintf
 *
 * In case your platform happens to implement optimized verisons of the helpers
 * above, you are able to make uACPI use those instead by overriding them like so:
 *
 * #define uacpi_memcpy my_fast_memcpy
 * #define uacpi_snprintf my_fast_snprintf
 */
#endif
