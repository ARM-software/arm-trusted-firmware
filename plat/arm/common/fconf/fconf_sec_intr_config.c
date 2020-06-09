/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <libfdt.h>
#include <plat/arm/common/fconf_sec_intr_config.h>

#define G0_INTR_NUM(i)		g0_intr_prop[3U * (i)]
#define G0_INTR_PRIORITY(i)	g0_intr_prop[3U * (i) + 1]
#define G0_INTR_CONFIG(i)	g0_intr_prop[3U * (i) + 2]

#define G1S_INTR_NUM(i)		g1s_intr_prop[3U * (i)]
#define G1S_INTR_PRIORITY(i)	g1s_intr_prop[3U * (i) + 1]
#define G1S_INTR_CONFIG(i)	g1s_intr_prop[3U * (i) + 2]

struct sec_intr_prop_t sec_intr_prop;

static void print_intr_prop(interrupt_prop_t prop)
{
	VERBOSE("FCONF: Secure Interrupt NUM: %d, PRI: %d, TYPE: %d\n",
		prop.intr_num, prop.intr_pri, prop.intr_cfg);
}

int fconf_populate_sec_intr_config(uintptr_t config)
{
	int node, err;
	uint32_t g0_intr_count, g1s_intr_count;
	uint32_t g0_intr_prop[SEC_INT_COUNT_MAX * 3];
	uint32_t g1s_intr_prop[SEC_INT_COUNT_MAX * 3];

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	node = fdt_node_offset_by_compatible(hw_config_dtb, -1,
						"arm,secure_interrupt_desc");
	if (node < 0) {
		ERROR("FCONF: Unable to locate node with %s compatible property\n",
						"arm,secure_interrupt_desc");
		return node;
	}

	/* Read number of Group 0 interrupts specified by platform */
	err = fdt_read_uint32(hw_config_dtb, node, "g0_intr_cnt", &g0_intr_count);
	if (err < 0) {
		ERROR("FCONF: Could not locate g0s_intr_cnt property\n");
		return err;
	}

	/* At least 1 Group 0 interrupt description has to be provided*/
	if (g0_intr_count < 1U) {
		ERROR("FCONF: Invalid number of Group 0 interrupts count specified\n");
		return -1;
	}

	/* Read number of Group 1 secure interrupts specified by platform */
	err = fdt_read_uint32(hw_config_dtb, node, "g1s_intr_cnt",
				&g1s_intr_count);
	if (err < 0) {
		ERROR("FCONF: Could not locate g1s_intr_cnt property\n");
		return err;
	}

	/* At least one Group 1 interrupt description has to be provided*/
	if (g1s_intr_count < 1U) {
		ERROR("FCONF: Invalid number of Group 1 secure interrupts count specified\n");
		return -1;
	}

	/*
	 * Check if the total number of secure interrupts described are within
	 * the limit defined statically by the platform.
	 */
	if ((g0_intr_count + g1s_intr_count) > SEC_INT_COUNT_MAX) {
		ERROR("FCONF: Total number of secure interrupts exceed limit the of %d\n",
				SEC_INT_COUNT_MAX);
		return -1;
	}

	sec_intr_prop.count = g0_intr_count + g1s_intr_count;

	/* Read the Group 0 interrupt descriptors */
	err = fdt_read_uint32_array(hw_config_dtb, node, "g0_intr_desc",
				g0_intr_count * 3, g0_intr_prop);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for 'g0s_intr_desc': %d\n", err);
		return err;
	}

	/* Read the Group 1 secure interrupt descriptors */
	err = fdt_read_uint32_array(hw_config_dtb, node, "g1s_intr_desc",
				g1s_intr_count * 3, g1s_intr_prop);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for 'g1s_intr_desc': %d\n", err);
		return err;
	}

	/* Populate Group 0 interrupt descriptors into fconf based C struct */
	for (uint32_t i = 0; i < g0_intr_count; i++) {
		interrupt_prop_t sec_intr_property;

		/* Secure Interrupt Group: INTR_GROUP0 i.e., 0x1 */
		sec_intr_property.intr_grp = 1;
		sec_intr_property.intr_num = G0_INTR_NUM(i);
		sec_intr_property.intr_pri = G0_INTR_PRIORITY(i);
		sec_intr_property.intr_cfg = G0_INTR_CONFIG(i);
		sec_intr_prop.descriptor[i] = sec_intr_property;
		print_intr_prop(sec_intr_property);
	}

	/* Populate G1 secure interrupt descriptors into fconf based C struct */
	for (uint32_t i = 0; i < g1s_intr_count; i++) {
		interrupt_prop_t sec_intr_property;

		/* Secure Interrupt Group: INTR_GROUP1S i.e., 0x0 */
		sec_intr_property.intr_grp = 0;
		sec_intr_property.intr_num = G1S_INTR_NUM(i);
		sec_intr_property.intr_pri = G1S_INTR_PRIORITY(i);
		sec_intr_property.intr_cfg = G1S_INTR_CONFIG(i);
		sec_intr_prop.descriptor[i + g0_intr_count] = sec_intr_property;
		print_intr_prop(sec_intr_property);
	}

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, sec_intr_prop, fconf_populate_sec_intr_config);
