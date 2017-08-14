/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <smmu_v3.h>

/* Test for pending invalidate */
#define INVAL_PENDING(base)	\
	smmuv3_read_s_init(base) & SMMU_S_INIT_INV_ALL_MASK

static inline uint32_t smmuv3_read_s_idr1(uintptr_t base)
{
	return mmio_read_32(base + SMMU_S_IDR1);
}

static inline uint32_t smmuv3_read_s_init(uintptr_t base)
{
	return mmio_read_32(base + SMMU_S_INIT);
}

static inline void smmuv3_write_s_init(uintptr_t base, uint32_t value)
{
	mmio_write_32(base + SMMU_S_INIT, value);
}

/*
 * Initialize the SMMU by invalidating all secure caches and TLBs.
 *
 * Returns 0 on success, and -1 on failure.
 */
int smmuv3_init(uintptr_t smmu_base)
{
	uint32_t idr1_reg;

	/*
	 * Invalidation of secure caches and TLBs is required only if the SMMU
	 * supports secure state. If not, it's implementation defined as to how
	 * SMMU_S_INIT register is accessed.
	 */
	idr1_reg = smmuv3_read_s_idr1(smmu_base);
	if (!((idr1_reg >> SMMU_S_IDR1_SECURE_IMPL_SHIFT) &
			SMMU_S_IDR1_SECURE_IMPL_MASK)) {
		return -1;
	}

	/* Initiate invalidation, and wait for it to finish */
	smmuv3_write_s_init(smmu_base, SMMU_S_INIT_INV_ALL_MASK);
	while (INVAL_PENDING(smmu_base))
		;

	return 0;
}
