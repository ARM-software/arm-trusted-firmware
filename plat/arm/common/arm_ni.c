/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

#define NI_CHILD_NODE_COUNT			4
#define NI_CHILD_POINTERS_START			8

#define NI_PMU_SECURE_CTRL			0x100
#define NI_PMU_SECURE_EVENT_OBSERVATION		0x108
#define NI_PMU_DEBUG_ENABLE			0x110
#define NI_COMP_NUM_SUBFEATURES			0x100
#define NI_COMP_SUBFEATURE_TYPE_START		0x108
#define NI_COMP_SUBFEATURE_SECURE_CTRL_START	0x308

#define SECURE_OVERRIDE_DEFAULT			BIT(0)
#define SECURE_EVENT_ENABLE			BIT(2)
#define NA_EVENT_ENABLE				BIT(3)
#define PMU_ENABLE				BIT(0)

#define NI_NODE_MASK				0x0000ffff
#define NI_NODE_TYPE(node_info)			(node_info & NI_NODE_MASK)
#define NI_CHILD_POINTER(i)			(NI_CHILD_POINTERS_START + (i * 4))
#define NI_COMP_SUBFEATURE_TYPE(i)		(NI_COMP_SUBFEATURE_TYPE_START + (i * 8))
#define NI_COMP_SUBFEATURE_SECURE_CTRL(i)	(NI_COMP_SUBFEATURE_SECURE_CTRL_START + (i * 8))

#define NI_PERIPHERAL_ID0			0xfe0
#define NI_PIDR0_PART_MASK			0xff
#define NI_PERIPHERAL_ID1			0xfe4
#define NI_PIDR1_PART_MASK			0xf
#define NI_PIDR1_PART_SHIFT			8

enum ni_part {
	NI_700 = 0x43b,
	NI_710AE = 0x43d,
	NI_TOWER = 0x43f,
};

enum ni_node_type {
	NI_INVALID_NODE = 0,
	NI_VOLTAGE_DOMAIN  = 1,
	NI_POWER_DOMAIN = 2,
	NI_CLOCK_DOMAIN = 3,
	NI_ASNI = 4,
	NI_AMNI = 5,
	NI_PMU = 6,
	NI_HSNI = 7,
	NI_HMNI = 8,
	NI_PMNI = 9,
	NI_CMNI = 14,
	NI_CFGNI = 15
};

enum ni_subfeature_type {
	NI_SUBFEATURE_APU = 0,
	NI_SUBFEATURE_ADDR_MAP = 1,
	NI_SUBFEATURE_FCU = 2,
	NI_SUBFEATURE_IDM = 3
};

static void ni_enable_pmu(uintptr_t pmu_addr)
{
	mmio_setbits_32(pmu_addr + NI_PMU_DEBUG_ENABLE, PMU_ENABLE);
}

static void ni_enable_fcu_ns_access(uintptr_t comp_addr)
{
	uint32_t subfeature_type;
	uint32_t subfeature_count;
	uint32_t subfeature_secure_ctrl;

	subfeature_count = mmio_read_32(comp_addr + NI_COMP_NUM_SUBFEATURES);
	for (uint32_t i = 0U; i < subfeature_count; i++) {
		subfeature_type =
			NI_NODE_TYPE(mmio_read_32(comp_addr + NI_COMP_SUBFEATURE_TYPE(i)));
		if (subfeature_type == NI_SUBFEATURE_FCU) {
			subfeature_secure_ctrl = comp_addr + NI_COMP_SUBFEATURE_SECURE_CTRL(i);
			mmio_setbits_32(subfeature_secure_ctrl, SECURE_OVERRIDE_DEFAULT);
		}
	}
}

static void ni_enable_pmu_ns_access(uintptr_t comp_addr)
{
	mmio_setbits_32(comp_addr + NI_PMU_SECURE_CTRL, SECURE_OVERRIDE_DEFAULT);
	mmio_setbits_32(comp_addr + NI_PMU_SECURE_EVENT_OBSERVATION,
			SECURE_EVENT_ENABLE | NA_EVENT_ENABLE);
}

static void ni_setup_component(uintptr_t comp_addr)
{
	uint32_t node_info;

	node_info = mmio_read_32(comp_addr);

	switch (NI_NODE_TYPE(node_info)) {
	case NI_ASNI:
	case NI_AMNI:
	case NI_HSNI:
	case NI_HMNI:
	case NI_PMNI:
		ni_enable_fcu_ns_access(comp_addr);
		break;
	case NI_PMU:
		ni_enable_pmu_ns_access(comp_addr);
		ni_enable_pmu(comp_addr);
		break;
	default:
		return;
	}
}

int plat_arm_ni_setup(uintptr_t global_cfg)
{
	uintptr_t vd_addr;
	uintptr_t pd_addr;
	uintptr_t cd_addr;
	uintptr_t comp_addr;
	uint32_t vd_count;
	uint32_t pd_count;
	uint32_t cd_count;
	uint32_t comp_count;
	uint32_t part;
	uint32_t reg;

	reg = mmio_read_32(global_cfg + NI_PERIPHERAL_ID0);
	part = reg & NI_PIDR0_PART_MASK;
	reg = mmio_read_32(global_cfg + NI_PERIPHERAL_ID1);
	part |= ((reg & NI_PIDR1_PART_MASK) << NI_PIDR1_PART_SHIFT);

	if (part != NI_TOWER) {
		ERROR("0x%x is not supported\n", part);
		return -EINVAL;
	}

	vd_count = mmio_read_32(global_cfg + NI_CHILD_NODE_COUNT);

	for (uint32_t i = 0U; i < vd_count; i++) {
		vd_addr = global_cfg + mmio_read_32(global_cfg + NI_CHILD_POINTER(i));
		pd_count = mmio_read_32(vd_addr + NI_CHILD_NODE_COUNT);

		for (uint32_t j = 0U; j < pd_count; j++) {
			pd_addr = global_cfg + mmio_read_32(vd_addr + NI_CHILD_POINTER(j));
			cd_count = mmio_read_32(pd_addr + NI_CHILD_NODE_COUNT);

			for (uint32_t k = 0U; k < cd_count; k++) {
				cd_addr = global_cfg + mmio_read_32(pd_addr + NI_CHILD_POINTER(k));
				comp_count = mmio_read_32(cd_addr + NI_CHILD_NODE_COUNT);

				for (uint32_t l = 0U; l < comp_count; l++) {
					comp_addr = global_cfg +
						mmio_read_32(cd_addr + NI_CHILD_POINTER(l));
					ni_setup_component(comp_addr);
				}
			}
		}
	}

	return 0;
}
