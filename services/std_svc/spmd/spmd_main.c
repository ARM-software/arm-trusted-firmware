/*
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <arch/aarch64/arch_features.h>
#include <bl31/bl31.h>
#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <common/tbbr/tbbr_img_def.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/el3_spmd_logical_sp.h>
#include <services/ffa_svc.h>
#include <services/spmc_svc.h>
#include <services/spmd_svc.h>
#include <smccc_helpers.h>
#include "spmd_private.h"

/*******************************************************************************
 * SPM Core context information.
 ******************************************************************************/
static spmd_spm_core_context_t spm_core_context[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * SPM Core attribute information is read from its manifest if the SPMC is not
 * at EL3. Else, it is populated from the SPMC directly.
 ******************************************************************************/
static spmc_manifest_attribute_t spmc_attrs;

/*******************************************************************************
 * SPM Core entry point information. Discovered on the primary core and reused
 * on secondary cores.
 ******************************************************************************/
static entry_point_info_t *spmc_ep_info;

/*******************************************************************************
 * SPM Core context on CPU based on mpidr.
 ******************************************************************************/
spmd_spm_core_context_t *spmd_get_context_by_mpidr(uint64_t mpidr)
{
	int core_idx = plat_core_pos_by_mpidr(mpidr);

	if (core_idx < 0) {
		ERROR("Invalid mpidr: %" PRIx64 ", returned ID: %d\n", mpidr, core_idx);
		panic();
	}

	return &spm_core_context[core_idx];
}

/*******************************************************************************
 * SPM Core context on current CPU get helper.
 ******************************************************************************/
spmd_spm_core_context_t *spmd_get_context(void)
{
	return spmd_get_context_by_mpidr(read_mpidr());
}

/*******************************************************************************
 * SPM Core ID getter.
 ******************************************************************************/
uint16_t spmd_spmc_id_get(void)
{
	return spmc_attrs.spmc_id;
}

/*******************************************************************************
 * Static function declaration.
 ******************************************************************************/
static int32_t spmd_init(void);
static int spmd_spmc_init(void *pm_addr);

static uint64_t spmd_smc_forward(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *cookie,
				 void *handle,
				 uint64_t flags);

/******************************************************************************
 * Builds an SPMD to SPMC direct message request.
 *****************************************************************************/
void spmd_build_spmc_message(gp_regs_t *gpregs, uint8_t target_func,
			     unsigned long long message)
{
	write_ctx_reg(gpregs, CTX_GPREG_X0, FFA_MSG_SEND_DIRECT_REQ_SMC32);
	write_ctx_reg(gpregs, CTX_GPREG_X1,
		(SPMD_DIRECT_MSG_ENDPOINT_ID << FFA_DIRECT_MSG_SOURCE_SHIFT) |
		 spmd_spmc_id_get());
	write_ctx_reg(gpregs, CTX_GPREG_X2, BIT(31) | target_func);
	write_ctx_reg(gpregs, CTX_GPREG_X3, message);

	/* Zero out x4-x7 for the direct request emitted towards the SPMC. */
	write_ctx_reg(gpregs, CTX_GPREG_X4, 0);
	write_ctx_reg(gpregs, CTX_GPREG_X5, 0);
	write_ctx_reg(gpregs, CTX_GPREG_X6, 0);
	write_ctx_reg(gpregs, CTX_GPREG_X7, 0);
}


/*******************************************************************************
 * This function takes an SPMC context pointer and performs a synchronous
 * SPMC entry.
 ******************************************************************************/
uint64_t spmd_spm_core_sync_entry(spmd_spm_core_context_t *spmc_ctx)
{
	uint64_t rc;

	assert(spmc_ctx != NULL);

	cm_set_context(&(spmc_ctx->cpu_ctx), SECURE);

	/* Restore the context assigned above */
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_restore(SECURE);
#else
	cm_el1_sysregs_context_restore(SECURE);
#endif
	cm_set_next_eret_context(SECURE);

	/* Enter SPMC */
	rc = spmd_spm_core_enter(&spmc_ctx->c_rt_ctx);

	/* Save secure state */
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_save(SECURE);
#else
	cm_el1_sysregs_context_save(SECURE);
#endif

	return rc;
}

/*******************************************************************************
 * This function returns to the place where spmd_spm_core_sync_entry() was
 * called originally.
 ******************************************************************************/
__dead2 void spmd_spm_core_sync_exit(uint64_t rc)
{
	spmd_spm_core_context_t *ctx = spmd_get_context();

	/* Get current CPU context from SPMC context */
	assert(cm_get_context(SECURE) == &(ctx->cpu_ctx));

	/*
	 * The SPMD must have initiated the original request through a
	 * synchronous entry into SPMC. Jump back to the original C runtime
	 * context with the value of rc in x0;
	 */
	spmd_spm_core_exit(ctx->c_rt_ctx, rc);

	panic();
}

/*******************************************************************************
 * Jump to the SPM Core for the first time.
 ******************************************************************************/
static int32_t spmd_init(void)
{
	spmd_spm_core_context_t *ctx = spmd_get_context();
	uint64_t rc;

	VERBOSE("SPM Core init start.\n");

	/* Primary boot core enters the SPMC for initialization. */
	ctx->state = SPMC_STATE_ON_PENDING;

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc != 0ULL) {
		ERROR("SPMC initialisation failed 0x%" PRIx64 "\n", rc);
		return 0;
	}

	ctx->state = SPMC_STATE_ON;

	VERBOSE("SPM Core init end.\n");

	spmd_logical_sp_set_spmc_initialized();
	rc = spmd_logical_sp_init();
	if (rc != 0) {
		WARN("SPMD Logical partitions failed init.\n");
	}

	return 1;
}

/*******************************************************************************
 * spmd_secure_interrupt_handler
 * Enter the SPMC for further handling of the secure interrupt by the SPMC
 * itself or a Secure Partition.
 ******************************************************************************/
static uint64_t spmd_secure_interrupt_handler(uint32_t id,
					      uint32_t flags,
					      void *handle,
					      void *cookie)
{
	spmd_spm_core_context_t *ctx = spmd_get_context();
	gp_regs_t *gpregs = get_gpregs_ctx(&ctx->cpu_ctx);
	unsigned int linear_id = plat_my_core_pos();
	int64_t rc;

	/* Sanity check the security state when the exception was generated */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

	/* Sanity check the pointer to this cpu's context */
	assert(handle == cm_get_context(NON_SECURE));

	/* Save the non-secure context before entering SPMC */
	cm_el1_sysregs_context_save(NON_SECURE);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_save(NON_SECURE);
#endif

	/* Convey the event to the SPMC through the FFA_INTERRUPT interface. */
	write_ctx_reg(gpregs, CTX_GPREG_X0, FFA_INTERRUPT);
	write_ctx_reg(gpregs, CTX_GPREG_X1, 0);
	write_ctx_reg(gpregs, CTX_GPREG_X2, 0);
	write_ctx_reg(gpregs, CTX_GPREG_X3, 0);
	write_ctx_reg(gpregs, CTX_GPREG_X4, 0);
	write_ctx_reg(gpregs, CTX_GPREG_X5, 0);
	write_ctx_reg(gpregs, CTX_GPREG_X6, 0);
	write_ctx_reg(gpregs, CTX_GPREG_X7, 0);

	/* Mark current core as handling a secure interrupt. */
	ctx->secure_interrupt_ongoing = true;

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc != 0ULL) {
		ERROR("%s failed (%" PRId64 ") on CPU%u\n", __func__, rc, linear_id);
	}

	ctx->secure_interrupt_ongoing = false;

	cm_el1_sysregs_context_restore(NON_SECURE);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_restore(NON_SECURE);
#endif
	cm_set_next_eret_context(NON_SECURE);

	SMC_RET0(&ctx->cpu_ctx);
}

#if (EL3_EXCEPTION_HANDLING == 0)
/*******************************************************************************
 * spmd_group0_interrupt_handler_nwd
 * Group0 secure interrupt in the normal world are trapped to EL3. Delegate the
 * handling of the interrupt to the platform handler, and return only upon
 * successfully handling the Group0 interrupt.
 ******************************************************************************/
static uint64_t spmd_group0_interrupt_handler_nwd(uint32_t id,
						  uint32_t flags,
						  void *handle,
						  void *cookie)
{
	uint32_t intid;

	/* Sanity check the security state when the exception was generated. */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

	/* Sanity check the pointer to this cpu's context. */
	assert(handle == cm_get_context(NON_SECURE));

	assert(id == INTR_ID_UNAVAILABLE);

	assert(plat_ic_get_pending_interrupt_type() == INTR_TYPE_EL3);

	intid = plat_ic_acknowledge_interrupt();

	if (plat_spmd_handle_group0_interrupt(intid) < 0) {
		ERROR("Group0 interrupt %u not handled\n", intid);
		panic();
	}

	/* Deactivate the corresponding Group0 interrupt. */
	plat_ic_end_of_interrupt(intid);

	return 0U;
}
#endif

/*******************************************************************************
 * spmd_handle_group0_intr_swd
 * SPMC delegates handling of Group0 secure interrupt to EL3 firmware using
 * FFA_EL3_INTR_HANDLE SMC call. Further, SPMD delegates the handling of the
 * interrupt to the platform handler, and returns only upon successfully
 * handling the Group0 interrupt.
 ******************************************************************************/
static uint64_t spmd_handle_group0_intr_swd(void *handle)
{
	uint32_t intid;

	/* Sanity check the pointer to this cpu's context */
	assert(handle == cm_get_context(SECURE));

	assert(plat_ic_get_pending_interrupt_type() == INTR_TYPE_EL3);

	intid = plat_ic_acknowledge_interrupt();

	/*
	 * TODO: Currently due to a limitation in SPMD implementation, the
	 * platform handler is expected to not delegate handling to NWd while
	 * processing Group0 secure interrupt.
	 */
	if (plat_spmd_handle_group0_interrupt(intid) < 0) {
		/* Group0 interrupt was not handled by the platform. */
		ERROR("Group0 interrupt %u not handled\n", intid);
		panic();
	}

	/* Deactivate the corresponding Group0 interrupt. */
	plat_ic_end_of_interrupt(intid);

	/* Return success. */
	SMC_RET8(handle, FFA_SUCCESS_SMC32, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		 FFA_PARAM_MBZ);
}

#if ENABLE_RME && SPMD_SPM_AT_SEL2 && !RESET_TO_BL31
static int spmd_dynamic_map_mem(uintptr_t base_addr, size_t size,
				 unsigned int attr, uintptr_t *align_addr,
				 size_t *align_size)
{
	uintptr_t base_addr_align;
	size_t mapped_size_align;
	int rc;

	/* Page aligned address and size if necessary */
	base_addr_align = page_align(base_addr, DOWN);
	mapped_size_align = page_align(size, UP);

	if ((base_addr != base_addr_align) &&
	    (size == mapped_size_align)) {
		mapped_size_align += PAGE_SIZE;
	}

	/*
	 * Map dynamically given region with its aligned base address and
	 * size
	 */
	rc = mmap_add_dynamic_region((unsigned long long)base_addr_align,
				     base_addr_align,
				     mapped_size_align,
				     attr);
	if (rc == 0) {
		*align_addr = base_addr_align;
		*align_size = mapped_size_align;
	}

	return rc;
}

static void spmd_do_sec_cpy(uintptr_t root_base_addr, uintptr_t sec_base_addr,
			    size_t size)
{
	uintptr_t root_base_addr_align, sec_base_addr_align;
	size_t root_mapped_size_align, sec_mapped_size_align;
	int rc;

	assert(root_base_addr != 0UL);
	assert(sec_base_addr != 0UL);
	assert(size != 0UL);

	/* Map the memory with required attributes */
	rc = spmd_dynamic_map_mem(root_base_addr, size, MT_RO_DATA | MT_ROOT,
				  &root_base_addr_align,
				  &root_mapped_size_align);
	if (rc != 0) {
		ERROR("%s %s %lu (%d)\n", "Error while mapping", "root region",
		      root_base_addr, rc);
		panic();
	}

	rc = spmd_dynamic_map_mem(sec_base_addr, size, MT_RW_DATA | MT_SECURE,
				  &sec_base_addr_align, &sec_mapped_size_align);
	if (rc != 0) {
		ERROR("%s %s %lu (%d)\n", "Error while mapping",
		      "secure region", sec_base_addr, rc);
		panic();
	}

	/* Do copy operation */
	(void)memcpy((void *)sec_base_addr, (void *)root_base_addr, size);

	/* Unmap root memory region */
	rc = mmap_remove_dynamic_region(root_base_addr_align,
					root_mapped_size_align);
	if (rc != 0) {
		ERROR("%s %s %lu (%d)\n", "Error while unmapping",
		      "root region", root_base_addr_align, rc);
		panic();
	}

	/* Unmap secure memory region */
	rc = mmap_remove_dynamic_region(sec_base_addr_align,
					sec_mapped_size_align);
	if (rc != 0) {
		ERROR("%s %s %lu (%d)\n", "Error while unmapping",
		      "secure region", sec_base_addr_align, rc);
		panic();
	}
}
#endif /* ENABLE_RME && SPMD_SPM_AT_SEL2 && !RESET_TO_BL31 */

/*******************************************************************************
 * Loads SPMC manifest and inits SPMC.
 ******************************************************************************/
static int spmd_spmc_init(void *pm_addr)
{
	cpu_context_t *cpu_ctx;
	unsigned int core_id;
	uint32_t ep_attr, flags;
	int rc;
	const struct dyn_cfg_dtb_info_t *image_info __unused;

	/* Load the SPM Core manifest */
	rc = plat_spm_core_manifest_load(&spmc_attrs, pm_addr);
	if (rc != 0) {
		WARN("No or invalid SPM Core manifest image provided by BL2\n");
		return rc;
	}

	/*
	 * Ensure that the SPM Core version is compatible with the SPM
	 * Dispatcher version.
	 */
	if ((spmc_attrs.major_version != FFA_VERSION_MAJOR) ||
	    (spmc_attrs.minor_version > FFA_VERSION_MINOR)) {
		WARN("Unsupported FFA version (%u.%u)\n",
		     spmc_attrs.major_version, spmc_attrs.minor_version);
		return -EINVAL;
	}

	VERBOSE("FFA version (%u.%u)\n", spmc_attrs.major_version,
	     spmc_attrs.minor_version);

	VERBOSE("SPM Core run time EL%x.\n",
	     SPMD_SPM_AT_SEL2 ? MODE_EL2 : MODE_EL1);

	/* Validate the SPMC ID, Ensure high bit is set */
	if (((spmc_attrs.spmc_id >> SPMC_SECURE_ID_SHIFT) &
			SPMC_SECURE_ID_MASK) == 0U) {
		WARN("Invalid ID (0x%x) for SPMC.\n", spmc_attrs.spmc_id);
		return -EINVAL;
	}

	/* Validate the SPM Core execution state */
	if ((spmc_attrs.exec_state != MODE_RW_64) &&
	    (spmc_attrs.exec_state != MODE_RW_32)) {
		WARN("Unsupported %s%x.\n", "SPM Core execution state 0x",
		     spmc_attrs.exec_state);
		return -EINVAL;
	}

	VERBOSE("%s%x.\n", "SPM Core execution state 0x",
		spmc_attrs.exec_state);

#if SPMD_SPM_AT_SEL2
	/* Ensure manifest has not requested AArch32 state in S-EL2 */
	if (spmc_attrs.exec_state == MODE_RW_32) {
		WARN("AArch32 state at S-EL2 is not supported.\n");
		return -EINVAL;
	}

	/*
	 * Check if S-EL2 is supported on this system if S-EL2
	 * is required for SPM
	 */
	if (!is_feat_sel2_supported()) {
		WARN("SPM Core run time S-EL2 is not supported.\n");
		return -EINVAL;
	}
#endif /* SPMD_SPM_AT_SEL2 */

	/* Initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if ((read_sctlr_el3() & SCTLR_EE_BIT) != 0ULL) {
		ep_attr |= EP_EE_BIG;
	}

	SET_PARAM_HEAD(spmc_ep_info, PARAM_EP, VERSION_1, ep_attr);

	/*
	 * Populate SPSR for SPM Core based upon validated parameters from the
	 * manifest.
	 */
	if (spmc_attrs.exec_state == MODE_RW_32) {
		spmc_ep_info->spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
						 SPSR_E_LITTLE,
						 DAIF_FIQ_BIT |
						 DAIF_IRQ_BIT |
						 DAIF_ABT_BIT);
	} else {

#if SPMD_SPM_AT_SEL2
		static const uint32_t runtime_el = MODE_EL2;
#else
		static const uint32_t runtime_el = MODE_EL1;
#endif
		spmc_ep_info->spsr = SPSR_64(runtime_el,
					     MODE_SP_ELX,
					     DISABLE_ALL_EXCEPTIONS);
	}

#if ENABLE_RME && SPMD_SPM_AT_SEL2 && !RESET_TO_BL31
	image_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, TOS_FW_CONFIG_ID);
	assert(image_info != NULL);

	if ((image_info->config_addr == 0UL) ||
	    (image_info->secondary_config_addr == 0UL) ||
	    (image_info->config_max_size == 0UL)) {
		return -EINVAL;
	}

	/* Copy manifest from root->secure region */
	spmd_do_sec_cpy(image_info->config_addr,
			image_info->secondary_config_addr,
			image_info->config_max_size);

	/* Update ep info of BL32 */
	assert(spmc_ep_info != NULL);
	spmc_ep_info->args.arg0 = image_info->secondary_config_addr;
#endif /* ENABLE_RME && SPMD_SPM_AT_SEL2 && !RESET_TO_BL31 */

	/* Set an initial SPMC context state for all cores. */
	for (core_id = 0U; core_id < PLATFORM_CORE_COUNT; core_id++) {
		spm_core_context[core_id].state = SPMC_STATE_OFF;

		/* Setup an initial cpu context for the SPMC. */
		cpu_ctx = &spm_core_context[core_id].cpu_ctx;
		cm_setup_context(cpu_ctx, spmc_ep_info);

		/*
		 * Pass the core linear ID to the SPMC through x4.
		 * (TF-A implementation defined behavior helping
		 * a legacy TOS migration to adopt FF-A).
		 */
		write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X4, core_id);
	}

	/* Register power management hooks with PSCI */
	psci_register_spd_pm_hook(&spmd_pm);

	/* Register init function for deferred init. */
	bl31_register_bl32_init(&spmd_init);

	INFO("SPM Core setup done.\n");

	/*
	 * Register an interrupt handler routing secure interrupts to SPMD
	 * while the NWd is running.
	 */
	flags = 0;
	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
					     spmd_secure_interrupt_handler,
					     flags);
	if (rc != 0) {
		panic();
	}

	/*
	 * Permit configurations where the SPM resides at S-EL1/2 and upon a
	 * Group0 interrupt triggering while the normal world runs, the
	 * interrupt is routed either through the EHF or directly to the SPMD:
	 *
	 * EL3_EXCEPTION_HANDLING=0: the Group0 interrupt is routed to the SPMD
	 *                   for handling by spmd_group0_interrupt_handler_nwd.
	 *
	 * EL3_EXCEPTION_HANDLING=1: the Group0 interrupt is routed to the EHF.
	 *
	 */
#if (EL3_EXCEPTION_HANDLING == 0)
	/*
	 * Register an interrupt handler routing Group0 interrupts to SPMD
	 * while the NWd is running.
	 */
	rc = register_interrupt_type_handler(INTR_TYPE_EL3,
					     spmd_group0_interrupt_handler_nwd,
					     flags);
	if (rc != 0) {
		panic();
	}
#endif

	return 0;
}

/*******************************************************************************
 * Initialize context of SPM Core.
 ******************************************************************************/
int spmd_setup(void)
{
	int rc;
	void *spmc_manifest;

	/*
	 * If the SPMC is at EL3, then just initialise it directly. The
	 * shenanigans of when it is at a lower EL are not needed.
	 */
	if (is_spmc_at_el3()) {
		/* Allow the SPMC to populate its attributes directly. */
		spmc_populate_attrs(&spmc_attrs);

		rc = spmc_setup();
		if (rc != 0) {
			WARN("SPMC initialisation failed 0x%x.\n", rc);
		}
		return 0;
	}

	spmc_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (spmc_ep_info == NULL) {
		WARN("No SPM Core image provided by BL2 boot loader.\n");
		return 0;
	}

	/* Under no circumstances will this parameter be 0 */
	assert(spmc_ep_info->pc != 0ULL);

	/*
	 * Check if BL32 ep_info has a reference to 'tos_fw_config'. This will
	 * be used as a manifest for the SPM Core at the next lower EL/mode.
	 */
	spmc_manifest = (void *)spmc_ep_info->args.arg0;
	if (spmc_manifest == NULL) {
		WARN("Invalid or absent SPM Core manifest.\n");
		return 0;
	}

	/* Load manifest, init SPMC */
	rc = spmd_spmc_init(spmc_manifest);
	if (rc != 0) {
		WARN("Booting device without SPM initialization.\n");
	}

	return 0;
}

/*******************************************************************************
 * Forward FF-A SMCs to the other security state.
 ******************************************************************************/
uint64_t spmd_smc_switch_state(uint32_t smc_fid,
			       bool secure_origin,
			       uint64_t x1,
			       uint64_t x2,
			       uint64_t x3,
			       uint64_t x4,
			       void *handle)
{
	unsigned int secure_state_in = (secure_origin) ? SECURE : NON_SECURE;
	unsigned int secure_state_out = (!secure_origin) ? SECURE : NON_SECURE;

	/* Save incoming security state */
#if SPMD_SPM_AT_SEL2
	if (secure_state_in == NON_SECURE) {
		cm_el1_sysregs_context_save(secure_state_in);
	}
	cm_el2_sysregs_context_save(secure_state_in);
#else
	cm_el1_sysregs_context_save(secure_state_in);
#endif

	/* Restore outgoing security state */
#if SPMD_SPM_AT_SEL2
	if (secure_state_out == NON_SECURE) {
		cm_el1_sysregs_context_restore(secure_state_out);
	}
	cm_el2_sysregs_context_restore(secure_state_out);
#else
	cm_el1_sysregs_context_restore(secure_state_out);
#endif
	cm_set_next_eret_context(secure_state_out);

#if SPMD_SPM_AT_SEL2
	/*
	 * If SPMC is at SEL2, save additional registers x8-x17, which may
	 * be used in FF-A calls such as FFA_PARTITION_INFO_GET_REGS.
	 * Note that technically, all SPMCs can support this, but this code is
	 * under ifdef to minimize breakage in case other SPMCs do not save
	 * and restore x8-x17.
	 * We also need to pass through these registers since not all FF-A ABIs
	 * modify x8-x17, in which case, SMCCC requires that these registers be
	 * preserved, so the SPMD passes through these registers and expects the
	 * SPMC to save and restore (potentially also modify) them.
	 */
	SMC_RET18(cm_get_context(secure_state_out), smc_fid, x1, x2, x3, x4,
			SMC_GET_GP(handle, CTX_GPREG_X5),
			SMC_GET_GP(handle, CTX_GPREG_X6),
			SMC_GET_GP(handle, CTX_GPREG_X7),
			SMC_GET_GP(handle, CTX_GPREG_X8),
			SMC_GET_GP(handle, CTX_GPREG_X9),
			SMC_GET_GP(handle, CTX_GPREG_X10),
			SMC_GET_GP(handle, CTX_GPREG_X11),
			SMC_GET_GP(handle, CTX_GPREG_X12),
			SMC_GET_GP(handle, CTX_GPREG_X13),
			SMC_GET_GP(handle, CTX_GPREG_X14),
			SMC_GET_GP(handle, CTX_GPREG_X15),
			SMC_GET_GP(handle, CTX_GPREG_X16),
			SMC_GET_GP(handle, CTX_GPREG_X17)
			);

#else
	SMC_RET8(cm_get_context(secure_state_out), smc_fid, x1, x2, x3, x4,
			SMC_GET_GP(handle, CTX_GPREG_X5),
			SMC_GET_GP(handle, CTX_GPREG_X6),
			SMC_GET_GP(handle, CTX_GPREG_X7));
#endif
}

/*******************************************************************************
 * Forward SMCs to the other security state.
 ******************************************************************************/
static uint64_t spmd_smc_forward(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *cookie,
				 void *handle,
				 uint64_t flags)
{
	if (is_spmc_at_el3() && !secure_origin) {
		return spmc_smc_handler(smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);
	}
	return spmd_smc_switch_state(smc_fid, secure_origin, x1, x2, x3, x4,
				     handle);

}

/*******************************************************************************
 * Return FFA_ERROR with specified error code
 ******************************************************************************/
uint64_t spmd_ffa_error_return(void *handle, int error_code)
{
	SMC_RET8(handle, (uint32_t) FFA_ERROR,
		 FFA_TARGET_INFO_MBZ, (uint32_t)error_code,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ);
}

/*******************************************************************************
 * spmd_check_address_in_binary_image
 ******************************************************************************/
bool spmd_check_address_in_binary_image(uint64_t address)
{
	assert(!check_uptr_overflow(spmc_attrs.load_address, spmc_attrs.binary_size));

	return ((address >= spmc_attrs.load_address) &&
		(address < (spmc_attrs.load_address + spmc_attrs.binary_size)));
}

/******************************************************************************
 * spmd_is_spmc_message
 *****************************************************************************/
static bool spmd_is_spmc_message(unsigned int ep)
{
	if (is_spmc_at_el3()) {
		return false;
	}

	return ((ffa_endpoint_destination(ep) == SPMD_DIRECT_MSG_ENDPOINT_ID)
		&& (ffa_endpoint_source(ep) == spmc_attrs.spmc_id));
}

/******************************************************************************
 * spmd_handle_spmc_message
 *****************************************************************************/
static int spmd_handle_spmc_message(unsigned long long msg,
		unsigned long long parm1, unsigned long long parm2,
		unsigned long long parm3, unsigned long long parm4)
{
	VERBOSE("%s %llx %llx %llx %llx %llx\n", __func__,
		msg, parm1, parm2, parm3, parm4);

	return -EINVAL;
}

/*******************************************************************************
 * This function forwards FF-A SMCs to either the main SPMD handler or the
 * SPMC at EL3, depending on the origin security state, if enabled.
 ******************************************************************************/
uint64_t spmd_ffa_smc_handler(uint32_t smc_fid,
			      uint64_t x1,
			      uint64_t x2,
			      uint64_t x3,
			      uint64_t x4,
			      void *cookie,
			      void *handle,
			      uint64_t flags)
{
	if (is_spmc_at_el3()) {
		/*
		 * If we have an SPMC at EL3 allow handling of the SMC first.
		 * The SPMC will call back through to SPMD handler if required.
		 */
		if (is_caller_secure(flags)) {
			return spmc_smc_handler(smc_fid,
						is_caller_secure(flags),
						x1, x2, x3, x4, cookie,
						handle, flags);
		}
	}
	return spmd_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
				handle, flags);
}

/*******************************************************************************
 * This function handles all SMCs in the range reserved for FFA. Each call is
 * either forwarded to the other security state or handled by the SPM dispatcher
 ******************************************************************************/
uint64_t spmd_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	unsigned int linear_id = plat_my_core_pos();
	spmd_spm_core_context_t *ctx = spmd_get_context();
	bool secure_origin;
	int32_t ret;
	uint32_t input_version;

	/* Determine which security state this SMC originated from */
	secure_origin = is_caller_secure(flags);

	VERBOSE("SPM(%u): 0x%x 0x%" PRIx64 " 0x%" PRIx64 " 0x%" PRIx64 " 0x%" PRIx64
		" 0x%" PRIx64 " 0x%" PRIx64 " 0x%" PRIx64 "\n",
		    linear_id, smc_fid, x1, x2, x3, x4,
		    SMC_GET_GP(handle, CTX_GPREG_X5),
		    SMC_GET_GP(handle, CTX_GPREG_X6),
		    SMC_GET_GP(handle, CTX_GPREG_X7));

	/*
	 * If there is an on-going info regs from EL3 SPMD LP, unconditionally
	 * return, we don't expect any other FF-A ABIs to be called between
	 * calls to FFA_PARTITION_INFO_GET_REGS.
	 */
	if (is_spmd_logical_sp_info_regs_req_in_progress(ctx)) {
		assert(secure_origin);
		spmd_spm_core_sync_exit(0ULL);
	}

	switch (smc_fid) {
	case FFA_ERROR:
		/*
		 * Check if this is the first invocation of this interface on
		 * this CPU. If so, then indicate that the SPM Core initialised
		 * unsuccessfully.
		 */
		if (secure_origin && (ctx->state == SPMC_STATE_ON_PENDING)) {
			spmd_spm_core_sync_exit(x2);
		}

		/*
		 * If there was an SPMD logical partition direct request on-going,
		 * return back to the SPMD logical partition so the error can be
		 * consumed.
		 */
		if (is_spmd_logical_sp_dir_req_in_progress(ctx)) {
			assert(secure_origin);
			spmd_spm_core_sync_exit(0ULL);
		}

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, cookie,
					handle, flags);
		break; /* not reached */

	case FFA_VERSION:
		input_version = (uint32_t)(0xFFFFFFFF & x1);
		/*
		 * If caller is secure and SPMC was initialized,
		 * return FFA_VERSION of SPMD.
		 * If caller is non secure and SPMC was initialized,
		 * forward to the EL3 SPMC if enabled, otherwise return
		 * the SPMC version if implemented at a lower EL.
		 * Sanity check to "input_version".
		 * If the EL3 SPMC is enabled, ignore the SPMC state as
		 * this is not used.
		 */
		if ((input_version & FFA_VERSION_BIT31_MASK) ||
		    (!is_spmc_at_el3() && (ctx->state == SPMC_STATE_RESET))) {
			ret = FFA_ERROR_NOT_SUPPORTED;
		} else if (!secure_origin) {
			if (is_spmc_at_el3()) {
				/*
				 * Forward the call directly to the EL3 SPMC, if
				 * enabled, as we don't need to wrap the call in
				 * a direct request.
				 */
				return spmd_smc_forward(smc_fid, secure_origin,
							x1, x2, x3, x4, cookie,
							handle, flags);
			}

			gp_regs_t *gpregs = get_gpregs_ctx(&ctx->cpu_ctx);
			uint64_t rc;

			if (spmc_attrs.major_version == 1 &&
			    spmc_attrs.minor_version == 0) {
				ret = MAKE_FFA_VERSION(spmc_attrs.major_version,
						       spmc_attrs.minor_version);
				SMC_RET8(handle, (uint32_t)ret,
					 FFA_TARGET_INFO_MBZ,
					 FFA_TARGET_INFO_MBZ,
					 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					 FFA_PARAM_MBZ);
				break;
			}
			/* Save non-secure system registers context */
			cm_el1_sysregs_context_save(NON_SECURE);
#if SPMD_SPM_AT_SEL2
			cm_el2_sysregs_context_save(NON_SECURE);
#endif

			/*
			 * The incoming request has FFA_VERSION as X0 smc_fid
			 * and requested version in x1. Prepare a direct request
			 * from SPMD to SPMC with FFA_VERSION framework function
			 * identifier in X2 and requested version in X3.
			 */
			spmd_build_spmc_message(gpregs,
						SPMD_FWK_MSG_FFA_VERSION_REQ,
						input_version);

			/*
			 * Ensure x8-x17 NS GP register values are untouched when returning
			 * from the SPMC.
			 */
			write_ctx_reg(gpregs, CTX_GPREG_X8, SMC_GET_GP(handle, CTX_GPREG_X8));
			write_ctx_reg(gpregs, CTX_GPREG_X9, SMC_GET_GP(handle, CTX_GPREG_X9));
			write_ctx_reg(gpregs, CTX_GPREG_X10, SMC_GET_GP(handle, CTX_GPREG_X10));
			write_ctx_reg(gpregs, CTX_GPREG_X11, SMC_GET_GP(handle, CTX_GPREG_X11));
			write_ctx_reg(gpregs, CTX_GPREG_X12, SMC_GET_GP(handle, CTX_GPREG_X12));
			write_ctx_reg(gpregs, CTX_GPREG_X13, SMC_GET_GP(handle, CTX_GPREG_X13));
			write_ctx_reg(gpregs, CTX_GPREG_X14, SMC_GET_GP(handle, CTX_GPREG_X14));
			write_ctx_reg(gpregs, CTX_GPREG_X15, SMC_GET_GP(handle, CTX_GPREG_X15));
			write_ctx_reg(gpregs, CTX_GPREG_X16, SMC_GET_GP(handle, CTX_GPREG_X16));
			write_ctx_reg(gpregs, CTX_GPREG_X17, SMC_GET_GP(handle, CTX_GPREG_X17));

			rc = spmd_spm_core_sync_entry(ctx);

			if ((rc != 0ULL) ||
			    (SMC_GET_GP(gpregs, CTX_GPREG_X0) !=
				FFA_MSG_SEND_DIRECT_RESP_SMC32) ||
			    (SMC_GET_GP(gpregs, CTX_GPREG_X2) !=
				(FFA_FWK_MSG_BIT |
				 SPMD_FWK_MSG_FFA_VERSION_RESP))) {
				ERROR("Failed to forward FFA_VERSION\n");
				ret = FFA_ERROR_NOT_SUPPORTED;
			} else {
				ret = SMC_GET_GP(gpregs, CTX_GPREG_X3);
			}

			/*
			 * x0-x4 are updated by spmd_smc_forward below.
			 * Zero out x5-x7 in the FFA_VERSION response.
			 */
			write_ctx_reg(gpregs, CTX_GPREG_X5, 0);
			write_ctx_reg(gpregs, CTX_GPREG_X6, 0);
			write_ctx_reg(gpregs, CTX_GPREG_X7, 0);

			/*
			 * Return here after SPMC has handled FFA_VERSION.
			 * The returned SPMC version is held in X3.
			 * Forward this version in X0 to the non-secure caller.
			 */
			return spmd_smc_forward(ret, true, FFA_PARAM_MBZ,
						FFA_PARAM_MBZ, FFA_PARAM_MBZ,
						FFA_PARAM_MBZ, cookie, gpregs,
						flags);
		} else {
			ret = MAKE_FFA_VERSION(FFA_VERSION_MAJOR,
					       FFA_VERSION_MINOR);
		}

		SMC_RET8(handle, (uint32_t)ret, FFA_TARGET_INFO_MBZ,
			 FFA_TARGET_INFO_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
			 FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ);
		break; /* not reached */

	case FFA_FEATURES:
		/*
		 * This is an optional interface. Do the minimal checks and
		 * forward to SPM Core which will handle it if implemented.
		 */

		/* Forward SMC from Normal world to the SPM Core */
		if (!secure_origin) {
			return spmd_smc_forward(smc_fid, secure_origin,
						x1, x2, x3, x4, cookie,
						handle, flags);
		}

		/*
		 * Return success if call was from secure world i.e. all
		 * FFA functions are supported. This is essentially a
		 * nop.
		 */
		SMC_RET8(handle, FFA_SUCCESS_SMC32, x1, x2, x3, x4,
			 SMC_GET_GP(handle, CTX_GPREG_X5),
			 SMC_GET_GP(handle, CTX_GPREG_X6),
			 SMC_GET_GP(handle, CTX_GPREG_X7));

		break; /* not reached */

	case FFA_ID_GET:
		/*
		 * Returns the ID of the calling FFA component.
		 */
		if (!secure_origin) {
			SMC_RET8(handle, FFA_SUCCESS_SMC32,
				 FFA_TARGET_INFO_MBZ, FFA_NS_ENDPOINT_ID,
				 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				 FFA_PARAM_MBZ);
		}

		SMC_RET8(handle, FFA_SUCCESS_SMC32,
			 FFA_TARGET_INFO_MBZ, spmc_attrs.spmc_id,
			 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
			 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
			 FFA_PARAM_MBZ);

		break; /* not reached */

	case FFA_SECONDARY_EP_REGISTER_SMC64:
		if (secure_origin) {
			ret = spmd_pm_secondary_ep_register(x1);

			if (ret < 0) {
				SMC_RET8(handle, FFA_ERROR_SMC64,
					FFA_TARGET_INFO_MBZ, ret,
					FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ);
			} else {
				SMC_RET8(handle, FFA_SUCCESS_SMC64,
					FFA_TARGET_INFO_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ);
			}
		}

		return spmd_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
		break; /* Not reached */

	case FFA_SPM_ID_GET:
		if (MAKE_FFA_VERSION(1, 1) > FFA_VERSION_COMPILED) {
			return spmd_ffa_error_return(handle,
						     FFA_ERROR_NOT_SUPPORTED);
		}
		/*
		 * Returns the ID of the SPMC or SPMD depending on the FF-A
		 * instance where this function is invoked
		 */
		if (!secure_origin) {
			SMC_RET8(handle, FFA_SUCCESS_SMC32,
				 FFA_TARGET_INFO_MBZ, spmc_attrs.spmc_id,
				 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				 FFA_PARAM_MBZ);
		}
		SMC_RET8(handle, FFA_SUCCESS_SMC32,
			 FFA_TARGET_INFO_MBZ, SPMD_DIRECT_MSG_ENDPOINT_ID,
			 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
			 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
			 FFA_PARAM_MBZ);

		break; /* not reached */

	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
	case FFA_MSG_SEND_DIRECT_REQ_SMC64:
		/*
		 * Regardless of secure_origin, SPMD logical partitions cannot
		 * handle direct messages. They can only initiate direct
		 * messages and consume direct responses or errors.
		 */
		if (is_spmd_lp_id(ffa_endpoint_source(x1)) ||
				  is_spmd_lp_id(ffa_endpoint_destination(x1))) {
			return spmd_ffa_error_return(handle,
						     FFA_ERROR_INVALID_PARAMETER
						     );
		}

		/*
		 * When there is an ongoing SPMD logical partition direct
		 * request, there cannot be another direct request. Return
		 * error in this case. Panic'ing is an option but that does
		 * not provide the opportunity for caller to abort based on
		 * error codes.
		 */
		if (is_spmd_logical_sp_dir_req_in_progress(ctx)) {
			assert(secure_origin);
			return spmd_ffa_error_return(handle,
						     FFA_ERROR_DENIED);
		}

		if (!secure_origin) {
			/* Validate source endpoint is non-secure for non-secure caller. */
			if (ffa_is_secure_world_id(ffa_endpoint_source(x1))) {
				return spmd_ffa_error_return(handle,
						FFA_ERROR_INVALID_PARAMETER);
			}
		}
		if (secure_origin && spmd_is_spmc_message(x1)) {
			ret = spmd_handle_spmc_message(x3, x4,
				SMC_GET_GP(handle, CTX_GPREG_X5),
				SMC_GET_GP(handle, CTX_GPREG_X6),
				SMC_GET_GP(handle, CTX_GPREG_X7));

			SMC_RET8(handle, FFA_SUCCESS_SMC32,
				FFA_TARGET_INFO_MBZ, ret,
				FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				FFA_PARAM_MBZ);
		} else {
			/* Forward direct message to the other world */
			return spmd_smc_forward(smc_fid, secure_origin,
						x1, x2, x3, x4, cookie,
						handle, flags);
		}
		break; /* Not reached */

	case FFA_MSG_SEND_DIRECT_RESP_SMC32:
	case FFA_MSG_SEND_DIRECT_RESP_SMC64:
		if (secure_origin && (spmd_is_spmc_message(x1) ||
		    is_spmd_logical_sp_dir_req_in_progress(ctx))) {
			spmd_spm_core_sync_exit(0ULL);
		} else {
			/* Forward direct message to the other world */
			return spmd_smc_forward(smc_fid, secure_origin,
						x1, x2, x3, x4, cookie,
						handle, flags);
		}
		break; /* Not reached */

	case FFA_RX_RELEASE:
	case FFA_RXTX_MAP_SMC32:
	case FFA_RXTX_MAP_SMC64:
	case FFA_RXTX_UNMAP:
	case FFA_PARTITION_INFO_GET:
#if MAKE_FFA_VERSION(1, 1) <= FFA_VERSION_COMPILED
	case FFA_NOTIFICATION_BITMAP_CREATE:
	case FFA_NOTIFICATION_BITMAP_DESTROY:
	case FFA_NOTIFICATION_BIND:
	case FFA_NOTIFICATION_UNBIND:
	case FFA_NOTIFICATION_SET:
	case FFA_NOTIFICATION_GET:
	case FFA_NOTIFICATION_INFO_GET:
	case FFA_NOTIFICATION_INFO_GET_SMC64:
	case FFA_MSG_SEND2:
	case FFA_RX_ACQUIRE:
#endif
	case FFA_MSG_RUN:
		/*
		 * Above calls should be invoked only by the Normal world and
		 * must not be forwarded from Secure world to Normal world.
		 */
		if (secure_origin) {
			return spmd_ffa_error_return(handle,
						     FFA_ERROR_NOT_SUPPORTED);
		}

		/* Forward the call to the other world */
		/* fallthrough */
	case FFA_MSG_SEND:
	case FFA_MEM_DONATE_SMC32:
	case FFA_MEM_DONATE_SMC64:
	case FFA_MEM_LEND_SMC32:
	case FFA_MEM_LEND_SMC64:
	case FFA_MEM_SHARE_SMC32:
	case FFA_MEM_SHARE_SMC64:
	case FFA_MEM_RETRIEVE_REQ_SMC32:
	case FFA_MEM_RETRIEVE_REQ_SMC64:
	case FFA_MEM_RETRIEVE_RESP:
	case FFA_MEM_RELINQUISH:
	case FFA_MEM_RECLAIM:
	case FFA_MEM_FRAG_TX:
	case FFA_MEM_FRAG_RX:
	case FFA_SUCCESS_SMC32:
	case FFA_SUCCESS_SMC64:
		/*
		 * If there is an ongoing direct request from an SPMD logical
		 * partition, return an error.
		 */
		if (is_spmd_logical_sp_dir_req_in_progress(ctx)) {
			assert(secure_origin);
			return spmd_ffa_error_return(handle,
					FFA_ERROR_DENIED);
		}

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, cookie,
					handle, flags);
		break; /* not reached */

	case FFA_MSG_WAIT:
		/*
		 * Check if this is the first invocation of this interface on
		 * this CPU from the Secure world. If so, then indicate that the
		 * SPM Core initialised successfully.
		 */
		if (secure_origin && (ctx->state == SPMC_STATE_ON_PENDING)) {
			spmd_spm_core_sync_exit(0ULL);
		}

		/* Forward the call to the other world */
		/* fallthrough */
	case FFA_INTERRUPT:
	case FFA_MSG_YIELD:
		/* This interface must be invoked only by the Secure world */
		if (!secure_origin) {
			return spmd_ffa_error_return(handle,
						      FFA_ERROR_NOT_SUPPORTED);
		}

		if (is_spmd_logical_sp_dir_req_in_progress(ctx)) {
			assert(secure_origin);
			return spmd_ffa_error_return(handle,
					FFA_ERROR_DENIED);
		}

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, cookie,
					handle, flags);
		break; /* not reached */

	case FFA_NORMAL_WORLD_RESUME:
		if (secure_origin && ctx->secure_interrupt_ongoing) {
			spmd_spm_core_sync_exit(0ULL);
		} else {
			return spmd_ffa_error_return(handle, FFA_ERROR_DENIED);
		}
		break; /* Not reached */
#if MAKE_FFA_VERSION(1, 1) <= FFA_VERSION_COMPILED
	case FFA_PARTITION_INFO_GET_REGS_SMC64:
		if (secure_origin) {
			return spmd_el3_populate_logical_partition_info(handle, x1,
								   x2, x3);
		}

		/* Call only supported with SMCCC 1.2+ */
		if (MAKE_SMCCC_VERSION(SMCCC_MAJOR_VERSION, SMCCC_MINOR_VERSION) < 0x10002) {
			return spmd_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
		}

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, cookie,
					handle, flags);
		break; /* Not reached */
#endif
	case FFA_EL3_INTR_HANDLE:
		if (secure_origin) {
			return spmd_handle_group0_intr_swd(handle);
		} else {
			return spmd_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
		}
	default:
		WARN("SPM: Unsupported call 0x%08x\n", smc_fid);
		return spmd_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}
}
