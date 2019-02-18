/*
 * Copyright (c) 2016-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <drivers/st/stpmic1.h>

#define I2C_TIMEOUT_MS		25

struct regul_struct {
	const char *dt_node_name;
	const uint16_t *voltage_table;
	uint8_t voltage_table_size;
	uint8_t control_reg;
	uint8_t low_power_reg;
	uint8_t pull_down_reg;
	uint8_t pull_down;
	uint8_t mask_reset_reg;
	uint8_t mask_reset;
};

static struct i2c_handle_s *pmic_i2c_handle;
static uint16_t pmic_i2c_addr;

/* Voltage tables in mV */
static const uint16_t buck1_voltage_table[] = {
	725,
	725,
	725,
	725,
	725,
	725,
	750,
	775,
	800,
	825,
	850,
	875,
	900,
	925,
	950,
	975,
	1000,
	1025,
	1050,
	1075,
	1100,
	1125,
	1150,
	1175,
	1200,
	1225,
	1250,
	1275,
	1300,
	1325,
	1350,
	1375,
	1400,
	1425,
	1450,
	1475,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
	1500,
};

static const uint16_t buck2_voltage_table[] = {
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1050,
	1050,
	1100,
	1100,
	1150,
	1150,
	1200,
	1200,
	1250,
	1250,
	1300,
	1300,
	1350,
	1350,
	1400,
	1400,
	1450,
	1450,
	1500,
};

static const uint16_t buck3_voltage_table[] = {
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1000,
	1100,
	1100,
	1100,
	1100,
	1200,
	1200,
	1200,
	1200,
	1300,
	1300,
	1300,
	1300,
	1400,
	1400,
	1400,
	1400,
	1500,
	1600,
	1700,
	1800,
	1900,
	2000,
	2100,
	2200,
	2300,
	2400,
	2500,
	2600,
	2700,
	2800,
	2900,
	3000,
	3100,
	3200,
	3300,
	3400,
};

static const uint16_t buck4_voltage_table[] = {
	600,
	625,
	650,
	675,
	700,
	725,
	750,
	775,
	800,
	825,
	850,
	875,
	900,
	925,
	950,
	975,
	1000,
	1025,
	1050,
	1075,
	1100,
	1125,
	1150,
	1175,
	1200,
	1225,
	1250,
	1275,
	1300,
	1300,
	1350,
	1350,
	1400,
	1400,
	1450,
	1450,
	1500,
	1600,
	1700,
	1800,
	1900,
	2000,
	2100,
	2200,
	2300,
	2400,
	2500,
	2600,
	2700,
	2800,
	2900,
	3000,
	3100,
	3200,
	3300,
	3400,
	3500,
	3600,
	3700,
	3800,
	3900,
};

static const uint16_t ldo1_voltage_table[] = {
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1800,
	1900,
	2000,
	2100,
	2200,
	2300,
	2400,
	2500,
	2600,
	2700,
	2800,
	2900,
	3000,
	3100,
	3200,
	3300,
};

static const uint16_t ldo2_voltage_table[] = {
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1800,
	1900,
	2000,
	2100,
	2200,
	2300,
	2400,
	2500,
	2600,
	2700,
	2800,
	2900,
	3000,
	3100,
	3200,
	3300,
};

static const uint16_t ldo3_voltage_table[] = {
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1800,
	1900,
	2000,
	2100,
	2200,
	2300,
	2400,
	2500,
	2600,
	2700,
	2800,
	2900,
	3000,
	3100,
	3200,
	3300,
	3300,
	3300,
	3300,
	3300,
	3300,
	3300,
	500,
	0xFFFF, /* VREFDDR */
};

static const uint16_t ldo5_voltage_table[] = {
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1700,
	1800,
	1900,
	2000,
	2100,
	2200,
	2300,
	2400,
	2500,
	2600,
	2700,
	2800,
	2900,
	3000,
	3100,
	3200,
	3300,
	3400,
	3500,
	3600,
	3700,
	3800,
	3900,
};

static const uint16_t ldo6_voltage_table[] = {
	900,
	1000,
	1100,
	1200,
	1300,
	1400,
	1500,
	1600,
	1700,
	1800,
	1900,
	2000,
	2100,
	2200,
	2300,
	2400,
	2500,
	2600,
	2700,
	2800,
	2900,
	3000,
	3100,
	3200,
	3300,
};

static const uint16_t ldo4_voltage_table[] = {
	3300,
};

static const uint16_t vref_ddr_voltage_table[] = {
	3300,
};

/* Table of Regulators in PMIC SoC */
static const struct regul_struct regulators_table[] = {
	{
		.dt_node_name	= "buck1",
		.voltage_table	= buck1_voltage_table,
		.voltage_table_size = ARRAY_SIZE(buck1_voltage_table),
		.control_reg	= BUCK1_CONTROL_REG,
		.low_power_reg	= BUCK1_PWRCTRL_REG,
		.pull_down_reg	= BUCK_PULL_DOWN_REG,
		.pull_down	= BUCK1_PULL_DOWN_SHIFT,
		.mask_reset_reg	= MASK_RESET_BUCK_REG,
		.mask_reset	= BUCK1_MASK_RESET,
	},
	{
		.dt_node_name	= "buck2",
		.voltage_table	= buck2_voltage_table,
		.voltage_table_size = ARRAY_SIZE(buck2_voltage_table),
		.control_reg	= BUCK2_CONTROL_REG,
		.low_power_reg	= BUCK2_PWRCTRL_REG,
		.pull_down_reg	= BUCK_PULL_DOWN_REG,
		.pull_down	= BUCK2_PULL_DOWN_SHIFT,
		.mask_reset_reg	= MASK_RESET_BUCK_REG,
		.mask_reset	= BUCK2_MASK_RESET,
	},
	{
		.dt_node_name	= "buck3",
		.voltage_table	= buck3_voltage_table,
		.voltage_table_size = ARRAY_SIZE(buck3_voltage_table),
		.control_reg	= BUCK3_CONTROL_REG,
		.low_power_reg	= BUCK3_PWRCTRL_REG,
		.pull_down_reg	= BUCK_PULL_DOWN_REG,
		.pull_down	= BUCK3_PULL_DOWN_SHIFT,
		.mask_reset_reg	= MASK_RESET_BUCK_REG,
		.mask_reset	= BUCK3_MASK_RESET,
	},
	{
		.dt_node_name	= "buck4",
		.voltage_table	= buck4_voltage_table,
		.voltage_table_size = ARRAY_SIZE(buck4_voltage_table),
		.control_reg	= BUCK4_CONTROL_REG,
		.low_power_reg	= BUCK4_PWRCTRL_REG,
		.pull_down_reg	= BUCK_PULL_DOWN_REG,
		.pull_down	= BUCK4_PULL_DOWN_SHIFT,
		.mask_reset_reg	= MASK_RESET_BUCK_REG,
		.mask_reset	= BUCK4_MASK_RESET,
	},
	{
		.dt_node_name	= "ldo1",
		.voltage_table	= ldo1_voltage_table,
		.voltage_table_size = ARRAY_SIZE(ldo1_voltage_table),
		.control_reg	= LDO1_CONTROL_REG,
		.low_power_reg	= LDO1_PWRCTRL_REG,
		.mask_reset_reg	= MASK_RESET_LDO_REG,
		.mask_reset	= LDO1_MASK_RESET,
	},
	{
		.dt_node_name	= "ldo2",
		.voltage_table	= ldo2_voltage_table,
		.voltage_table_size = ARRAY_SIZE(ldo2_voltage_table),
		.control_reg	= LDO2_CONTROL_REG,
		.low_power_reg	= LDO2_PWRCTRL_REG,
		.mask_reset_reg	= MASK_RESET_LDO_REG,
		.mask_reset	= LDO2_MASK_RESET,
	},
	{
		.dt_node_name	= "ldo3",
		.voltage_table	= ldo3_voltage_table,
		.voltage_table_size = ARRAY_SIZE(ldo3_voltage_table),
		.control_reg	= LDO3_CONTROL_REG,
		.low_power_reg	= LDO3_PWRCTRL_REG,
		.mask_reset_reg	= MASK_RESET_LDO_REG,
		.mask_reset	= LDO3_MASK_RESET,
	},
	{
		.dt_node_name	= "ldo4",
		.voltage_table	= ldo4_voltage_table,
		.voltage_table_size = ARRAY_SIZE(ldo4_voltage_table),
		.control_reg	= LDO4_CONTROL_REG,
		.low_power_reg	= LDO4_PWRCTRL_REG,
		.mask_reset_reg	= MASK_RESET_LDO_REG,
		.mask_reset	= LDO4_MASK_RESET,
	},
	{
		.dt_node_name	= "ldo5",
		.voltage_table	= ldo5_voltage_table,
		.voltage_table_size = ARRAY_SIZE(ldo5_voltage_table),
		.control_reg	= LDO5_CONTROL_REG,
		.low_power_reg	= LDO5_PWRCTRL_REG,
		.mask_reset_reg	= MASK_RESET_LDO_REG,
		.mask_reset	= LDO5_MASK_RESET,
	},
	{
		.dt_node_name	= "ldo6",
		.voltage_table	= ldo6_voltage_table,
		.voltage_table_size = ARRAY_SIZE(ldo6_voltage_table),
		.control_reg	= LDO6_CONTROL_REG,
		.low_power_reg	= LDO6_PWRCTRL_REG,
		.mask_reset_reg	= MASK_RESET_LDO_REG,
		.mask_reset	= LDO6_MASK_RESET,
	},
	{
		.dt_node_name	= "vref_ddr",
		.voltage_table	= vref_ddr_voltage_table,
		.voltage_table_size = ARRAY_SIZE(vref_ddr_voltage_table),
		.control_reg	= VREF_DDR_CONTROL_REG,
		.low_power_reg	= VREF_DDR_PWRCTRL_REG,
		.mask_reset_reg	= MASK_RESET_LDO_REG,
		.mask_reset	= VREF_DDR_MASK_RESET,
	},
};

#define MAX_REGUL	ARRAY_SIZE(regulators_table)

static const struct regul_struct *get_regulator_data(const char *name)
{
	uint8_t i;

	for (i = 0 ; i < MAX_REGUL ; i++) {
		if (strncmp(name, regulators_table[i].dt_node_name,
			    strlen(regulators_table[i].dt_node_name)) == 0) {
			return &regulators_table[i];
		}
	}

	/* Regulator not found */
	panic();
	return NULL;
}

static uint8_t voltage_to_index(const char *name, uint16_t millivolts)
{
	const struct regul_struct *regul = get_regulator_data(name);
	uint8_t i;

	for (i = 0 ; i < regul->voltage_table_size ; i++) {
		if (regul->voltage_table[i] == millivolts) {
			return i;
		}
	}

	/* Voltage not found */
	panic();

	return 0;
}

int stpmic1_powerctrl_on(void)
{
	return stpmic1_register_update(MAIN_CONTROL_REG, PWRCTRL_PIN_VALID,
				       PWRCTRL_PIN_VALID);
}

int stpmic1_switch_off(void)
{
	return stpmic1_register_update(MAIN_CONTROL_REG, 1,
				       SOFTWARE_SWITCH_OFF_ENABLED);
}

int stpmic1_regulator_enable(const char *name)
{
	const struct regul_struct *regul = get_regulator_data(name);

	return stpmic1_register_update(regul->control_reg, BIT(0), BIT(0));
}

int stpmic1_regulator_disable(const char *name)
{
	const struct regul_struct *regul = get_regulator_data(name);

	return stpmic1_register_update(regul->control_reg, 0, BIT(0));
}

uint8_t stpmic1_is_regulator_enabled(const char *name)
{
	uint8_t val;
	const struct regul_struct *regul = get_regulator_data(name);

	if (stpmic1_register_read(regul->control_reg, &val) != 0) {
		panic();
	}

	return (val & 0x1U);
}

int stpmic1_regulator_voltage_set(const char *name, uint16_t millivolts)
{
	uint8_t voltage_index = voltage_to_index(name, millivolts);
	const struct regul_struct *regul = get_regulator_data(name);
	uint8_t mask;

	/* Voltage can be set for buck<N> or ldo<N> (except ldo4) regulators */
	if (strncmp(name, "buck", 4) == 0) {
		mask = BUCK_VOLTAGE_MASK;
	} else if ((strncmp(name, "ldo", 3) == 0) &&
		   (strncmp(name, "ldo4", 4) != 0)) {
		mask = LDO_VOLTAGE_MASK;
	} else {
		return 0;
	}

	return stpmic1_register_update(regul->control_reg,
				       voltage_index << LDO_BUCK_VOLTAGE_SHIFT,
				       mask);
}

int stpmic1_regulator_pull_down_set(const char *name)
{
	const struct regul_struct *regul = get_regulator_data(name);

	if (regul->pull_down_reg != 0) {
		return stpmic1_register_update(regul->pull_down_reg,
					       BIT(regul->pull_down),
					       LDO_BUCK_PULL_DOWN_MASK <<
					       regul->pull_down);
	}

	return 0;
}

int stpmic1_regulator_mask_reset_set(const char *name)
{
	const struct regul_struct *regul = get_regulator_data(name);

	return stpmic1_register_update(regul->mask_reset_reg,
				       BIT(regul->mask_reset),
				       LDO_BUCK_RESET_MASK <<
				       regul->mask_reset);
}

int stpmic1_regulator_voltage_get(const char *name)
{
	const struct regul_struct *regul = get_regulator_data(name);
	uint8_t value;
	uint8_t mask;

	/* Voltage can be set for buck<N> or ldo<N> (except ldo4) regulators */
	if (strncmp(name, "buck", 4) == 0) {
		mask = BUCK_VOLTAGE_MASK;
	} else if ((strncmp(name, "ldo", 3) == 0) &&
		   (strncmp(name, "ldo4", 4) != 0)) {
		mask = LDO_VOLTAGE_MASK;
	} else {
		return 0;
	}

	if (stpmic1_register_read(regul->control_reg, &value))
		return -1;

	value = (value & mask) >> LDO_BUCK_VOLTAGE_SHIFT;

	if (value > regul->voltage_table_size)
		return -1;

	return (int)regul->voltage_table[value];
}

int stpmic1_register_read(uint8_t register_id,  uint8_t *value)
{
	return stm32_i2c_mem_read(pmic_i2c_handle, pmic_i2c_addr,
				  (uint16_t)register_id,
				  I2C_MEMADD_SIZE_8BIT, value,
				  1, I2C_TIMEOUT_MS);
}

int stpmic1_register_write(uint8_t register_id, uint8_t value)
{
	int status;

	status = stm32_i2c_mem_write(pmic_i2c_handle, pmic_i2c_addr,
				     (uint16_t)register_id,
				     I2C_MEMADD_SIZE_8BIT, &value,
				     1, I2C_TIMEOUT_MS);

#if ENABLE_ASSERTIONS
	if (status != 0) {
		return status;
	}

	if ((register_id != WATCHDOG_CONTROL_REG) && (register_id <= 0x40U)) {
		uint8_t readval;

		status = stpmic1_register_read(register_id, &readval);
		if (status != 0) {
			return status;
		}

		if (readval != value) {
			return -1;
		}
	}
#endif

	return status;
}

int stpmic1_register_update(uint8_t register_id, uint8_t value, uint8_t mask)
{
	int status;
	uint8_t val;

	status = stpmic1_register_read(register_id, &val);
	if (status != 0) {
		return status;
	}

	val = (val & ~mask) | (value & mask);

	return stpmic1_register_write(register_id, val);
}

void stpmic1_bind_i2c(struct i2c_handle_s *i2c_handle, uint16_t i2c_addr)
{
	pmic_i2c_handle = i2c_handle;
	pmic_i2c_addr = i2c_addr;
}

void stpmic1_dump_regulators(void)
{
	uint32_t i;

	for (i = 0U; i < MAX_REGUL; i++) {
		const char *name __unused = regulators_table[i].dt_node_name;

		VERBOSE("PMIC regul %s: %sable, %dmV",
			name,
			stpmic1_is_regulator_enabled(name) ? "en" : "dis",
			stpmic1_regulator_voltage_get(name));
	}
}

int stpmic1_get_version(unsigned long *version)
{
	int rc;
	uint8_t read_val;

	rc = stpmic1_register_read(VERSION_STATUS_REG, &read_val);
	if (rc) {
		return -1;
	}

	*version = (unsigned long)read_val;

	return 0;
}
