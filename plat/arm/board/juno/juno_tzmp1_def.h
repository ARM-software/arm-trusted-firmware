/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_TZMP1_DEF_H
#define JUNO_TZMP1_DEF_H

/*
 * Public memory regions for both protected and non-protected mode
 *
 * OPTEE shared memory 0xFEE00000 - 0xFEFFFFFF
 */
#define JUNO_AP_TZC_SHARE_DRAM1_SIZE		ULL(0x02000000)
#define JUNO_AP_TZC_SHARE_DRAM1_BASE		(ARM_AP_TZC_DRAM1_BASE - \
						 JUNO_AP_TZC_SHARE_DRAM1_SIZE)
#define JUNO_AP_TZC_SHARE_DRAM1_END		(ARM_AP_TZC_DRAM1_BASE - 1)

/* ARM_MEDIA_FEATURES for MEDIA GPU Protect Mode Test */
#define JUNO_TZC400_NSAID_FPGA_MEDIA_SECURE	8	/* GPU/DPU protected, VPU outbuf */
#define JUNO_TZC400_NSAID_FPGA_VIDEO_PROTECTED	7	/* VPU protected */
#define JUNO_TZC400_NSAID_FPGA_VIDEO_PRIVATE	10	/* VPU private (firmware) */

#define JUNO_VPU_TZC_PRIV_DRAM1_SIZE	ULL(0x02000000)
#define JUNO_VPU_TZC_PRIV_DRAM1_BASE	(JUNO_AP_TZC_SHARE_DRAM1_BASE - \
					 JUNO_VPU_TZC_PRIV_DRAM1_SIZE)
#define JUNO_VPU_TZC_PRIV_DRAM1_END	(JUNO_AP_TZC_SHARE_DRAM1_BASE - 1)

/* Video input protected buffer follows upper item */
#define JUNO_VPU_TZC_PROT_DRAM1_SIZE	ULL(0x06000000)
#define JUNO_VPU_TZC_PROT_DRAM1_BASE	(JUNO_VPU_TZC_PRIV_DRAM1_BASE - \
					 JUNO_VPU_TZC_PROT_DRAM1_SIZE)
#define JUNO_VPU_TZC_PROT_DRAM1_END	(JUNO_VPU_TZC_PRIV_DRAM1_BASE - 1)

/* Video, graphics and display shares same NSAID and same protected buffer */
#define JUNO_MEDIA_TZC_PROT_DRAM1_SIZE	ULL(0x0e000000)
#define JUNO_MEDIA_TZC_PROT_DRAM1_BASE	(JUNO_VPU_TZC_PROT_DRAM1_BASE - \
					 JUNO_MEDIA_TZC_PROT_DRAM1_SIZE)
#define JUNO_MEDIA_TZC_PROT_DRAM1_END	(JUNO_VPU_TZC_PROT_DRAM1_BASE - 1)

/* Rest of DRAM1 are Non-Secure public buffer */
#define JUNO_NS_DRAM1_PT1_BASE		ARM_DRAM1_BASE
#define JUNO_NS_DRAM1_PT1_END		(JUNO_MEDIA_TZC_PROT_DRAM1_BASE - 1)
#define JUNO_NS_DRAM1_PT1_SIZE		(JUNO_NS_DRAM1_PT1_END -	\
					 JUNO_NS_DRAM1_PT1_BASE + 1)

/* TZC filter flags */
#define JUNO_MEDIA_TZC_NS_DEV_ACCESS	(PLAT_ARM_TZC_NS_DEV_ACCESS |	\
		TZC_REGION_ACCESS_RD(JUNO_TZC400_NSAID_FPGA_MEDIA_SECURE))

/* VPU / GPU /DPU protected access */
#define JUNO_MEDIA_TZC_PROT_ACCESS \
		(TZC_REGION_ACCESS_RDWR(JUNO_TZC400_NSAID_FPGA_MEDIA_SECURE) | \
		TZC_REGION_ACCESS_WR(TZC400_NSAID_AP))

#define JUNO_VPU_TZC_PROT_ACCESS \
		(TZC_REGION_ACCESS_RDWR(JUNO_TZC400_NSAID_FPGA_VIDEO_PROTECTED))

#define JUNO_VPU_TZC_PRIV_ACCESS \
		(TZC_REGION_ACCESS_RDWR(JUNO_TZC400_NSAID_FPGA_VIDEO_PRIVATE))

/*******************************************************************************
 * Mali-DP650 related constants
 ******************************************************************************/
/* Base address of DP650 */
#define DP650_BASE			0x6f200000
/* offset to PROT_NSAID register */
#define DP650_PROT_NSAID_OFFSET		0x10004
/* config to PROT_NSAID register */
#define DP650_PROT_NSAID_CONFIG		0x08008888

/*******************************************************************************
 * Mali-V550 related constants
 ******************************************************************************/
/* Base address of V550 */
#define V550_BASE			0x6f030000
/* offset to PROTCTRL register */
#define V550_PROTCTRL_OFFSET		0x0040
/* config to PROTCTRL register */
#define V550_PROTCTRL_CONFIG		0xa8700000

#endif /* JUNO_TZMP1_DEF_H */
