/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/css/sds.h>
#include <libfdt.h>
#include <plat/common/platform.h>

#include "n1sdp_def.h"
#include <plat/arm/common/plat_arm.h>

/*
 * Platform information structure stored in SDS.
 * This structure holds information about platform's DDR
 * size which will be used to zero out the memory before
 * enabling the ECC capability as well as information
 * about multichip setup
 * 	- multichip mode
 * 	- secondary_count
 * 	- Local DDR size in GB, DDR memory in master board
 * 	- Remote DDR size in GB, DDR memory in secondary board
 */
struct n1sdp_plat_info {
	bool multichip_mode;
	uint8_t secondary_count;
	uint8_t local_ddr_size;
	uint8_t remote_ddr_size;
} __packed;

/*******************************************************************************
 * This function inserts Platform information via device tree nodes as,
 *	platform-info {
 *		multichip-mode = <0x0>;
 *		secondary-chip-count = <0x0>;
 *		local-ddr-size = <0x0>;
 *		remote-ddr-size = <0x0>;
 *	};
 ******************************************************************************/
static int plat_n1sdp_append_config_node(struct n1sdp_plat_info *plat_info)
{
	bl_mem_params_node_t *mem_params;
	void *fdt;
	int nodeoffset, err;

	mem_params = get_bl_mem_params_node(NT_FW_CONFIG_ID);
	if (mem_params == NULL) {
		ERROR("NT_FW CONFIG base address is NULL\n");
		return -1;
	}

	fdt = (void *)(mem_params->image_info.image_base);

	/* Check the validity of the fdt */
	if (fdt_check_header(fdt) != 0) {
		ERROR("Invalid NT_FW_CONFIG DTB passed\n");
		return -1;
	}

	nodeoffset = fdt_subnode_offset(fdt, 0, "platform-info");
	if (nodeoffset < 0) {
		ERROR("NT_FW_CONFIG: Failed to get platform-info node offset\n");
		return -1;
	}

	err = fdt_setprop_u32(fdt, nodeoffset, "multichip-mode",
			plat_info->multichip_mode);
	if (err < 0) {
		ERROR("NT_FW_CONFIG: Failed to set multichip-mode\n");
		return -1;
	}

	err = fdt_setprop_u32(fdt, nodeoffset, "secondary-chip-count",
			plat_info->secondary_count);
	if (err < 0) {
		ERROR("NT_FW_CONFIG: Failed to set secondary-chip-count\n");
		return -1;
	}

	err = fdt_setprop_u32(fdt, nodeoffset, "local-ddr-size",
			plat_info->local_ddr_size);
	if (err < 0) {
		ERROR("NT_FW_CONFIG: Failed to set local-ddr-size\n");
		return -1;
	}

	err = fdt_setprop_u32(fdt, nodeoffset, "remote-ddr-size",
			plat_info->remote_ddr_size);
	if (err < 0) {
		ERROR("NT_FW_CONFIG: Failed to set remote-ddr-size\n");
		return -1;
	}

	flush_dcache_range((uintptr_t)fdt, mem_params->image_info.image_size);

	return 0;
}

/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
bl_params_t *plat_get_next_bl_params(void)
{
	int ret;
	struct n1sdp_plat_info plat_info;

	ret = sds_init();
	if (ret != SDS_OK) {
		ERROR("SDS initialization failed. ret:%d\n", ret);
		panic();
	}

	ret = sds_struct_read(N1SDP_SDS_PLATFORM_INFO_STRUCT_ID,
				N1SDP_SDS_PLATFORM_INFO_OFFSET,
				&plat_info,
				N1SDP_SDS_PLATFORM_INFO_SIZE,
				SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK) {
		ERROR("Error getting platform info from SDS. ret:%d\n", ret);
		panic();
	}

	/* Validate plat_info SDS */
	if ((plat_info.local_ddr_size == 0U)
		|| (plat_info.local_ddr_size > N1SDP_MAX_DDR_CAPACITY_GB)
		|| (plat_info.remote_ddr_size > N1SDP_MAX_DDR_CAPACITY_GB)
		|| (plat_info.secondary_count > N1SDP_MAX_SECONDARY_COUNT)
		){
		ERROR("platform info SDS is corrupted\n");
		panic();
	}

	ret = plat_n1sdp_append_config_node(&plat_info);
	if (ret != 0) {
		panic();
	}

	return arm_get_next_bl_params();
}
