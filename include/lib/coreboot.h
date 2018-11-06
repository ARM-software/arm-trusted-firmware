/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COREBOOT_H
#define COREBOOT_H

#include <stdint.h>

typedef struct {
	uint32_t type;			/* always 2 (memory-mapped) on ARM */
	uint32_t baseaddr;
	uint32_t baud;
	uint32_t regwidth;		/* in bytes, i.e. usually 4 */
	uint32_t input_hertz;
	uint32_t uart_pci_addr;		/* unused on current ARM systems */
} coreboot_serial_t;
extern coreboot_serial_t coreboot_serial;

void coreboot_table_setup(void *base);

#endif /* COREBOOT_H */
