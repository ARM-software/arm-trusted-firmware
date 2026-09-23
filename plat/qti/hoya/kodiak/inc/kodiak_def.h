/*
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef KODIAK_DEF_H
#define KODIAK_DEF_H

#include <hoya_def.h>

/*----------------------------------------------------------------------------*/
/* IP protected memory (TA execution area)                                    */
/*----------------------------------------------------------------------------*/
#define QTI_PIMEM_BASE			0x1c000000
#define QTI_PIMEM_LIMIT			0x20000000

/*----------------------------------------------------------------------------*/
/* UART related constants. */
/*----------------------------------------------------------------------------*/
#define PLAT_QTI_UART_BASE			0x994000

/*----------------------------------------------------------------------------*/
/* Peripherals base addresses */
/*----------------------------------------------------------------------------*/
#define QTI_SEC_PRNG_BASE			0x10D0000

/*----------------------------------------------------------------------------*/
/* Device address space for mapping. Excluding starting 4K */
/*----------------------------------------------------------------------------*/
#define QTI_DEVICE_BASE				0x1000
#define QTI_DEVICE_SIZE				(0x1C000000 - QTI_DEVICE_BASE)

/*----------------------------------------------------------------------------*/
/* AOP CMD DB  address space for mapping */
/*----------------------------------------------------------------------------*/
#define QTI_AOP_CMD_DB_BASE			0x80860000
#define QTI_AOP_CMD_DB_SIZE			0x00020000
/*----------------------------------------------------------------------------*/
/* SMEM base address                                                          */
/*----------------------------------------------------------------------------*/
#define QTI_SMEM_BASE				0x80900000
#define QTI_SMEM_SIZE				0x00200000
/*----------------------------------------------------------------------------*/
/* LC PON register offsets */
/*----------------------------------------------------------------------------*/
#define PON_PS_HOLD_RESET_CTL			0x852
#define PON_PS_HOLD_RESET_CTL2			0x853
/*----------------------------------------------------------------------------*/
/* APSS HM registers */
/*----------------------------------------------------------------------------*/
#define QTI_APSS_HM_BASE			0x17800000
#define QTI_APSS_HM_SIZE			0x00d99000
/*----------------------------------------------------------------------------*/
/* QTIMER registers                                                           */
/*----------------------------------------------------------------------------*/
#define QTI_QTIMER_BASE				0x17C20000
/*----------------------------------------------------------------------------*/
/* AOSS registers */
/*----------------------------------------------------------------------------*/
#define QTI_AOSS_BASE				0x0b000000
#define QTI_AOSS_SIZE				0x04000000
/*----------------------------------------------------------------------------*/
/* CORE_TOP_CSR */
/*----------------------------------------------------------------------------*/
#define QTI_CORE_TOP_CSR_BASE			0x01f00000
#define QTI_CORE_TOP_CSR_BASE_SIZE		0x00100000
/*----------------------------------------------------------------------------*/
/* Platform-specific SMMU config base addresses */
/*----------------------------------------------------------------------------*/
#define QTI_SMMU_CFG_BASE			0x15000000
#define QTI_GPU_SMMU_CFG_BASE			0x03d60000

/*----------------------------------------------------------------------------*/
/* Chipset specific NOC error interrupt IDs.                                  */
/*----------------------------------------------------------------------------*/
#define PLAT_INT_ID_A1_NOC_ERROR		(0xC9)
#define PLAT_INT_ID_A2_NOC_ERROR		(0xEA)
#define PLAT_INT_ID_SYSTEM_NOC_ERROR		(0xC8)
#define PLAT_INT_ID_LPASS_AGNOC_ERROR		(0x143)
#define PLAT_INT_ID_NSP_NOC_ERROR		(0x1CE)

#endif /* KODIAK_DEF_H */
