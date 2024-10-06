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

#include <uacpi/event.h>

// This fixed event is internal-only, and we don't expose it in the enum
#define UACPI_FIXED_EVENT_GLOBAL_LOCK 0

UACPI_ALWAYS_OK_FOR_REDUCED_HARDWARE(
    uacpi_status uacpi_initialize_events(void)
)
UACPI_STUB_IF_REDUCED_HARDWARE(
    void uacpi_deinitialize_events(void)
)

UACPI_ALWAYS_OK_FOR_REDUCED_HARDWARE(
    uacpi_status uacpi_events_match_post_dynamic_table_load(void)
)

UACPI_ALWAYS_ERROR_FOR_REDUCED_HARDWARE(
    uacpi_status uacpi_clear_all_events(void)
)
