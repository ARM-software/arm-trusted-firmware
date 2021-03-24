/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef NXP_SMMU_H
#define NXP_SMMU_H

#define SMMU_SCR0		(0x0)
#define SMMU_NSCR0		(0x400)

#define SCR0_CLIENTPD_MASK	0x00000001
#define SCR0_USFCFG_MASK	0x00000400

static inline void bypass_smmu(uintptr_t smmu_base_addr)
{
	uint32_t val;

	val = (mmio_read_32(smmu_base_addr + SMMU_SCR0) | SCR0_CLIENTPD_MASK) &
		~(SCR0_USFCFG_MASK);
	mmio_write_32((smmu_base_addr + SMMU_SCR0), val);

	val = (mmio_read_32(smmu_base_addr + SMMU_NSCR0) | SCR0_CLIENTPD_MASK) &
		~(SCR0_USFCFG_MASK);
	mmio_write_32((smmu_base_addr + SMMU_NSCR0), val);
}

#endif
