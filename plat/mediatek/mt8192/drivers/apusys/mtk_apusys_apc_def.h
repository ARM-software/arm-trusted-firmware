/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTK_APUSYS_APC_DEF_H__
#define __MTK_APUSYS_APC_DEF_H__

#include <lib/mmio.h>

enum APUSYS_APC_ERR_STATUS {
	APUSYS_APC_OK = 0x0,

	APUSYS_APC_ERR_GENERIC = 0x1000,
	APUSYS_APC_ERR_INVALID_CMD = 0x1001,
	APUSYS_APC_ERR_SLAVE_TYPE_NOT_SUPPORTED = 0x1002,
	APUSYS_APC_ERR_SLAVE_IDX_NOT_SUPPORTED = 0x1003,
	APUSYS_APC_ERR_DOMAIN_NOT_SUPPORTED = 0x1004,
	APUSYS_APC_ERR_PERMISSION_NOT_SUPPORTED = 0x1005,
	APUSYS_APC_ERR_OUT_OF_BOUNDARY = 0x1006,
	APUSYS_APC_ERR_REQ_TYPE_NOT_SUPPORTED = 0x1007,
};

enum APUSYS_APC_PERM_TYPE {
	NO_PROTECTION = 0U,
	SEC_RW_ONLY = 1U,
	SEC_RW_NS_R = 2U,
	FORBIDDEN = 3U,
	PERM_NUM = 4U,
};

enum APUSYS_APC_DOMAIN_ID {
	DOMAIN_0 = 0U,
	DOMAIN_1 = 1U,
	DOMAIN_2 = 2U,
	DOMAIN_3 = 3U,
	DOMAIN_4 = 4U,
	DOMAIN_5 = 5U,
	DOMAIN_6 = 6U,
	DOMAIN_7 = 7U,
	DOMAIN_8 = 8U,
	DOMAIN_9 = 9U,
	DOMAIN_10 = 10U,
	DOMAIN_11 = 11U,
	DOMAIN_12 = 12U,
	DOMAIN_13 = 13U,
	DOMAIN_14 = 14U,
	DOMAIN_15 = 15U,
};

struct APC_DOM_16 {
	unsigned char d0_permission;
	unsigned char d1_permission;
	unsigned char d2_permission;
	unsigned char d3_permission;
	unsigned char d4_permission;
	unsigned char d5_permission;
	unsigned char d6_permission;
	unsigned char d7_permission;
	unsigned char d8_permission;
	unsigned char d9_permission;
	unsigned char d10_permission;
	unsigned char d11_permission;
	unsigned char d12_permission;
	unsigned char d13_permission;
	unsigned char d14_permission;
	unsigned char d15_permission;
};

#define APUSYS_APC_AO_ATTR(DEV_NAME, PERM_ATTR0, PERM_ATTR1, \
		PERM_ATTR2, PERM_ATTR3, PERM_ATTR4, PERM_ATTR5, \
		PERM_ATTR6, PERM_ATTR7, PERM_ATTR8, PERM_ATTR9, \
		PERM_ATTR10, PERM_ATTR11, PERM_ATTR12, PERM_ATTR13, \
		PERM_ATTR14, PERM_ATTR15) \
	{(unsigned char)PERM_ATTR0, (unsigned char)PERM_ATTR1, \
	(unsigned char)PERM_ATTR2, (unsigned char)PERM_ATTR3, \
	(unsigned char)PERM_ATTR4, (unsigned char)PERM_ATTR5, \
	(unsigned char)PERM_ATTR6, (unsigned char)PERM_ATTR7, \
	(unsigned char)PERM_ATTR8, (unsigned char)PERM_ATTR9, \
	(unsigned char)PERM_ATTR10, (unsigned char)PERM_ATTR11, \
	(unsigned char)PERM_ATTR12, (unsigned char)PERM_ATTR13, \
	(unsigned char)PERM_ATTR14, (unsigned char)PERM_ATTR15}

#define apuapc_writel(VAL, REG)		mmio_write_32((uintptr_t)REG, VAL)
#define apuapc_readl(REG)		mmio_read_32((uintptr_t)REG)

/* APUSYS APC AO  Registers */
#define APUSYS_APC_AO_BASE            APUSYS_APC_AO_WRAPPER_BASE
#define APUSYS_APC_CON                (APUSYS_APC_AO_BASE + 0x00F00)
#define APUSYS_SYS0_APC_LOCK_0        (APUSYS_APC_AO_BASE + 0x00700)

/* APUSYS NOC_DPAC_AO Registers */
#define APUSYS_NOC_DAPC_CON	      (APUSYS_NOC_DAPC_AO_BASE + 0x00F00)

#define APUSYS_NOC_DAPC_GAP_BOUNDARY    4U
#define APUSYS_NOC_DAPC_JUMP_GAP        12U

#define APUSYS_APC_SYS0_AO_SLAVE_NUM_IN_1_DOM       16U
#define APUSYS_APC_SYS0_AO_DOM_NUM                  16U
#define APUSYS_APC_SYS0_AO_SLAVE_NUM                59U

#define APUSYS_APC_SYS0_LOCK_BIT_APU_SCTRL_REVISER  11U
#define APUSYS_APC_SYS0_LOCK_BIT_APUSYS_AO_5        5U

#define APUSYS_NOC_DAPC_AO_SLAVE_NUM_IN_1_DOM       16U
#define APUSYS_NOC_DAPC_AO_DOM_NUM                  16U
#define APUSYS_NOC_DAPC_AO_SLAVE_NUM                27U

#endif /* __MTK_APUSYS_APC_DEF_H__ */
