/*
 * Copyright (C) 2023-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <common/fdt_wrappers.h>

#include <drivers/delay_timer.h>
#include <drivers/st/regulator.h>
#include <drivers/st/stm32mp_ddr.h>

#include <libfdt.h>

#include <platform_def.h>

#if STM32MP_DDR3_TYPE
struct ddr3_supply {
	struct rdev *vdd;
	struct rdev *vref;
	struct rdev *vtt;
};

static void ddr3_supply_read(void *fdt, int node, struct ddr3_supply *supply)
{
	supply->vdd = regulator_get_by_supply_name(fdt, node, "vdd");
	supply->vref = regulator_get_by_supply_name(fdt, node, "vref");
	supply->vtt = regulator_get_by_supply_name(fdt, node, "vtt");
}

static int ddr_power_init(void *fdt, int node)
{
	int status;
	struct ddr3_supply supply;

	ddr3_supply_read(fdt, node, &supply);
	if ((supply.vdd == NULL) || (supply.vref == NULL) || (supply.vtt == NULL)) {
		return -ENOENT;
	}

	/*
	 * DDR3 power on sequence is:
	 * enable VREF_DDR, VTT_DDR, VPP_DDR
	 */
	status = regulator_set_min_voltage(supply.vdd);
	if (status != 0) {
		return status;
	}

	status = regulator_enable(supply.vdd);
	if (status != 0) {
		return status;
	}

	status = regulator_enable(supply.vref);
	if (status != 0) {
		return status;
	}

	return regulator_enable(supply.vtt);
}
#endif /* STM32MP_DDR3_TYPE */

#if STM32MP_DDR4_TYPE
struct ddr4_supply {
	struct rdev *vdd;
	struct rdev *vref;
	struct rdev *vtt;
	struct rdev *vpp;
};

static void ddr4_supply_read(void *fdt, int node, struct ddr4_supply *supply)
{
	supply->vpp = regulator_get_by_supply_name(fdt, node, "vpp");
	supply->vdd = regulator_get_by_supply_name(fdt, node, "vdd");
	supply->vref = regulator_get_by_supply_name(fdt, node, "vref");
	supply->vtt = regulator_get_by_supply_name(fdt, node, "vtt");
}

static int ddr_power_init(void *fdt, int node)
{
	int status;
	struct ddr4_supply supply;

	ddr4_supply_read(fdt, node, &supply);
	if ((supply.vpp == NULL) || (supply.vdd == NULL) || (supply.vref == NULL) ||
	    (supply.vtt == NULL)) {
		return -ENOENT;
	}

	/*
	 * DDR4 power on sequence is:
	 * enable VPP_DDR
	 * enable VREF_DDR, VTT_DDR, VPP_DDR
	 */
	status = regulator_set_min_voltage(supply.vpp);
	if (status != 0) {
		return status;
	}

	status = regulator_set_min_voltage(supply.vdd);
	if (status != 0) {
		return status;
	}

	status = regulator_enable(supply.vpp);
	if (status != 0) {
		return status;
	}

	status = regulator_enable(supply.vdd);
	if (status != 0) {
		return status;
	}

	status = regulator_enable(supply.vref);
	if (status != 0) {
		return status;
	}

	return regulator_enable(supply.vtt);
}
#endif /* STM32MP_DDR4_TYPE */

#if STM32MP_LPDDR4_TYPE
struct lpddr4_supply {
	struct rdev *vdd1;
	struct rdev *vdd2;
	struct rdev *vddq;
};

static void lpddr4_supply_read(void *fdt, int node, struct lpddr4_supply *supply)
{
	supply->vdd1 = regulator_get_by_supply_name(fdt, node, "vdd1");
	supply->vdd2 = regulator_get_by_supply_name(fdt, node, "vdd2");
	supply->vddq = regulator_get_by_supply_name(fdt, node, "vddq");
}

static int ddr_power_init(void *fdt, int node)
{
	int status;
	struct lpddr4_supply supply;

	lpddr4_supply_read(fdt, node, &supply);
	if ((supply.vdd1 == NULL) || (supply.vdd2 == NULL) || (supply.vddq == NULL)) {
		return -ENOENT;
	}

	/*
	 * LPDDR4 power on sequence is:
	 * enable VDD1_DDR
	 * enable VDD2_DDR
	 * enable VDDQ_DDR
	 */
	status = regulator_set_min_voltage(supply.vdd1);
	if (status != 0) {
		return status;
	}

	status = regulator_set_min_voltage(supply.vdd2);
	if (status != 0) {
		return status;
	}

	status = regulator_set_min_voltage(supply.vddq);
	if (status != 0) {
		return status;
	}

	status = regulator_enable(supply.vdd1);
	if (status != 0) {
		return status;
	}

	status = regulator_enable(supply.vdd2);
	if (status != 0) {
		return status;
	}

	return regulator_enable(supply.vddq);
}
#endif /* STM32MP_LPDDR4_TYPE */

int stm32mp_board_ddr_power_init(enum ddr_type ddr_type)
{
	void *fdt = NULL;
	int node;

	VERBOSE("DDR power init, ddr_type = %u\n", ddr_type);

#if STM32MP_DDR3_TYPE
	assert(ddr_type == STM32MP_DDR3);
#elif STM32MP_DDR4_TYPE
	assert(ddr_type == STM32MP_DDR4);
#elif STM32MP_LPDDR4_TYPE
	assert(ddr_type == STM32MP_LPDDR4);
#else
	ERROR("DDR type (%u) not supported\n", ddr_type);
	panic();
#endif

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_DDR_COMPAT);
	if (node < 0) {
		ERROR("%s: Cannot read DDR node in DT\n", __func__);
		return -EINVAL;
	}

	return ddr_power_init(fdt, node);
}
