/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <ras_arch.h>
#include <utils_def.h>

/*
 * Probe for error in memory-mapped registers containing error records
 * implemented Standard Error Record format. Upon detecting an error, set probe
 * data to the index of the record in error, and return 1; otherwise, return 0.
 */
int ser_probe_memmap(uintptr_t base, unsigned int size_num_k, int *probe_data)
{
	int num_records, num_group_regs, i;
	uint64_t gsr;

	assert(base != 0);

	/* Only 4K supported for now */
	assert(size_num_k == STD_ERR_NODE_SIZE_NUM_K);

	num_records = (mmio_read_32(ERR_DEVID(base, size_num_k)) & ERR_DEVID_MASK);

	/* A group register shows error status for 2^6 error records */
	num_group_regs = (num_records >> 6) + 1;

	/* Iterate through group registers to find a record in error */
	for (i = 0; i < num_group_regs; i++) {
		gsr = mmio_read_64(ERR_GSR(base, size_num_k, i));
		if (gsr == 0)
			continue;

		/* Return the index of the record in error */
		if (probe_data != NULL)
			*probe_data = ((i << 6) + __builtin_ctz(gsr));

		return 1;
	}

	return 0;
}

/*
 * Probe for error in System Registers where error records are implemented in
 * Standard Error Record format. Upon detecting an error, set probe data to the
 * index of the record in error, and return 1; otherwise, return 0.
 */
int ser_probe_sysreg(unsigned int idx_start, unsigned int num_idx, int *probe_data)
{
	int i;
	uint64_t status;
	unsigned int max_idx __unused = read_erridr_el1() & ERRIDR_MASK;

	assert(idx_start < max_idx);
	assert(check_u32_overflow(idx_start, num_idx) == 0);
	assert((idx_start + num_idx - 1) < max_idx);

	for (i = 0; i < num_idx; i++) {
		/* Select the error record */
		ser_sys_select_record(idx_start + i);

		/* Retrieve status register from the error record */
		status = read_erxstatus_el1();

		/* Check for valid field in status */
		if (ERR_STATUS_GET_FIELD(status, V)) {
			if (probe_data != NULL)
				*probe_data = i;
			return 1;
		}
	}

	return 0;
}
