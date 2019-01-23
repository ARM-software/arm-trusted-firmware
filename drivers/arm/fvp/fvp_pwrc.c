/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/fvp/fvp_pwrc.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/*
 * TODO: Someday there will be a generic power controller api. At the moment
 * each platform has its own pwrc so just exporting functions is fine.
 */
ARM_INSTANTIATE_LOCK;

unsigned int fvp_pwrc_get_cpu_wkr(u_register_t mpidr)
{
	return PSYSR_WK(fvp_pwrc_read_psysr(mpidr));
}

unsigned int fvp_pwrc_read_psysr(u_register_t mpidr)
{
	unsigned int rc;
	arm_lock_get();
	mmio_write_32(PWRC_BASE + PSYSR_OFF, (unsigned int) mpidr);
	rc = mmio_read_32(PWRC_BASE + PSYSR_OFF);
	arm_lock_release();
	return rc;
}

void fvp_pwrc_write_pponr(u_register_t mpidr)
{
	arm_lock_get();
	mmio_write_32(PWRC_BASE + PPONR_OFF, (unsigned int) mpidr);
	arm_lock_release();
}

void fvp_pwrc_write_ppoffr(u_register_t mpidr)
{
	arm_lock_get();
	mmio_write_32(PWRC_BASE + PPOFFR_OFF, (unsigned int) mpidr);
	arm_lock_release();
}

void fvp_pwrc_set_wen(u_register_t mpidr)
{
	arm_lock_get();
	mmio_write_32(PWRC_BASE + PWKUPR_OFF,
		      (unsigned int) (PWKUPR_WEN | mpidr));
	arm_lock_release();
}

void fvp_pwrc_clr_wen(u_register_t mpidr)
{
	arm_lock_get();
	mmio_write_32(PWRC_BASE + PWKUPR_OFF,
		      (unsigned int) mpidr);
	arm_lock_release();
}

void fvp_pwrc_write_pcoffr(u_register_t mpidr)
{
	arm_lock_get();
	mmio_write_32(PWRC_BASE + PCOFFR_OFF, (unsigned int) mpidr);
	arm_lock_release();
}

/* Nothing else to do here apart from initializing the lock */
void __init plat_arm_pwrc_setup(void)
{
	arm_lock_init();
}



