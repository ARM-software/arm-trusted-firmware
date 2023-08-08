/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/fvp/fvp_pwrc.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

#define FVP_PWRC_ID_MASK U(0x00FFFFFF)

/*
 * TODO: Someday there will be a generic power controller api. At the moment
 * each platform has its own pwrc so just exporting functions is fine.
 */
ARM_INSTANTIATE_LOCK;

/*
 * Core ID field is 24 bits wide and extracted from MPIDR.
 * Bits[23:16] represent Affinity Level 2
 * Bits[15:8] represent Affinity Level 1
 * Bits[7:0] represent Affinity Level 0
 */
static unsigned int fvp_pwrc_core_id(u_register_t mpidr)
{
	return (unsigned int)(mpidr & FVP_PWRC_ID_MASK);
}

unsigned int fvp_pwrc_get_cpu_wkr(u_register_t mpidr)
{
	unsigned int id = fvp_pwrc_core_id(mpidr);

	return PSYSR_WK(fvp_pwrc_read_psysr(id));
}

unsigned int fvp_pwrc_read_psysr(u_register_t mpidr)
{
	unsigned int rc;
	unsigned int id = fvp_pwrc_core_id(mpidr);

	arm_lock_get();
	mmio_write_32(PWRC_BASE + PSYSR_OFF, id);
	rc = mmio_read_32(PWRC_BASE + PSYSR_OFF);
	arm_lock_release();
	return rc;
}

void fvp_pwrc_write_pponr(u_register_t mpidr)
{
	unsigned int id = fvp_pwrc_core_id(mpidr);

	arm_lock_get();
	mmio_write_32(PWRC_BASE + PPONR_OFF, id);
	arm_lock_release();
}

void fvp_pwrc_write_ppoffr(u_register_t mpidr)
{
	unsigned int id = fvp_pwrc_core_id(mpidr);

	arm_lock_get();
	mmio_write_32(PWRC_BASE + PPOFFR_OFF, id);
	arm_lock_release();
}

void fvp_pwrc_set_wen(u_register_t mpidr)
{
	unsigned int id = fvp_pwrc_core_id(mpidr);

	arm_lock_get();
	mmio_write_32(PWRC_BASE + PWKUPR_OFF,
		      (unsigned int) (PWKUPR_WEN | id));
	arm_lock_release();
}

void fvp_pwrc_clr_wen(u_register_t mpidr)
{
	unsigned int id = fvp_pwrc_core_id(mpidr);

	arm_lock_get();
	mmio_write_32(PWRC_BASE + PWKUPR_OFF, id);
	arm_lock_release();
}

void fvp_pwrc_write_pcoffr(u_register_t mpidr)
{
	unsigned int id = fvp_pwrc_core_id(mpidr);

	arm_lock_get();
	mmio_write_32(PWRC_BASE + PCOFFR_OFF, id);
	arm_lock_release();
}

/* Nothing else to do here apart from initializing the lock */
void __init plat_arm_pwrc_setup(void)
{
	arm_lock_init();
}



