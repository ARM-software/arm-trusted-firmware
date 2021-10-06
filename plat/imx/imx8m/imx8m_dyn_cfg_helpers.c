/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2022, Linaro.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#if MEASURED_BOOT
#include <common/desc_image_load.h>
#endif
#include <common/fdt_wrappers.h>
#include <libfdt.h>

#define DTB_PROP_HW_LOG_ADDR	"tpm_event_log_addr"
#define DTB_PROP_HW_LOG_SIZE	"tpm_event_log_size"

#if MEASURED_BOOT

static int imx8m_event_log_fdt_init_overlay(uintptr_t dt_base, int dt_size)
{
	int ret;
	int offset;
	void *dtb = (void *)dt_base;

	ret = fdt_create_empty_tree(dtb, dt_size);
	if (ret < 0) {
		ERROR("cannot create empty dtb tree: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	offset = fdt_path_offset(dtb, "/");
	if (offset < 0) {
		ERROR("cannot find root of the tree: %s\n",
		       fdt_strerror(offset));
		return offset;
	}

	offset = fdt_add_subnode(dtb, offset, "fragment@0");
	if (offset < 0) {
		ERROR("cannot add fragment node: %s\n",
		       fdt_strerror(offset));
		return offset;
	}

	ret = fdt_setprop_string(dtb, offset, "target-path", "/");
	if (ret < 0) {
		ERROR("cannot set target-path property: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	offset = fdt_add_subnode(dtb, offset, "__overlay__");
	if (offset < 0) {
		ERROR("cannot add __overlay__ node: %s\n",
		       fdt_strerror(offset));
		return ret;
	}

	offset = fdt_add_subnode(dtb, offset, "tpm_event_log");
	if (offset < 0) {
		ERROR("cannot add tpm_event_log node: %s\n",
		       fdt_strerror(offset));
		return offset;
	}

	ret = fdt_setprop_string(dtb, offset, "compatible",
				 "arm,tpm_event_log");
	if (ret < 0) {
		ERROR("cannot set compatible property: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	ret = fdt_setprop_u64(dtb, offset, "tpm_event_log_addr", 0);
	if (ret < 0) {
		ERROR("cannot set tpm_event_log_addr property: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	ret = fdt_setprop_u32(dtb, offset, "tpm_event_log_size", 0);
	if (ret < 0) {
		ERROR("cannot set tpm_event_log_size property: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	return ret;
}

/*
 * Write the Event Log address and its size in the DTB.
 *
 * This function is supposed to be called only by BL2.
 *
 * Returns:
 *	0 = success
 *    < 0 = error
 */
static int imx8m_set_event_log_info(uintptr_t config_base,
				  uintptr_t log_addr, size_t log_size)
{
	/* As libfdt uses void *, we can't avoid this cast */
	void *dtb = (void *)config_base;
	const char *compatible_tpm = "arm,tpm_event_log";
	uint64_t base = cpu_to_fdt64(log_addr);
	uint32_t sz = cpu_to_fdt32(log_size);
	int err, node;

	err = fdt_open_into(dtb, dtb, PLAT_IMX8M_DTO_MAX_SIZE);
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
	 * Find the TPM node in device tree.
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

	err = fdt_pack(dtb);
	if (err < 0) {
		ERROR("Failed to pack Device Tree at %p: error %d\n", dtb, err);
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
 * in the QEMU DTB.
 *
 * This function is supposed to be called only by BL2.
 *
 * Returns:
 *	0 = success
 *    < 0 = error
 */
int imx8m_set_nt_fw_info(size_t log_size, uintptr_t *ns_log_addr)
{
	uintptr_t ns_addr;
	int err;

	assert(ns_log_addr != NULL);

	ns_addr = PLAT_IMX8M_DTO_BASE + PLAT_IMX8M_DTO_MAX_SIZE;

	imx8m_event_log_fdt_init_overlay(PLAT_IMX8M_DTO_BASE,
					  PLAT_IMX8M_DTO_MAX_SIZE);

	/* Write the Event Log address and its size in the DTB */
	err = imx8m_set_event_log_info(PLAT_IMX8M_DTO_BASE,
					ns_addr, log_size);

	/* Return Event Log address in Non-secure memory */
	*ns_log_addr = (err < 0) ? 0UL : ns_addr;
	return err;
}

#endif /* MEASURED_BOOT */
