/*
 * Copyright (c) 2025, Texas Instruments. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/smccc.h>
#include <k3_sip_svc.h>
#include <ti_sci.h>
#include <ti_sci_protocol.h>

/* K3 FUSE BUFFER STRUCT */
struct k3_fuse_writebuff {
	uint32_t ver_info;
	uint32_t tisci_id;
	void *payload;
} __packed;

/**
 * ti_fuse_writebuiff_handler - Handler for fuse writebuff SMCCC.
 *
 * @x1: The starting memory address of the buffer
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_fuse_writebuff_handler(u_register_t x1)
{
	int ret;
	const struct k3_fuse_writebuff *k3_fuse_buff;

	if (x1 == 0U) {
		ERROR("x1 param is NULL\n");
		return SMC_UNK;
	}
	k3_fuse_buff = (const struct k3_fuse_writebuff *)x1;
	NOTICE("Writebuff version info: 0x%x\n", k3_fuse_buff->ver_info);
	if (k3_fuse_buff->tisci_id == TISCI_MSG_KEY_WRITER_LITE) {

		/*
		 * Buffer needs to be visible to the TISCI co-processor, since
		 * the size is unknown, flush the entire page
		 */
		flush_dcache_range((uintptr_t)k3_fuse_buff, PAGE_SIZE);

		ret = ti_sci_keywriter_lite((unsigned long)&k3_fuse_buff->payload);
		if (ret) {
			ERROR("Keywriter Lite Failed: (%d)\n", ret);
			return SMC_UNK;
		}
	} else {
		ERROR("Invalid TISCI ID (0x%x)\n", k3_fuse_buff->tisci_id);
		return SMC_UNK;
	}

	return 0;
}
