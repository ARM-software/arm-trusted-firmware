/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _STDARG_H
#define _STDARG_H

#define va_list __builtin_va_list
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)
#define va_copy(to, from) __builtin_va_copy(to, from)
#define va_arg(to, type) __builtin_va_arg(to, type)

#endif
