/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SHIKRA_DEF_H
#define SHIKRA_DEF_H

#include <bruin_def.h>

/*----------------------------------------------------------------------------*/
/* IP protected memory (TA execution area) limit                              */
/*----------------------------------------------------------------------------*/
#define QTI_PIMEM_LIMIT			0x14000000

/*****************************************************************************/
/* Memory mapped I/O  */
/*****************************************************************************/

/*----------------------------------------------------------------------------*/
/* GIC-600 constants */
/*----------------------------------------------------------------------------*/
#define BASE_GICD_BASE			0x0F200000
#define BASE_GICR_BASE			0x0F240000
#define BASE_GICC_BASE			0x0
#define BASE_GICH_BASE			0x0
#define BASE_GICV_BASE			0x0

#define QTI_GICD_BASE			BASE_GICD_BASE
#define QTI_GICR_BASE			BASE_GICR_BASE
#define QTI_GICC_BASE			BASE_GICC_BASE

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* UART base. The GENI register layout comes from bruin_def.h. */
/*----------------------------------------------------------------------------*/
#define PLAT_QTI_UART_BASE			0x04A80000

/*----------------------------------------------------------------------------*/
/* Peripherals base addresses */
/*----------------------------------------------------------------------------*/
#define QTI_SEC_PRNG_BASE			0x04450000

/*----------------------------------------------------------------------------*/
/* Device address space for mapping. Excluding starting 4K */
/*----------------------------------------------------------------------------*/
#define QTI_DEVICE_SIZE				(0x14000000 - QTI_DEVICE_BASE)

/*----------------------------------------------------------------------------*/
/* LC PON register offsets */
/*----------------------------------------------------------------------------*/
#define PON_PS_HOLD_RESET_CTL			0x852
#define PON_PS_HOLD_RESET_CTL2			0x853
/*----------------------------------------------------------------------------*/
/* APSS HM registers */
/*----------------------------------------------------------------------------*/
#define QTI_APSS_HM_BASE			0x0F000000
#define QTI_APSS_HM_SIZE			0x00E00000
/*----------------------------------------------------------------------------*/
/* CORE_TOP_CSR */
/*----------------------------------------------------------------------------*/
#define QTI_CORE_TOP_CSR_BASE			0x00300000
#define QTI_CORE_TOP_CSR_BASE_SIZE		0x00100000
/*----------------------------------------------------------------------------*/
/* QTIMER registers                                                           */
/*----------------------------------------------------------------------------*/
#define QTI_QTIMER_BASE				0x0F420000
/*----------------------------------------------------------------------------*/
/* SMEM base address                                                          */
/*----------------------------------------------------------------------------*/
#define QTI_SMEM_BASE				0x86000000
/*----------------------------------------------------------------------------*/
/* Interrupt IDs */
/*----------------------------------------------------------------------------*/
#define PLAT_INT_ID_SEC_WDOG_BARK		0x132
#define PLAT_INT_ID_NON_SEC_WDOG_BITE		0x21

#define PLAT_INT_ID_VMIDMT_ERR_CLT_SEC		0x197
#define PLAT_INT_ID_VMIDMT_ERR_CLT_NONSEC	0x198
#define PLAT_INT_ID_VMIDMT_ERR_CFG_SEC		0x199
#define PLAT_INT_ID_VMIDMT_ERR_CFG_NONSEC	0x19A

#define PLAT_INT_ID_XPU_SEC			0x194
#define PLAT_INT_ID_XPU_NON_SEC			0x195

#define PLAT_INT_ID_SYSTEM_NOC_ERROR		0xF9
#define PLAT_INT_ID_CONFIG_NOC_ERROR		0xFA
#define PLAT_INT_ID_MEM_NOC_ERROR		0xFC
#define PLAT_INT_ID_DC_NOC_ERROR		0x11D

/*----------------------------------------------------------------------------*/
#endif /* SHIKRA_DEF_H */
