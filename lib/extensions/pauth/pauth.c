/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/extensions/pauth.h>

extern uint64_t bl1_apiakey[2];

void __no_pauth pauth_init_enable_el3(void)
{
	pauth_init();
	pauth_enable_el3();
}

void __no_pauth pauth_init_enable_el1(void)
{
	pauth_init();
	pauth_enable_el1();
}

void pauth_init(void)
{
	uint128_t keys = plat_init_apkey();
	uint64_t key_lo = LO_64(keys);
	uint64_t key_hi = HI_64(keys);

	/* Program instruction key A used by the Trusted Firmware */
	write_apiakeylo_el1(key_lo);
	write_apiakeyhi_el1(key_hi);

#if IMAGE_BL31
	/*
	 * In the warmboot entrypoint, cpu_data may have been used before data
	 * caching was enabled.  Flush the caches so nothing stale is
	 * overwritten.
	 */
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	flush_cpu_data(apiakey);
#endif
	set_cpu_data(apiakey[0], key_lo);
	set_cpu_data(apiakey[1], key_hi);

#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	flush_cpu_data(apiakey);
#endif
#elif IMAGE_BL1
	bl1_apiakey[0] = key_lo;
	bl1_apiakey[1] = key_hi;
#endif
}

/*
 * Begin checking function calls at the current EL. This function must not have
 * PAuth guards because the signing will be a NOP and attempting to authenticate
 * will fail. Includes an ISB to avoid accidental failures.
 */
void __no_pauth pauth_enable_el3(void)
{
	write_sctlr_el3(read_sctlr_el3() | SCTLR_EnIA_BIT);

	if (is_feat_pauth_lr_supported()) {
		write_sctlr2_el3(read_sctlr2_el3() | SCTLR2_EnPACM_BIT);
	}

	isb();
}

void __no_pauth pauth_enable_el1(void)
{
	write_sctlr_el1(read_sctlr_el1() | SCTLR_EnIA_BIT);

	if (is_feat_pauth_lr_supported()) {
		write_sctlr2_el1(read_sctlr2_el1() | SCTLR2_EnPACM_BIT);
	}

	isb();
}

void pauth_enable_el2(void)
{
	u_register_t hcr_el2 = read_hcr_el2();
	/*
	 * For Armv8.3 pointer authentication feature, disable traps to EL2 when
	 * accessing key registers or using pointer authentication instructions
	 * from lower ELs.
	 */
	hcr_el2 |= (HCR_API_BIT | HCR_APK_BIT);

	write_hcr_el2(hcr_el2);
}

void __no_pauth pauth_disable_el1(void)
{
	write_sctlr_el1(read_sctlr_el1() & ~SCTLR_EnIA_BIT);
	isb(); /* usually called by caller, here it's for compatibility */
}

void __no_pauth pauth_disable_el3(void)
{
	write_sctlr_el3(read_sctlr_el3() & ~SCTLR_EnIA_BIT);
	isb(); /* usually called by caller, here it's for compatibility */
}
