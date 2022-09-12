/*
 * Copyright (c) 2022, Linaro.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#if MEASURED_BOOT
#include <common/desc_image_load.h>
#endif
#include <common/fdt_wrappers.h>
#include <platform_def.h>

#include <libfdt.h>

#ifdef SPD_opteed
/*
 * Currently OP-TEE does not support reading DTBs from Secure memory
 * and this property should be removed when this feature is supported.
 */
#define DTB_PROP_HW_SM_LOG_ADDR	"tpm_event_log_sm_addr"
#endif

#define DTB_PROP_HW_LOG_ADDR	"tpm_event_log_addr"
#define DTB_PROP_HW_LOG_SIZE    "tpm_event_log_size"

#if MEASURED_BOOT

#ifdef SPD_opteed
int qemu_set_tee_fw_info(uintptr_t config_base, uintptr_t log_addr,
			 size_t log_size)
{
	int offs, err = 0;
	void *dtb = (void *)config_base;
	const char *compatible = "arm,tpm_event_log";
	uint64_t sec_base = cpu_to_fdt64(log_addr);
	uint32_t sz = cpu_to_fdt32(log_size);

	offs = fdtw_find_or_add_subnode(dtb, 0, "tpm-event-log");
	if (offs < 0) {
		ERROR("Failed to add node tpm-event-log %d\n", offs);
		return offs;
	}

	if (fdt_appendprop(dtb, offs, "compatible", compatible,
			   strlen(compatible) + 1) < 0) {
		return -1;
	}

	err = fdt_setprop(dtb, offs, DTB_PROP_HW_SM_LOG_ADDR, &sec_base, 8);
	if (err < 0) {
		ERROR("Failed to add log addr err %d\n", err);
		return err;
	}

	err = fdt_setprop(dtb, offs, DTB_PROP_HW_LOG_SIZE, &sz, 4);
	if (err < 0) {
		ERROR("Failed to add log addr err %d\n", err);
		return err;
	}

	return err;
}
#endif

/*
 * Write the Event Log address and its size in the DTB.
 *
 * This function is supposed to be called only by BL2.
 *
 * Returns:
 *	0 = success
 *    < 0 = error
 */
static int qemu_set_event_log_info(uintptr_t config_base,
#ifdef SPD_opteed
				  uintptr_t sm_log_addr,
#endif
				  uintptr_t log_addr, size_t log_size)
{
	/* As libfdt uses void *, we can't avoid this cast */
	void *dtb = (void *)config_base;
	const char *compatible_tpm = "tcg,tpm-tis-mmio";
	uint64_t base = cpu_to_fdt64(log_addr);
	uint32_t sz = cpu_to_fdt32(log_size);
	int err, node;

	err = fdt_open_into(dtb, dtb, PLAT_QEMU_DT_MAX_SIZE);
	if (err < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", dtb, err);
		return err;
	}

	/*
	 * Verify that the DTB is valid, before attempting to write to it,
	 * and get the DTB root node.
	 */

	/* Check if the pointer to DT is correct */
	err = fdt_check_header(dtb);
	if (err < 0) {
		WARN("Invalid DTB file passed\n");
		return err;
	}

	/*
	 * Find the TPM node in device tree. On qemu, we assume it will
	 * be sw-tpm.
	 */
	node = fdt_node_offset_by_compatible(dtb, -1, compatible_tpm);
	if (node < 0) {
		ERROR("The compatible property '%s' not%s", compatible_tpm,
			" found in the config\n");
		return node;
	}

	err = fdt_setprop(dtb, node, DTB_PROP_HW_LOG_ADDR, &base, 8);
	if (err < 0) {
		ERROR("Failed to add log addr err %d\n", err);
		return err;
	}

	err = fdt_setprop(dtb, node, DTB_PROP_HW_LOG_SIZE, &sz, 4);
	if (err < 0) {
		ERROR("Failed to add log addr err %d\n", err);
		return err;
	}

#ifdef SPD_opteed
	err = qemu_set_tee_fw_info(config_base, sm_log_addr, log_size);
	if (err < 0) {
		ERROR("Failed to add tpm-event-node at %p: err %d\n", dtb, err);
		return err;
	}
#endif

	err = fdt_pack(dtb);
	if (err < 0) {
		ERROR("Failed to pack Device Tree at %p: err %d\n", dtb, err);
		return err;
	}

	/*
	 * Ensure that the info written to the DTB is visible
	 * to other images.
	 */
	flush_dcache_range(config_base, fdt_totalsize(dtb));

	return err;
}

/*
 * This function writes the Event Log address and its size
 * in the TOS_FW_CONFIG DTB.
 *
 * This function is supposed to be called only by BL2.
 *
 * Returns:
 *	0 = success
 *    < 0 = error
 */
int qemu_set_tos_fw_info(uintptr_t config_base, uintptr_t log_addr,
			size_t log_size)
{
	int err = 0;

	assert(config_base != 0UL);
	assert(log_addr != 0UL);

	/*
	 * FIXME - add code to add/update Log address and it's
	 * size in TOS FW CONFIG.
	 * For now we don't have support for TOS FW config in OP-TEE.
	 * So leave this function blank
	 */

	return err;
}

/*
 * This function writes the Event Log address and its size
 * in the QEMU DTB.
 *
 * This function is supposed to be called only by BL2.
 *
 * Returns:
 *	0 = success
 *    < 0 = error
 */
int qemu_set_nt_fw_info(
#ifdef SPD_opteed
			uintptr_t log_addr,
#endif
			size_t log_size, uintptr_t *ns_log_addr)
{
	uintptr_t ns_addr;
	int err;

	assert(ns_log_addr != NULL);

	ns_addr = PLAT_QEMU_DT_BASE + PLAT_QEMU_DT_MAX_SIZE;

	/* Write the Event Log address and its size in the DTB */
	err = qemu_set_event_log_info(PLAT_QEMU_DT_BASE,
#ifdef SPD_opteed
					log_addr,
#endif
					ns_addr, log_size);

	/* Return Event Log address in Non-secure memory */
	*ns_log_addr = (err < 0) ? 0UL : ns_addr;
	return err;
}
#endif /* MEASURED_BOOT */
