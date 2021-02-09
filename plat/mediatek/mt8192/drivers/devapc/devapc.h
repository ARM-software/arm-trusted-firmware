/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEVAPC_H
#define DEVAPC_H

#include <stdint.h>
#include <platform_def.h>

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/
void devapc_init(void);

/******************************************************************************
 * STRUCTURE DEFINITION
 ******************************************************************************/
enum DEVAPC_PERM_TYPE {
	NO_PROTECTION = 0,
	SEC_RW_ONLY,
	SEC_RW_NS_R,
	FORBIDDEN,
	PERM_NUM,
};

enum DOMAIN_ID {
	DOMAIN_0 = 0,
	DOMAIN_1,
	DOMAIN_2,
	DOMAIN_3,
	DOMAIN_4,
	DOMAIN_5,
	DOMAIN_6,
	DOMAIN_7,
	DOMAIN_8,
	DOMAIN_9,
	DOMAIN_10,
	DOMAIN_11,
	DOMAIN_12,
	DOMAIN_13,
	DOMAIN_14,
	DOMAIN_15,
};

/* Slave Type */
enum DEVAPC_SLAVE_TYPE_SIMPLE {
	SLAVE_TYPE_INFRA = 0,
	SLAVE_TYPE_PERI,
	SLAVE_TYPE_PERI2,
	SLAVE_TYPE_PERI_PAR,
};

enum DEVAPC_SYS_INDEX {
	DEVAPC_SYS0 = 0,
	DEVAPC_SYS1,
	DEVAPC_SYS2,
};

enum DEVAPC_SLAVE_TYPE {
	SLAVE_TYPE_INFRA_AO_SYS0 = 0,
	SLAVE_TYPE_INFRA_AO_SYS1,
	SLAVE_TYPE_INFRA_AO_SYS2,
	SLAVE_TYPE_PERI_AO_SYS0,
	SLAVE_TYPE_PERI_AO_SYS1,
	SLAVE_TYPE_PERI_AO_SYS2,
	SLAVE_TYPE_PERI_AO2_SYS0,
	SLAVE_TYPE_PERI_PAR_AO_SYS0,
};

/* Slave Num */
enum DEVAPC_SLAVE_NUM {
	SLAVE_NUM_INFRA_AO_SYS0 = 23,
	SLAVE_NUM_INFRA_AO_SYS1 = 256,
	SLAVE_NUM_INFRA_AO_SYS2 = 70,
	SLAVE_NUM_PERI_AO_SYS0 = 105,
	SLAVE_NUM_PERI_AO_SYS1 = 66,
	SLAVE_NUM_PERI_AO_SYS2 = 1,
	SLAVE_NUM_PERI_AO2_SYS0 = 115,
	SLAVE_NUM_PERI_PAR_AO_SYS0 = 27,
};

enum DEVAPC_SYS_DOM_NUM {
	DOM_NUM_INFRA_AO_SYS0 = 16,
	DOM_NUM_INFRA_AO_SYS1 = 4,
	DOM_NUM_INFRA_AO_SYS2 = 4,
	DOM_NUM_PERI_AO_SYS0 = 16,
	DOM_NUM_PERI_AO_SYS1 = 8,
	DOM_NUM_PERI_AO_SYS2 = 4,
	DOM_NUM_PERI_AO2_SYS0 = 16,
	DOM_NUM_PERI_PAR_AO_SYS0 = 16,
};

enum DEVAPC_CFG_INDEX {
	DEVAPC_DEBUGSYS_INDEX = 57,
};

struct APC_INFRA_PERI_DOM_16 {
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

struct APC_INFRA_PERI_DOM_8 {
	unsigned char d0_permission;
	unsigned char d1_permission;
	unsigned char d2_permission;
	unsigned char d3_permission;
	unsigned char d4_permission;
	unsigned char d5_permission;
	unsigned char d6_permission;
	unsigned char d7_permission;
};

struct APC_INFRA_PERI_DOM_4 {
	unsigned char d0_permission;
	unsigned char d1_permission;
	unsigned char d2_permission;
	unsigned char d3_permission;
};

#define DAPC_INFRA_AO_SYS0_ATTR(DEV_NAME, PERM_ATTR0, PERM_ATTR1, \
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

#define DAPC_INFRA_AO_SYS1_ATTR(DEV_NAME, PERM_ATTR0, PERM_ATTR1, \
		PERM_ATTR2, PERM_ATTR3) \
	{(unsigned char)PERM_ATTR0, (unsigned char)PERM_ATTR1, \
	(unsigned char)PERM_ATTR2, (unsigned char)PERM_ATTR3}

#define DAPC_PERI_AO_SYS1_ATTR(DEV_NAME, PERM_ATTR0, PERM_ATTR1, \
		PERM_ATTR2, PERM_ATTR3, PERM_ATTR4, PERM_ATTR5, \
		PERM_ATTR6, PERM_ATTR7) \
	{(unsigned char)PERM_ATTR0, (unsigned char)PERM_ATTR1, \
	(unsigned char)PERM_ATTR2, (unsigned char)PERM_ATTR3, \
	(unsigned char)PERM_ATTR4, (unsigned char)PERM_ATTR5, \
	(unsigned char)PERM_ATTR6, (unsigned char)PERM_ATTR7}

#define DAPC_INFRA_AO_SYS2_ATTR(...)	DAPC_INFRA_AO_SYS1_ATTR(__VA_ARGS__)
#define DAPC_PERI_AO_SYS0_ATTR(...)	DAPC_INFRA_AO_SYS0_ATTR(__VA_ARGS__)
#define DAPC_PERI_AO_SYS2_ATTR(...)	DAPC_INFRA_AO_SYS1_ATTR(__VA_ARGS__)
#define DAPC_PERI_AO2_SYS0_ATTR(...)	DAPC_INFRA_AO_SYS0_ATTR(__VA_ARGS__)
#define DAPC_PERI_PAR_AO_SYS0_ATTR(...)	DAPC_INFRA_AO_SYS0_ATTR(__VA_ARGS__)

/******************************************************************************
 * UTILITY DEFINITION
 ******************************************************************************/
#define devapc_writel(VAL, REG)		mmio_write_32((uintptr_t)REG, VAL)
#define devapc_readl(REG)		mmio_read_32((uintptr_t)REG)

/******************************************************************************/
/* Device APC AO for INFRA AO */
#define DEVAPC_INFRA_AO_SYS0_D0_APC_0		(DEVAPC_INFRA_AO_BASE + 0x0000)
#define DEVAPC_INFRA_AO_SYS1_D0_APC_0		(DEVAPC_INFRA_AO_BASE + 0x1000)
#define DEVAPC_INFRA_AO_SYS2_D0_APC_0		(DEVAPC_INFRA_AO_BASE + 0x2000)

#define DEVAPC_INFRA_AO_MAS_SEC_0		(DEVAPC_INFRA_AO_BASE + 0x0A00)

/******************************************************************************/
/* Device APC AO for PERI AO */
#define DEVAPC_PERI_AO_SYS0_D0_APC_0		(DEVAPC_PERI_AO_BASE + 0x0000)
#define DEVAPC_PERI_AO_SYS1_D0_APC_0		(DEVAPC_PERI_AO_BASE + 0x1000)
#define DEVAPC_PERI_AO_SYS2_D0_APC_0		(DEVAPC_PERI_AO_BASE + 0x2000)

#define DEVAPC_PERI_AO_MAS_SEC_0		(DEVAPC_PERI_AO_BASE + 0x0A00)

/******************************************************************************/
/* Device APC AO for PERI AO2 */
#define DEVAPC_PERI_AO2_SYS0_D0_APC_0		(DEVAPC_PERI_AO2_BASE + 0x0000)

/******************************************************************************/
/* Device APC AO for PERI PAR AO */
#define DEVAPC_PERI_PAR_AO_SYS0_D0_APC_0	(DEVAPC_PERI_PAR_AO_BASE + 0x0000)

#define DEVAPC_PERI_PAR_AO_MAS_SEC_0		(DEVAPC_PERI_PAR_AO_BASE + 0x0A00)

/******************************************************************************/


/******************************************************************************
 * Variable DEFINITION
 ******************************************************************************/
#define MOD_NO_IN_1_DEVAPC              16

#endif /* DEVAPC_H */

