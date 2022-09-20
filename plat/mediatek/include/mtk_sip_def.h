/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_SIP_DEF_H
#define MTK_SIP_DEF_H

/* Define SiP SMC ID here */
#define MTK_SIP_SMC_FROM_NS_EL1_TABLE(_func) \
	_func(MTK_SIP_KERNEL_TIME_SYNC, 0x202) \
	_func(MTK_SIP_KERNEL_DFD, 0x205) \
	_func(MTK_SIP_KERNEL_MSDC, 0x273) \
	_func(MTK_SIP_VCORE_CONTROL, 0x506) \
	_func(MTK_SIP_IOMMU_CONTROL, 0x514) \
	_func(MTK_SIP_AUDIO_CONTROL, 0x517) \
	_func(MTK_SIP_APUSYS_CONTROL, 0x51E) \
	_func(MTK_SIP_DP_CONTROL, 0x523) \
	_func(MTK_SIP_KERNEL_GIC_OP, 0x526)

#define MTK_SIP_SMC_FROM_BL33_TABLE(_func) \
	_func(MTK_SIP_KERNEL_BOOT, 0x115)

#endif /* MTK_SIP_DEF_H */
