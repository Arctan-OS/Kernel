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
#include <uacpi/internal/shareable.h>

#define BUGGED_REFCOUNT 0xFFFFFFFF

void uacpi_shareable_init(uacpi_handle handle)
{
    struct uacpi_shareable *shareable = handle;
    shareable->reference_count = 1;
}

uacpi_bool uacpi_bugged_shareable(uacpi_handle handle)
{
    struct uacpi_shareable *shareable = handle;

    if (uacpi_unlikely(shareable->reference_count == 0))
        shareable->reference_count = BUGGED_REFCOUNT;

    return shareable->reference_count == BUGGED_REFCOUNT;
}

void uacpi_make_shareable_bugged(uacpi_handle handle)
{
    struct uacpi_shareable *shareable = handle;
    shareable->reference_count = BUGGED_REFCOUNT;
}

uacpi_u32 uacpi_shareable_ref(uacpi_handle handle)
{
    struct uacpi_shareable *shareable = handle;

    if (uacpi_unlikely(uacpi_bugged_shareable(shareable)))
        return BUGGED_REFCOUNT;

    return shareable->reference_count++;
}

uacpi_u32 uacpi_shareable_unref(uacpi_handle handle)
{
    struct uacpi_shareable *shareable = handle;

    if (uacpi_unlikely(uacpi_bugged_shareable(shareable)))
        return BUGGED_REFCOUNT;

    return shareable->reference_count--;
}

void uacpi_shareable_unref_and_delete_if_last(
    uacpi_handle handle, void (*do_free)(uacpi_handle)
)
{
    if (handle == UACPI_NULL)
        return;

    if (uacpi_unlikely(uacpi_bugged_shareable(handle)))
        return;

    if (uacpi_shareable_unref(handle) == 1)
        do_free(handle);
}

uacpi_u32 uacpi_shareable_refcount(uacpi_handle handle)
{
    struct uacpi_shareable *shareable = handle;
    return shareable->reference_count;
}
