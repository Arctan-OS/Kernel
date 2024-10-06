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

/*
 * Compiler-specific attributes/macros go here. This is the default placeholder
 * that should work for MSVC/GCC/clang.
 */

#ifdef UACPI_OVERRIDE_COMPILER
#include "uacpi_compiler.h"
#else

#define UACPI_ALIGN(x) __declspec(align(x))

#ifdef _MSC_VER
    #include <intrin.h>

    #define UACPI_ALWAYS_INLINE __forceinline

    #define UACPI_PACKED(decl)  \
        __pragma(pack(push, 1)) \
        decl;                   \
        __pragma(pack(pop))
#else
    #define UACPI_ALWAYS_INLINE inline __attribute__((always_inline))
    #define UACPI_PACKED(decl) decl __attribute__((packed));
#endif

#ifdef __GNUC__
    #define uacpi_unlikely(expr) __builtin_expect(!!(expr), 0)
    #define uacpi_likely(expr)   __builtin_expect(!!(expr), 1)

    #if __has_attribute(__fallthrough__)
        #define UACPI_FALLTHROUGH __attribute__((__fallthrough__))
    #endif

    #define UACPI_MAYBE_UNUSED __attribute__ ((unused))

    #define UACPI_NO_UNUSED_PARAMETER_WARNINGS_BEGIN             \
        _Pragma("GCC diagnostic push")                           \
        _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")

    #define UACPI_NO_UNUSED_PARAMETER_WARNINGS_END \
        _Pragma("GCC diagnostic pop")

    #ifdef __clang__
        #define UACPI_PRINTF_DECL(fmt_idx, args_idx) \
            __attribute__((format(printf, fmt_idx, args_idx)))
    #else
        #define UACPI_PRINTF_DECL(fmt_idx, args_idx) \
            __attribute__((format(gnu_printf, fmt_idx, args_idx)))
    #endif
#else
    #define uacpi_unlikely(expr) expr
    #define uacpi_likely(expr)   expr

    #define UACPI_MAYBE_UNUSED

    #define UACPI_NO_UNUSED_PARAMETER_WARNINGS_BEGIN
    #define UACPI_NO_UNUSED_PARAMETER_WARNINGS_END

    #define UACPI_PRINTF_DECL(fmt_idx, args_idx)
#endif

#ifndef UACPI_FALLTHROUGH
    #define UACPI_FALLTHROUGH do {} while (0)
#endif

#ifndef UACPI_POINTER_SIZE
    #ifdef _WIN32
        #ifdef _WIN64
            #define UACPI_POINTER_SIZE 8
        #else
            #define UACPI_POINTER_SIZE 4
        #endif
    #elif defined(__GNUC__)
        #define UACPI_POINTER_SIZE __SIZEOF_POINTER__
    #else
        #error Failed to detect pointer size
    #endif
#endif

#endif
