/*
 * Copyright (c) 2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <common/debug.h>
#include <lib/mmio.h>

#include <plat_ocm_coherency.h>
#include <platform_def.h>

/*
 * Register non hash mem regions addresses
 */
#define POR_RNSAM_NODE_INFO_U_RNFBESAM_NID12    U(0xF8168000)
#define NON_HASH_MEM_REGION_REG0        U(POR_RNSAM_NODE_INFO_U_RNFBESAM_NID12 + 0xC08)
#define NON_HASH_MEM_REGION_REG1        U(POR_RNSAM_NODE_INFO_U_RNFBESAM_NID12 + 0xC0C)
#define NON_HASH_MEM_REGION_REG2        U(POR_RNSAM_NODE_INFO_U_RNFBESAM_NID12 + 0xC10)
#define NON_HASH_MEM_REGION_REG3        U(POR_RNSAM_NODE_INFO_U_RNFBESAM_NID12 + 0xC14)
#define NON_HASH_MEM_REGION_REG4        U(POR_RNSAM_NODE_INFO_U_RNFBESAM_NID12 + 0xC18)
#define NON_HASH_MEM_REGION_REG5        U(POR_RNSAM_NODE_INFO_U_RNFBESAM_NID12 + 0xC1C)
#define NON_HASH_MEM_REGION_REG6        U(POR_RNSAM_NODE_INFO_U_RNFBESAM_NID12 + 0xC20)
#define NON_HASH_MEM_REGION_REG7        U(POR_RNSAM_NODE_INFO_U_RNFBESAM_NID12 + 0xC24)

#define REGION_BASE_ADDR_VALUE	U(0x2E)
#define REGION_BASE_ADDR_SHIFT	9

#define REGION_BASE_ADDRESS_MASK	GENMASK(30, REGION_BASE_ADDR_SHIFT)
#define REGION_VALID_BIT		BIT(0)

/*
 * verify the register configured as non-hashed
 */
#define IS_NON_HASHED_REGION(reg) \
((FIELD_GET(REGION_BASE_ADDRESS_MASK, mmio_read_32(reg)) == REGION_BASE_ADDR_VALUE) && \
						(mmio_read_32(reg) & REGION_VALID_BIT))

/*
 * Splitter registers
 */
#define FPX_SPLITTER_0          U(0xECC20000)
#define FPX_SPLITTER_1          U(0xECD20000)
#define FPX_SPLITTER_2          U(0xECE20000)
#define FPX_SPLITTER_3          U(0xECF20000)
#define OCM_ADDR_DIST_MODE      BIT(16)

#define OCM_COHERENT	0
#define OCM_NOT_COHERENT	1
#define TFA_NOT_IN_OCM	2

/*
 * Function that verifies the OCM is coherent or not with the following checks:
 * verify that OCM is in non hashed region or not if not then verify
 * OCM_ADDR_DIST_MODE bit in splitter registers is set.
 */
int32_t check_ocm_coherency(void)
{
	int32_t status = OCM_COHERENT;
	/* isolation should be disabled in order to read these registers */
	if ((IS_NON_HASHED_REGION(NON_HASH_MEM_REGION_REG0) ||
	     IS_NON_HASHED_REGION(NON_HASH_MEM_REGION_REG1) ||
	     IS_NON_HASHED_REGION(NON_HASH_MEM_REGION_REG2) ||
	     IS_NON_HASHED_REGION(NON_HASH_MEM_REGION_REG3) ||
	     IS_NON_HASHED_REGION(NON_HASH_MEM_REGION_REG4) ||
	     IS_NON_HASHED_REGION(NON_HASH_MEM_REGION_REG5) ||
	     IS_NON_HASHED_REGION(NON_HASH_MEM_REGION_REG6) ||
	     IS_NON_HASHED_REGION(NON_HASH_MEM_REGION_REG7))) {
		WARN("OCM is not configured as coherent\n");
		status = OCM_NOT_COHERENT;
	} else {
		/* verify OCM_ADDR_DIST_MODE bit in splitter registers is set */
		if (!((mmio_read_32(FPX_SPLITTER_0) & OCM_ADDR_DIST_MODE) &&
		      (mmio_read_32(FPX_SPLITTER_1) & OCM_ADDR_DIST_MODE) &&
		      (mmio_read_32(FPX_SPLITTER_2) & OCM_ADDR_DIST_MODE) &&
		      (mmio_read_32(FPX_SPLITTER_3) & OCM_ADDR_DIST_MODE))) {
			WARN("OCM is not configured as coherent\n");
			status = OCM_NOT_COHERENT;
		}
	}
	return status;
}

