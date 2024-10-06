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
#include <uacpi/namespace.h>

#define UACPI_NAMESPACE_NODE_FLAG_ALIAS (1 << 0)

/*
 * This node has been uninstalled and has no object associated with it.
 *
 * This is used to handle edge cases where an object needs to reference
 * a namespace node, where the node might end up going out of scope before
 * the object lifetime ends.
 */
#define UACPI_NAMESPACE_NODE_FLAG_DANGLING (1u << 1)

#define UACPI_NAMESPACE_NODE_PREDEFINED (1u << 31)

typedef struct uacpi_namespace_node {
    struct uacpi_shareable shareable;
    uacpi_object_name name;
    uacpi_u32 flags;
    uacpi_object *object;
    struct uacpi_namespace_node *parent;
    struct uacpi_namespace_node *child;
    struct uacpi_namespace_node *next;
} uacpi_namespace_node;

uacpi_status uacpi_initialize_namespace(void);
void uacpi_deinitialize_namespace(void);

uacpi_namespace_node *uacpi_namespace_node_alloc(uacpi_object_name name);
void uacpi_namespace_node_unref(uacpi_namespace_node *node);

uacpi_status uacpi_node_install(uacpi_namespace_node *parent, uacpi_namespace_node *node);
void uacpi_node_uninstall(uacpi_namespace_node *node);

uacpi_namespace_node *uacpi_namespace_node_find_sub_node(
    uacpi_namespace_node *parent,
    uacpi_object_name name
);

uacpi_bool uacpi_namespace_node_is_dangling(uacpi_namespace_node *node);
uacpi_bool uacpi_namespace_node_is_predefined(uacpi_namespace_node *node);
