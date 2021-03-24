/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CSU_H
#define CSU_H

#define CSU_SEC_ACCESS_REG_OFFSET	(0x0021CU)

/* Macros defining access permissions to configure
 * the regions controlled by Central Security Unit.
 */
enum csu_cslx_access {
	CSU_NS_SUP_R = (0x8U),
	CSU_NS_SUP_W = (0x80U),
	CSU_NS_SUP_RW = (0x88U),
	CSU_NS_USER_R = (0x4U),
	CSU_NS_USER_W = (0x40U),
	CSU_NS_USER_RW = (0x44U),
	CSU_S_SUP_R = (0x2U),
	CSU_S_SUP_W = (0x20U),
	CSU_S_SUP_RW = (0x22U),
	CSU_S_USER_R = (0x1U),
	CSU_S_USER_W = (0x10U),
	CSU_S_USER_RW = (0x11U),
	CSU_ALL_RW = (0xffU),
};

struct csu_ns_dev_st {
	uintptr_t ind;
	uint32_t val;
};

void enable_layerscape_ns_access(struct csu_ns_dev_st *csu_ns_dev,
				 uint32_t num, uintptr_t nxp_csu_addr);

#endif
