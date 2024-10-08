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

enum uacpi_register {
    UACPI_REGISTER_PM1_STS = 0,
    UACPI_REGISTER_PM1_EN,
    UACPI_REGISTER_PM1_CNT,
    UACPI_REGISTER_PM_TMR,
    UACPI_REGISTER_PM2_CNT,
    UACPI_REGISTER_SLP_CNT,
    UACPI_REGISTER_SLP_STS,
    UACPI_REGISTER_RESET,
    UACPI_REGISTER_SMI_CMD,
    UACPI_REGISTER_MAX = UACPI_REGISTER_SMI_CMD,
};

uacpi_status uacpi_read_register(enum uacpi_register, uacpi_u64*);

uacpi_status uacpi_write_register(enum uacpi_register, uacpi_u64);
uacpi_status uacpi_write_registers(enum uacpi_register, uacpi_u64, uacpi_u64);

enum uacpi_register_field {
    UACPI_REGISTER_FIELD_TMR_STS = 0,
    UACPI_REGISTER_FIELD_BM_STS,
    UACPI_REGISTER_FIELD_GBL_STS,
    UACPI_REGISTER_FIELD_PWRBTN_STS,
    UACPI_REGISTER_FIELD_SLPBTN_STS,
    UACPI_REGISTER_FIELD_RTC_STS,
    UACPI_REGISTER_FIELD_PCIEX_WAKE_STS,
    UACPI_REGISTER_FIELD_HWR_WAK_STS,
    UACPI_REGISTER_FIELD_WAK_STS,
    UACPI_REGISTER_FIELD_TMR_EN,
    UACPI_REGISTER_FIELD_GBL_EN,
    UACPI_REGISTER_FIELD_PWRBTN_EN,
    UACPI_REGISTER_FIELD_SLPBTN_EN,
    UACPI_REGISTER_FIELD_RTC_EN,
    UACPI_REGISTER_FIELD_PCIEXP_WAKE_DIS,
    UACPI_REGISTER_FIELD_SCI_EN,
    UACPI_REGISTER_FIELD_BM_RLD,
    UACPI_REGISTER_FIELD_GBL_RLS,
    UACPI_REGISTER_FIELD_SLP_TYP,
    UACPI_REGISTER_FIELD_HWR_SLP_TYP,
    UACPI_REGISTER_FIELD_SLP_EN,
    UACPI_REGISTER_FIELD_HWR_SLP_EN,
    UACPI_REGISTER_FIELD_ARB_DIS,
    UACPI_REGISTER_FIELD_MAX = UACPI_REGISTER_FIELD_ARB_DIS,
};

uacpi_status uacpi_read_register_field(enum uacpi_register_field, uacpi_u64*);
uacpi_status uacpi_write_register_field(enum uacpi_register_field, uacpi_u64);
