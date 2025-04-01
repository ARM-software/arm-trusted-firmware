/*
 * Copyright (C) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/regulator.h>
#include <drivers/st/stm32_i2c.h>
#include <drivers/st/stm32mp_pmic2.h>
#include <drivers/st/stpmic2.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <libfdt.h>

#include <platform_def.h>

#define PMIC_NODE_NOT_FOUND	1

struct regul_handle_s {
	const uint32_t id;
	uint16_t bypass_mv;
};

static struct pmic_handle_s pmic2_handle;
static struct i2c_handle_s i2c_handle;

/* This driver is monoinstance */
static struct pmic_handle_s *pmic2;

static int dt_get_pmic_node(void *fdt)
{
	static int node = -FDT_ERR_BADOFFSET;

	if (node == -FDT_ERR_BADOFFSET) {
		node = fdt_node_offset_by_compatible(fdt, -1, "st,stpmic2");
	}

	return node;
}

int dt_pmic_status(void)
{
	static int status = -FDT_ERR_BADVALUE;
	int node;
	void *fdt;

	if (status != -FDT_ERR_BADVALUE) {
		return status;
	}

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = dt_get_pmic_node(fdt);
	if (node <= 0) {
		status = -FDT_ERR_NOTFOUND;

		return status;
	}

	status = DT_SECURE;

	return status;
}

/*
 * Get PMIC and its I2C bus configuration from the device tree.
 * Return 0 on success, negative on error, 1 if no PMIC node is defined.
 */
static int dt_pmic2_i2c_config(struct dt_node_info *i2c_info,
			       struct stm32_i2c_init_s *init,
			       uint32_t *i2c_addr)
{
	static int i2c_node = -FDT_ERR_NOTFOUND;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	if (i2c_node == -FDT_ERR_NOTFOUND) {
		int pmic_node;
		const fdt32_t *cuint;

		pmic_node = dt_get_pmic_node(fdt);
		if (pmic_node < 0) {
			return PMIC_NODE_NOT_FOUND;
		}

		cuint = fdt_getprop(fdt, pmic_node, "reg", NULL);
		if (cuint == NULL) {
			return -FDT_ERR_NOTFOUND;
		}

		*i2c_addr = fdt32_to_cpu(*cuint) << 1;
		if (*i2c_addr > UINT16_MAX) {
			return -FDT_ERR_BADVALUE;
		}

		i2c_node = fdt_parent_offset(fdt, pmic_node);
		if (i2c_node < 0) {
			return -FDT_ERR_NOTFOUND;
		}
	}

	dt_fill_device_info(i2c_info, i2c_node);
	if (i2c_info->base == 0U) {
		return -FDT_ERR_NOTFOUND;
	}

	i2c_info->status = DT_SECURE;

	return stm32_i2c_get_setup_from_fdt(fdt, i2c_node, init);
}

bool initialize_pmic_i2c(void)
{
	int ret;
	struct dt_node_info i2c_info;
	struct i2c_handle_s *i2c = &i2c_handle;
	uint32_t i2c_addr = 0U;
	struct stm32_i2c_init_s i2c_init;

	ret = dt_pmic2_i2c_config(&i2c_info, &i2c_init, &i2c_addr);
	if (ret < 0) {
		ERROR("I2C configuration failed %d\n", ret);
		panic();
	}

	if (ret != 0) {
		return false;
	}

	/* Initialize PMIC I2C */
	i2c->i2c_base_addr		= i2c_info.base;
	i2c->dt_status			= i2c_info.status;
	i2c->clock			= i2c_info.clock;
	i2c->i2c_state			= I2C_STATE_RESET;
	i2c_init.own_address1		= i2c_addr;
	i2c_init.addressing_mode	= I2C_ADDRESSINGMODE_7BIT;
	i2c_init.dual_address_mode	= I2C_DUALADDRESS_DISABLE;
	i2c_init.own_address2		= 0;
	i2c_init.own_address2_masks	= I2C_OAR2_OA2NOMASK;
	i2c_init.general_call_mode	= I2C_GENERALCALL_DISABLE;
	i2c_init.no_stretch_mode	= I2C_NOSTRETCH_DISABLE;
	i2c_init.analog_filter		= 1;
	i2c_init.digital_filter_coef	= 0;

	ret = stm32_i2c_init(i2c, &i2c_init);
	if (ret != 0) {
		ERROR("Cannot initialize I2C %x (%d)\n",
		      i2c->i2c_base_addr, ret);
		panic();
	}

	if (!stm32_i2c_is_device_ready(i2c, i2c_addr, 1,
				       I2C_TIMEOUT_BUSY_MS)) {
		ERROR("I2C device not ready\n");
		panic();
	}

	pmic2 = &pmic2_handle;
	pmic2->i2c_handle = &i2c_handle;
	pmic2->i2c_addr = i2c_addr;

	return true;
}

static int pmic2_set_state(const struct regul_description *desc, bool enable)
{
	struct regul_handle_s *regul = (struct regul_handle_s *)desc->driver_data;

	VERBOSE("%s: set state to %d\n", desc->node_name, enable);

	return stpmic2_regulator_set_state(pmic2, regul->id, enable);
}

static int pmic2_get_state(const struct regul_description *desc)
{
	struct regul_handle_s *regul = (struct regul_handle_s *)desc->driver_data;
	bool enabled;

	VERBOSE("%s: get state\n", desc->node_name);

	if (stpmic2_regulator_get_state(pmic2, regul->id, &enabled) < 0) {
		panic();
	}

	return enabled;
}

static int pmic2_get_voltage(const struct regul_description *desc)
{
	struct regul_handle_s *regul = (struct regul_handle_s *)desc->driver_data;
	uint16_t mv;

	VERBOSE("%s: get volt\n", desc->node_name);

	if (regul->bypass_mv != 0U) {
		int ret;

		/* If the regul is in bypass mode, return bypass value */
		ret = stpmic2_regulator_get_prop(pmic2, regul->id, STPMIC2_BYPASS);
		if (ret < 0) {
			return ret;
		}

		if (ret == 1) {
			return regul->bypass_mv;
		}
	};

	if (stpmic2_regulator_get_voltage(pmic2, regul->id, &mv) < 0) {
		panic();
	}

	return mv;
}

static int pmic2_set_voltage(const struct regul_description *desc, uint16_t mv)
{
	struct regul_handle_s *regul = (struct regul_handle_s *)desc->driver_data;

	VERBOSE("%s: set volt\n", desc->node_name);

	if (regul->bypass_mv != 0U) {
		int ret;

		/* If the regul is in bypass mode, authorize bypass mV */
		ret = stpmic2_regulator_get_prop(pmic2, regul->id, STPMIC2_BYPASS);
		if (ret < 0) {
			return ret;
		}

		if ((ret == 1) && (mv != regul->bypass_mv)) {
			return -EPERM;
		}
	};

	return stpmic2_regulator_set_voltage(pmic2, regul->id, mv);
}

static int pmic2_list_voltages(const struct regul_description *desc,
			       const uint16_t **levels, size_t *count)
{
	struct regul_handle_s *regul = (struct regul_handle_s *)desc->driver_data;

	VERBOSE("%s: list volt\n", desc->node_name);

	if (regul->bypass_mv != 0U) {
		int ret;

		ret = stpmic2_regulator_get_prop(pmic2, regul->id, STPMIC2_BYPASS);
		if (ret < 0) {
			return ret;
		}

		/* bypass is enabled, return a list with only bypass mV */
		if (ret == 1) {
			if (count != NULL) {
				*count = 1U;
			}
			if (levels != NULL) {
				*levels = &regul->bypass_mv;
			}
			return 0;
		}
	};

	return stpmic2_regulator_levels_mv(pmic2, regul->id, levels, count);
}

static int pmic2_set_flag(const struct regul_description *desc, uint16_t flag)
{
	struct regul_handle_s *regul = (struct regul_handle_s *)desc->driver_data;
	uint32_t id = regul->id;
	int ret = -EPERM;

	VERBOSE("%s: set_flag 0x%x\n", desc->node_name, flag);

	switch (flag) {
	case REGUL_PULL_DOWN:
		ret = stpmic2_regulator_set_prop(pmic2, id, STPMIC2_PULL_DOWN, 1U);
		break;
	case REGUL_OCP:
		ret = stpmic2_regulator_set_prop(pmic2, id, STPMIC2_OCP, 1U);
		break;
	case REGUL_SINK_SOURCE:
		ret = stpmic2_regulator_set_prop(pmic2, id, STPMIC2_SINK_SOURCE, 1U);
		break;
	case REGUL_ENABLE_BYPASS:
		ret = stpmic2_regulator_set_prop(pmic2, id, STPMIC2_BYPASS, 1U);
		break;
	case REGUL_MASK_RESET:
		ret = stpmic2_regulator_set_prop(pmic2, id, STPMIC2_MASK_RESET, 1U);
		break;
	default:
		ERROR("Invalid flag %u", flag);
		panic();
	}

	if (ret != 0) {
		return -EPERM;
	}

	return 0;
}

int stpmic2_set_prop(const struct regul_description *desc, uint16_t prop, uint32_t value)
{
	struct regul_handle_s *regul = (struct regul_handle_s *)desc->driver_data;
	int ret;

	VERBOSE("%s: set_prop 0x%x val=%u\n", desc->node_name, prop, value);

	ret = stpmic2_regulator_set_prop(pmic2, regul->id, prop, value);
	if (ret != 0)
		return -EPERM;

	return 0;
}

static struct regul_ops pmic2_ops = {
	.set_state = pmic2_set_state,
	.get_state = pmic2_get_state,
	.set_voltage = pmic2_set_voltage,
	.get_voltage = pmic2_get_voltage,
	.list_voltages = pmic2_list_voltages,
	.set_flag = pmic2_set_flag,
};

#define DEFINE_PMIC_REGUL_HANDLE(rid) \
[(rid)] = { \
	.id = (rid), \
}

static struct regul_handle_s pmic2_regul_handles[STPMIC2_NB_REG] = {
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_BUCK1),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_BUCK2),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_BUCK3),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_BUCK4),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_BUCK5),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_BUCK6),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_BUCK7),

	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_LDO1),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_LDO2),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_LDO3),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_LDO4),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_LDO5),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_LDO6),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_LDO7),
	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_LDO8),

	DEFINE_PMIC_REGUL_HANDLE(STPMIC2_REFDDR),
};

#define DEFINE_REGUL(rid, name) \
[rid] = { \
	.node_name = name, \
	.ops = &pmic2_ops, \
	.driver_data = &pmic2_regul_handles[rid], \
}

static const struct regul_description pmic2_descs[STPMIC2_NB_REG] = {
	DEFINE_REGUL(STPMIC2_BUCK1, "buck1"),
	DEFINE_REGUL(STPMIC2_BUCK2, "buck2"),
	DEFINE_REGUL(STPMIC2_BUCK3, "buck3"),
	DEFINE_REGUL(STPMIC2_BUCK4, "buck4"),
	DEFINE_REGUL(STPMIC2_BUCK5, "buck5"),
	DEFINE_REGUL(STPMIC2_BUCK6, "buck6"),
	DEFINE_REGUL(STPMIC2_BUCK7, "buck7"),

	DEFINE_REGUL(STPMIC2_LDO1, "ldo1"),
	DEFINE_REGUL(STPMIC2_LDO2, "ldo2"),
	DEFINE_REGUL(STPMIC2_LDO3, "ldo3"),
	DEFINE_REGUL(STPMIC2_LDO4, "ldo4"),
	DEFINE_REGUL(STPMIC2_LDO5, "ldo5"),
	DEFINE_REGUL(STPMIC2_LDO6, "ldo6"),
	DEFINE_REGUL(STPMIC2_LDO7, "ldo7"),
	DEFINE_REGUL(STPMIC2_LDO8, "ldo8"),

	DEFINE_REGUL(STPMIC2_REFDDR, "refddr"),
};

static int register_pmic2(void)
{
	void *fdt;
	int pmic_node, regulators_node, subnode;

	VERBOSE("Register pmic2\n");

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	pmic_node = dt_get_pmic_node(fdt);
	if (pmic_node < 0) {
		return pmic_node;
	}

	regulators_node = fdt_subnode_offset(fdt, pmic_node, "regulators");
	if (regulators_node < 0) {
		return -ENOENT;
	}

	fdt_for_each_subnode(subnode, fdt, regulators_node) {
		const char *reg_name = fdt_get_name(fdt, subnode, NULL);
		const struct regul_description *desc;
		unsigned int i;
		int ret;
		const fdt32_t *cuint;

		for (i = 0; i < STPMIC2_NB_REG; i++) {
			desc = &pmic2_descs[i];
			if (strcmp(desc->node_name, reg_name) == 0) {
				break;
			}
		}
		assert(i < STPMIC2_NB_REG);

		ret = regulator_register(desc, subnode);
		if (ret != 0) {
			WARN("%s:%d failed to register %s\n", __func__,
			     __LINE__, reg_name);
			return ret;
		}

		cuint = fdt_getprop(fdt, subnode, "st,regulator-bypass-microvolt", NULL);
		if (cuint != NULL) {
			struct regul_handle_s *regul = (struct regul_handle_s *)desc->driver_data;

			regul->bypass_mv = (uint16_t)(fdt32_to_cpu(*cuint) / 1000U);
			VERBOSE("%s: bypass voltage=%umV\n", desc->node_name,
				regul->bypass_mv);
		}

		if (fdt_getprop(fdt, subnode, "st,mask-reset", NULL)  != NULL) {
			VERBOSE("%s: set mask-reset\n", desc->node_name);
			ret = pmic2_set_flag(desc, REGUL_MASK_RESET);
			if (ret != 0) {
				ERROR("set mask-reset failed\n");
				return ret;
			}
		}

		if (fdt_getprop(fdt, subnode, "st,regulator-sink-source", NULL) != NULL) {
			VERBOSE("%s: set regulator-sink-source\n", desc->node_name);
			ret = pmic2_set_flag(desc, REGUL_SINK_SOURCE);
			if (ret != 0) {
				ERROR("set regulator-sink-source failed\n");
				return ret;
			}
		}
	}

	return 0;
}

void initialize_pmic(void)
{
	int ret;
	uint8_t val;

	ret = initialize_pmic_i2c();
	if (!ret) {
		VERBOSE("No PMIC2\n");
		return;
	}

	if (stpmic2_get_version(pmic2, &val) != 0) {
		ERROR("Failed to access PMIC\n");
		panic();
	}
	INFO("PMIC2 version = 0x%02x\n", val);

	if (stpmic2_get_product_id(pmic2, &val) != 0) {
		ERROR("Failed to access PMIC\n");
		panic();
	}
	INFO("PMIC2 product ID = 0x%02x\n", val);

	ret = register_pmic2();
	if (ret < 0) {
		ERROR("Register pmic2 failed\n");
		panic();
	}

	stpmic2_dump_regulators(pmic2);
}
