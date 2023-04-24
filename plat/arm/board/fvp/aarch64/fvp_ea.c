/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <bl31/ea_handle.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>

/*
 * This source file with custom plat_ea_handler function is compiled only when
 * building TF-A with compile option PLATFORM_TEST_EA_FFH
 */

/* Test address(non-existent) used in tftf to cause External aborts */
#define TEST_ADDRESS	UL(0x7FFFF000)

void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
#ifdef PLATFORM_TEST_EA_FFH
	u_register_t elr_el3;
	u_register_t fault_address;
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	el3_state_t *el3_ctx = get_el3state_ctx(ctx);
	gp_regs_t *gpregs_ctx = get_gpregs_ctx(ctx);
	unsigned int level = (unsigned int)GET_EL(read_spsr_el3());

	fault_address = read_ctx_reg(gpregs_ctx, CTX_GPREG_X0);

	if ((level < MODE_EL3) && (fault_address == TEST_ADDRESS)) {
		if (ea_reason == ERROR_EA_SYNC) {
			INFO("Handled sync EA from lower EL at address 0x%lx\n", fault_address);
			/* To avoid continuous faults, forward return address */
			elr_el3 = read_ctx_reg(el3_ctx, CTX_ELR_EL3);
			elr_el3 += 4;
			write_ctx_reg(el3_ctx, CTX_ELR_EL3, elr_el3);
			return;
		} else if (ea_reason == ERROR_EA_ASYNC) {
			INFO("Handled Serror from lower EL at address 0x%lx\n", fault_address);
			return;
		}
	}
#endif
	plat_default_ea_handler(ea_reason, syndrome, cookie, handle, flags);
}
