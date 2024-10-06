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

#include <uacpi/kernel_api.h>
#include <uacpi/internal/context.h>

#ifdef UACPI_FORMATTED_LOGGING
#define uacpi_log uacpi_kernel_log
#else
UACPI_PRINTF_DECL(2, 3)
void uacpi_log(uacpi_log_level, const uacpi_char*, ...);
#endif

#define uacpi_log_lvl(lvl, ...) \
    do { if (uacpi_should_log(lvl)) uacpi_log(lvl, __VA_ARGS__); } while (0)

#define uacpi_debug(...) uacpi_log_lvl(UACPI_LOG_DEBUG, __VA_ARGS__)
#define uacpi_trace(...) uacpi_log_lvl(UACPI_LOG_TRACE, __VA_ARGS__)
#define uacpi_info(...)  uacpi_log_lvl(UACPI_LOG_INFO, __VA_ARGS__)
#define uacpi_warn(...)  uacpi_log_lvl(UACPI_LOG_WARN, __VA_ARGS__)
#define uacpi_error(...) uacpi_log_lvl(UACPI_LOG_ERROR, __VA_ARGS__)
