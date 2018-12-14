/*
 * Copyright (c) 2017-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>

#include <libfdt.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32mp1_clk.h>
#include <drivers/st/stm32mp1_pmic.h>
#include <drivers/st/stpmu1.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <stm32mp1_def.h>
#include <stm32mp1_dt.h>

/* I2C Timing hard-coded value, for I2C clock source is HSI at 64MHz */
#define I2C_TIMING			0x10D07DB5

#define I2C_TIMEOUT			0xFFFFF

#define MASK_RESET_BUCK3		BIT(2)

#define STPMU1_LDO12356_OUTPUT_MASK	(uint8_t)(GENMASK(6, 2))
#define STPMU1_LDO12356_OUTPUT_SHIFT	2
#define STPMU1_LDO3_MODE		(uint8_t)(BIT(7))
#define STPMU1_LDO3_DDR_SEL		31U
#define STPMU1_LDO3_1800000		(9U << STPMU1_LDO12356_OUTPUT_SHIFT)

#define STPMU1_BUCK_OUTPUT_SHIFT	2
#define STPMU1_BUCK3_1V8		(39U << STPMU1_BUCK_OUTPUT_SHIFT)

#define STPMU1_DEFAULT_START_UP_DELAY_MS	1

static struct i2c_handle_s i2c_handle;
static uint32_t pmic_i2c_addr;

static int dt_get_pmic_node(void *fdt)
{
	return fdt_node_offset_by_compatible(fdt, -1, "st,stpmu1");
}

bool dt_check_pmic(void)
{
	int node;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return false;
	}

	node = dt_get_pmic_node(fdt);
	if (node < 0) {
		VERBOSE("%s: No PMIC node found in DT\n", __func__);
		return false;
	}

	return fdt_check_status(node);
}

static int dt_pmic_i2c_config(struct dt_node_info *i2c_info)
{
	int pmic_node, i2c_node;
	void *fdt;
	const fdt32_t *cuint;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	pmic_node = dt_get_pmic_node(fdt);
	if (pmic_node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	cuint = fdt_getprop(fdt, pmic_node, "reg", NULL);
	if (cuint == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	pmic_i2c_addr = fdt32_to_cpu(*cuint) << 1;
	if (pmic_i2c_addr > UINT16_MAX) {
		return -EINVAL;
	}

	i2c_node = fdt_parent_offset(fdt, pmic_node);
	if (i2c_node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	dt_fill_device_info(i2c_info, i2c_node);
	if (i2c_info->base == 0U) {
		return -FDT_ERR_NOTFOUND;
	}

	return dt_set_pinctrl_config(i2c_node);
}

int dt_pmic_enable_boot_on_regulators(void)
{
	int pmic_node, regulators_node, regulator_node;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	pmic_node = dt_get_pmic_node(fdt);
	if (pmic_node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	regulators_node = fdt_subnode_offset(fdt, pmic_node, "regulators");

	fdt_for_each_subnode(regulator_node, fdt, regulators_node) {
		const fdt32_t *cuint;
		const char *node_name;
		uint16_t voltage;

		if (fdt_getprop(fdt, regulator_node, "regulator-boot-on",
				NULL) == NULL) {
			continue;
		}

		cuint = fdt_getprop(fdt, regulator_node,
				    "regulator-min-microvolt", NULL);
		if (cuint == NULL) {
			continue;
		}

		/* DT uses microvolts, whereas driver awaits millivolts */
		voltage = (uint16_t)(fdt32_to_cpu(*cuint) / 1000U);
		node_name = fdt_get_name(fdt, regulator_node, NULL);

		if (stpmu1_is_regulator_enabled(node_name) == 0U) {
			int status;

			status = stpmu1_regulator_voltage_set(node_name,
							      voltage);
			if (status != 0) {
				return status;
			}

			status = stpmu1_regulator_enable(node_name);
			if (status != 0) {
				return status;
			}
		}
	}

	return 0;
}

void initialize_pmic_i2c(void)
{
	int ret;
	struct dt_node_info i2c_info;

	if (dt_pmic_i2c_config(&i2c_info) != 0) {
		ERROR("I2C configuration failed\n");
		panic();
	}

	if (stm32mp1_clk_enable((uint32_t)i2c_info.clock) < 0) {
		ERROR("I2C clock enable failed\n");
		panic();
	}

	/* Initialize PMIC I2C */
	i2c_handle.i2c_base_addr		= i2c_info.base;
	i2c_handle.i2c_init.timing		= I2C_TIMING;
	i2c_handle.i2c_init.own_address1	= pmic_i2c_addr;
	i2c_handle.i2c_init.addressing_mode	= I2C_ADDRESSINGMODE_7BIT;
	i2c_handle.i2c_init.dual_address_mode	= I2C_DUALADDRESS_DISABLE;
	i2c_handle.i2c_init.own_address2	= 0;
	i2c_handle.i2c_init.own_address2_masks	= I2C_OAR2_OA2NOMASK;
	i2c_handle.i2c_init.general_call_mode	= I2C_GENERALCALL_DISABLE;
	i2c_handle.i2c_init.no_stretch_mode	= I2C_NOSTRETCH_DISABLE;

	ret = stm32_i2c_init(&i2c_handle);
	if (ret != 0) {
		ERROR("Cannot initialize I2C %x (%d)\n",
		      i2c_handle.i2c_base_addr, ret);
		panic();
	}

	ret = stm32_i2c_config_analog_filter(&i2c_handle,
					     I2C_ANALOGFILTER_ENABLE);
	if (ret != 0) {
		ERROR("Cannot initialize I2C analog filter (%d)\n", ret);
		panic();
	}

	ret = stm32_i2c_is_device_ready(&i2c_handle, (uint16_t)pmic_i2c_addr, 1,
					I2C_TIMEOUT);
	if (ret != 0) {
		ERROR("I2C device not ready (%d)\n", ret);
		panic();
	}

	stpmu1_bind_i2c(&i2c_handle, (uint16_t)pmic_i2c_addr);
}

void initialize_pmic(void)
{
	int status;
	uint8_t read_val;

	initialize_pmic_i2c();

	status = stpmu1_register_read(VERSION_STATUS_REG, &read_val);
	if (status != 0) {
		panic();
	}

	INFO("PMIC version = 0x%x\n", read_val);

	/* Keep VDD on during the reset cycle */
	status = stpmu1_register_update(MASK_RESET_BUCK_REG,
					MASK_RESET_BUCK3,
					MASK_RESET_BUCK3);
	if (status != 0) {
		panic();
	}
}

int pmic_ddr_power_init(enum ddr_type ddr_type)
{
	bool buck3_at_1v8 = false;
	uint8_t read_val;
	int status;

	switch (ddr_type) {
	case STM32MP_DDR3:
		/* Set LDO3 to sync mode */
		status = stpmu1_register_read(LDO3_CONTROL_REG, &read_val);
		if (status != 0) {
			return status;
		}

		read_val &= ~STPMU1_LDO3_MODE;
		read_val &= ~STPMU1_LDO12356_OUTPUT_MASK;
		read_val |= STPMU1_LDO3_DDR_SEL << STPMU1_LDO12356_OUTPUT_SHIFT;

		status = stpmu1_register_write(LDO3_CONTROL_REG, read_val);
		if (status != 0) {
			return status;
		}

		status = stpmu1_regulator_voltage_set("buck2", 1350);
		if (status != 0) {
			return status;
		}

		status = stpmu1_regulator_enable("buck2");
		if (status != 0) {
			return status;
		}

		mdelay(STPMU1_DEFAULT_START_UP_DELAY_MS);

		status = stpmu1_regulator_enable("vref_ddr");
		if (status != 0) {
			return status;
		}

		mdelay(STPMU1_DEFAULT_START_UP_DELAY_MS);

		status = stpmu1_regulator_enable("ldo3");
		if (status != 0) {
			return status;
		}

		mdelay(STPMU1_DEFAULT_START_UP_DELAY_MS);
		break;

	case STM32MP_LPDDR2:
		/*
		 * Set LDO3 to 1.8V
		 * Set LDO3 to bypass mode if BUCK3 = 1.8V
		 * Set LDO3 to normal mode if BUCK3 != 1.8V
		 */
		status = stpmu1_register_read(BUCK3_CONTROL_REG, &read_val);
		if (status != 0) {
			return status;
		}

		if ((read_val & STPMU1_BUCK3_1V8) == STPMU1_BUCK3_1V8) {
			buck3_at_1v8 = true;
		}

		status = stpmu1_register_read(LDO3_CONTROL_REG, &read_val);
		if (status != 0) {
			return status;
		}

		read_val &= ~STPMU1_LDO3_MODE;
		read_val &= ~STPMU1_LDO12356_OUTPUT_MASK;
		read_val |= STPMU1_LDO3_1800000;
		if (buck3_at_1v8) {
			read_val |= STPMU1_LDO3_MODE;
		}

		status = stpmu1_register_write(LDO3_CONTROL_REG, read_val);
		if (status != 0) {
			return status;
		}

		status = stpmu1_regulator_voltage_set("buck2", 1200);
		if (status != 0) {
			return status;
		}

		status = stpmu1_regulator_enable("ldo3");
		if (status != 0) {
			return status;
		}

		mdelay(STPMU1_DEFAULT_START_UP_DELAY_MS);

		status = stpmu1_regulator_enable("buck2");
		if (status != 0) {
			return status;
		}

		mdelay(STPMU1_DEFAULT_START_UP_DELAY_MS);

		status = stpmu1_regulator_enable("vref_ddr");
		if (status != 0) {
			return status;
		}

		mdelay(STPMU1_DEFAULT_START_UP_DELAY_MS);
		break;

	default:
		break;
	};

	return 0;
}
