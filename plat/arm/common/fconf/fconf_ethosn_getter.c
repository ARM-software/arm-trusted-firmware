/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <libfdt.h>
#include <plat/arm/common/fconf_ethosn_getter.h>

struct ethosn_config_t ethosn_config;

static uint8_t fdt_node_get_status(const void *fdt, int node)
{
	int len;
	uint8_t status = ETHOSN_STATUS_DISABLED;
	const char *node_status;

	node_status = fdt_getprop(fdt, node, "status", &len);
	if (node_status == NULL ||
	    (len == 5 && /* Includes null character */
	     strncmp(node_status, "okay", 4U) == 0)) {
		status = ETHOSN_STATUS_ENABLED;
	}

	return status;
}

int fconf_populate_ethosn_config(uintptr_t config)
{
	int ethosn_node;
	int sub_node;
	uint8_t ethosn_status;
	uint32_t core_count = 0U;
	uint32_t core_addr_idx = 0U;
	const void *hw_conf_dtb = (const void *)config;

	/* Find offset to node with 'ethosn' compatible property */
	ethosn_node = fdt_node_offset_by_compatible(hw_conf_dtb, -1, "ethosn");
	if (ethosn_node < 0) {
		ERROR("FCONF: Can't find 'ethosn' compatible node in dtb\n");
		return ethosn_node;
	}

	/* If the Arm Ethos-N NPU is disabled the core check can be skipped */
	ethosn_status = fdt_node_get_status(hw_conf_dtb, ethosn_node);
	if (ethosn_status == ETHOSN_STATUS_DISABLED) {
		return 0;
	}

	fdt_for_each_subnode(sub_node, hw_conf_dtb, ethosn_node) {
		int err;
		uintptr_t addr;
		uint8_t status;

		/* Check that the sub node is "ethosn-core" compatible */
		if (fdt_node_check_compatible(hw_conf_dtb, sub_node,
					      "ethosn-core") != 0) {
			/* Ignore incompatible sub node */
			continue;
		}

		/* Including disabled cores */
		if (core_addr_idx >= ETHOSN_CORE_NUM_MAX) {
			ERROR("FCONF: Reached max number of Arm Ethos-N NPU cores\n");
			return -1;
		}

		status = fdt_node_get_status(hw_conf_dtb, ethosn_node);
		if (status == ETHOSN_STATUS_DISABLED) {
			++core_addr_idx;
			continue;
		}

		err = fdt_get_reg_props_by_index(hw_conf_dtb, ethosn_node,
						 core_addr_idx, &addr, NULL);
		if (err < 0) {
			ERROR("FCONF: Failed to read reg property for Arm Ethos-N NPU core %u\n",
			      core_addr_idx);
			return err;
		}

		ethosn_config.core_addr[core_count++] = addr;
		++core_addr_idx;
	}

	if ((sub_node < 0) && (sub_node != -FDT_ERR_NOTFOUND)) {
		ERROR("FCONF: Failed to parse sub nodes\n");
		return sub_node;
	}

	/* The Arm Ethos-N NPU can't be used if no cores were found */
	if (core_count == 0) {
		ERROR("FCONF: No Arm Ethos-N NPU cores found\n");
		return -1;
	}

	ethosn_config.num_cores = core_count;
	ethosn_config.status = ethosn_status;

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, ethosn_config, fconf_populate_ethosn_config);
