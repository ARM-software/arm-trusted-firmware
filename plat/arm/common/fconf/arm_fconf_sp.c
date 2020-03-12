/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/fdt_wrappers.h>
#include <drivers/io/io_storage.h>
#include <lib/object_pool.h>
#include <libfdt.h>
#include <plat/arm/common/arm_fconf_getter.h>
#include <plat/arm/common/arm_fconf_io_storage.h>
#include <plat/arm/common/fconf_arm_sp_getter.h>
#include <platform_def.h>
#include <tools_share/firmware_image_package.h>

#ifdef IMAGE_BL2

bl_mem_params_node_t sp_mem_params_descs[MAX_SP_IDS];

struct arm_sp_t arm_sp;

int fconf_populate_arm_sp(uintptr_t config)
{
	int sp_node, node, err;
	union uuid_helper_t uuid_helper;
	unsigned int index = 0;
	const unsigned int sp_start_index = MAX_NUMBER_IDS - MAX_SP_IDS;

	/* As libfdt use void *, we can't avoid this cast */
	const void *dtb = (void *)config;

	/* Assert the node offset point to "arm,sp" compatible property */
	const char *compatible_str = "arm,sp";

	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s in dtb\n", compatible_str);
		return node;
	}

	fdt_for_each_subnode(sp_node, dtb, node) {
		err = fdtw_read_array(dtb, sp_node, "uuid", 4,
				      &uuid_helper.word);
		if (err < 0) {
			ERROR("FCONF: cannot read SP uuid\n");
			return -1;
		}

		arm_sp.uuids[index] = uuid_helper;

		err = fdtw_read_cells(dtb, sp_node, "load-address", 1,
			&arm_sp.load_addr[index]);
		if (err < 0) {
			ERROR("FCONF: cannot read SP load address\n");
			return -1;
		}

		VERBOSE("FCONF: %s UUID %x-%x-%x-%x load_addr=%lx\n",
			__func__,
			uuid_helper.word[0],
			uuid_helper.word[1],
			uuid_helper.word[2],
			uuid_helper.word[3],
			arm_sp.load_addr[index]);

		/* Add SP information in mem param descriptor */
		sp_mem_params_descs[index].image_id = sp_start_index + index;
		SET_PARAM_HEAD(&sp_mem_params_descs[index].image_info,
					PARAM_IMAGE_BINARY, VERSION_2, 0);
		sp_mem_params_descs[index].image_info.image_max_size =
							ARM_SP_MAX_SIZE;
		sp_mem_params_descs[index].next_handoff_image_id =
							INVALID_IMAGE_ID;
		sp_mem_params_descs[index].image_info.image_base =
							arm_sp.load_addr[index];

		/* Add SP information in IO policies structure */
		policies[sp_start_index + index].image_spec =
						(uintptr_t)&arm_sp.uuids[index];
		policies[sp_start_index + index].dev_handle = &fip_dev_handle;
		policies[sp_start_index + index].check = open_fip;

		index++;

		if (index >= MAX_SP_IDS) {
			ERROR("FCONF: reached max number of SPs\n");
			return -1;
		}
	}

	if ((sp_node < 0) && (sp_node != -FDT_ERR_NOTFOUND)) {
		ERROR("%d: fdt_for_each_subnode(): %d\n", __LINE__, node);
		return sp_node;
	}

	arm_sp.number_of_sp = index;
	return 0;
}

FCONF_REGISTER_POPULATOR(TB_FW, arm_sp, fconf_populate_arm_sp);

#endif /* IMAGE_BL2 */
