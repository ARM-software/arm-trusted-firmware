/*
 * Copyright (c) 2019, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPE_H
#define SPE_H

#include <stdint.h>

#include <drivers/console.h>

/*
 * Initialize a new spe console instance and register it with the console
 * framework. The |console| pointer must point to storage that will be valid
 * for the lifetime of the console, such as a global or static local variable.
 * Its contents will be reinitialized from scratch.
 */
int console_spe_register(uintptr_t baseaddr, uint32_t clock, uint32_t baud,
			 console_t *console);

#endif /* SPE_H */
