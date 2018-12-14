/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Driver for implementation defined features that are identical in ARM GICv3
* implementations (GIC-500 and GIC-600 for now). This driver only overrides
* APIs that are different to those generic ones in GICv3 driver.
 */

#include <assert.h>

#include <arch_helpers.h>
#include <drivers/arm/arm_gicv3_common.h>
#include <drivers/arm/gicv3.h>

#include "gicv3_private.h"

/*
 * Flush the internal GIC cache of the LPIs pending tables to memory before
 * saving the state of the Redistributor. This is required before powering off
 * the GIC when the pending status must be preserved.
 * `rdist_proc_num` is the processor number corresponding to the Redistributor of the
 * current CPU.
 */
void arm_gicv3_distif_pre_save(unsigned int rdist_proc_num)
{
	uintptr_t gicr_base = 0;

	assert(gicv3_driver_data);
	assert(gicv3_driver_data->rdistif_base_addrs);

	/*
	 * The GICR_WAKER.Sleep bit should be set only when both
	 * GICR_WAKER.ChildrenAsleep and GICR_WAKER.ProcessorSleep are set on
	 * all the Redistributors.
	 */
	for (unsigned int i = 0; i < gicv3_driver_data->rdistif_num; i++) {
		gicr_base = gicv3_driver_data->rdistif_base_addrs[i];
		assert(gicr_base);
		assert(gicr_read_waker(gicr_base) & WAKER_CA_BIT);
		assert(gicr_read_waker(gicr_base) & WAKER_PS_BIT);
	}

	gicr_base = gicv3_driver_data->rdistif_base_addrs[rdist_proc_num];
	/*
	 * According to the TRM, there is only one instance of the
	 * GICR_WAKER.Sleep and GICR_WAKER.Quiescent bits that can be accessed
	 * through any of the Redistributor.
	 */

	/*
	 * Set GICR_WAKER.Sleep
	 * After this point, the system must be configured so that the
	 * wake_request signals for the right cores are asserted when a wakeup
	 * interrupt is detected. The GIC will not be able to do that anymore
	 * when the GICR_WAKER.Sleep bit is set to 1.
	 */
	gicr_write_waker(gicr_base, gicr_read_waker(gicr_base) | WAKER_SL_BIT);

	/* Wait until the GICR_WAKER.Quiescent bit is set */
	while (!(gicr_read_waker(gicr_base) & WAKER_QSC_BIT))
		;
}

/*
 * Allow the LPIs pending state to be read back from the tables in memory after
 * having restored the state of the GIC Redistributor.
 */
void arm_gicv3_distif_post_restore(unsigned int rdist_proc_num)
{
	uintptr_t gicr_base;

	assert(gicv3_driver_data);
	assert(gicv3_driver_data->rdistif_base_addrs);

	/*
	 * According to the TRM, there is only one instance of the
	 * GICR_WAKER.Sleep and GICR_WAKER.Quiescent bits that can be accessed
	 * through any of the Redistributor.
	 */
	gicr_base = gicv3_driver_data->rdistif_base_addrs[rdist_proc_num];
	assert(gicr_base);

	/*
	 * If the GIC had power removed, the GICR_WAKER state will be reset.
	 * Since the GICR_WAKER.Sleep and GICR_WAKER.Quiescent bits are cleared,
	 * we can exit early. This also prevents the following assert from
	 * erroneously triggering.
	 */
	if (!(gicr_read_waker(gicr_base) & WAKER_SL_BIT))
		return;

	/*
	 * Writes to GICR_WAKER.Sleep bit are ignored if GICR_WAKER.Quiescent
	 * bit is not set. We should be alright on power on path, therefore
	 * coming out of sleep and Quiescent should be set, but we assert in
	 * case.
	 */
	assert(gicr_read_waker(gicr_base) & WAKER_QSC_BIT);

	/* Clear GICR_WAKER.Sleep */
	gicr_write_waker(gicr_base, gicr_read_waker(gicr_base) & ~WAKER_SL_BIT);

	/*
	 * We don't know if the effects of setting GICR_WAKER.Sleep bit is
	 * instantaneous, so we wait until the interface is not Quiescent
	 * anymore.
	 */
	while (gicr_read_waker(gicr_base) & WAKER_QSC_BIT)
		;
}

