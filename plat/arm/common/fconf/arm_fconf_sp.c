/*
 * Copyright (c) 2020-2022, ARM Limited and Contributors. All rights reserved.
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
	uint32_t val32;
	const unsigned int sip_start = SP_PKG1_ID;
	unsigned int sip_index = sip_start;
#if defined(ARM_COT_dualroot)
	const unsigned int sip_end = sip_start + MAX_SP_IDS / 2;
	/* Allocating index range for platform SPs */
	const unsigned int plat_start = SP_PKG5_ID;
	unsigned int plat_index = plat_start;
	const unsigned int plat_end = plat_start + MAX_SP_IDS / 2;
	bool is_plat_owned = false;
#endif /* ARM_COT_dualroot */

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
		if (index == MAX_SP_IDS) {
			ERROR("FCONF: Reached max number of SPs\n");
			return -1;
		}

#if defined(ARM_COT_dualroot)
		if ((sip_index == sip_end) || (plat_index == plat_end)) {
			ERROR("FCONF: Reached max number of plat/SiP SPs\n");
			return -1;
		}
#endif /* ARM_COT_dualroot */

		/* Read UUID */
		err = fdtw_read_uuid(dtb, sp_node, "uuid", 16,
				     (uint8_t *)&uuid_helper);
		if (err < 0) {
			ERROR("FCONF: cannot read SP uuid\n");
			return -1;
		}

		arm_sp.uuids[index] = uuid_helper;

		/* Read Load address */
		err = fdt_read_uint32(dtb, sp_node, "load-address", &val32);
		if (err < 0) {
			ERROR("FCONF: cannot read SP load address\n");
			return -1;
		}
		arm_sp.load_addr[index] = val32;

		VERBOSE("FCONF: %s UUID"
			" %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x"
			" load_addr=%lx\n",
			__func__,
			uuid_helper.uuid_struct.time_low[0], uuid_helper.uuid_struct.time_low[1],
			uuid_helper.uuid_struct.time_low[2], uuid_helper.uuid_struct.time_low[3],
			uuid_helper.uuid_struct.time_mid[0], uuid_helper.uuid_struct.time_mid[1],
			uuid_helper.uuid_struct.time_hi_and_version[0],
			uuid_helper.uuid_struct.time_hi_and_version[1],
			uuid_helper.uuid_struct.clock_seq_hi_and_reserved,
			uuid_helper.uuid_struct.clock_seq_low,
			uuid_helper.uuid_struct.node[0], uuid_helper.uuid_struct.node[1],
			uuid_helper.uuid_struct.node[2], uuid_helper.uuid_struct.node[3],
			uuid_helper.uuid_struct.node[4], uuid_helper.uuid_struct.node[5],
			arm_sp.load_addr[index]);

		/* Read owner field only for dualroot CoT */
#if defined(ARM_COT_dualroot)
		/* Owner is an optional field, no need to catch error */
		fdtw_read_string(dtb, sp_node, "owner",
				arm_sp.owner[index], ARM_SP_OWNER_NAME_LEN);

		/* If owner is empty mark it as SiP owned */
		if ((strncmp(arm_sp.owner[index], "SiP",
			     ARM_SP_OWNER_NAME_LEN) == 0) ||
		    (strncmp(arm_sp.owner[index], "",
			     ARM_SP_OWNER_NAME_LEN) == 0)) {
			is_plat_owned = false;
		} else if (strcmp(arm_sp.owner[index], "Plat") == 0) {
			is_plat_owned = true;
		} else {
			ERROR("FCONF: %s is not a valid SP owner\n",
			      arm_sp.owner[index]);
			return -1;
		}
		/*
		 * Add SP information in mem param descriptor and IO policies
		 * structure.
		 */
		if (is_plat_owned) {
			sp_mem_params_descs[index].image_id = plat_index;
			policies[plat_index].image_spec =
						(uintptr_t)&arm_sp.uuids[index];
			policies[plat_index].dev_handle = &fip_dev_handle;
			policies[plat_index].check = open_fip;
			plat_index++;
		} else
#endif /* ARM_COT_dualroot */
		{
			sp_mem_params_descs[index].image_id = sip_index;
			policies[sip_index].image_spec =
						(uintptr_t)&arm_sp.uuids[index];
			policies[sip_index].dev_handle = &fip_dev_handle;
			policies[sip_index].check = open_fip;
			sip_index++;
		}
		SET_PARAM_HEAD(&sp_mem_params_descs[index].image_info,
					PARAM_IMAGE_BINARY, VERSION_2, 0);
		sp_mem_params_descs[index].image_info.image_max_size =
							ARM_SP_MAX_SIZE;
		sp_mem_params_descs[index].next_handoff_image_id =
							INVALID_IMAGE_ID;
		sp_mem_params_descs[index].image_info.image_base =
							arm_sp.load_addr[index];
		index++;
	}

	if ((sp_node < 0) && (sp_node != -FDT_ERR_NOTFOUND)) {
		ERROR("%u: fdt_for_each_subnode(): %d\n", __LINE__, node);
		return sp_node;
	}

	arm_sp.number_of_sp = index;
	return 0;
}

FCONF_REGISTER_POPULATOR(TB_FW, arm_sp, fconf_populate_arm_sp);

#endif /* IMAGE_BL2 */
