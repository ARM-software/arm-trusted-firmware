/*
 * Copyright (c) 2013-2025, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2015-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <arch.h>
#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include "mssr.h"
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include "ptp.h"
#include "pwrc.h"
#include "timer.h"

#include "rcar_def.h"
#include "rcar_private.h"
#include "rcar_version.h"

struct entry_point_info *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	bl2_to_bl31_params_mem_t *from_bl2 =
		(bl2_to_bl31_params_mem_t *)PARAMS_BASE;
	entry_point_info_t *next_image_info = (type == NON_SECURE) ?
					      &from_bl2->bl33_ep_info :
					      &from_bl2->bl32_ep_info;

	return next_image_info->pc ? next_image_info : NULL;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	rcar_console_boot_init();

	NOTICE("BL3-1 : Rev.%s\n", version_of_renesas);
}

void bl31_plat_arch_setup(void)
{
	static const uintptr_t BL31_RO_BASE = BL_CODE_BASE;
	static const uintptr_t BL31_RO_LIMIT = BL_CODE_END;

	rcar_configure_mmu_el3(BL31_BASE,
			       BL31_LIMIT - BL31_BASE,
			       BL31_RO_BASE, BL31_RO_LIMIT);

	rcar_pwrc_code_copy_to_system_ram();
}

static const uintptr_t gicr_base_addrs[2] = {
	PLAT_ARM_GICR_BASE,	/* GICR Base address of the primary CPU */
	0U			/* Zero Termination */
};

void bl31_platform_setup(void)
{
	/* Initialize generic timer */
	u_register_t reg_cntfid = RCAR_CNTC_EXTAL;

	rcar_mssr_setup();

	/* Update memory mapped and register based frequency */
	write_cntfrq_el0(reg_cntfid);
	mmio_write_32(ARM_SYS_CNTCTL_BASE + CNTFID_OFF, reg_cntfid);

	/* Enable the system level generic timer */
	mmio_write_32(RCAR_CNTC_BASE + CNTCR_OFF, CNTCR_FCREQ(0) | CNTCR_EN);

	gic_set_gicr_frames(gicr_base_addrs);

	rcar_pwrc_setup();
	rcar_ptp_setup();
}

const spd_pm_ops_t rcar_pm = {
	.svc_migrate_info = rcar_pwrc_cpu_migrate_info,
};

void bl31_plat_runtime_setup(void)
{
	psci_register_spd_pm_hook(&rcar_pm);

	rcar_console_runtime_init();
	console_switch_state(CONSOLE_FLAG_RUNTIME);
}
