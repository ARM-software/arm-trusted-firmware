/*
 * Copyright (c) 2017-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <libfdt.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_i2c.h>
#include <drivers/st/stm32mp_pmic.h>
#include <drivers/st/stpmic1.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define STPMIC1_LDO12356_OUTPUT_MASK	(uint8_t)(GENMASK(6, 2))
#define STPMIC1_LDO12356_OUTPUT_SHIFT	2
#define STPMIC1_LDO3_MODE		(uint8_t)(BIT(7))
#define STPMIC1_LDO3_DDR_SEL		31U
#define STPMIC1_LDO3_1800000		(9U << STPMIC1_LDO12356_OUTPUT_SHIFT)

#define STPMIC1_BUCK_OUTPUT_SHIFT	2
#define STPMIC1_BUCK3_1V8		(39U << STPMIC1_BUCK_OUTPUT_SHIFT)

#define STPMIC1_DEFAULT_START_UP_DELAY_MS	1

static struct i2c_handle_s i2c_handle;
static uint32_t pmic_i2c_addr;

static int dt_get_pmic_node(void *fdt)
{
	return fdt_node_offset_by_compatible(fdt, -1, "st,stpmic1");
}

int dt_pmic_status(void)
{
	int node;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = dt_get_pmic_node(fdt);
	if (node <= 0) {
		return -FDT_ERR_NOTFOUND;
	}

	return fdt_get_status(node);
}

/*
 * Get PMIC and its I2C bus configuration from the device tree.
 * Return 0 on success, negative on error, 1 if no PMIC node is found.
 */
static int dt_pmic_i2c_config(struct dt_node_info *i2c_info,
			      struct stm32_i2c_init_s *init)
{
	int pmic_node, i2c_node;
	void *fdt;
	const fdt32_t *cuint;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	pmic_node = dt_get_pmic_node(fdt);
	if (pmic_node < 0) {
		return 1;
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

	return stm32_i2c_get_setup_from_fdt(fdt, i2c_node, init);
}

int dt_pmic_configure_boot_on_regulators(void)
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
		const char *node_name = fdt_get_name(fdt, regulator_node, NULL);
		uint16_t voltage;
		int status;

#if defined(IMAGE_BL2)
		if ((fdt_getprop(fdt, regulator_node, "regulator-boot-on",
				 NULL) == NULL) &&
		    (fdt_getprop(fdt, regulator_node, "regulator-always-on",
				 NULL) == NULL)) {
#else
		if (fdt_getprop(fdt, regulator_node, "regulator-boot-on",
				NULL) == NULL) {
#endif
			continue;
		}

		if (fdt_getprop(fdt, regulator_node, "regulator-pull-down",
				NULL) != NULL) {

			status = stpmic1_regulator_pull_down_set(node_name);
			if (status != 0) {
				return status;
			}
		}

		if (fdt_getprop(fdt, regulator_node, "st,mask-reset",
				NULL) != NULL) {

			status = stpmic1_regulator_mask_reset_set(node_name);
			if (status != 0) {
				return status;
			}
		}

		cuint = fdt_getprop(fdt, regulator_node,
				    "regulator-min-microvolt", NULL);
		if (cuint == NULL) {
			continue;
		}

		/* DT uses microvolts, whereas driver awaits millivolts */
		voltage = (uint16_t)(fdt32_to_cpu(*cuint) / 1000U);

		status = stpmic1_regulator_voltage_set(node_name, voltage);
		if (status != 0) {
			return status;
		}

		if (stpmic1_is_regulator_enabled(node_name) == 0U) {
			status = stpmic1_regulator_enable(node_name);
			if (status != 0) {
				return status;
			}
		}
	}

	return 0;
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

void initialize_pmic(void)
{
	unsigned long pmic_version;

	if (!initialize_pmic_i2c()) {
		VERBOSE("No PMIC\n");
		return;
	}

	if (stpmic1_get_version(&pmic_version) != 0) {
		ERROR("Failed to access PMIC\n");
		panic();
	}

	INFO("PMIC version = 0x%02lx\n", pmic_version);
	stpmic1_dump_regulators();

#if defined(IMAGE_BL2)
	if (dt_pmic_configure_boot_on_regulators() != 0) {
		panic();
	};
#endif
}

int pmic_ddr_power_init(enum ddr_type ddr_type)
{
	bool buck3_at_1v8 = false;
	uint8_t read_val;
	int status;

	switch (ddr_type) {
	case STM32MP_DDR3:
		/* Set LDO3 to sync mode */
		status = stpmic1_register_read(LDO3_CONTROL_REG, &read_val);
		if (status != 0) {
			return status;
		}

		read_val &= ~STPMIC1_LDO3_MODE;
		read_val &= ~STPMIC1_LDO12356_OUTPUT_MASK;
		read_val |= STPMIC1_LDO3_DDR_SEL <<
			    STPMIC1_LDO12356_OUTPUT_SHIFT;

		status = stpmic1_register_write(LDO3_CONTROL_REG, read_val);
		if (status != 0) {
			return status;
		}

		status = stpmic1_regulator_voltage_set("buck2", 1350);
		if (status != 0) {
			return status;
		}

		status = stpmic1_regulator_enable("buck2");
		if (status != 0) {
			return status;
		}

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		status = stpmic1_regulator_enable("vref_ddr");
		if (status != 0) {
			return status;
		}

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		status = stpmic1_regulator_enable("ldo3");
		if (status != 0) {
			return status;
		}

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);
		break;

	case STM32MP_LPDDR2:
	case STM32MP_LPDDR3:
		/*
		 * Set LDO3 to 1.8V
		 * Set LDO3 to bypass mode if BUCK3 = 1.8V
		 * Set LDO3 to normal mode if BUCK3 != 1.8V
		 */
		status = stpmic1_register_read(BUCK3_CONTROL_REG, &read_val);
		if (status != 0) {
			return status;
		}

		if ((read_val & STPMIC1_BUCK3_1V8) == STPMIC1_BUCK3_1V8) {
			buck3_at_1v8 = true;
		}

		status = stpmic1_register_read(LDO3_CONTROL_REG, &read_val);
		if (status != 0) {
			return status;
		}

		read_val &= ~STPMIC1_LDO3_MODE;
		read_val &= ~STPMIC1_LDO12356_OUTPUT_MASK;
		read_val |= STPMIC1_LDO3_1800000;
		if (buck3_at_1v8) {
			read_val |= STPMIC1_LDO3_MODE;
		}

		status = stpmic1_register_write(LDO3_CONTROL_REG, read_val);
		if (status != 0) {
			return status;
		}

		status = stpmic1_regulator_voltage_set("buck2", 1200);
		if (status != 0) {
			return status;
		}

		status = stpmic1_regulator_enable("ldo3");
		if (status != 0) {
			return status;
		}

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		status = stpmic1_regulator_enable("buck2");
		if (status != 0) {
			return status;
		}

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		status = stpmic1_regulator_enable("vref_ddr");
		if (status != 0) {
			return status;
		}

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);
		break;

	default:
		break;
	};

	return 0;
}
