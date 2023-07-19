/*
 * Copyright (c) 2021-2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MSM8916_MMAP_H
#define MSM8916_MMAP_H

#define PCNOC_BASE		0x00000000
#define PCNOC_SIZE		SZ_128M
#define APCS_BASE		0x0b000000
#define APCS_SIZE		SZ_8M

#define MPM_BASE		(PCNOC_BASE + 0x04a0000)
#define MPM_PS_HOLD		(MPM_BASE + 0xb000)

#define TLMM_BASE		(PCNOC_BASE + 0x1000000)
#define TLMM_GPIO_CFG(n)	(TLMM_BASE + ((n) * 0x1000))

#define GCC_BASE		(PCNOC_BASE + 0x1800000)

#define APPS_SMMU_BASE		(PCNOC_BASE + 0x1e00000)
#define APPS_SMMU_QCOM		(APPS_SMMU_BASE + 0xf0000)
#define GPU_SMMU_BASE		(PCNOC_BASE + 0x1f00000)

#define BLSP1_BASE		(PCNOC_BASE + 0x7880000)
#define BLSP1_UART_BASE(n)	(BLSP1_BASE + 0x2f000 + (((n) - 1) * 0x1000))
#define BLSP_UART_BASE		BLSP1_UART_BASE(QTI_UART_NUM)

#define APCS_QGIC2_BASE		(APCS_BASE + 0x00000)
#define APCS_QGIC2_GICD		(APCS_QGIC2_BASE + 0x0000)
#define APCS_QGIC2_GICC		(APCS_QGIC2_BASE + 0x2000)
#define APCS_BANKED_ACS		(APCS_BASE + 0x08000)
#define APCS_BANKED_SAW2	(APCS_BASE + 0x09000)

#define _APCS_CLUSTER(cluster)	(APCS_BASE + ((cluster) * 0x100000))
#define _APCS_CPU(cluster, cpu)	(_APCS_CLUSTER(cluster) + ((cpu) * 0x10000))
#define APCS_CFG(cluster)	(_APCS_CLUSTER(cluster) + 0x10000)
#define APCS_GLB(cluster)	(_APCS_CLUSTER(cluster) + 0x11000)
#define APCS_L2_SAW2(cluster)	(_APCS_CLUSTER(cluster) + 0x12000)
#define APCS_QTMR(cluster)	(_APCS_CLUSTER(cluster) + 0x20000)
#define APCS_ALIAS_ACS(cluster, cpu)	(_APCS_CPU(cluster, cpu) + 0x88000)
#define APCS_ALIAS_SAW2(cluster, cpu)	(_APCS_CPU(cluster, cpu) + 0x89000)

/* Only on platforms with multiple clusters (e.g. MSM8939) */
#define APCS_CCI_BASE		(APCS_BASE + 0x1c0000)
#define APCS_CCI_SAW2		(APCS_BASE + 0x1d2000)
#define APCS_CCI_ACS		(APCS_BASE + 0x1d4000)

#endif /* MSM8916_MMAP_H */
