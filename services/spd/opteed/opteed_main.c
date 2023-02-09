/*
 * Copyright (c) 2013-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/*******************************************************************************
 * This is the Secure Payload Dispatcher (SPD). The dispatcher is meant to be a
 * plug-in component to the Secure Monitor, registered as a runtime service. The
 * SPD is expected to be a functional extension of the Secure Payload (SP) that
 * executes in Secure EL1. The Secure Monitor will delegate all SMCs targeting
 * the Trusted OS/Applications range to the dispatcher. The SPD will either
 * handle the request locally or delegate it to the Secure Payload. It is also
 * responsible for initialising and maintaining communication with the SP.
 ******************************************************************************/
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/coreboot.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/optee_utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#if OPTEE_ALLOW_SMC_LOAD
#include <libfdt.h>
#endif  /* OPTEE_ALLOW_SMC_LOAD */
#include <plat/common/platform.h>
#include <tools_share/uuid.h>

#include "opteed_private.h"
#include "teesmc_opteed.h"

/*******************************************************************************
 * Address of the entrypoint vector table in OPTEE. It is
 * initialised once on the primary core after a cold boot.
 ******************************************************************************/
struct optee_vectors *optee_vector_table;

/*******************************************************************************
 * Array to keep track of per-cpu OPTEE state
 ******************************************************************************/
optee_context_t opteed_sp_context[OPTEED_CORE_COUNT];
uint32_t opteed_rw;

#if OPTEE_ALLOW_SMC_LOAD
static bool opteed_allow_load;
/* OP-TEE image loading service UUID */
DEFINE_SVC_UUID2(optee_image_load_uuid,
	0xb1eafba3, 0x5d31, 0x4612, 0xb9, 0x06,
	0xc4, 0xc7, 0xa4, 0xbe, 0x3c, 0xc0);

#define OPTEED_FDT_SIZE 256
static uint8_t fdt_buf[OPTEED_FDT_SIZE] __aligned(CACHE_WRITEBACK_GRANULE);

#else
static int32_t opteed_init(void);
#endif

uint64_t dual32to64(uint32_t high, uint32_t low)
{
	return ((uint64_t)high << 32) | low;
}

/*******************************************************************************
 * This function is the handler registered for S-EL1 interrupts by the
 * OPTEED. It validates the interrupt and upon success arranges entry into
 * the OPTEE at 'optee_fiq_entry()' for handling the interrupt.
 ******************************************************************************/
static uint64_t opteed_sel1_interrupt_handler(uint32_t id,
					    uint32_t flags,
					    void *handle,
					    void *cookie)
{
	uint32_t linear_id;
	optee_context_t *optee_ctx;

	/* Check the security state when the exception was generated */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

	/* Sanity check the pointer to this cpu's context */
	assert(handle == cm_get_context(NON_SECURE));

	/* Save the non-secure context before entering the OPTEE */
	cm_el1_sysregs_context_save(NON_SECURE);

	/* Get a reference to this cpu's OPTEE context */
	linear_id = plat_my_core_pos();
	optee_ctx = &opteed_sp_context[linear_id];
	assert(&optee_ctx->cpu_ctx == cm_get_context(SECURE));

	cm_set_elr_el3(SECURE, (uint64_t)&optee_vector_table->fiq_entry);
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	/*
	 * Tell the OPTEE that it has to handle an FIQ (synchronously).
	 * Also the instruction in normal world where the interrupt was
	 * generated is passed for debugging purposes. It is safe to
	 * retrieve this address from ELR_EL3 as the secure context will
	 * not take effect until el3_exit().
	 */
	SMC_RET1(&optee_ctx->cpu_ctx, read_elr_el3());
}

/*******************************************************************************
 * OPTEE Dispatcher setup. The OPTEED finds out the OPTEE entrypoint and type
 * (aarch32/aarch64) if not already known and initialises the context for entry
 * into OPTEE for its initialization.
 ******************************************************************************/
static int32_t opteed_setup(void)
{
#if OPTEE_ALLOW_SMC_LOAD
	opteed_allow_load = true;
	INFO("Delaying OP-TEE setup until we receive an SMC call to load it\n");
	return 0;
#else
	entry_point_info_t *optee_ep_info;
	uint32_t linear_id;
	uint64_t opteed_pageable_part;
	uint64_t opteed_mem_limit;
	uint64_t dt_addr;

	linear_id = plat_my_core_pos();

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.  TODO: Add support to
	 * conditionally include the SPD service
	 */
	optee_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!optee_ep_info) {
		WARN("No OPTEE provided by BL2 boot loader, Booting device"
			" without OPTEE initialization. SMC`s destined for OPTEE"
			" will return SMC_UNK\n");
		return 1;
	}

	/*
	 * If there's no valid entry point for SP, we return a non-zero value
	 * signalling failure initializing the service. We bail out without
	 * registering any handlers
	 */
	if (!optee_ep_info->pc)
		return 1;

	opteed_rw = optee_ep_info->args.arg0;
	opteed_pageable_part = optee_ep_info->args.arg1;
	opteed_mem_limit = optee_ep_info->args.arg2;
	dt_addr = optee_ep_info->args.arg3;

	opteed_init_optee_ep_state(optee_ep_info,
				opteed_rw,
				optee_ep_info->pc,
				opteed_pageable_part,
				opteed_mem_limit,
				dt_addr,
				&opteed_sp_context[linear_id]);

	/*
	 * All OPTEED initialization done. Now register our init function with
	 * BL31 for deferred invocation
	 */
	bl31_register_bl32_init(&opteed_init);

	return 0;
#endif  /* OPTEE_ALLOW_SMC_LOAD */
}

/*******************************************************************************
 * This function passes control to the OPTEE image (BL32) for the first time
 * on the primary cpu after a cold boot. It assumes that a valid secure
 * context has already been created by opteed_setup() which can be directly
 * used.  It also assumes that a valid non-secure context has been
 * initialised by PSCI so it does not need to save and restore any
 * non-secure state. This function performs a synchronous entry into
 * OPTEE. OPTEE passes control back to this routine through a SMC. This returns
 * a non-zero value on success and zero on failure.
 ******************************************************************************/
static int32_t
opteed_init_with_entry_point(entry_point_info_t *optee_entry_point)
{
	uint32_t linear_id = plat_my_core_pos();
	optee_context_t *optee_ctx = &opteed_sp_context[linear_id];
	uint64_t rc;
	assert(optee_entry_point);

	cm_init_my_context(optee_entry_point);

	/*
	 * Arrange for an entry into OPTEE. It will be returned via
	 * OPTEE_ENTRY_DONE case
	 */
	rc = opteed_synchronous_sp_entry(optee_ctx);
	assert(rc != 0);

	return rc;
}

#if !OPTEE_ALLOW_SMC_LOAD
static int32_t opteed_init(void)
{
	entry_point_info_t *optee_entry_point;
	/*
	 * Get information about the OP-TEE (BL32) image. Its
	 * absence is a critical failure.
	 */
	optee_entry_point = bl31_plat_get_next_image_ep_info(SECURE);
	return opteed_init_with_entry_point(optee_entry_point);
}
#endif  /* !OPTEE_ALLOW_SMC_LOAD */

#if OPTEE_ALLOW_SMC_LOAD
#if COREBOOT
/*
 * Adds a firmware/coreboot node with the coreboot table information to a device
 * tree. Returns zero on success or if there is no coreboot table information;
 * failure code otherwise.
 */
static int add_coreboot_node(void *fdt)
{
	int ret;
	uint64_t coreboot_table_addr;
	uint32_t coreboot_table_size;
	struct {
		uint64_t addr;
		uint32_t size;
	} reg_node;
	coreboot_get_table_location(&coreboot_table_addr, &coreboot_table_size);
	if (!coreboot_table_addr || !coreboot_table_size) {
		WARN("Unable to get coreboot table location for device tree");
		return 0;
	}
	ret = fdt_begin_node(fdt, "firmware");
	if (ret)
		return ret;

	ret = fdt_property(fdt, "ranges", NULL, 0);
	if (ret)
		return ret;

	ret = fdt_begin_node(fdt, "coreboot");
	if (ret)
		return ret;

	ret = fdt_property_string(fdt, "compatible", "coreboot");
	if (ret)
		return ret;

	reg_node.addr = cpu_to_fdt64(coreboot_table_addr);
	reg_node.size = cpu_to_fdt32(coreboot_table_size);
	ret = fdt_property(fdt, "reg", &reg_node,
				sizeof(uint64_t) + sizeof(uint32_t));
	if (ret)
		return ret;

	ret = fdt_end_node(fdt);
	if (ret)
		return ret;

	return fdt_end_node(fdt);
}
#endif /* COREBOOT */

/*
 * Creates a device tree for passing into OP-TEE. Currently is populated with
 * the coreboot table address.
 * Returns 0 on success, error code otherwise.
 */
static int create_opteed_dt(void)
{
	int ret;

	ret = fdt_create(fdt_buf, OPTEED_FDT_SIZE);
	if (ret)
		return ret;

	ret = fdt_finish_reservemap(fdt_buf);
	if (ret)
		return ret;

	ret = fdt_begin_node(fdt_buf, "");
	if (ret)
		return ret;

#if COREBOOT
	ret = add_coreboot_node(fdt_buf);
	if (ret)
		return ret;
#endif /* COREBOOT */

	ret = fdt_end_node(fdt_buf);
	if (ret)
		return ret;

	return fdt_finish(fdt_buf);
}

/*******************************************************************************
 * This function is responsible for handling the SMC that loads the OP-TEE
 * binary image via a non-secure SMC call. It takes the size and physical
 * address of the payload as parameters.
 ******************************************************************************/
static int32_t opteed_handle_smc_load(uint64_t data_size, uint32_t data_pa)
{
	uintptr_t data_va = data_pa;
	uint64_t mapped_data_pa;
	uintptr_t mapped_data_va;
	uint64_t data_map_size;
	int32_t rc;
	optee_header_t *image_header;
	uint8_t *image_ptr;
	uint64_t target_pa;
	uint64_t target_end_pa;
	uint64_t image_pa;
	uintptr_t image_va;
	optee_image_t *curr_image;
	uintptr_t target_va;
	uint64_t target_size;
	entry_point_info_t optee_ep_info;
	uint32_t linear_id = plat_my_core_pos();
	uint64_t dt_addr = 0;

	mapped_data_pa = page_align(data_pa, DOWN);
	mapped_data_va = mapped_data_pa;
	data_map_size = page_align(data_size + (mapped_data_pa - data_pa), UP);

	/*
	 * We do not validate the passed in address because we are trusting the
	 * non-secure world at this point still.
	 */
	rc = mmap_add_dynamic_region(mapped_data_pa, mapped_data_va,
				     data_map_size, MT_MEMORY | MT_RO | MT_NS);
	if (rc != 0) {
		return rc;
	}

	image_header = (optee_header_t *)data_va;
	if (image_header->magic != TEE_MAGIC_NUM_OPTEE ||
	    image_header->version != 2 || image_header->nb_images != 1) {
		mmap_remove_dynamic_region(mapped_data_va, data_map_size);
		return -EINVAL;
	}

	image_ptr = (uint8_t *)data_va + sizeof(optee_header_t) +
			sizeof(optee_image_t);
	if (image_header->arch == 1) {
		opteed_rw = OPTEE_AARCH64;
	} else {
		opteed_rw = OPTEE_AARCH32;
	}

	curr_image = &image_header->optee_image_list[0];
	image_pa = dual32to64(curr_image->load_addr_hi,
			      curr_image->load_addr_lo);
	image_va = image_pa;
	target_end_pa = image_pa + curr_image->size;

	/* Now also map the memory we want to copy it to. */
	target_pa = page_align(image_pa, DOWN);
	target_va = target_pa;
	target_size = page_align(target_end_pa, UP) - target_pa;

	rc = mmap_add_dynamic_region(target_pa, target_va, target_size,
				     MT_MEMORY | MT_RW | MT_SECURE);
	if (rc != 0) {
		mmap_remove_dynamic_region(mapped_data_va, data_map_size);
		return rc;
	}

	INFO("Loaded OP-TEE via SMC: size %d addr 0x%" PRIx64 "\n",
	     curr_image->size, image_va);

	memcpy((void *)image_va, image_ptr, curr_image->size);
	flush_dcache_range(target_pa, target_size);

	mmap_remove_dynamic_region(mapped_data_va, data_map_size);
	mmap_remove_dynamic_region(target_va, target_size);

	/* Save the non-secure state */
	cm_el1_sysregs_context_save(NON_SECURE);

	rc = create_opteed_dt();
	if (rc) {
		ERROR("Failed device tree creation %d\n", rc);
		return rc;
	}
	dt_addr = (uint64_t)fdt_buf;
	flush_dcache_range(dt_addr, OPTEED_FDT_SIZE);

	opteed_init_optee_ep_state(&optee_ep_info,
				   opteed_rw,
				   image_pa,
				   0,
				   0,
				   dt_addr,
				   &opteed_sp_context[linear_id]);
	if (opteed_init_with_entry_point(&optee_ep_info) == 0) {
		rc = -EFAULT;
	}

	/* Restore non-secure state */
	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	return rc;
}
#endif  /* OPTEE_ALLOW_SMC_LOAD */

/*******************************************************************************
 * This function is responsible for handling all SMCs in the Trusted OS/App
 * range from the non-secure state as defined in the SMC Calling Convention
 * Document. It is also responsible for communicating with the Secure
 * payload to delegate work and return results back to the non-secure
 * state. Lastly it will also return any information that OPTEE needs to do
 * the work assigned to it.
 ******************************************************************************/
static uintptr_t opteed_smc_handler(uint32_t smc_fid,
			 u_register_t x1,
			 u_register_t x2,
			 u_register_t x3,
			 u_register_t x4,
			 void *cookie,
			 void *handle,
			 u_register_t flags)
{
	cpu_context_t *ns_cpu_context;
	uint32_t linear_id = plat_my_core_pos();
	optee_context_t *optee_ctx = &opteed_sp_context[linear_id];
	uint64_t rc;

	/*
	 * Determine which security state this SMC originated from
	 */

	if (is_caller_non_secure(flags)) {
#if OPTEE_ALLOW_SMC_LOAD
		if (opteed_allow_load && smc_fid == NSSMC_OPTEED_CALL_UID) {
			/* Provide the UUID of the image loading service. */
			SMC_UUID_RET(handle, optee_image_load_uuid);
		}
		if (smc_fid == NSSMC_OPTEED_CALL_LOAD_IMAGE) {
			/*
			 * TODO: Consider wiping the code for SMC loading from
			 * memory after it has been invoked similar to what is
			 * done under RECLAIM_INIT, but extended to happen
			 * later.
			 */
			if (!opteed_allow_load) {
				SMC_RET1(handle, -EPERM);
			}

			opteed_allow_load = false;
			uint64_t data_size = dual32to64(x1, x2);
			uint64_t data_pa = dual32to64(x3, x4);
			if (!data_size || !data_pa) {
				/*
				 * This is invoked when the OP-TEE image didn't
				 * load correctly in the kernel but we want to
				 * block off loading of it later for security
				 * reasons.
				 */
				SMC_RET1(handle, -EINVAL);
			}
			SMC_RET1(handle, opteed_handle_smc_load(
					data_size, data_pa));
		}
#endif  /* OPTEE_ALLOW_SMC_LOAD */
		/*
		 * This is a fresh request from the non-secure client.
		 * The parameters are in x1 and x2. Figure out which
		 * registers need to be preserved, save the non-secure
		 * state and send the request to the secure payload.
		 */
		assert(handle == cm_get_context(NON_SECURE));

		cm_el1_sysregs_context_save(NON_SECURE);

		/*
		 * We are done stashing the non-secure context. Ask the
		 * OP-TEE to do the work now. If we are loading vi an SMC,
		 * then we also need to init this CPU context if not done
		 * already.
		 */
		if (optee_vector_table == NULL) {
			SMC_RET1(handle, -EINVAL);
		}

		if (get_optee_pstate(optee_ctx->state) ==
		    OPTEE_PSTATE_UNKNOWN) {
			opteed_cpu_on_finish_handler(0);
		}

		/*
		 * Verify if there is a valid context to use, copy the
		 * operation type and parameters to the secure context
		 * and jump to the fast smc entry point in the secure
		 * payload. Entry into S-EL1 will take place upon exit
		 * from this function.
		 */
		assert(&optee_ctx->cpu_ctx == cm_get_context(SECURE));

		/* Set appropriate entry for SMC.
		 * We expect OPTEE to manage the PSTATE.I and PSTATE.F
		 * flags as appropriate.
		 */
		if (GET_SMC_TYPE(smc_fid) == SMC_TYPE_FAST) {
			cm_set_elr_el3(SECURE, (uint64_t)
					&optee_vector_table->fast_smc_entry);
		} else {
			cm_set_elr_el3(SECURE, (uint64_t)
					&optee_vector_table->yield_smc_entry);
		}

		cm_el1_sysregs_context_restore(SECURE);
		cm_set_next_eret_context(SECURE);

		write_ctx_reg(get_gpregs_ctx(&optee_ctx->cpu_ctx),
			      CTX_GPREG_X4,
			      read_ctx_reg(get_gpregs_ctx(handle),
					   CTX_GPREG_X4));
		write_ctx_reg(get_gpregs_ctx(&optee_ctx->cpu_ctx),
			      CTX_GPREG_X5,
			      read_ctx_reg(get_gpregs_ctx(handle),
					   CTX_GPREG_X5));
		write_ctx_reg(get_gpregs_ctx(&optee_ctx->cpu_ctx),
			      CTX_GPREG_X6,
			      read_ctx_reg(get_gpregs_ctx(handle),
					   CTX_GPREG_X6));
		/* Propagate hypervisor client ID */
		write_ctx_reg(get_gpregs_ctx(&optee_ctx->cpu_ctx),
			      CTX_GPREG_X7,
			      read_ctx_reg(get_gpregs_ctx(handle),
					   CTX_GPREG_X7));

		SMC_RET4(&optee_ctx->cpu_ctx, smc_fid, x1, x2, x3);
	}

	/*
	 * Returning from OPTEE
	 */

	switch (smc_fid) {
	/*
	 * OPTEE has finished initialising itself after a cold boot
	 */
	case TEESMC_OPTEED_RETURN_ENTRY_DONE:
		/*
		 * Stash the OPTEE entry points information. This is done
		 * only once on the primary cpu
		 */
		assert(optee_vector_table == NULL);
		optee_vector_table = (optee_vectors_t *) x1;

		if (optee_vector_table) {
			set_optee_pstate(optee_ctx->state, OPTEE_PSTATE_ON);

			/*
			 * OPTEE has been successfully initialized.
			 * Register power management hooks with PSCI
			 */
			psci_register_spd_pm_hook(&opteed_pm);

			/*
			 * Register an interrupt handler for S-EL1 interrupts
			 * when generated during code executing in the
			 * non-secure state.
			 */
			flags = 0;
			set_interrupt_rm_flag(flags, NON_SECURE);
			rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
						opteed_sel1_interrupt_handler,
						flags);
			if (rc)
				panic();
		}

		/*
		 * OPTEE reports completion. The OPTEED must have initiated
		 * the original request through a synchronous entry into
		 * OPTEE. Jump back to the original C runtime context.
		 */
		opteed_synchronous_sp_exit(optee_ctx, x1);
		break;


	/*
	 * These function IDs is used only by OP-TEE to indicate it has
	 * finished:
	 * 1. turning itself on in response to an earlier psci
	 *    cpu_on request
	 * 2. resuming itself after an earlier psci cpu_suspend
	 *    request.
	 */
	case TEESMC_OPTEED_RETURN_ON_DONE:
	case TEESMC_OPTEED_RETURN_RESUME_DONE:


	/*
	 * These function IDs is used only by the SP to indicate it has
	 * finished:
	 * 1. suspending itself after an earlier psci cpu_suspend
	 *    request.
	 * 2. turning itself off in response to an earlier psci
	 *    cpu_off request.
	 */
	case TEESMC_OPTEED_RETURN_OFF_DONE:
	case TEESMC_OPTEED_RETURN_SUSPEND_DONE:
	case TEESMC_OPTEED_RETURN_SYSTEM_OFF_DONE:
	case TEESMC_OPTEED_RETURN_SYSTEM_RESET_DONE:

		/*
		 * OPTEE reports completion. The OPTEED must have initiated the
		 * original request through a synchronous entry into OPTEE.
		 * Jump back to the original C runtime context, and pass x1 as
		 * return value to the caller
		 */
		opteed_synchronous_sp_exit(optee_ctx, x1);
		break;

	/*
	 * OPTEE is returning from a call or being preempted from a call, in
	 * either case execution should resume in the normal world.
	 */
	case TEESMC_OPTEED_RETURN_CALL_DONE:
		/*
		 * This is the result from the secure client of an
		 * earlier request. The results are in x0-x3. Copy it
		 * into the non-secure context, save the secure state
		 * and return to the non-secure state.
		 */
		assert(handle == cm_get_context(SECURE));
		cm_el1_sysregs_context_save(SECURE);

		/* Get a reference to the non-secure context */
		ns_cpu_context = cm_get_context(NON_SECURE);
		assert(ns_cpu_context);

		/* Restore non-secure state */
		cm_el1_sysregs_context_restore(NON_SECURE);
		cm_set_next_eret_context(NON_SECURE);

		SMC_RET4(ns_cpu_context, x1, x2, x3, x4);

	/*
	 * OPTEE has finished handling a S-EL1 FIQ interrupt. Execution
	 * should resume in the normal world.
	 */
	case TEESMC_OPTEED_RETURN_FIQ_DONE:
		/* Get a reference to the non-secure context */
		ns_cpu_context = cm_get_context(NON_SECURE);
		assert(ns_cpu_context);

		/*
		 * Restore non-secure state. There is no need to save the
		 * secure system register context since OPTEE was supposed
		 * to preserve it during S-EL1 interrupt handling.
		 */
		cm_el1_sysregs_context_restore(NON_SECURE);
		cm_set_next_eret_context(NON_SECURE);

		SMC_RET0((uint64_t) ns_cpu_context);

	default:
		panic();
	}
}

/* Define an OPTEED runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	opteed_fast,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_FAST,
	opteed_setup,
	opteed_smc_handler
);

/* Define an OPTEED runtime service descriptor for yielding SMC calls */
DECLARE_RT_SVC(
	opteed_std,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_YIELD,
	NULL,
	opteed_smc_handler
);
