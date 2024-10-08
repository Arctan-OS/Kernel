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

typedef struct uacpi_namespace_node uacpi_namespace_node;

uacpi_namespace_node *uacpi_namespace_root(void);

typedef enum uacpi_predefined_namespace {
    UACPI_PREDEFINED_NAMESPACE_ROOT = 0,
    UACPI_PREDEFINED_NAMESPACE_GPE,
    UACPI_PREDEFINED_NAMESPACE_PR,
    UACPI_PREDEFINED_NAMESPACE_SB,
    UACPI_PREDEFINED_NAMESPACE_SI,
    UACPI_PREDEFINED_NAMESPACE_TZ,
    UACPI_PREDEFINED_NAMESPACE_GL,
    UACPI_PREDEFINED_NAMESPACE_OS,
    UACPI_PREDEFINED_NAMESPACE_OSI,
    UACPI_PREDEFINED_NAMESPACE_REV,
    UACPI_PREDEFINED_NAMESPACE_MAX = UACPI_PREDEFINED_NAMESPACE_REV,
} uacpi_predefined_namespace;
uacpi_namespace_node *uacpi_namespace_get_predefined(
    uacpi_predefined_namespace
);

uacpi_object *uacpi_namespace_node_get_object(const uacpi_namespace_node *node);
uacpi_object_name uacpi_namespace_node_name(const uacpi_namespace_node *node);

uacpi_size uacpi_namespace_node_depth(const uacpi_namespace_node *node);

uacpi_namespace_node *uacpi_namespace_node_find(
    uacpi_namespace_node *parent,
    const uacpi_char *path
);

/*
 * Same as uacpi_namespace_node_find, except the search recurses upwards when
 * the namepath consists of only a single nameseg. Usually, this behavior is
 * only desired if resolving a namepath specified in an aml-provided object,
 * such as a package element.
 */
uacpi_namespace_node *uacpi_namespace_node_resolve_from_aml_namepath(
    uacpi_namespace_node *scope,
    const uacpi_char *path
);

typedef enum uacpi_ns_iteration_decision {
    // Continue to the next child of this node
    UACPI_NS_ITERATION_DECISION_CONTINUE,

    /*
     * Don't go any deeper, instead continue to the next peer of the
     * parent node currently being iterated.
     */
    UACPI_NS_ITERATION_DECISION_NEXT_PEER,

    // Abort iteration
    UACPI_NS_ITERATION_DECISION_BREAK,
} uacpi_ns_iteration_decision;

typedef uacpi_ns_iteration_decision
    (*uacpi_iteration_callback)(void *user, uacpi_namespace_node *node);

void uacpi_namespace_for_each_node_depth_first(
    uacpi_namespace_node *parent,
    uacpi_iteration_callback callback,
    void *user
);

const uacpi_char *uacpi_namespace_node_generate_absolute_path(
    const uacpi_namespace_node *node
);
void uacpi_free_absolute_path(const uacpi_char *path);

#ifdef __cplusplus
}
#endif
