/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/fdt_wrappers.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <libfdt.h>

#include "fpga_private.h"
#include <plat/common/platform.h>
#include <platform_def.h>

static entry_point_info_t bl33_image_ep_info;
volatile uint32_t secondary_core_spinlock;

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return 0ULL;
#endif
}

uint32_t fpga_get_spsr_for_bl33_entry(void)
{
	return SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Add this core to the VALID mpids list */
	fpga_valid_mpids[plat_my_core_pos()] = VALID_MPID;

	/*
	 * Notify the secondary CPUs that the C runtime is ready
	 * so they can announce themselves.
	 */
	secondary_core_spinlock = C_RUNTIME_READY_KEY;
	dsbish();
	sev();

	fpga_console_init();

	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = fpga_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	/* Set x0-x3 for the primary CPU as expected by the kernel */
	bl33_image_ep_info.args.arg0 = (u_register_t)FPGA_PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
}

void bl31_plat_arch_setup(void)
{
}

void bl31_platform_setup(void)
{
	/* Write frequency to CNTCRL and initialize timer */
	generic_delay_timer_init();

	/*
	 * Before doing anything else, wait for some time to ensure that
	 * the secondary CPUs have populated the fpga_valid_mpids array.
	 * As the number of secondary cores is unknown and can even be 0,
	 * it is not possible to rely on any signal from them, so use a
	 * delay instead.
	 */
	mdelay(5);

	/*
	 * On the event of a cold reset issued by, for instance, a reset pin
	 * assertion, we cannot guarantee memory to be initialized to zero.
	 * In such scenario, if the secondary cores reached
	 * plat_secondary_cold_boot_setup before the primary one initialized
	 * .BSS, we could end up having a race condition if the spinlock
	 * was not cleared before.
	 *
	 * Similarly, if there were a reset before the spinlock had been
	 * cleared, the secondary cores would find the lock opened before
	 * .BSS is cleared, causing another race condition.
	 *
	 * So clean the spinlock as soon as we think it is safe to reduce the
	 * chances of any race condition on a reset.
	 */
	secondary_core_spinlock = 0UL;

	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_fpga_gic_init();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;
	next_image_info = &bl33_image_ep_info;

	/* Only expecting BL33: the kernel will run in EL2NS */
	assert(type == NON_SECURE);

	/* None of the images can have 0x0 as the entrypoint */
	if (next_image_info->pc) {
		return next_image_info;
	} else {
		return NULL;
	}
}

unsigned int plat_get_syscnt_freq2(void)
{
	const void *fdt = (void *)(uintptr_t)FPGA_PRELOADED_DTB_BASE;
	int node;

	node = fdt_node_offset_by_compatible(fdt, 0, "arm,armv8-timer");
	if (node < 0) {
		return FPGA_DEFAULT_TIMER_FREQUENCY;
	}

	return fdt_read_uint32_default(fdt, node, "clock-frequency",
				       FPGA_DEFAULT_TIMER_FREQUENCY);
}

void bl31_plat_enable_mmu(uint32_t flags)
{
	/* TODO: determine if MMU needs to be enabled */
}
