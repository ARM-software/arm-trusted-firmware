/*
 * Copyright (c) 2023-2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_DAPC_V1_H
#define APUSYS_DAPC_V1_H

#include <lib/mmio.h>

/******************************************************************************
 * STRUCTURE DEFINITION
 ******************************************************************************/
enum apusys_apc_err_status {
	APUSYS_APC_OK				= 0x0,
	APUSYS_APC_ERR_GENERIC			= 0x1000,
	APUSYS_APC_ERR_INVALID_CMD		= 0x1001,
	APUSYS_APC_ERR_SLAVE_TYPE_NOT_SUPPORTED	= 0x1002,
	APUSYS_APC_ERR_SLAVE_IDX_NOT_SUPPORTED	= 0x1003,
	APUSYS_APC_ERR_DOMAIN_NOT_SUPPORTED	= 0x1004,
	APUSYS_APC_ERR_PERMISSION_NOT_SUPPORTED	= 0x1005,
	APUSYS_APC_ERR_OUT_OF_BOUNDARY		= 0x1006,
	APUSYS_APC_ERR_REQ_TYPE_NOT_SUPPORTED	= 0x1007,
};

enum apusys_apc_perm_type {
	NO_PROTECTION	= 0,
	SEC_RW_ONLY	= 1,
	SEC_RW_NS_R	= 2,
	FORBIDDEN	= 3,
	PERM_NUM	= 4,
};

enum apusys_apc_domain_id {
	DOMAIN_0	=  0,
	DOMAIN_1	=  1,
	DOMAIN_2	=  2,
	DOMAIN_3	=  3,
	DOMAIN_4	=  4,
	DOMAIN_5	=  5,
	DOMAIN_6	=  6,
	DOMAIN_7	=  7,
	DOMAIN_8	=  8,
	DOMAIN_9	=  9,
	DOMAIN_10	= 10,
	DOMAIN_11	= 11,
	DOMAIN_12	= 12,
	DOMAIN_13	= 13,
	DOMAIN_14	= 14,
	DOMAIN_15	= 15,
};

struct apc_dom_16 {
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

#define APUSYS_APC_AO_ATTR(DEV_NAME,					       \
			   PERM_ATTR0,  PERM_ATTR1,  PERM_ATTR2,  PERM_ATTR3,  \
			   PERM_ATTR4,  PERM_ATTR5,  PERM_ATTR6,  PERM_ATTR7,  \
			   PERM_ATTR8,  PERM_ATTR9,  PERM_ATTR10, PERM_ATTR11, \
			   PERM_ATTR12, PERM_ATTR13, PERM_ATTR14, PERM_ATTR15) \
	{(unsigned char)PERM_ATTR0,  (unsigned char)PERM_ATTR1,  \
	 (unsigned char)PERM_ATTR2,  (unsigned char)PERM_ATTR3,  \
	 (unsigned char)PERM_ATTR4,  (unsigned char)PERM_ATTR5,  \
	 (unsigned char)PERM_ATTR6,  (unsigned char)PERM_ATTR7,  \
	 (unsigned char)PERM_ATTR8,  (unsigned char)PERM_ATTR9,  \
	 (unsigned char)PERM_ATTR10, (unsigned char)PERM_ATTR11, \
	 (unsigned char)PERM_ATTR12, (unsigned char)PERM_ATTR13, \
	 (unsigned char)PERM_ATTR14, (unsigned char)PERM_ATTR15}

typedef enum apusys_apc_err_status (*dapc_cfg_func)(uint32_t slave,
						    enum apusys_apc_domain_id domain_id,
						    enum apusys_apc_perm_type perm);

/* Register */
#define DEVAPC_DOM_SIZE			(0x40)
#define DEVAPC_REG_SIZE			(4)

/* APUSYS APC offsets */
#define APUSYS_DAPC_CON_VIO_MASK	(0x80000000)
#define APUSYS_DAPC_CON(base)		((base) + 0x00f00)

/******************************************************************************
 * DAPC Common Function
 ******************************************************************************/
#define SET_APUSYS_DAPC_V1(dapc, cfg) \
	set_apusys_dapc_v1(dapc, ARRAY_SIZE(dapc), cfg)

#define DUMP_APUSYS_DAPC_V1(apc) \
	dump_apusys_dapc_v1(#apc, apc##_BASE, \
			    (apc##_SLAVE_NUM / apc##_SLAVE_NUM_IN_1_DOM), apc##_DOM_NUM)

enum apusys_apc_err_status set_apusys_dapc_v1(const struct apc_dom_16 *dapc,
					      uint32_t size, dapc_cfg_func cfg);

void dump_apusys_dapc_v1(const char *name, uintptr_t base, uint32_t reg_num, uint32_t dom_num);

/******************************************************************************
 * DAPC Permission Policy
 ******************************************************************************/
#define SLAVE_FORBID_EXCEPT_D0_SEC_RW(domain)				 \
	APUSYS_APC_AO_ATTR(domain,					 \
			   SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN, FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_SEC_RW_D5_NO_PROTECT(domain)		     \
	APUSYS_APC_AO_ATTR(domain,					     \
			   SEC_RW_ONLY, FORBIDDEN,     FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   NO_PROTECTION, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,     FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,     FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D5_NO_PROTECT(domain)			   \
	APUSYS_APC_AO_ATTR(domain,					   \
			   FORBIDDEN, FORBIDDEN,     FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN, NO_PROTECTION, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN, FORBIDDEN,     FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN, FORBIDDEN,     FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_SEC_RW_NS_R_D5_NO_PROTECT(domain)	     \
	APUSYS_APC_AO_ATTR(domain,					     \
			   SEC_RW_NS_R, FORBIDDEN,     FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   NO_PROTECTION, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,     FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,     FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D7_NO_PROTECT(domain)			   \
	APUSYS_APC_AO_ATTR(domain,					   \
			   FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,     \
			   FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION, \
			   FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,     \
			   FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D5_D7_NO_PROTECT(domain)			       \
	APUSYS_APC_AO_ATTR(domain,					       \
			   FORBIDDEN, FORBIDDEN,     FORBIDDEN, FORBIDDEN,     \
			   FORBIDDEN, NO_PROTECTION, FORBIDDEN, NO_PROTECTION, \
			   FORBIDDEN, FORBIDDEN,     FORBIDDEN, FORBIDDEN,     \
			   FORBIDDEN, FORBIDDEN,     FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_D5_NO_PROTECT(domain)			       \
	APUSYS_APC_AO_ATTR(domain,					       \
			   NO_PROTECTION, FORBIDDEN,     FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     NO_PROTECTION, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,     FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,     FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_D11_NO_PROTECT_D3_D5_D8_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   NO_PROTECTION, FORBIDDEN,   FORBIDDEN, SEC_RW_ONLY, \
			   FORBIDDEN,     SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   SEC_RW_ONLY,   FORBIDDEN,   FORBIDDEN, NO_PROTECTION, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_SEC_RW_NS_R_D3_D5_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   SEC_RW_NS_R, FORBIDDEN,   FORBIDDEN, SEC_RW_ONLY, \
			   FORBIDDEN,   SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D5_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   FORBIDDEN, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN, SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN, FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_SEC_RW_NS_R_D5_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   SEC_RW_NS_R, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_NO_PROTECT_D3_D5_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   NO_PROTECTION, FORBIDDEN,   FORBIDDEN, SEC_RW_ONLY, \
			   FORBIDDEN,     SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D3_D5_SEC_RW_D0_D4_D11_NO_PROTECT(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   NO_PROTECTION, FORBIDDEN,   FORBIDDEN, SEC_RW_ONLY, \
			   NO_PROTECTION, SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, NO_PROTECTION, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D5_SEC_RW_D0_NO_PROTECT(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   NO_PROTECTION, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_D5_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   SEC_RW_ONLY, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_NO_PROTECT_D5_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   NO_PROTECTION, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D3_D5_SEC_RW_D0_D11_NO_PROTECT(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   NO_PROTECTION, FORBIDDEN,   FORBIDDEN, SEC_RW_ONLY, \
			   FORBIDDEN,     SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, NO_PROTECTION, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_NO_PROTECT_D5_D8_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   NO_PROTECTION, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   SEC_RW_ONLY,   FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_D3_D5_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   SEC_RW_ONLY, FORBIDDEN,   FORBIDDEN, SEC_RW_ONLY, \
			   FORBIDDEN,   SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_SEC_RW_NS_R_D5_D8_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   SEC_RW_NS_R, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   SEC_RW_ONLY, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D5_D8_SEC_RW(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   SEC_RW_ONLY, FORBIDDEN, FORBIDDEN, \
			   SEC_RW_ONLY, FORBIDDEN,   FORBIDDEN, FORBIDDEN, \
			   FORBIDDEN,   FORBIDDEN,   FORBIDDEN, FORBIDDEN)

#define SLAVE_FORBID_EXCEPT_D0_D5_D7_D14_NO_PROTECT(domain) \
	APUSYS_APC_AO_ATTR(domain, \
			   NO_PROTECTION, FORBIDDEN,     FORBIDDEN,     FORBIDDEN, \
			   FORBIDDEN,     NO_PROTECTION, FORBIDDEN,     NO_PROTECTION, \
			   FORBIDDEN,     FORBIDDEN,     FORBIDDEN,     FORBIDDEN, \
			   FORBIDDEN,     FORBIDDEN,     NO_PROTECTION, FORBIDDEN)

#endif /* APUSYS_DAPC_V1_H */
