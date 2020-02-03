/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <errno.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include "uniphier.h"

#define UNIPHIER_ROM_RSV0		0x0

#define UNIPHIER_SLFRSTSEL		0x10
#define   UNIPHIER_SLFRSTSEL_MASK		GENMASK(1, 0)
#define UNIPHIER_SLFRSTCTL		0x14
#define   UNIPHIER_SLFRSTCTL_RST		BIT(0)

#define MPIDR_AFFINITY_INVALID		((u_register_t)-1)

static uintptr_t uniphier_rom_rsv_base;
static uintptr_t uniphier_slfrst_base;

uintptr_t uniphier_sec_entrypoint;

void uniphier_warmboot_entrypoint(void);
void __dead2 uniphier_fake_pwr_down(void);
u_register_t uniphier_holding_pen_release;
static int uniphier_psci_scp_mode;

static int uniphier_psci_pwr_domain_on(u_register_t mpidr)
{
	uniphier_holding_pen_release = mpidr;
	flush_dcache_range((uint64_t)&uniphier_holding_pen_release,
			   sizeof(uniphier_holding_pen_release));

	mmio_write_64(uniphier_rom_rsv_base + UNIPHIER_ROM_RSV0,
		      (uint64_t)&uniphier_warmboot_entrypoint);
	sev();

	return PSCI_E_SUCCESS;
}

static void uniphier_psci_pwr_domain_off(const psci_power_state_t *target_state)
{
	uniphier_gic_cpuif_disable();
}

static void uniphier_psci_pwr_domain_on_finish(
					const psci_power_state_t *target_state)
{
	uniphier_gic_pcpu_init();
	uniphier_gic_cpuif_enable();

	uniphier_cci_enable();
}

static void __dead2 uniphier_psci_pwr_domain_pwr_down_wfi(
					const psci_power_state_t *target_state)
{
	/*
	 * The Boot ROM cannot distinguish warm and cold resets.
	 * Instead of the CPU reset, fake it.
	 */
	uniphier_holding_pen_release = MPIDR_AFFINITY_INVALID;
	flush_dcache_range((uint64_t)&uniphier_holding_pen_release,
			   sizeof(uniphier_holding_pen_release));

	uniphier_fake_pwr_down();
}

static void uniphier_self_system_reset(void)
{
	mmio_clrbits_32(uniphier_slfrst_base + UNIPHIER_SLFRSTSEL,
			UNIPHIER_SLFRSTSEL_MASK);
	mmio_setbits_32(uniphier_slfrst_base + UNIPHIER_SLFRSTCTL,
			UNIPHIER_SLFRSTCTL_RST);
}

static void __dead2 uniphier_psci_system_off(void)
{
	if (uniphier_psci_scp_mode) {
		uniphier_scp_system_off();
	} else {
		NOTICE("SCP is disabled; can't shutdown the system.\n");
		NOTICE("Resetting the system instead.\n");
		uniphier_self_system_reset();
	}

	wfi();
	ERROR("UniPhier System Off: operation not handled.\n");
	panic();
}

static void __dead2 uniphier_psci_system_reset(void)
{
	if (uniphier_psci_scp_mode)
		uniphier_scp_system_reset();
	else
		uniphier_self_system_reset();

	wfi();
	ERROR("UniPhier System Reset: operation not handled.\n");
	panic();
}

static const struct plat_psci_ops uniphier_psci_ops = {
	.pwr_domain_on = uniphier_psci_pwr_domain_on,
	.pwr_domain_off = uniphier_psci_pwr_domain_off,
	.pwr_domain_on_finish = uniphier_psci_pwr_domain_on_finish,
	.pwr_domain_pwr_down_wfi = uniphier_psci_pwr_domain_pwr_down_wfi,
	.system_off = uniphier_psci_system_off,
	.system_reset = uniphier_psci_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	uniphier_sec_entrypoint = sec_entrypoint;
	flush_dcache_range((uint64_t)&uniphier_sec_entrypoint,
			   sizeof(uniphier_sec_entrypoint));

	*psci_ops = &uniphier_psci_ops;

	return 0;
}

struct uniphier_psci_ctrl_base {
	uintptr_t rom_rsv_base;
	uintptr_t slfrst_base;
};

static const struct uniphier_psci_ctrl_base uniphier_psci_ctrl_base[] = {
	[UNIPHIER_SOC_LD11] = {
		.rom_rsv_base = 0x59801200,
		.slfrst_base = 0x61843000,
	},
	[UNIPHIER_SOC_LD20] = {
		.rom_rsv_base = 0x59801200,
		.slfrst_base = 0x61843000,
	},
	[UNIPHIER_SOC_PXS3] = {
		.rom_rsv_base = 0x59801200,
		.slfrst_base = 0x61843000,
	},
};

void uniphier_psci_init(unsigned int soc)
{
	assert(soc < ARRAY_SIZE(uniphier_psci_ctrl_base));
	uniphier_rom_rsv_base = uniphier_psci_ctrl_base[soc].rom_rsv_base;
	uniphier_slfrst_base = uniphier_psci_ctrl_base[soc].slfrst_base;

	if (uniphier_get_boot_master(soc) == UNIPHIER_BOOT_MASTER_SCP) {
		uniphier_psci_scp_mode = uniphier_scp_is_running();
		flush_dcache_range((uint64_t)&uniphier_psci_scp_mode,
				   sizeof(uniphier_psci_scp_mode));

		if (uniphier_psci_scp_mode)
			uniphier_scp_open_com();
	}
}
