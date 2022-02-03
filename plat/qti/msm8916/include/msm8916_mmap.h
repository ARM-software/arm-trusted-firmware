/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MSM8916_MMAP_H
#define MSM8916_MMAP_H

#define PCNOC_BASE		0x00000000
#define PCNOC_SIZE		0x8000000	/* 128 MiB */
#define APCS_BASE		0x0b000000
#define APCS_SIZE		0x800000	/* 8 MiB */

#define MPM_BASE		(PCNOC_BASE + 0x04a0000)
#define MPM_PS_HOLD		(MPM_BASE + 0xb000)

#define TLMM_BASE		(PCNOC_BASE + 0x1000000)
#define TLMM_GPIO_CFG(n)	(TLMM_BASE + ((n) * 0x1000))

#define GCC_BASE		(PCNOC_BASE + 0x1800000)

#define APPS_SMMU_BASE		(PCNOC_BASE + 0x1e00000)
#define APPS_SMMU_QCOM		(APPS_SMMU_BASE + 0xf0000)

#define BLSP_UART1_BASE		(PCNOC_BASE + 0x78af000)
#define BLSP_UART2_BASE		(PCNOC_BASE + 0x78b0000)

#define APCS_QGIC2_BASE		(APCS_BASE + 0x00000)
#define APCS_QGIC2_GICD		(APCS_QGIC2_BASE + 0x0000)
#define APCS_QGIC2_GICC		(APCS_QGIC2_BASE + 0x2000)
#define APCS_BANKED_ACS		(APCS_BASE + 0x08000)
#define APCS_BANKED_SAW2	(APCS_BASE + 0x09000)
#define APCS_CFG		(APCS_BASE + 0x10000)
#define APCS_GLB		(APCS_BASE + 0x11000)
#define APCS_L2_SAW2		(APCS_BASE + 0x12000)
#define APCS_QTMR		(APCS_BASE + 0x20000)
#define APCS_ALIAS_ACS(cpu)	(APCS_BASE + 0x88000 + ((cpu) * 0x10000))
#define APCS_ALIAS_SAW2(cpu)	(APCS_BASE + 0x89000 + ((cpu) * 0x10000))

#endif /* MSM8916_MMAP_H */
