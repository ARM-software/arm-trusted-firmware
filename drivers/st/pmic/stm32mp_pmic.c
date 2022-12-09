/*
 * Copyright (c) 2017-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/regulator.h>
#include <drivers/st/stm32_i2c.h>
#include <drivers/st/stm32mp_pmic.h>
#include <drivers/st/stpmic1.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <libfdt.h>

#include <platform_def.h>

#define PMIC_NODE_NOT_FOUND	1
#define NB_REG			14U

static struct i2c_handle_s i2c_handle;
static uint32_t pmic_i2c_addr;

static int register_pmic(void);

static int dt_get_pmic_node(void *fdt)
{
	static int node = -FDT_ERR_BADOFFSET;

	if (node == -FDT_ERR_BADOFFSET) {
		node = fdt_node_offset_by_compatible(fdt, -1, "st,stpmic1");
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

	status = (int)fdt_get_status(node);

	return status;
}

static bool dt_pmic_is_secure(void)
{
	int status = dt_pmic_status();

	return (status >= 0) &&
	       (status == DT_SECURE) &&
	       (i2c_handle.dt_status == DT_SECURE);
}

/*
 * Get PMIC and its I2C bus configuration from the device tree.
 * Return 0 on success, negative on error, 1 if no PMIC node is defined.
 */
static int dt_pmic_i2c_config(struct dt_node_info *i2c_info,
			      struct stm32_i2c_init_s *init)
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

		pmic_i2c_addr = fdt32_to_cpu(*cuint) << 1;
		if (pmic_i2c_addr > UINT16_MAX) {
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

	return stm32_i2c_get_setup_from_fdt(fdt, i2c_node, init);
}

bool initialize_pmic_i2c(void)
{
	int ret;
	struct dt_node_info i2c_info;
	struct i2c_handle_s *i2c = &i2c_handle;
	struct stm32_i2c_init_s i2c_init;

	ret = dt_pmic_i2c_config(&i2c_info, &i2c_init);
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
	i2c_init.own_address1		= pmic_i2c_addr;
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

	if (!stm32_i2c_is_device_ready(i2c, pmic_i2c_addr, 1,
				       I2C_TIMEOUT_BUSY_MS)) {
		ERROR("I2C device not ready\n");
		panic();
	}

	stpmic1_bind_i2c(i2c, (uint16_t)pmic_i2c_addr);

	return true;
}

static void register_pmic_shared_peripherals(void)
{
	uintptr_t i2c_base = i2c_handle.i2c_base_addr;

	if (dt_pmic_is_secure()) {
		stm32mp_register_secure_periph_iomem(i2c_base);
	} else {
		if (i2c_base != 0U) {
			stm32mp_register_non_secure_periph_iomem(i2c_base);
		}
	}
}

void initialize_pmic(void)
{
	if (!initialize_pmic_i2c()) {
		VERBOSE("No PMIC\n");
		return;
	}

	register_pmic_shared_peripherals();

	if (register_pmic() < 0) {
		panic();
	}

	if (stpmic1_powerctrl_on() < 0) {
		panic();
	}

}

#if DEBUG
void print_pmic_info_and_debug(void)
{
	unsigned long pmic_version;

	if (stpmic1_get_version(&pmic_version) != 0) {
		ERROR("Failed to access PMIC\n");
		panic();
	}

	INFO("PMIC version = 0x%02lx\n", pmic_version);
}
#endif

int pmic_ddr_power_init(enum ddr_type ddr_type)
{
	int status;
	uint16_t buck3_min_mv;
	struct rdev *buck2, *buck3, *vref;
	struct rdev *ldo3 __unused;

	buck2 = regulator_get_by_name("buck2");
	if (buck2 == NULL) {
		return -ENOENT;
	}

#if STM32MP15
	ldo3 = regulator_get_by_name("ldo3");
	if (ldo3 == NULL) {
		return -ENOENT;
	}
#endif

	vref = regulator_get_by_name("vref_ddr");
	if (vref == NULL) {
		return -ENOENT;
	}

	switch (ddr_type) {
	case STM32MP_DDR3:
#if STM32MP15
		status = regulator_set_flag(ldo3, REGUL_SINK_SOURCE);
		if (status != 0) {
			return status;
		}
#endif

		status = regulator_set_min_voltage(buck2);
		if (status != 0) {
			return status;
		}

		status = regulator_enable(buck2);
		if (status != 0) {
			return status;
		}

		status = regulator_enable(vref);
		if (status != 0) {
			return status;
		}

#if STM32MP15
		status = regulator_enable(ldo3);
		if (status != 0) {
			return status;
		}
#endif
		break;

	case STM32MP_LPDDR2:
	case STM32MP_LPDDR3:
		/*
		 * Set LDO3 to 1.8V
		 * Set LDO3 to bypass mode if BUCK3 = 1.8V
		 * Set LDO3 to normal mode if BUCK3 != 1.8V
		 */
		buck3 = regulator_get_by_name("buck3");
		if (buck3 == NULL) {
			return -ENOENT;
		}

		regulator_get_range(buck3, &buck3_min_mv, NULL);

#if STM32MP15
		if (buck3_min_mv != 1800) {
			status = regulator_set_min_voltage(ldo3);
			if (status != 0) {
				return status;
			}
		} else {
			status = regulator_set_flag(ldo3, REGUL_ENABLE_BYPASS);
			if (status != 0) {
				return status;
			}
		}
#endif

		status = regulator_set_min_voltage(buck2);
		if (status != 0) {
			return status;
		}

#if STM32MP15
		status = regulator_enable(ldo3);
		if (status != 0) {
			return status;
		}
#endif

		status = regulator_enable(buck2);
		if (status != 0) {
			return status;
		}

		status = regulator_enable(vref);
		if (status != 0) {
			return status;
		}
		break;

	default:
		break;
	};

	return 0;
}

int pmic_voltages_init(void)
{
#if STM32MP13
	struct rdev *buck1, *buck4;
	int status;

	buck1 = regulator_get_by_name("buck1");
	if (buck1 == NULL) {
		return -ENOENT;
	}

	buck4 = regulator_get_by_name("buck4");
	if (buck4 == NULL) {
		return -ENOENT;
	}

	status = regulator_set_min_voltage(buck1);
	if (status != 0) {
		return status;
	}

	status = regulator_set_min_voltage(buck4);
	if (status != 0) {
		return status;
	}
#endif

	return 0;
}

enum {
	STPMIC1_BUCK1 = 0,
	STPMIC1_BUCK2,
	STPMIC1_BUCK3,
	STPMIC1_BUCK4,
	STPMIC1_LDO1,
	STPMIC1_LDO2,
	STPMIC1_LDO3,
	STPMIC1_LDO4,
	STPMIC1_LDO5,
	STPMIC1_LDO6,
	STPMIC1_VREF_DDR,
	STPMIC1_BOOST,
	STPMIC1_VBUS_OTG,
	STPMIC1_SW_OUT,
};

static int pmic_set_state(const struct regul_description *desc, bool enable)
{
	VERBOSE("%s: set state to %d\n", desc->node_name, enable);

	if (enable == STATE_ENABLE) {
		return stpmic1_regulator_enable(desc->node_name);
	} else {
		return stpmic1_regulator_disable(desc->node_name);
	}
}

static int pmic_get_state(const struct regul_description *desc)
{
	VERBOSE("%s: get state\n", desc->node_name);

	return stpmic1_is_regulator_enabled(desc->node_name);
}

static int pmic_get_voltage(const struct regul_description *desc)
{
	VERBOSE("%s: get volt\n", desc->node_name);

	return stpmic1_regulator_voltage_get(desc->node_name);
}

static int pmic_set_voltage(const struct regul_description *desc, uint16_t mv)
{
	VERBOSE("%s: get volt\n", desc->node_name);

	return stpmic1_regulator_voltage_set(desc->node_name, mv);
}

static int pmic_list_voltages(const struct regul_description *desc,
			      const uint16_t **levels, size_t *count)
{
	VERBOSE("%s: list volt\n", desc->node_name);

	return stpmic1_regulator_levels_mv(desc->node_name, levels, count);
}

static int pmic_set_flag(const struct regul_description *desc, uint16_t flag)
{
	VERBOSE("%s: set_flag 0x%x\n", desc->node_name, flag);

	switch (flag) {
	case REGUL_OCP:
		return stpmic1_regulator_icc_set(desc->node_name);

	case REGUL_ACTIVE_DISCHARGE:
		return stpmic1_active_discharge_mode_set(desc->node_name);

	case REGUL_PULL_DOWN:
		return stpmic1_regulator_pull_down_set(desc->node_name);

	case REGUL_MASK_RESET:
		return stpmic1_regulator_mask_reset_set(desc->node_name);

	case REGUL_SINK_SOURCE:
		return stpmic1_regulator_sink_mode_set(desc->node_name);

	case REGUL_ENABLE_BYPASS:
		return stpmic1_regulator_bypass_mode_set(desc->node_name);

	default:
		return -EINVAL;
	}
}

static const struct regul_ops pmic_ops = {
	.set_state = pmic_set_state,
	.get_state = pmic_get_state,
	.set_voltage = pmic_set_voltage,
	.get_voltage = pmic_get_voltage,
	.list_voltages = pmic_list_voltages,
	.set_flag = pmic_set_flag,
};

#define DEFINE_REGU(name) { \
	.node_name = (name), \
	.ops = &pmic_ops, \
	.driver_data = NULL, \
	.enable_ramp_delay = 1000, \
}

static const struct regul_description pmic_regs[NB_REG] = {
	[STPMIC1_BUCK1] = DEFINE_REGU("buck1"),
	[STPMIC1_BUCK2] = DEFINE_REGU("buck2"),
	[STPMIC1_BUCK3] = DEFINE_REGU("buck3"),
	[STPMIC1_BUCK4] = DEFINE_REGU("buck4"),
	[STPMIC1_LDO1] = DEFINE_REGU("ldo1"),
	[STPMIC1_LDO2] = DEFINE_REGU("ldo2"),
	[STPMIC1_LDO3] = DEFINE_REGU("ldo3"),
	[STPMIC1_LDO4] = DEFINE_REGU("ldo4"),
	[STPMIC1_LDO5] = DEFINE_REGU("ldo5"),
	[STPMIC1_LDO6] = DEFINE_REGU("ldo6"),
	[STPMIC1_VREF_DDR] = DEFINE_REGU("vref_ddr"),
	[STPMIC1_BOOST] = DEFINE_REGU("boost"),
	[STPMIC1_VBUS_OTG] = DEFINE_REGU("pwr_sw1"),
	[STPMIC1_SW_OUT] = DEFINE_REGU("pwr_sw2"),
};

static int register_pmic(void)
{
	void *fdt;
	int pmic_node, regulators_node, subnode;

	VERBOSE("Register pmic\n");

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

		for (i = 0U; i < NB_REG; i++) {
			desc = &pmic_regs[i];
			if (strcmp(desc->node_name, reg_name) == 0) {
				break;
			}
		}
		assert(i < NB_REG);

		ret = regulator_register(desc, subnode);
		if (ret != 0) {
			WARN("%s:%d failed to register %s\n", __func__,
			     __LINE__, reg_name);
			return ret;
		}
	}

	return 0;
}
