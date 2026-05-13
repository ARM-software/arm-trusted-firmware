/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <common/smc_validation_framework.h>
#include <lib/el3_runtime/aarch64/context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/smccc.h>
#include <plat/arm/common/arm_sip_svc.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>
#include <smccc_helpers.h>

/**
 * SiP Service Handlers
 */

#if PLAT_TEST_SPM
/**
 * Extracts the IRQ parameter via the validation framework and sets the
 * pending state in the interrupt controller.
 */
uintptr_t arm_sip_set_interrupt_pending_handler(void *handle, bool secure_origin)
{
	u_register_t irq_reg;

	if (!secure_origin) {
		SMC_RET1(handle, SMC_UNK);
	}

	if (smc_get_arg(handle, 1, &irq_reg) != SMC_OK) {
		SMC_RET1(handle, SMC_INVALID_PARAM);
	}

	uint32_t irq = (uint32_t)irq_reg;

	VERBOSE("SiP Call: Set interrupt pending %d\n", irq);
	plat_ic_set_interrupt_pending(irq);

	SMC_RET1(handle, SMC_OK);
}
#endif

#if defined(SPD_spmd) && SPMD_SPM_AT_SEL2 == 1
/**
 * Transitions the target memory granules between Secure and Non-Secure PAS.
 * Wraps the original plat_protect_memory behavior with validation framework
 * parameter extraction for TOCTOU protection. No behavioral changes.
 */
uintptr_t arm_sip_protect_mem_handler(void *handle, bool protect,
				      bool secure_origin)
{
	uintptr_t base;
	size_t size;
	uint64_t last_updated = 0;

	if (!is_feat_rme_supported()) {
		SMC_RET1(handle, SMC_UNK);
	}

	if (!secure_origin) {
		SMC_RET1(handle, SMC_UNK);
	}

	/*
	 * Security Merit: extract base/size into EL3-local shadow copies via
	 * the validation framework, closing the TOCTOU race window before any
	 * further validation or GPT transitions occur.
	 */
	if (smc_get_mem_range(handle, 1, 2, &base, &size) != SMC_OK) {
		SMC_RET1(handle, SMC_INVALID_PARAM);
	}

	if (!is_aligned(base, PAGE_SIZE_4KB) ||
	    !is_aligned(size, PAGE_SIZE_4KB)) {
		VERBOSE("Protect memory parameter base or size is not 4k aligned.\n");
		SMC_RET1(handle, SMC_INVALID_PARAM);
	}

	if ((ULONG_MAX - base) < size) {
		VERBOSE("Base + Size results in overflow.\n");
		SMC_RET1(handle, SMC_INVALID_PARAM);
	}

	/* Perform PAS delegation using protected EL3-local parameters */
	for (uintptr_t it = base; it < (base + size); it += PAGE_SIZE_4KB) {
		uint64_t granule_count = 1;
		uint64_t ret;

		if (protect) {
			ret = gpt_transition_pas(it, &granule_count,
						 GPT_GPI_SECURE,
						 SMC_FROM_SECURE);
		} else {
			ret = gpt_transition_pas(it, &granule_count, GPT_GPI_NS,
						 SMC_FROM_SECURE);
		}

		switch (ret) {
		case 0:
			last_updated = it;
			break;
		case -EINVAL:
			SMC_RET2(handle, SMC_INVALID_PARAM, last_updated);
			break; /* Shall not be reached. */
		case -EPERM:
			SMC_RET2(handle, SMC_DENIED, last_updated);
			break; /* Shall not be reached. */
		default:
			ERROR("Unexpected return\n");
			panic();
		}
	}

	SMC_RET1(handle, SMC_OK);
}
#endif
