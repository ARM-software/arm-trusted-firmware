/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <drivers/coreboot/cbmem_console.h>
#include <common/debug.h>
#include <lib/coreboot.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

/*
 * Structures describing coreboot's in-memory descriptor tables. See
 * <coreboot>/src/commonlib/include/commonlib/coreboot_tables.h for
 * canonical implementation.
 */

typedef struct {
	char signature[4];
	uint32_t header_bytes;
	uint32_t header_checksum;
	uint32_t table_bytes;
	uint32_t table_checksum;
	uint32_t table_entries;
} cb_header_t;

typedef enum {
	CB_TAG_MEMORY = 0x1,
	CB_TAG_SERIAL = 0xf,
	CB_TAG_CBMEM_CONSOLE = 0x17,
} cb_tag_t;

typedef struct {
	uint32_t tag;
	uint32_t size;
	union {
		coreboot_memrange_t memranges[COREBOOT_MAX_MEMRANGES];
		coreboot_serial_t serial;
		uint64_t uint64;
	};
} cb_entry_t;

coreboot_memrange_t coreboot_memranges[COREBOOT_MAX_MEMRANGES];
coreboot_serial_t coreboot_serial;

/*
 * The coreboot table is parsed before the MMU is enabled (i.e. with strongly
 * ordered memory), so we cannot make unaligned accesses. The table entries
 * immediately follow one another without padding, so nothing after the header
 * is guaranteed to be naturally aligned. Therefore, we need to define safety
 * functions that can read unaligned integers.
 */
static uint32_t read_le32(uint32_t *p)
{
	uintptr_t addr = (uintptr_t)p;
	return mmio_read_8(addr)		|
	       mmio_read_8(addr + 1) << 8	|
	       mmio_read_8(addr + 2) << 16	|
	       mmio_read_8(addr + 3) << 24;
}
static uint64_t read_le64(uint64_t *p)
{
	return read_le32((void *)p) | (uint64_t)read_le32((void *)p + 4) << 32;
}

static void expand_and_mmap(uintptr_t baseaddr, size_t size)
{
	uintptr_t pageaddr = round_down(baseaddr, PAGE_SIZE);
	size_t expanded = round_up(baseaddr - pageaddr + size, PAGE_SIZE);
	mmap_add_region(pageaddr, pageaddr, expanded,
			MT_MEMORY | MT_RW | MT_NS | MT_EXECUTE_NEVER);
}

static void setup_cbmem_console(uintptr_t baseaddr)
{
	static console_cbmc_t console;
	assert(!console.console.base);	/* should only have one CBMEM console */

	/* CBMEM console structure stores its size in first header field. */
	uint32_t size = *(uint32_t *)baseaddr;
	expand_and_mmap(baseaddr, size);
	console_cbmc_register(baseaddr, &console);
	console_set_scope(&console.console, CONSOLE_FLAG_BOOT |
					    CONSOLE_FLAG_RUNTIME |
					    CONSOLE_FLAG_CRASH);
}

coreboot_memory_t coreboot_get_memory_type(uintptr_t start, size_t size)
{
	int i;

	for (i = 0; i < COREBOOT_MAX_MEMRANGES; i++) {
		coreboot_memrange_t *range = &coreboot_memranges[i];

		if (range->type == CB_MEM_NONE)
			break;	/* end of table reached */
		if ((start >= range->start) &&
		    (start - range->start < range->size) &&
		    (size <= range->size - (start - range->start))) {
			return range->type;
		}
	}

	return CB_MEM_NONE;
}

void coreboot_table_setup(void *base)
{
	cb_header_t *header = base;
	void *ptr;
	int i;

	if (strncmp(header->signature, "LBIO", 4)) {
		ERROR("coreboot table signature corrupt!\n");
		return;
	}

	ptr = base + header->header_bytes;
	for (i = 0; i < header->table_entries; i++) {
		size_t size;
		cb_entry_t *entry = ptr;

		if (ptr - base >= header->header_bytes + header->table_bytes) {
			ERROR("coreboot table exceeds its bounds!\n");
			break;
		}

		switch (read_le32(&entry->tag)) {
		case CB_TAG_MEMORY:
			size = read_le32(&entry->size) -
			       offsetof(cb_entry_t, memranges);
			if (size > sizeof(coreboot_memranges)) {
				ERROR("Need to truncate coreboot memranges!\n");
				size = sizeof(coreboot_memranges);
			}
			memcpy(&coreboot_memranges, &entry->memranges, size);
			break;
		case CB_TAG_SERIAL:
			memcpy(&coreboot_serial, &entry->serial,
			       sizeof(coreboot_serial));
			break;
		case CB_TAG_CBMEM_CONSOLE:
			setup_cbmem_console(read_le64(&entry->uint64));
			break;
		default:
			/* There are many tags TF doesn't need to care about. */
			break;
		}

		ptr += read_le32(&entry->size);
	}
}
