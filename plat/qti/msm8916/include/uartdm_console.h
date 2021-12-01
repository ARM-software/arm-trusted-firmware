/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UARTDM_CONSOLE_H
#define UARTDM_CONSOLE_H

int console_uartdm_register(console_t *console, uintptr_t base_addr);

#endif /* UARTDM_CONSOLE_H */
