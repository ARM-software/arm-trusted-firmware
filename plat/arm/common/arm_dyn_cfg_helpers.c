/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#if MEASURED_BOOT
#include <common/desc_image_load.h>
#endif
#include <common/fdt_wrappers.h>

#include <libfdt.h>

#include <plat/arm/common/arm_dyn_cfg_helpers.h>
#include <plat/arm/common/plat_arm.h>

#define DTB_PROP_MBEDTLS_HEAP_ADDR "mbedtls_heap_addr"
#define DTB_PROP_MBEDTLS_HEAP_SIZE "mbedtls_heap_size"

#if MEASURED_BOOT
#define DTB_PROP_BL2_HASH_DATA	"bl2_hash_data"
#ifdef SPD_opteed
/*
 * Currently OP-TEE does not support reading DTBs from Secure memory
 * and this property should be removed when this feature is supported.
 */
#define DTB_PROP_HW_SM_LOG_ADDR	"tpm_event_log_sm_addr"
#endif
#define DTB_PROP_HW_LOG_ADDR	"tpm_event_log_addr"
#define DTB_PROP_HW_LOG_SIZE    "tpm_event_log_size"

static int dtb_root = -1;
#endif /* MEASURED_BOOT */

/*******************************************************************************
 * Validate the tb_fw_config is a valid DTB file and returns the node offset
 * to "arm,tb_fw" property.
 * Arguments:
 *	void *dtb - pointer to the TB_FW_CONFIG in memory
 *	int *node - Returns the node offset to "arm,tb_fw" property if found.
 *
 * Returns 0 on success and -1 on error.
 ******************************************************************************/
int arm_dyn_tb_fw_cfg_init(void *dtb, int *node)
{
	assert(dtb != NULL);
	assert(node != NULL);

	/* Check if the pointer to DT is correct */
	if (fdt_check_header(dtb) != 0) {
		WARN("Invalid DTB file passed as%s\n", " TB_FW_CONFIG");
		return -1;
	}

	/* Assert the node offset point to "arm,tb_fw" compatible property */
	*node = fdt_node_offset_by_compatible(dtb, -1, "arm,tb_fw");
	if (*node < 0) {
		WARN("The compatible property '%s' not%s", "arm,tb_fw",
			" found in the config\n");
		return -1;
	}

	VERBOSE("Dyn cfg: '%s'%s", "arm,tb_fw", " found in the config\n");
	return 0;
}

/*
 * This function writes the Mbed TLS heap address and size in the DTB. When it
 * is called, it is guaranteed that a DTB is available. However it is not
 * guaranteed that the shared Mbed TLS heap implementation is used. Thus we
 * return error code from here and it's the responsibility of the caller to
 * determine the action upon error.
 *
 * This function is supposed to be called only by BL1.
 *
 * Returns:
 *	0 = success
 *     -1 = error
 */
int arm_set_dtb_mbedtls_heap_info(void *dtb, void *heap_addr, size_t heap_size)
{
#if !MEASURED_BOOT
	int dtb_root;
#endif
	/*
	 * Verify that the DTB is valid, before attempting to write to it,
	 * and get the DTB root node.
	 */
	int err = arm_dyn_tb_fw_cfg_init(dtb, &dtb_root);
	if (err < 0) {
		ERROR("Invalid%s loaded. Unable to get root node\n",
			" TB_FW_CONFIG");
		return -1;
	}

	/*
	 * Write the heap address and size in the DTB.
	 *
	 * NOTE: The variables heap_addr and heap_size are corrupted
	 * by the "fdtw_write_inplace_cells" function. After the
	 * function calls they must NOT be reused.
	 */
	err = fdtw_write_inplace_cells(dtb, dtb_root,
		DTB_PROP_MBEDTLS_HEAP_ADDR, 2, &heap_addr);
	if (err < 0) {
		ERROR("%sDTB property '%s'\n",
			"Unable to write ", DTB_PROP_MBEDTLS_HEAP_ADDR);
		return -1;
	}

	err = fdtw_write_inplace_cells(dtb, dtb_root,
		DTB_PROP_MBEDTLS_HEAP_SIZE, 1, &heap_size);
	if (err < 0) {
		ERROR("%sDTB property '%s'\n",
			"Unable to write ", DTB_PROP_MBEDTLS_HEAP_SIZE);
		return -1;
	}

	return 0;
}

#if MEASURED_BOOT
/*
 * This function writes the BL2 hash data in HW_FW_CONFIG DTB.
 * When it is called, it is guaranteed that a DTB is available.
 *
 * This function is supposed to be called only by BL1.
 *
 * Returns:
 *	0 = success
 *    < 0 = error
 */
int arm_set_bl2_hash_info(void *dtb, void *data)
{
	assert(dtb_root >= 0);

	/*
	 * Write the BL2 hash data in the DTB.
	 */
	return fdtw_write_inplace_bytes(dtb, dtb_root,
					DTB_PROP_BL2_HASH_DATA,
					TCG_DIGEST_SIZE, data);
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
static int arm_set_event_log_info(uintptr_t config_base,
#ifdef SPD_opteed
				  uintptr_t sm_log_addr,
#endif
				  uintptr_t log_addr, size_t log_size)
{
	/* As libfdt uses void *, we can't avoid this cast */
	void *dtb = (void *)config_base;
	const char *compatible = "arm,tpm_event_log";
	int err, node;

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

	/* Assert the node offset point to compatible property */
	node = fdt_node_offset_by_compatible(dtb, -1, compatible);
	if (node < 0) {
		WARN("The compatible property '%s' not%s", compatible,
			" found in the config\n");
		return node;
	}

	VERBOSE("Dyn cfg: '%s'%s", compatible, " found in the config\n");

#ifdef SPD_opteed
	if (sm_log_addr != 0UL) {
		err = fdtw_write_inplace_cells(dtb, node,
			DTB_PROP_HW_SM_LOG_ADDR, 2, &sm_log_addr);
		if (err < 0) {
			ERROR("%sDTB property '%s'\n",
				"Unable to write ", DTB_PROP_HW_SM_LOG_ADDR);
			return err;
		}
	}
#endif
	err = fdtw_write_inplace_cells(dtb, node,
		DTB_PROP_HW_LOG_ADDR, 2, &log_addr);
	if (err < 0) {
		ERROR("%sDTB property '%s'\n",
			"Unable to write ", DTB_PROP_HW_LOG_ADDR);
		return err;
	}

	err = fdtw_write_inplace_cells(dtb, node,
		DTB_PROP_HW_LOG_SIZE, 1, &log_size);
	if (err < 0) {
		ERROR("%sDTB property '%s'\n",
			"Unable to write ", DTB_PROP_HW_LOG_SIZE);
	} else {
		/*
		 * Ensure that the info written to the DTB is visible
		 * to other images.
		 */
		flush_dcache_range(config_base, fdt_totalsize(dtb));
	}

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
int arm_set_tos_fw_info(uintptr_t config_base, uintptr_t log_addr,
			size_t log_size)
{
	int err;

	assert(config_base != 0UL);
	assert(log_addr != 0UL);

	/* Write the Event Log address and its size in the DTB */
	err = arm_set_event_log_info(config_base,
#ifdef SPD_opteed
					0UL,
#endif
					log_addr, log_size);
	if (err < 0) {
		ERROR("%sEvent Log data to TOS_FW_CONFIG\n",
					"Unable to write ");
	}

	return err;
}

/*
 * This function writes the Event Log address and its size
 * in the NT_FW_CONFIG DTB.
 *
 * This function is supposed to be called only by BL2.
 *
 * Returns:
 *	0 = success
 *    < 0 = error
 */
int arm_set_nt_fw_info(uintptr_t config_base,
#ifdef SPD_opteed
			uintptr_t log_addr,
#endif
			size_t log_size, uintptr_t *ns_log_addr)
{
	uintptr_t ns_addr;
	const bl_mem_params_node_t *cfg_mem_params;
	int err;

	assert(config_base != 0UL);
	assert(ns_log_addr != NULL);

	/* Get the config load address and size from NT_FW_CONFIG */
	cfg_mem_params = get_bl_mem_params_node(NT_FW_CONFIG_ID);
	assert(cfg_mem_params != NULL);

	/* Calculate Event Log address in Non-secure memory */
	ns_addr = cfg_mem_params->image_info.image_base +
			cfg_mem_params->image_info.image_max_size;

	/* Check for memory space */
	if ((uint64_t)(ns_addr + log_size) > ARM_NS_DRAM1_END) {
		return -1;
	}

	/* Write the Event Log address and its size in the DTB */
	err = arm_set_event_log_info(config_base,
#ifdef SPD_opteed
					log_addr,
#endif
					ns_addr, log_size);

	/* Return Event Log address in Non-secure memory */
	*ns_log_addr = (err < 0) ? 0UL : ns_addr;
	return err;
}
#endif /* MEASURED_BOOT */
