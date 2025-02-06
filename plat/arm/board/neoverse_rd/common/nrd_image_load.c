/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <libfdt.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/css/sds.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <nrd_variant.h>

/*
 * Information about the isolated CPUs obtained from SDS.
 */
struct isolated_cpu_mpid_list {
	uint64_t num_entries; /* Number of entries in the list */
	uint64_t mpid_list[PLATFORM_CORE_COUNT]; /* List of isolated CPU MPIDs */
};

/* Function to read isolated CPU MPID list from SDS. */
void plat_arm_nrd_get_isolated_cpu_list(struct isolated_cpu_mpid_list *list)
{
	int ret;

	ret = sds_init(SDS_SCP_AP_REGION_ID);
	if (ret != SDS_OK) {
		ERROR("SDS initialization failed, error: %d\n", ret);
		panic();
	}

	ret = sds_struct_read(SDS_SCP_AP_REGION_ID,
			SDS_ISOLATED_CPU_LIST_ID, 0, &list->num_entries,
			sizeof(list->num_entries), SDS_ACCESS_MODE_CACHED);
	if (ret != SDS_OK) {
		INFO("SDS CPU num elements read failed, error: %d\n", ret);
		list->num_entries = 0;
		return;
	}

	if (list->num_entries > PLATFORM_CORE_COUNT) {
		ERROR("Isolated CPU list count %ld greater than max"
		      " number supported %d\n",
		      list->num_entries, PLATFORM_CORE_COUNT);
		panic();
	} else if (list->num_entries == 0) {
		INFO("SDS isolated CPU list is empty\n");
		return;
	}

	ret = sds_struct_read(SDS_SCP_AP_REGION_ID,
			SDS_ISOLATED_CPU_LIST_ID,
			sizeof(list->num_entries),
			&list->mpid_list,
			sizeof(list->mpid_list[0]) * list->num_entries,
			SDS_ACCESS_MODE_CACHED);
	if (ret != SDS_OK) {
		ERROR("SDS CPU list read failed. error: %d\n", ret);
		panic();
	}
}

/*******************************************************************************
 * This function inserts Platform information via device tree nodes as,
 * system-id {
 *    platform-id = <0>;
 *    config-id = <0>;
 *    isolated-cpu-list = <0>
 * }
 ******************************************************************************/
static int plat_nrd_append_config_node(void)
{
	bl_mem_params_node_t *mem_params;
	void *fdt;
	int nodeoffset, err;
	unsigned int platid = 0, platcfg = 0;
	struct isolated_cpu_mpid_list cpu_mpid_list = {0};

	mem_params = get_bl_mem_params_node(NT_FW_CONFIG_ID);
	if (mem_params == NULL) {
		ERROR("NT_FW CONFIG base address is NULL");
		return -1;
	}

	fdt = (void *)(mem_params->image_info.image_base);

	/* Check the validity of the fdt */
	if (fdt_check_header(fdt) != 0) {
		ERROR("Invalid NT_FW_CONFIG DTB passed\n");
		return -1;
	}

	nodeoffset = fdt_subnode_offset(fdt, 0, "system-id");
	if (nodeoffset < 0) {
		ERROR("Failed to get system-id node offset\n");
		return -1;
	}

	platid = plat_arm_nrd_get_platform_id();
	err = fdt_setprop_u32(fdt, nodeoffset, "platform-id", platid);
	if (err < 0) {
		ERROR("Failed to set platform-id\n");
		return -1;
	}

	platcfg = plat_arm_nrd_get_config_id();
	err = fdt_setprop_u32(fdt, nodeoffset, "config-id", platcfg);
	if (err < 0) {
		ERROR("Failed to set config-id\n");
		return -1;
	}

	platcfg = plat_arm_nrd_get_multi_chip_mode();
	err = fdt_setprop_u32(fdt, nodeoffset, "multi-chip-mode", platcfg);
	if (err < 0) {
		ERROR("Failed to set multi-chip-mode\n");
		return -1;
	}

	plat_arm_nrd_get_isolated_cpu_list(&cpu_mpid_list);
	if (cpu_mpid_list.num_entries > 0) {
		err = fdt_setprop(fdt, nodeoffset, "isolated-cpu-list",
				&cpu_mpid_list,
				(sizeof(cpu_mpid_list.num_entries) *
				(cpu_mpid_list.num_entries + 1)));
		if (err < 0) {
			ERROR("Failed to set isolated-cpu-list, error: %d\n",
			      err);
		}
	}

	flush_dcache_range((uintptr_t)fdt, mem_params->image_info.image_size);

	return 0;
}

/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
bl_params_t *plat_get_next_bl_params(void)
{
	struct bl_params *arm_bl_params;
	int ret;

	ret = plat_nrd_append_config_node();
	if (ret != 0)
		panic();

	arm_bl_params = arm_get_next_bl_params();

#if !EL3_PAYLOAD_BASE
	const struct dyn_cfg_dtb_info_t *fw_config_info;
	bl_mem_params_node_t *param_node;
	uintptr_t fw_config_base = 0UL;

	/* Get BL31 image node */
	param_node = get_bl_mem_params_node(BL31_IMAGE_ID);
	assert(param_node != NULL);

	/* Get fw_config load address */
	fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, FW_CONFIG_ID);
	assert(fw_config_info != NULL);

	fw_config_base = fw_config_info->config_addr;
	assert(fw_config_base != 0UL);

	/*
	 * Get the entry point info of next executable image and override
	 * arg1 of entry point info with fw_config base address
	 */
	param_node->ep_info.args.arg1 = (uint64_t)fw_config_base;

#endif
	return arm_bl_params;
}
