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

struct ethosn_config_t ethosn_config = {.num_cores = 0};

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
	const void *hw_conf_dtb = (const void *)config;

	/* Find offset to node with 'ethosn' compatible property */
	INFO("Probing Arm Ethos-N NPU\n");
	uint32_t total_core_count = 0U;

	fdt_for_each_compatible_node(hw_conf_dtb, ethosn_node, "ethosn") {
		int sub_node;
		uint8_t ethosn_status;
		uint32_t device_core_count = 0U;

		/* If the Arm Ethos-N NPU is disabled the core check can be skipped */
		ethosn_status = fdt_node_get_status(hw_conf_dtb, ethosn_node);
		if (ethosn_status == ETHOSN_STATUS_DISABLED) {
			continue;
		}

		fdt_for_each_subnode(sub_node, hw_conf_dtb, ethosn_node) {
			int err;
			uintptr_t core_addr;
			uint8_t core_status;

			if (total_core_count >= ETHOSN_CORE_NUM_MAX) {
				ERROR("FCONF: Reached max number of Arm Ethos-N NPU cores\n");
				return -FDT_ERR_BADSTRUCTURE;
			}

			/* Check that the sub node is "ethosn-core" compatible */
			if (fdt_node_check_compatible(hw_conf_dtb,
						      sub_node,
						      "ethosn-core") != 0) {
				/* Ignore incompatible sub node */
				continue;
			}

			core_status = fdt_node_get_status(hw_conf_dtb, sub_node);
			if (core_status == ETHOSN_STATUS_DISABLED) {
				continue;
			}

			err = fdt_get_reg_props_by_index(hw_conf_dtb,
							 ethosn_node,
							 device_core_count,
							 &core_addr,
							 NULL);
			if (err < 0) {
				ERROR(
				"FCONF: Failed to read reg property for Arm Ethos-N NPU core %u\n",
						device_core_count);
				return err;
			}

			INFO("NPU core probed at address 0x%lx\n", core_addr);
			ethosn_config.core[total_core_count].addr = core_addr;
			total_core_count++;
			device_core_count++;
		}

		if ((sub_node < 0) && (sub_node != -FDT_ERR_NOTFOUND)) {
			ERROR("FCONF: Failed to parse sub nodes\n");
			return -FDT_ERR_BADSTRUCTURE;
		}

		if (device_core_count == 0U) {
			ERROR(
			"FCONF: Enabled Arm Ethos-N NPU device must have at least one enabled core\n");
			return -FDT_ERR_BADSTRUCTURE;
		}
	}

	if (total_core_count == 0U) {
		ERROR("FCONF: Can't find 'ethosn' compatible node in dtb\n");
		return -FDT_ERR_BADSTRUCTURE;
	}

	ethosn_config.num_cores = total_core_count;

	INFO("%d NPU core%s probed\n",
	     ethosn_config.num_cores,
	     ethosn_config.num_cores > 1 ? "s" : "");

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, ethosn_config, fconf_populate_ethosn_config);
