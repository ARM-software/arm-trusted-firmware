/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/css/sds.h>
#include <libfdt.h>

#include "morello_def.h"
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#ifdef TARGET_PLATFORM_FVP
/*
 * Platform information structure stored in SDS.
 * This structure holds information about platform's DDR
 * size which is an information about multichip setup
 *	- Local DDR size in bytes, DDR memory in main board
 */
struct morello_plat_info {
	uint64_t local_ddr_size;
} __packed;
#else
/*
 * Platform information structure stored in SDS.
 * This structure holds information about platform's DDR
 * size which is an information about multichip setup
 *	- Local DDR size in bytes, DDR memory in main board
 *	- Remote DDR size in bytes, DDR memory in remote board
 *	- remote_chip_count
 *	- multichip mode
 *	- scc configuration
 */
struct morello_plat_info {
	uint64_t local_ddr_size;
	uint64_t remote_ddr_size;
	uint8_t remote_chip_count;
	bool multichip_mode;
	uint32_t scc_config;
} __packed;
#endif

/* In client mode, a part of the DDR memory is reserved for Tag bits.
 * Calculate the usable memory size after subtracting the Tag memory.
 */
static inline uint64_t get_mem_client_mode(uint64_t size)
{
	return (size - (size / 128ULL));
}

/*******************************************************************************
 * This function inserts Platform information via device tree nodes as,
 *	platform-info {
 *		local-ddr-size = <0x0 0x0>;
 *#ifdef TARGET_PLATFORM_SOC
 *		remote-ddr-size = <0x0 0x0>;
 *		remote-chip-count = <0x0>;
 *		multichip-mode = <0x0>;
 *		scc-config = <0x0>;
 *#endif
 *	};
 ******************************************************************************/
static int plat_morello_append_config_node(struct morello_plat_info *plat_info)
{
	bl_mem_params_node_t *mem_params;
	void *fdt;
	int nodeoffset, err;
	uint64_t usable_mem_size;

	usable_mem_size = plat_info->local_ddr_size;

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

#ifdef TARGET_PLATFORM_SOC
	err = fdt_setprop_u64(fdt, nodeoffset, "remote-ddr-size",
			plat_info->remote_ddr_size);
	if (err < 0) {
		ERROR("NT_FW_CONFIG: Failed to set remote-ddr-size\n");
		return -1;
	}

	err = fdt_setprop_u32(fdt, nodeoffset, "remote-chip-count",
			plat_info->remote_chip_count);
	if (err < 0) {
		ERROR("NT_FW_CONFIG: Failed to set remote-chip-count\n");
		return -1;
	}

	err = fdt_setprop_u32(fdt, nodeoffset, "multichip-mode",
			plat_info->multichip_mode);
	if (err < 0) {
		ERROR("NT_FW_CONFIG: Failed to set multichip-mode\n");
		return -1;
	}

	err = fdt_setprop_u32(fdt, nodeoffset, "scc-config",
			plat_info->scc_config);
	if (err < 0) {
		ERROR("NT_FW_CONFIG: Failed to set scc-config\n");
		return -1;
	}

	if (plat_info->scc_config & MORELLO_SCC_CLIENT_MODE_MASK) {
		usable_mem_size = get_mem_client_mode(plat_info->local_ddr_size);
	}
#endif
	err = fdt_setprop_u64(fdt, nodeoffset, "local-ddr-size",
			usable_mem_size);
	if (err < 0) {
		ERROR("NT_FW_CONFIG: Failed to set local-ddr-size\n");
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
	struct morello_plat_info plat_info;

	ret = sds_init();
	if (ret != SDS_OK) {
		ERROR("SDS initialization failed. ret:%d\n", ret);
		panic();
	}

	ret = sds_struct_read(MORELLO_SDS_PLATFORM_INFO_STRUCT_ID,
				MORELLO_SDS_PLATFORM_INFO_OFFSET,
				&plat_info,
				MORELLO_SDS_PLATFORM_INFO_SIZE,
				SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK) {
		ERROR("Error getting platform info from SDS. ret:%d\n", ret);
		panic();
	}

	/* Validate plat_info SDS */
#ifdef TARGET_PLATFORM_FVP
	if (plat_info.local_ddr_size == 0U) {
#else
	if ((plat_info.local_ddr_size == 0U)
		|| (plat_info.local_ddr_size > MORELLO_MAX_DDR_CAPACITY)
		|| (plat_info.remote_ddr_size > MORELLO_MAX_DDR_CAPACITY)
		|| (plat_info.remote_chip_count > MORELLO_MAX_REMOTE_CHIP_COUNT)
		){
#endif
		ERROR("platform info SDS is corrupted\n");
		panic();
	}

	ret = plat_morello_append_config_node(&plat_info);
	if (ret != 0) {
		panic();
	}

	return arm_get_next_bl_params();
}
