/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <cdefs.h>
#include <drivers/arm/smmu_v3.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

/* SMMU poll number of retries */
#define SMMU_POLL_TIMEOUT_US	U(1000)

static int __init smmuv3_poll(uintptr_t smmu_reg, uint32_t mask,
				uint32_t value)
{
	uint32_t reg_val;
	uint64_t timeout;

	/* Set 1ms timeout value */
	timeout = timeout_init_us(SMMU_POLL_TIMEOUT_US);
	do {
		reg_val = mmio_read_32(smmu_reg);
		if ((reg_val & mask) == value)
			return 0;
	} while (!timeout_elapsed(timeout));

	ERROR("Timeout polling SMMUv3 register @%p\n", (void *)smmu_reg);
	ERROR("Read value 0x%x, expected 0x%x\n", reg_val,
		value == 0U ? reg_val & ~mask : reg_val | mask);
	return -1;
}

/*
 * Abort all incoming transactions in order to implement a default
 * deny policy on reset.
 */
int __init smmuv3_security_init(uintptr_t smmu_base)
{
	/* Attribute update has completed when SMMU_(S)_GBPA.Update bit is 0 */
	if (smmuv3_poll(smmu_base + SMMU_GBPA, SMMU_GBPA_UPDATE, 0U) != 0U)
		return -1;

	/*
	 * SMMU_(S)_CR0 resets to zero with all streams bypassing the SMMU,
	 * so just abort all incoming transactions.
	 */
	mmio_setbits_32(smmu_base + SMMU_GBPA,
			SMMU_GBPA_UPDATE | SMMU_GBPA_ABORT);

	if (smmuv3_poll(smmu_base + SMMU_GBPA, SMMU_GBPA_UPDATE, 0U) != 0U)
		return -1;

	/* Check if the SMMU supports secure state */
	if ((mmio_read_32(smmu_base + SMMU_S_IDR1) &
				SMMU_S_IDR1_SECURE_IMPL) == 0U)
		return 0;

	/* Abort all incoming secure transactions */
	if (smmuv3_poll(smmu_base + SMMU_S_GBPA, SMMU_S_GBPA_UPDATE, 0U) != 0U)
		return -1;

	mmio_setbits_32(smmu_base + SMMU_S_GBPA,
			SMMU_S_GBPA_UPDATE | SMMU_S_GBPA_ABORT);

	return smmuv3_poll(smmu_base + SMMU_S_GBPA, SMMU_S_GBPA_UPDATE, 0U);
}

/*
 * Initialize the SMMU by invalidating all secure caches and TLBs.
 * Abort all incoming transactions in order to implement a default
 * deny policy on reset
 */
int __init smmuv3_init(uintptr_t smmu_base)
{
	/* Abort all incoming transactions */
	if (smmuv3_security_init(smmu_base) != 0)
		return -1;

	/* Check if the SMMU supports secure state */
	if ((mmio_read_32(smmu_base + SMMU_S_IDR1) &
				SMMU_S_IDR1_SECURE_IMPL) == 0U)
		return 0;
	/*
	 * Initiate invalidation of secure caches and TLBs if the SMMU
	 * supports secure state. If not, it's implementation defined
	 * as to how SMMU_S_INIT register is accessed.
	 */
	mmio_write_32(smmu_base + SMMU_S_INIT, SMMU_S_INIT_INV_ALL);

	/* Wait for global invalidation operation to finish */
	return smmuv3_poll(smmu_base + SMMU_S_INIT,
				SMMU_S_INIT_INV_ALL, 0U);
}
