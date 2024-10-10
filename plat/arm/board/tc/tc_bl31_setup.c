/*
 * Copyright (c) 2020-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>
#include <tc_plat.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/arm/dsu.h>
#include <drivers/arm/sbsa.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#ifdef PLATFORM_TEST_TFM_TESTSUITE
#include <psa/crypto_platform.h>
#include <psa/crypto_types.h>
#include <psa/crypto_values.h>
#endif /* PLATFORM_TEST_TFM_TESTSUITE */
#include <psa/error.h>

#include <plat/common/platform.h>
#include <tc_rse_comms.h>

#ifdef PLATFORM_TEST_TFM_TESTSUITE
/*
 * We pretend using an external RNG (through MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG
 * mbedTLS config option) so we need to provide an implementation of
 * mbedtls_psa_external_get_random(). Provide a fake one, since we do not
 * actually use any of external RNG and this function is only needed during
 * the execution of TF-M testsuite during exporting the public part of the
 * delegated attestation key.
 */
psa_status_t mbedtls_psa_external_get_random(
			mbedtls_psa_external_random_context_t *context,
			uint8_t *output, size_t output_size,
			size_t *output_length)
{
	for (size_t i = 0U; i < output_size; i++) {
		output[i] = (uint8_t)(read_cntpct_el0() & 0xFFU);
	}

	*output_length = output_size;

	return PSA_SUCCESS;
}
#endif /* PLATFORM_TEST_TFM_TESTSUITE */

static scmi_channel_plat_info_t tc_scmi_plat_info = {
	.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
	.db_reg_addr = PLAT_CSS_MHU_BASE + MHU_V3_SENDER_REG_SET(0),
	.db_preserve_mask = 0xfffffffe,
	.db_modify_mask = 0x1,
	.ring_doorbell = &mhu_ring_doorbell,
};

/* the bottom 3 AMU group 1 counters */
#define MPMM_GEARS ((1 << 0) | (1 << 1) | (1 << 2))

uint16_t plat_amu_aux_enables[PLATFORM_CORE_COUNT] = {
	MPMM_GEARS, MPMM_GEARS, MPMM_GEARS, MPMM_GEARS,
	MPMM_GEARS, MPMM_GEARS, MPMM_GEARS, MPMM_GEARS,
#if PLATFORM_CORE_COUNT == 14
	MPMM_GEARS, MPMM_GEARS, MPMM_GEARS, MPMM_GEARS,
	MPMM_GEARS, MPMM_GEARS
#endif
};

const dsu_driver_data_t plat_dsu_data = {
	.clusterpwrdwn_pwrdn = false,
	.clusterpwrdwn_memret = false,
	.clusterpwrctlr_cachepwr = CLUSTERPWRCTLR_CACHEPWR_RESET,
	.clusterpwrctlr_funcret = CLUSTERPWRCTLR_FUNCRET_RESET
};

static void enable_ns_mcn_pmu(void)
{
	/*
	 * Enable non-secure access to MCN PMU registers
	 */
	for (int i = 0; i < MCN_INSTANCES; i++) {
		uintptr_t mcn_scr = MCN_MICROARCH_BASE_ADDR(i) +
			MCN_SCR_OFFSET;
		mmio_setbits_32(mcn_scr, 1 << MCN_SCR_PMU_BIT);
	}
}

#if TARGET_PLATFORM == 3
static void set_mcn_slc_alloc_mode(void)
{
	/*
	 * SLC WRALLOCMODE and RDALLOCMODE are configured by default to
	 * 0b01 (always alloc), configure both to 0b10 (use bus signal
	 * attribute from interface).
	 */
	for (int i = 0; i < MCN_INSTANCES; i++) {
		uintptr_t slccfg_ctl_ns = MCN_MPAM_NS_BASE_ADDR(i) +
			MPAM_SLCCFG_CTL_OFFSET;
		uintptr_t slccfg_ctl_s = MCN_MPAM_S_BASE_ADDR(i) +
			MPAM_SLCCFG_CTL_OFFSET;

		mmio_clrsetbits_32(slccfg_ctl_ns,
				   (SLC_RDALLOCMODE_MASK | SLC_WRALLOCMODE_MASK),
				   (SLC_ALLOC_BUS_SIGNAL_ATTR << SLC_RDALLOCMODE_SHIFT) |
				   (SLC_ALLOC_BUS_SIGNAL_ATTR << SLC_WRALLOCMODE_SHIFT));
		mmio_clrsetbits_32(slccfg_ctl_s,
				   (SLC_RDALLOCMODE_MASK | SLC_WRALLOCMODE_MASK),
				   (SLC_ALLOC_BUS_SIGNAL_ATTR << SLC_RDALLOCMODE_SHIFT) |
				   (SLC_ALLOC_BUS_SIGNAL_ATTR << SLC_WRALLOCMODE_SHIFT));
	}
}
#endif

#if defined(TARGET_FLAVOUR_FPGA) && TARGET_PLATFORM == 4
/*
 * Configure MTU tag registers to initialize the MTE carveout.
 * This isn't required for FVP builds, as FVPs do not emulate
 * MTE in such a way that it requires a physical careveout.
 */
static void set_mcn_mtu_tag_addr(void)
{
	for (int i = 0; i < MCN_INSTANCES; i++) {
		uintptr_t mtu_tag_addr_base_lo = MCN_MTU_BASE_ADDR(i) +
			MTU_TAG_ADDR_BASE_OFFSET;
		uintptr_t mtu_tag_addr_base_hi = MCN_MTU_BASE_ADDR(i) +
			MTU_TAG_ADDR_BASE_OFFSET + 4;

		/* Enter MCN config state. */
		mmio_write_32(MCN_CRP_BASE_ADDR(i) +
			      MCN_CRP_ARCH_STATE_REQ_OFFSET, MCN_CONFIG_STATE);
		while (mmio_read_32(MCN_CRP_BASE_ADDR(i) +
				    MCN_CRP_ARCH_STATE_CUR_OFFSET) != MCN_CONFIG_STATE)
			;

		dsb();
		isb();

		mmio_write_32(mtu_tag_addr_base_lo,
			      (uint32_t)(TC_MTU_TAG_ADDR_BASE & 0xFFFFFFFF));
		mmio_write_32(mtu_tag_addr_base_hi,
			      (uint32_t)((TC_MTU_TAG_ADDR_BASE >> 32) & 0xFFFFFFFF));

		dsb();
		isb();

		/* Return to MCN run state. */
		mmio_write_32(MCN_CRP_BASE_ADDR(i) +
			      MCN_CRP_ARCH_STATE_REQ_OFFSET, MCN_RUN_STATE);
		while (mmio_read_32(MCN_CRP_BASE_ADDR(i) +
				    MCN_CRP_ARCH_STATE_CUR_OFFSET) != MCN_RUN_STATE)
			;
	}
}
#endif

void bl31_platform_setup(void)
{
	psa_status_t status;

	tc_bl31_common_platform_setup();
	enable_ns_mcn_pmu();
#if TARGET_PLATFORM == 3
	set_mcn_slc_alloc_mode();
	plat_arm_ni_setup(NCI_BASE_ADDR);
#endif

	/* Initialise RSE communication channel */
	status = plat_rse_comms_init();
	if (status != PSA_SUCCESS) {
		ERROR("Failed to initialize RSE communication channel - psa_status = %d\n", status);
	}
#if defined(TARGET_FLAVOUR_FPGA) && TARGET_PLATFORM == 4
	set_mcn_mtu_tag_addr();
#endif
}

scmi_channel_plat_info_t *plat_css_get_scmi_info(unsigned int channel_id __unused)
{

	return &tc_scmi_plat_info;

}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

	arm_bl31_early_platform_setup(arg0, arg1, arg2, arg3);

#if !TRANSFER_LIST
	/* Fill the properties struct with the info from the config dtb */
	fconf_populate("FW_CONFIG", arg1);
#endif
}

#ifdef PLATFORM_TESTS
static __dead2 void tc_run_platform_tests(void)
{
	int tests_failed;

	printf("\nStarting platform tests...\n");

#ifdef PLATFORM_TEST_NV_COUNTERS
	tests_failed = nv_counter_test();
#elif PLATFORM_TEST_TFM_TESTSUITE
	tests_failed = run_platform_tests();
#endif

	printf("Platform tests %s.\n",
	       (tests_failed != 0) ? "failed" : "succeeded");

	/* Suspend booting, no matter the tests outcome. */
	printf("Suspend booting...\n");
	plat_error_handler(-1);
}
#endif

void tc_bl31_common_platform_setup(void)
{
	arm_bl31_platform_setup();

	gic_set_gicr_frames(arm_gicr_base_addrs);

#ifdef PLATFORM_TESTS
	tc_run_platform_tests();
#endif
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	return css_scmi_override_pm_ops(ops);
}

void __init bl31_plat_arch_setup(void)
{
	arm_bl31_plat_arch_setup();

	/*
	 * When TRANSFER_LIST is enabled, HW_CONFIG is included in Transfer List
	 * as an entry with the tag TL_TAG_FDT. In this case, the configuration
	 * is already available, so the fconf_populate mechanism is not needed.
	 * The code block below is only required when TRANSFER_LIST is not used.
	 */
#if !TRANSFER_LIST
	/* HW_CONFIG was also loaded by BL2 */
	const struct dyn_cfg_dtb_info_t *hw_config_info;

	hw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, HW_CONFIG_ID);
	assert(hw_config_info != NULL);

	fconf_populate("HW_CONFIG", hw_config_info->config_addr);
#endif
}

#if defined(SPD_spmd) && (SPMC_AT_EL3 == 0)
void tc_bl31_plat_runtime_setup(void)
{
	/* Start secure watchdog timer. */
	plat_arm_secure_wdt_start();

	arm_bl31_plat_runtime_setup();
}

void bl31_plat_runtime_setup(void)
{
	tc_bl31_plat_runtime_setup();
}

/*
 * Platform handler for Group0 secure interrupt.
 */
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	/* Trusted Watchdog timer is the only source of Group0 interrupt now. */
	if (intid == SBSA_SECURE_WDOG_INTID) {
		/* Refresh the timer. */
		plat_arm_secure_wdt_refresh();

		return 0;
	}

	return -1;
}
#endif /*defined(SPD_spmd) && (SPMC_AT_EL3 == 0)*/
