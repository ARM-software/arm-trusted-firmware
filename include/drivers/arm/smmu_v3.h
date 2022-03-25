/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_V3_H
#define SMMU_V3_H

#include <stdint.h>
#include <lib/utils_def.h>
#include <platform_def.h>

/* SMMUv3 register offsets from device base */
#define SMMU_CR0	U(0x0020)
#define SMMU_CR0ACK	U(0x0024)
#define SMMU_GBPA	U(0x0044)
#define SMMU_S_IDR1	U(0x8004)
#define SMMU_S_INIT	U(0x803c)
#define SMMU_S_GBPA	U(0x8044)

/*
 * TODO: SMMU_ROOT_PAGE_OFFSET is platform specific.
 * Currently defined as a command line model parameter.
 */
#if ENABLE_RME

#define SMMU_ROOT_PAGE_OFFSET	(PLAT_ARM_SMMUV3_ROOT_REG_OFFSET)
#define SMMU_ROOT_IDR0		U(SMMU_ROOT_PAGE_OFFSET + 0x0000)
#define SMMU_ROOT_IIDR		U(SMMU_ROOT_PAGE_OFFSET + 0x0008)
#define SMMU_ROOT_CR0		U(SMMU_ROOT_PAGE_OFFSET + 0x0020)
#define SMMU_ROOT_CR0ACK	U(SMMU_ROOT_PAGE_OFFSET + 0x0024)
#define SMMU_ROOT_GPT_BASE	U(SMMU_ROOT_PAGE_OFFSET + 0x0028)
#define SMMU_ROOT_GPT_BASE_CFG	U(SMMU_ROOT_PAGE_OFFSET + 0x0030)
#define SMMU_ROOT_GPF_FAR	U(SMMU_ROOT_PAGE_OFFSET + 0x0038)
#define SMMU_ROOT_GPT_CFG_FAR	U(SMMU_ROOT_PAGE_OFFSET + 0x0040)
#define SMMU_ROOT_TLBI		U(SMMU_ROOT_PAGE_OFFSET + 0x0050)
#define SMMU_ROOT_TLBI_CTRL	U(SMMU_ROOT_PAGE_OFFSET + 0x0058)

#endif /* ENABLE_RME */

/* SMMU_CR0 and SMMU_CR0ACK register fields */
#define SMMU_CR0_SMMUEN			(1UL << 0)

/* SMMU_GBPA register fields */
#define SMMU_GBPA_UPDATE		(1UL << 31)
#define SMMU_GBPA_ABORT			(1UL << 20)

/* SMMU_S_IDR1 register fields */
#define SMMU_S_IDR1_SECURE_IMPL		(1UL << 31)

/* SMMU_S_INIT register fields */
#define SMMU_S_INIT_INV_ALL		(1UL << 0)

/* SMMU_S_GBPA register fields */
#define SMMU_S_GBPA_UPDATE		(1UL << 31)
#define SMMU_S_GBPA_ABORT		(1UL << 20)

/* SMMU_ROOT_IDR0 register fields */
#define SMMU_ROOT_IDR0_ROOT_IMPL	(1UL << 0)

/* SMMU_ROOT_CR0 register fields */
#define SMMU_ROOT_CR0_GPCEN		(1UL << 1)
#define SMMU_ROOT_CR0_ACCESSEN		(1UL << 0)

int smmuv3_init(uintptr_t smmu_base);
int smmuv3_security_init(uintptr_t smmu_base);

int smmuv3_ns_set_abort_all(uintptr_t smmu_base);

#endif /* SMMU_V3_H */
