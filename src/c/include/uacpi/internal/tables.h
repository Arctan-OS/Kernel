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

#include <uacpi/internal/context.h>
#include <uacpi/internal/interpreter.h>
#include <uacpi/types.h>
#include <uacpi/status.h>
#include <uacpi/tables.h>

enum uacpi_table_origin {
    UACPI_TABLE_ORIGIN_FIRMWARE_VIRTUAL = 0,
    UACPI_TABLE_ORIGIN_FIRMWARE_PHYSICAL,

    UACPI_TABLE_ORIGIN_HOST_VIRTUAL,
    UACPI_TABLE_ORIGIN_HOST_PHYSICAL,
};

struct uacpi_installed_table {
    uacpi_phys_addr phys_addr;
    struct acpi_sdt_hdr hdr;
    void *ptr;

    uacpi_u16 reference_count;

#define UACPI_TABLE_LOADED (1 << 0)
#define UACPI_TABLE_CSUM_VERIFIED (1 << 1)
#define UACPI_TABLE_INVALID (1 << 2)
    uacpi_u8 flags;
    uacpi_u8 origin;
};

uacpi_status uacpi_initialize_tables(void);
void uacpi_deinitialize_tables(void);

uacpi_bool uacpi_signatures_match(const void *const lhs, const void *const rhs);
uacpi_status uacpi_check_table_signature(void *table, const uacpi_char *expect);
uacpi_status uacpi_verify_table_checksum(void *table, uacpi_size size);

uacpi_status uacpi_table_install_physical_with_origin(
    uacpi_phys_addr phys, enum uacpi_table_origin origin, uacpi_table *out_table
);
uacpi_status uacpi_table_install_with_origin(
    void *virt, enum uacpi_table_origin origin, uacpi_table *out_table
);

void uacpi_table_mark_as_loaded(uacpi_size idx);

uacpi_status uacpi_table_load_with_cause(
    uacpi_size idx, enum uacpi_table_load_cause cause
);

enum uacpi_table_iteration_decision {
    UACPI_TABLE_ITERATION_DECISION_CONTINUE,
    UACPI_TABLE_ITERATION_DECISION_BREAK,
};
typedef enum uacpi_table_iteration_decision (*uacpi_table_iteration_callback)
    (void *user, struct uacpi_installed_table *tbl, uacpi_size idx);

uacpi_status uacpi_for_each_table(
    uacpi_size base_idx, uacpi_table_iteration_callback, void *user
);

typedef uacpi_bool (*uacpi_table_match_callback)
    (struct uacpi_installed_table *tbl);

uacpi_status uacpi_table_match(
    uacpi_size base_idx, uacpi_table_match_callback, uacpi_table *out_table
);

#define UACPI_PRI_TBL_HDR "'%.4s' (OEM ID '%.6s' OEM Table ID '%.8s')"
#define UACPI_FMT_TBL_HDR(hdr) (hdr)->signature, (hdr)->oemid, (hdr)->oem_table_id
