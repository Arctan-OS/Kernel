%if 0
/**
 * @file sse.asm
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is part of Arctan.
 *
 * Arctan is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
*/
%endif
bits 32
%define FXSAVE_SIZE 512

global _osxsave_support
extern fxsave_space
_osxsave_support:   push ecx
                    push edx
                    push eax
                    lea eax, [rel fxsave_space]
                    fxsave [eax]


                    mov ecx, 0
                    xgetbv
                    or eax, 0b111
                    xsetbv
                    pop eax
                    pop edx
                    pop ecx

                    ret
