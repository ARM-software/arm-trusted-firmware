/*
 * Copyright (c) 2024, The Linux Foundation. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __QTISECLIB_DEFS_PLAT_H__
#define __QTISECLIB_DEFS_PLAT_H__

#define QTISECLIB_PLAT_CLUSTER_COUNT   1
#define QTISECLIB_PLAT_CORE_COUNT      8

#define BL31_BASE                0x86200000
#define BL31_SIZE                0x00100000

/*----------------------------------------------------------------------------*/
/* AOP CMD DB  address space for mapping */
/*----------------------------------------------------------------------------*/
#define QTI_AOP_CMD_DB_BASE         0x85F20000
#define QTI_AOP_CMD_DB_SIZE         0x00020000

/* Chipset specific secure interrupt number/ID defs. */
#define QTISECLIB_INT_ID_SEC_WDOG_BARK          (0x204)
#define QTISECLIB_INT_ID_NON_SEC_WDOG_BITE      (0x21)

#define QTISECLIB_INT_ID_VMIDMT_ERR_CLT_SEC     (0xE6)
#define QTISECLIB_INT_ID_VMIDMT_ERR_CLT_NONSEC  (0xE7)
#define QTISECLIB_INT_ID_VMIDMT_ERR_CFG_SEC     (0xE8)
#define QTISECLIB_INT_ID_VMIDMT_ERR_CFG_NONSEC  (0xE9)

#define QTISECLIB_INT_ID_XPU_SEC                (0xE3)
#define QTISECLIB_INT_ID_XPU_NON_SEC            (0xE4)

//NOC INterrupt
#define QTISECLIB_INT_ID_A1_NOC_ERROR        (0x18B)
#define QTISECLIB_INT_ID_CONFIG_NOC_ERROR    (0xE2)
#define QTISECLIB_INT_ID_DC_NOC_ERROR        (0x122)
#define QTISECLIB_INT_ID_MEM_NOC_ERROR       (0x6C) //GEM_NOC
#define QTISECLIB_INT_ID_SYSTEM_NOC_ERROR    (0xC6)
#define QTISECLIB_INT_ID_MMSS_NOC_ERROR      (0xBA)

#endif /* __QTISECLIB_DEFS_PLAT_H__ */
