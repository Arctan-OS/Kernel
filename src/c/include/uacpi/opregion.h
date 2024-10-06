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

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Install an address space handler to a device node.
 * The handler is recursively connected to all of the operation regions of
 * type 'space' underneath 'device_node'. Note that this recursion stops as
 * soon as another device node that already has an address space handler of
 * this type installed is encountered.
 */
uacpi_status uacpi_install_address_space_handler(
    uacpi_namespace_node *device_node, enum uacpi_address_space space,
    uacpi_region_handler handler, uacpi_handle handler_context
);

/*
 * Uninstall the handler of type 'space' from a given device node.
 */
uacpi_status uacpi_uninstall_address_space_handler(
    uacpi_namespace_node *device_node,
    enum uacpi_address_space space
);

/*
 * Execute _REG(space, ACPI_REG_CONNECT) for all of the opregions with this
 * address space underneath this device. This should only be called manually
 * if you want to register an early handler that must be available before the
 * call to uacpi_namespace_initialize().
 */
uacpi_status uacpi_reg_all_opregions(
    uacpi_namespace_node *device_node,
    enum uacpi_address_space space
);

#ifdef __cplusplus
}
#endif
