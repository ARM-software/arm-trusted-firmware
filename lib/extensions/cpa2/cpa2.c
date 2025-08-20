/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>

/*
 * Check that the platform virtual address is less than or equal to 54 bits, if
 * not then CPA will corrupt the addresses.
 */
#if PLAT_VIRT_ADDR_SPACE_SIZE >= (1 << 54)
#error "FEAT_CPA2 can only be enabled with VA <= 54 bits! See Arm ARM rule IQDPCR."
#endif

void cpa2_enable_el3(void)
{
	write_sctlr2_el3(read_sctlr2_el3() | SCTLR2_CPTA_BIT |
			 SCTLR2_CPTM_BIT);
}
