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

#include <uacpi/acpi.h>
#include <uacpi/types.h>
#include <uacpi/uacpi.h>
#include <uacpi/internal/dynamic_array.h>
#include <uacpi/internal/shareable.h>
#include <uacpi/context.h>

struct uacpi_runtime_context {
    /*
     * A local copy of FADT that has been verified & converted to most optimal
     * format for faster access to the registers.
     */
    struct acpi_fadt fadt;

    /*
     * A cached pointer to FACS so that we don't have to look it up in interrupt
     * contexts as we can't take mutexes.
     */
    struct acpi_facs *facs;

    /*
     * pm1{a,b}_evt_blk split into two registers for convenience
     */
    struct acpi_gas pm1a_status_blk;
    struct acpi_gas pm1b_status_blk;
    struct acpi_gas pm1a_enable_blk;
    struct acpi_gas pm1b_enable_blk;

    uacpi_u64 flags;

#define UACPI_SLEEP_TYP_INVALID 0xFF
    uacpi_u8 last_sleep_typ_a;
    uacpi_u8 last_sleep_typ_b;

    uacpi_u8 s0_sleep_typ_a;
    uacpi_u8 s0_sleep_typ_b;

    /*
     * This is a per-table value but we mimic the NT implementation:
     * treat all other definition blocks as if they were the same revision
     * as DSDT.
     */
    uacpi_bool is_rev1;
    uacpi_bool global_lock_acquired;

#ifndef UACPI_REDUCED_HARDWARE
    uacpi_bool is_hardware_reduced;
    uacpi_bool has_global_lock;
    uacpi_handle sci_handle;
#endif
    uacpi_u32 loop_timeout_seconds;
    uacpi_u32 max_call_stack_depth;

    uacpi_u32 global_lock_seq_num;
    uacpi_handle *global_lock_mutex;

#ifndef UACPI_REDUCED_HARDWARE
    uacpi_handle *global_lock_event;
    uacpi_handle *global_lock_spinlock;
    uacpi_bool global_lock_pending;
#endif

    uacpi_u8 log_level;
    uacpi_u8 init_level;
};

static inline const uacpi_char *uacpi_init_level_to_string(uacpi_u8 lvl)
{
    switch (lvl) {
    case UACPI_INIT_LEVEL_EARLY:
        return "early";
    case UACPI_INIT_LEVEL_SUBSYSTEM_INITIALIZED:
        return "subsystem initialized";
    case UACPI_INIT_LEVEL_NAMESPACE_LOADED:
        return "namespace loaded";
    case UACPI_INIT_LEVEL_NAMESPACE_INITIALIZED:
        return "namespace initialized";
    default:
        return "<invalid>";
    }
}

#define UACPI_ENSURE_INIT_LEVEL_AT_LEAST(lvl)                               \
    do {                                                                    \
        if (uacpi_unlikely(g_uacpi_rt_ctx.init_level < lvl)) {              \
            uacpi_error(                                                    \
                "while evaluating %s: init level %d (%s) is too low, "      \
                "expected at least %d (%s)\n", __FUNCTION__,                \
                g_uacpi_rt_ctx.init_level,                                  \
                uacpi_init_level_to_string(g_uacpi_rt_ctx.init_level), lvl, \
                uacpi_init_level_to_string(lvl)                             \
            );                                                              \
            return UACPI_STATUS_INIT_LEVEL_MISMATCH;                        \
        }                                                                   \
    } while (0)

#define UACPI_ENSURE_INIT_LEVEL_IS(lvl)                                     \
    do {                                                                    \
        if (uacpi_unlikely(g_uacpi_rt_ctx.init_level != lvl)) {             \
            uacpi_error(                                                    \
                "while evaluating %s: invalid init level %d (%s), "         \
                "expected %d (%s)\n", __FUNCTION__,                         \
                g_uacpi_rt_ctx.init_level,                                  \
                uacpi_init_level_to_string(g_uacpi_rt_ctx.init_level), lvl, \
                uacpi_init_level_to_string(lvl)                             \
            );                                                              \
            return UACPI_STATUS_INIT_LEVEL_MISMATCH;                        \
        }                                                                   \
    } while (0)

extern struct uacpi_runtime_context g_uacpi_rt_ctx;

static inline uacpi_bool uacpi_check_flag(uacpi_u64 flag)
{
    return (g_uacpi_rt_ctx.flags & flag) == flag;
}

static inline uacpi_bool uacpi_should_log(enum uacpi_log_level lvl)
{
    return lvl <= g_uacpi_rt_ctx.log_level;
}

static inline uacpi_bool uacpi_is_hardware_reduced(void)
{
#ifndef UACPI_REDUCED_HARDWARE
    return g_uacpi_rt_ctx.is_hardware_reduced;
#else
    return UACPI_TRUE;
#endif
}
