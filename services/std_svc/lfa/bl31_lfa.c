/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/build_message.h>
#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <services/bl31_lfa.h>
#include <services/lfa_holding_pen.h>
#include <services/lfa_svc.h>

static const uintptr_t TEXT_START = (uintptr_t)__TEXT_START__;

/* Static function prototypes. */
static int lfa_bl31_check(uintptr_t base_va, size_t size, uint64_t **entry);
static int lfa_r_bl31_copy(uintptr_t va, uint64_t *entry, void *src, size_t size, bool reset);

static inline size_t lfa_bl31_errata_size(void)
{
	return ERRATA_END - ERRATA_START;
}

static inline size_t lfa_bl31_cpu_ops_size(void)
{
	return CPU_OPS_END - CPU_OPS_START;
}

static int lfa_bl31_patch_region(struct lfa_component_status *activation,
				 uintptr_t dst_va, size_t size, bool reset)
{
	uint64_t *entry = NULL;
	size_t src_offset;
	uintptr_t src_va;
	uintptr_t patch_start;
	uintptr_t patch_end;
	size_t patch_size;

	patch_start = dst_va & ~(PAGE_SIZE - 1U);
	patch_end = (dst_va + size + PAGE_SIZE - 1U) & ~(PAGE_SIZE - 1U);
	patch_size = patch_end - patch_start;

	if ((patch_start < TEXT_START) ||
	    ((patch_size > activation->image_size) &&
	    (activation->image_size != 0))) {
		ERROR("BL31 LFA: invalid patch region dst=0x%lx size=0x%zx image_size=0x%lx\n",
		      dst_va, size, activation->image_size);
		return LFA_CRITICAL_ERROR;
	}

	src_offset = patch_start - TEXT_START;
	if ((activation->image_size != 0) &&
	    ((src_offset > activation->image_size) ||
	    (patch_size > (activation->image_size - src_offset)))) {
		ERROR("BL31 LFA: staged image too small for patch region 0x%lx size 0x%zx "
		      "aligned_start 0x%lx aligned_size 0x%zx\n",
		      dst_va, size, patch_start, patch_size);
		return LFA_CRITICAL_ERROR;
	}

	src_va = activation->image_address + src_offset;

	if (lfa_bl31_check(patch_start, patch_size, &entry) != 0) {
		ERROR("BL31 LFA: xlat check failed for patch region dst=0x%lx size=0x%zx "
		      "aligned_start 0x%lx aligned_size 0x%zx\n",
		      dst_va, size, patch_start, patch_size);
		return LFA_CRITICAL_ERROR;
	}

	lfa_r_bl31_copy(patch_start, entry, (void *)src_va, patch_size, reset);
	return LFA_SUCCESS;
}

int32_t lfa_bl31_prime(struct lfa_component_status *activation)
{
	// TODO: Check that activation->image_size is not too large.
	return LFA_SUCCESS;
}

/* Function that actually performs the activation of the firmware image */
static int lfa_bl31_image_activation(struct lfa_component_status *activation,
	uint64_t ep_address, uint64_t context_id)
{
	VERBOSE("BL31 LFA: Starting BL31 Image Activation (core %d)\n", plat_my_core_pos());
	VERBOSE("  Address: 0x%lX\n", activation->image_address);
	VERBOSE("  Size:    %ld bytes\n", activation->image_size);
	VERBOSE("  Reset:   %s\n", activation->reset ? "yes" : "no");

#if !ENABLE_RUNTIME_INSTRUMENTATION
	/*
	 * Used to avoid console corruption. Not used if measuring performance
	 * since printouts are removed.
	 */
	console_flush();
#endif

	/* Prepare for warm reset and setup NS entrypoint for primary cores */
	if (activation->reset) {
		if (prepare_warm_reset(ep_address, context_id,
				(uint64_t)&lfa_r_holding_lock_var) != LFA_SUCCESS) {
			lfa_r_holding_unlock(&lfa_r_holding_lock_var);
			return LFA_CRITICAL_ERROR;
		}
	}

	if (lfa_bl31_patch_region(activation, CPU_OPS_START,
				  lfa_bl31_cpu_ops_size(), false) != LFA_SUCCESS) {
		return LFA_CRITICAL_ERROR;
	}

	/*
	 * Patch the errata section last so reset, if requested, is triggered
	 * only after cpu_ops has been updated to point at the new errata code.
	 */
	return lfa_bl31_patch_region(activation, ERRATA_START,
				      lfa_bl31_errata_size(),
				      activation->reset);
}

int32_t lfa_bl31_activate(struct lfa_component_status *activation,
	uint64_t ep_address, uint64_t context_id)
{
	uint32_t core_pos = plat_my_core_pos();
	int ret = LFA_SUCCESS;

	lfa_r_ep_addresses[core_pos] = ep_address;
	lfa_r_context_ids[core_pos] = context_id;

	/* Case when Firmware requires CPU Rendezvous for LFA Activation */
	if (activation->cpu_rendezvous) {
		/*
		 * Only one core will return true from this function and it
		 * controls the activation process.
		 */
		if (lfa_holding_start()) {
			VERBOSE("BL31 LFA: CPU %d is leader core\n", core_pos);

			/*
			 * Before we release other cores from the holding pen
			 * try to get the image info. That way if this fails the
			 * system will still be in a recoverable state.
			 */
			ret = plat_lfa_get_image_info(activation->component_id,
						      &activation->image_address,
						      &activation->image_size);
			if (ret) {
				ERROR("BL31 LFA: Could not get image info!\n");
				lfa_holding_release(LFA_CRITICAL_ERROR);
				return LFA_CRITICAL_ERROR;
			}

			/*
			 * Only one core will load the relocatable code module and
			 * acquire the relocatable lock, then release the rest of the
			 * cores which will then wait for us to release the relocatable
			 * lock once the activation is complete.
			 */
			lfa_load_relocatable();
			lfa_r_holding_lock(&lfa_r_holding_lock_var);
			lfa_holding_release(LFA_SUCCESS);

			/*
			 * Perform image activation. Will not return if reset
			 * requested.
			 */
			ret = lfa_bl31_image_activation(activation, ep_address, context_id);

			/* Unlock holding pen for all other cores */
			lfa_r_holding_unlock(&lfa_r_holding_lock_var);

			if (ret) {
				ERROR("BL31 LFA: failed image activation with error: %d\n", ret);
				return ret;
			}

			INFO("BL31 LFA: Successful image activation!\n");
		} else {
			/* Wait until release from rendezvous holding pen. */
			ret = lfa_holding_wait();
			if (ret != LFA_SUCCESS) {
				return ret;
			}

			if (activation->reset) {
				/*
				 * Prepare for warm reset and setup NS
				 * entrypoint for secondary cores
				 */
				if (prepare_warm_reset(ep_address, context_id,
						(uint64_t)&lfa_r_holding_lock_var) != LFA_SUCCESS) {
					return LFA_CRITICAL_ERROR;
				}

				/*
				 * Wait until activation of the image is
				 * complete and warm reset occurs on main core
				 * then reset.
				 */
				lfa_r_holding_wait_warm_reset(&lfa_r_holding_lock_var);
			} else {
				/*
				 * Wait until main core is completed with
				 * activation
				 */
				lfa_r_holding_wait(&lfa_r_holding_lock_var);
			}
		}
	}

	/*
	 * No CPU rendezvous required. Perform LFA Activate with single core
	 * NOTE: Assumption that only one core will call activate with
	 * CPU_RENDEZVOUS=0
	 */
	else {
		/*
		 * Loading relocatable module normally happens in the holding
		 * pen. Since there is no holding pen without CPU rendezvous we
		 * are doing it here instead.
		 */
		lfa_load_relocatable();

		/* Perform actual image activation */
		ret = lfa_bl31_image_activation(activation, ep_address, context_id);
		if (ret) {
			ERROR("Failed image activation with error: %d\n", ret);
			return ret;
		}
	}

	NOTICE("BL31 Live Activation Without Reset %u\n", plat_my_core_pos());
	NOTICE("  Version : %s\n", build_version_string);
	NOTICE("  %s\n", build_message);

	return ret;
}

int32_t lfa_bl31_cancel(struct lfa_component_status *activation)
{
	/* Let CPUs out of the holding pen if needed. */
	lfa_holding_release(LFA_ACTIVATION_FAILED);

	return LFA_SUCCESS;
}

struct lfa_component_ops bl31_activator = {
	.prime = lfa_bl31_prime,
	.activate = lfa_bl31_activate,
	.cancel = lfa_bl31_cancel,
	.may_reset_cpu = true,
	.cpu_rendezvous_required = true,
};

struct lfa_component_ops *get_bl31_activator(void)
{
	return &bl31_activator;
}

uint64_t *lfa_get_xlat_table_entry(uintptr_t virtual_addr, const xlat_ctx_t *ctx)
{
	uint64_t *table = ctx->base_table;
	unsigned int entries = ctx->base_table_entries;
	unsigned int start_level = GET_XLAT_TABLE_LEVEL_BASE((unsigned long long)ctx->va_max_address + 1ULL);

	for (unsigned int level = start_level; level <= XLAT_TABLE_LEVEL_MAX; level++) {
		uint64_t idx, desc, desc_type;

		idx = XLAT_TABLE_IDX(virtual_addr, level);
		if (idx >= entries) {
			return NULL;
		}

		desc = table[idx];
		desc_type = desc & DESC_MASK;

		if (desc_type == INVALID_DESC) {
			return NULL;
		}

		if (level == XLAT_TABLE_LEVEL_MAX) {
			/*
			 * Only page descriptors allowed at the final lookup
			 * level.
			 */
			assert(desc_type == PAGE_DESC);
			return &table[idx];
		}

		if (desc_type == BLOCK_DESC) {
			return &table[idx];
		}

		assert(desc_type == TABLE_DESC);
		table = (uint64_t *)(uintptr_t)(desc & TABLE_ADDR_MASK);
		entries = XLAT_TABLE_ENTRIES;
	}

	return NULL;
}

int lfa_bl31_check(uintptr_t base_va, size_t size, uint64_t **entry)
{
	const xlat_ctx_t *ctx = get_xlat_tables();

	assert(ctx != NULL);
	assert(ctx->initialized);

	if (!IS_PAGE_ALIGNED(base_va)) {
		WARN("%s: Address 0x%lx is not aligned on a page boundary.\n", __func__, base_va);
		return -EINVAL;
	}

	if (size == 0U) {
		WARN("%s: Size is 0.\n", __func__);
		return -EINVAL;
	}

	size_t pages_count = size / PAGE_SIZE;

	VERBOSE("BL31 update will impact %zu pages starting from address 0x%lx...\n",
		pages_count, base_va);

	*entry = lfa_get_xlat_table_entry(base_va, ctx);

	uint64_t *last = lfa_get_xlat_table_entry(base_va + (pages_count - 1) * PAGE_SIZE, ctx);

	/* Check that all descriptors are in the same page. */
	if ((((uint64_t)*entry) >> 12) != ((((uint64_t)last) >> 12))) {
		ERROR("All page descriptors are not part of the same page.\n");
		return -EINVAL;
	}

	return 0;
}

__attribute__((noinline, section(".lfa_relocatable_code")))
int lfa_r_bl31_copy(uintptr_t base_va, uint64_t *entry, void *src, size_t size, bool reset)
{
	do {
		uint64_t desc = *entry;
		size_t count = MIN(PAGE_SIZE, size) / sizeof(uint64_t);

		*entry = INVALID_DESC;
		dsbishst();
		tlbivae3is(TLBI_ADDR(base_va));
		dsbish();
		isb();
		*entry = (desc & ~LOWER_ATTRS(AP_RO));
		dsbish();

		for (uint64_t i = 0; i < count; i++) {
			((uint64_t *) base_va)[i] = ((uint64_t *) src)[i];
		}
		if (size <= PAGE_SIZE) {
			for (uint64_t i = count * sizeof(uint64_t); i < size; i++) {
				((uint8_t *) base_va)[i] = ((uint8_t *) src)[i];
			}
			size = 0;
		} else {
			size -= PAGE_SIZE;
		}

		*entry = INVALID_DESC;
		dsbishst();
		tlbivae3is(TLBI_ADDR(base_va));
		dsbish();
		isb();
		*entry = desc;

		base_va += PAGE_SIZE;
		src += PAGE_SIZE;
		entry++;
	} while (size > 0);

	dsbish();

	if (reset) {
		wfi();
	}

	return 0;
}
