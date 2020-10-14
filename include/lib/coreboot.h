/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
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

#define COREBOOT_MAX_MEMRANGES	32	/* libpayload also uses this limit */

typedef struct __packed {
	uint64_t start;
	uint64_t size;
	uint32_t type;
} coreboot_memrange_t;
extern coreboot_memrange_t coreboot_memranges[COREBOOT_MAX_MEMRANGES];

typedef enum {
	CB_MEM_NONE		= 0,	/* coreboot will never report this */
	CB_MEM_RAM		= 1,
	CB_MEM_RESERVED		= 2,
	CB_MEM_ACPI		= 3,
	CB_MEM_NVS		= 4,
	CB_MEM_UNUSABLE		= 5,
	CB_MEM_VENDOR_RSVD	= 6,
	CB_MEM_TABLE		= 16,
} coreboot_memory_t;

coreboot_memory_t coreboot_get_memory_type(uintptr_t start, size_t size);
void coreboot_table_setup(void *base);

#endif /* COREBOOT_H */
