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
	uint32_t val32;
	bool is_plat_owned = false;
	const unsigned int sip_start = SP_PKG1_ID;
	unsigned int sip_index = sip_start;
	const unsigned int sip_end = sip_start + MAX_SP_IDS / 2;
	const unsigned int plat_start = SP_PKG5_ID;
	unsigned int plat_index = plat_start;
	const unsigned int plat_end = plat_start + MAX_SP_IDS / 2;
	unsigned int j;

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
		if ((index == MAX_SP_IDS) || (sip_index == sip_end)
		    || (plat_index == plat_end)) {
			ERROR("FCONF: Reached max number of SPs\n");
			return -1;
		}

		/* Read UUID */
		err = fdt_read_uint32_array(dtb, sp_node, "uuid", 4,
					    uuid_helper.word);
		if (err < 0) {
			ERROR("FCONF: cannot read SP uuid\n");
			return -1;
		}

		/* Convert uuid from big endian to little endian */
		for (j = 0U; j < 4U; j++) {
			uuid_helper.word[j] =
				((uuid_helper.word[j] >> 24U) & 0xff) |
				((uuid_helper.word[j] << 8U) & 0xff0000) |
				((uuid_helper.word[j] >> 8U) & 0xff00) |
				((uuid_helper.word[j] << 24U) & 0xff000000);
		}

		arm_sp.uuids[index] = uuid_helper;
		VERBOSE("FCONF: %s UUID %x-%x-%x-%x load_addr=%lx\n",
			__func__,
			uuid_helper.word[0],
			uuid_helper.word[1],
			uuid_helper.word[2],
			uuid_helper.word[3],
			arm_sp.load_addr[index]);

		/* Read Load address */
		err = fdt_read_uint32(dtb, sp_node, "load-address", &val32);
		if (err < 0) {
			ERROR("FCONF: cannot read SP load address\n");
			return -1;
		}
		arm_sp.load_addr[index] = val32;

		/* Read owner field only for dualroot CoT */
#if defined(ARM_COT_dualroot)
		/* Owner is an optional field, no need to catch error */
		fdtw_read_string(dtb, sp_node, "owner",
				arm_sp.owner[index], ARM_SP_OWNER_NAME_LEN);
#endif
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
		} else {
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
