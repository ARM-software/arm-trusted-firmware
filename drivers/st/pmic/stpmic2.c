/*
 * Copyright (C) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/st/stpmic2.h>

#define RET_SUCCESS			0
#define RET_ERROR_NOT_SUPPORTED		-1
#define RET_ERROR_GENERIC		-2
#define RET_ERROR_BAD_PARAMETERS	-3

#define I2C_TIMEOUT_MS			25

#define VOLTAGE_INDEX_INVALID		((size_t)~0U)

struct regul_struct {
	const char *name;
	const uint16_t *volt_table;
	uint8_t volt_table_size;
	uint8_t volt_cr;
	uint8_t volt_shift;
	uint8_t en_cr;
	uint8_t alt_en_cr;
	uint8_t msrt_reg;
	uint8_t msrt_mask;
	uint8_t pd_reg;
	uint8_t pd_val;
	uint8_t ocp_reg;
	uint8_t ocp_mask;
};

/* Voltage tables in mV */
static const uint16_t buck1236_volt_table[] = {
	500U, 510U, 520U, 530U, 540U, 550U, 560U, 570U, 580U, 590U,
	600U, 610U, 620U, 630U, 640U, 650U, 660U, 670U, 680U, 690U,
	700U, 710U, 720U, 730U, 740U, 750U, 760U, 770U, 780U, 790U,
	800U, 810U, 820U, 830U, 840U, 850U, 860U, 870U, 880U, 890U,
	900U, 910U, 920U, 930U, 940U, 950U, 960U, 970U, 980U, 990U,
	1000U, 1010U, 1020U, 1030U, 1040U, 1050U, 1060U, 1070U, 1080U, 1090U,
	1100U, 1110U, 1120U, 1130U, 1140U, 1150U, 1160U, 1170U, 1180U, 1190U,
	1200U, 1210U, 1220U, 1230U, 1240U, 1250U, 1260U, 1270U, 1280U, 1290U,
	1300U, 1310U, 1320U, 1330U, 1340U, 1350U, 1360U, 1370U, 1380U, 1390U,
	1400U, 1410U, 1420U, 1430U, 1440U, 1450U, 1460U, 1470U, 1480U, 1490U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U
};

static const uint16_t buck457_volt_table[] = {
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U, 1500U,
	1500U, 1600U, 1700U, 1800U, 1900U, 2000U, 2100U, 2200U, 2300U, 2400U,
	2500U, 2600U, 2700U, 2800U, 2900U, 3000U, 3100U, 3200U, 3300U, 3400U,
	3500U, 3600U, 3700U, 3800U, 3900U, 4000U, 4100U, 4200U
};

static const uint16_t ldo235678_volt_table[] = {
	900U, 1000U, 1100U, 1200U, 1300U, 1400U, 1500U, 1600U, 1700U, 1800U,
	1900U, 2000U, 2100U, 2200U, 2300U, 2400U, 2500U, 2600U, 2700U, 2800U,
	2900U, 3000U, 3100U, 3200U, 3300U, 3400U, 3500U, 3600U, 3700U, 3800U,
	3900U, 4000U
};

static const uint16_t ldo1_volt_table[] = {
	1800U,
};

static const uint16_t ldo4_volt_table[] = {
	3300U,
};

static const uint16_t refddr_volt_table[] = {
	0,
};

#define DEFINE_BUCK(regu_name, ID, pd, table) { \
	.name			= regu_name, \
	.volt_table		= table, \
	.volt_table_size	= ARRAY_SIZE(table), \
	.en_cr			= ID ## _MAIN_CR2, \
	.volt_cr		= ID ## _MAIN_CR1, \
	.alt_en_cr		= ID ## _ALT_CR2, \
	.msrt_reg		= BUCKS_MRST_CR, \
	.msrt_mask		= ID ## _MRST, \
	.pd_reg			= pd, \
	.pd_val			= ID ## _PD_FAST, \
	.ocp_reg		= FS_OCP_CR1, \
	.ocp_mask		= FS_OCP_ ## ID, \
}

#define DEFINE_LDOx(regu_name, ID, table) { \
	.name			= regu_name, \
	.volt_table		= table, \
	.volt_table_size	= ARRAY_SIZE(table), \
	.volt_shift		= LDO_VOLT_SHIFT, \
	.en_cr			= ID ## _MAIN_CR, \
	.volt_cr		= ID ## _MAIN_CR, \
	.alt_en_cr		= ID ## _ALT_CR, \
	.msrt_reg		= LDOS_MRST_CR, \
	.msrt_mask		= ID ## _MRST, \
	.pd_reg			= LDOS_PD_CR1, \
	.pd_val			= ID ## _PD, \
	.ocp_reg		= FS_OCP_CR2, \
	.ocp_mask		= FS_OCP_ ## ID, \
}

#define DEFINE_REFDDR(regu_name, ID, table) { \
	.name			= regu_name, \
	.volt_table		= table, \
	.volt_table_size	= ARRAY_SIZE(table), \
	.en_cr			= ID ## _MAIN_CR, \
	.volt_cr		= ID ## _MAIN_CR, \
	.alt_en_cr		= ID ## _ALT_CR, \
	.msrt_reg		= BUCKS_MRST_CR, \
	.msrt_mask		= ID ## _MRST, \
	.pd_reg			= LDOS_PD_CR2, \
	.pd_val			= ID ## _PD, \
	.ocp_reg		= FS_OCP_CR1, \
	.ocp_mask		= FS_OCP_ ## ID, \
}

/* Table of Regulators in PMIC SoC */
static const struct regul_struct regul_table[STPMIC2_NB_REG] = {
	[STPMIC2_BUCK1] = DEFINE_BUCK("buck1", BUCK1, BUCKS_PD_CR1,
				      buck1236_volt_table),
	[STPMIC2_BUCK2] = DEFINE_BUCK("buck2", BUCK2, BUCKS_PD_CR1,
				      buck1236_volt_table),
	[STPMIC2_BUCK3] = DEFINE_BUCK("buck3", BUCK3, BUCKS_PD_CR1,
				      buck1236_volt_table),
	[STPMIC2_BUCK4] = DEFINE_BUCK("buck4", BUCK4, BUCKS_PD_CR1,
				      buck457_volt_table),
	[STPMIC2_BUCK5] = DEFINE_BUCK("buck5", BUCK5, BUCKS_PD_CR2,
				      buck457_volt_table),
	[STPMIC2_BUCK6] = DEFINE_BUCK("buck6", BUCK6, BUCKS_PD_CR2,
				      buck1236_volt_table),
	[STPMIC2_BUCK7] = DEFINE_BUCK("buck7", BUCK7, BUCKS_PD_CR2,
				      buck457_volt_table),

	[STPMIC2_REFDDR] = DEFINE_REFDDR("refddr", REFDDR, refddr_volt_table),

	[STPMIC2_LDO1] = DEFINE_LDOx("ldo1", LDO1, ldo1_volt_table),
	[STPMIC2_LDO2] = DEFINE_LDOx("ldo2", LDO2, ldo235678_volt_table),
	[STPMIC2_LDO3] = DEFINE_LDOx("ldo3", LDO3, ldo235678_volt_table),
	[STPMIC2_LDO4] = DEFINE_LDOx("ldo4", LDO4, ldo4_volt_table),
	[STPMIC2_LDO5] = DEFINE_LDOx("ldo5", LDO5, ldo235678_volt_table),
	[STPMIC2_LDO6] = DEFINE_LDOx("ldo6", LDO6, ldo235678_volt_table),
	[STPMIC2_LDO7] = DEFINE_LDOx("ldo7", LDO7, ldo235678_volt_table),
	[STPMIC2_LDO8] = DEFINE_LDOx("ldo8", LDO8, ldo235678_volt_table),

};

int stpmic2_register_read(struct pmic_handle_s *pmic,
			  uint8_t register_id, uint8_t *value)
{
	int ret = stm32_i2c_mem_read(pmic->i2c_handle,
				     pmic->i2c_addr,
				     (uint16_t)register_id,
				     I2C_MEMADD_SIZE_8BIT, value,
				     1, I2C_TIMEOUT_MS);
	if (ret != 0) {
		ERROR("Failed to read reg:0x%x\n", register_id);
	}

	return ret;
}

int stpmic2_register_write(struct pmic_handle_s *pmic,
			   uint8_t register_id, uint8_t value)
{
	uint8_t val = value;
	int ret = stm32_i2c_mem_write(pmic->i2c_handle,
				      pmic->i2c_addr,
				      (uint16_t)register_id,
				      I2C_MEMADD_SIZE_8BIT, &val,
				      1, I2C_TIMEOUT_MS);
	if (ret != 0) {
		ERROR("Failed to write reg:0x%x\n", register_id);
	}

	return ret;
}

int stpmic2_register_update(struct pmic_handle_s *pmic,
			    uint8_t register_id, uint8_t value, uint8_t mask)
{
	int status;
	uint8_t val = 0U;

	status = stpmic2_register_read(pmic, register_id, &val);
	if (status != 0) {
		return status;
	}

	val = (val & ((uint8_t)~mask)) | (value & mask);

	VERBOSE("REG:0x%x v=0x%x mask=0x%x -> 0x%x\n",
		register_id, value, mask, val);

	return stpmic2_register_write(pmic, register_id, val);
}

int stpmic2_regulator_set_state(struct pmic_handle_s *pmic,
				uint8_t id, bool enable)
{
	const struct regul_struct *regul = &regul_table[id];

	if (enable) {
		return stpmic2_register_update(pmic, regul->en_cr, 1U, 1U);
	} else {
		return stpmic2_register_update(pmic, regul->en_cr, 0, 1U);
	}
}

int stpmic2_regulator_get_state(struct pmic_handle_s *pmic,
				uint8_t id, bool *enabled)
{
	const struct regul_struct *regul = &regul_table[id];
	uint8_t val;

	if (stpmic2_register_read(pmic, regul->en_cr, &val) != 0) {
		return RET_ERROR_GENERIC;
	}

	*enabled = (val & 1U) == 1U;

	return RET_SUCCESS;
}

int stpmic2_regulator_levels_mv(struct pmic_handle_s *pmic,
				uint8_t id, const uint16_t **levels,
				size_t *levels_count)
{
	const struct regul_struct *regul = &regul_table[id];

	if (regul == NULL) {
		return RET_ERROR_BAD_PARAMETERS;
	}

	if (levels_count != NULL) {
		*levels_count = regul->volt_table_size;
	}
	if (levels != NULL) {
		*levels = regul->volt_table;
	}

	return RET_SUCCESS;
}

int stpmic2_regulator_get_voltage(struct pmic_handle_s *pmic,
				  uint8_t id, uint16_t *val)
{
	const struct regul_struct *regul = &regul_table[id];
	uint8_t value = 0U;
	uint8_t mask;

	if (regul->volt_table_size == 0U) {
		return RET_ERROR_GENERIC;
	}

	mask = regul->volt_table_size - 1U;
	if (mask != 0U) {
		if (stpmic2_register_read(pmic, regul->volt_cr, &value) != 0) {
			return RET_ERROR_GENERIC;
		}

		value = (value >> regul->volt_shift) & mask;
	}

	if (value > regul->volt_table_size) {
		return RET_ERROR_GENERIC;
	}

	*val = regul->volt_table[value];

	return RET_SUCCESS;
}

static size_t voltage_to_index(const struct regul_struct *regul,
			       uint16_t millivolts)
{
	unsigned int i;

	assert(regul->volt_table);
	for (i = 0U; i < regul->volt_table_size; i++) {
		if (regul->volt_table[i] == millivolts) {
			return i;
		}
	}

	return VOLTAGE_INDEX_INVALID;
}

int stpmic2_regulator_set_voltage(struct pmic_handle_s *pmic,
				  uint8_t id, uint16_t millivolts)
{
	const struct regul_struct *regul = &regul_table[id];
	size_t index;
	uint8_t mask;

	if (!regul->volt_table_size) {
		return RET_SUCCESS;
	}

	mask = regul->volt_table_size - 1U;

	index = voltage_to_index(regul, millivolts);
	if (index == VOLTAGE_INDEX_INVALID) {
		return RET_ERROR_GENERIC;
	}

	return stpmic2_register_update(pmic, regul->volt_cr,
				       index << regul->volt_shift,
				       mask << regul->volt_shift);
}

/* update both normal and alternate register */
static int stpmic2_update_en_crs(struct pmic_handle_s *pmic, uint8_t id,
				 uint8_t value, uint8_t mask)
{
	const struct regul_struct *regul = &regul_table[id];

	if (stpmic2_register_update(pmic, regul->en_cr, value, mask) != 0) {
		return RET_ERROR_GENERIC;
	}

	if (stpmic2_register_update(pmic, regul->alt_en_cr, value, mask) != 0) {
		return RET_ERROR_GENERIC;
	}

	return RET_SUCCESS;
}

int stpmic2_regulator_get_prop(struct pmic_handle_s *pmic, uint8_t id,
			       enum stpmic2_prop_id prop)
{
	const struct regul_struct *regul = &regul_table[id];
	uint8_t val;

	VERBOSE("%s: get prop 0x%x\n", regul->name, prop);

	switch (prop) {
	case STPMIC2_BYPASS:
		if ((id <= STPMIC2_BUCK7) || (id == STPMIC2_LDO1) ||
		    (id == STPMIC2_LDO4) || (id == STPMIC2_REFDDR)) {
			return 0;
		}

		if (stpmic2_register_read(pmic, regul->en_cr, &val) != 0) {
			return -EIO;
		}

		if ((val & LDO_BYPASS) != 0) {
			return 1;
		}

		break;
	default:
		ERROR("Invalid prop %u\n", prop);
		panic();
	}

	return 0;
}

int stpmic2_regulator_set_prop(struct pmic_handle_s *pmic, uint8_t id,
			       enum stpmic2_prop_id prop, uint32_t arg)
{
	const struct regul_struct *regul = &regul_table[id];

	VERBOSE("%s: set prop 0x%x arg=%u\n", regul->name, prop, arg);

	switch (prop) {
	case STPMIC2_PULL_DOWN:
		return stpmic2_register_update(pmic, regul->pd_reg,
					       regul->pd_val,
					       regul->pd_val);
	case STPMIC2_MASK_RESET:
		if (!regul->msrt_mask) {
			return RET_ERROR_NOT_SUPPORTED;
		}
		/* enable mask reset */
		return stpmic2_register_update(pmic, regul->msrt_reg,
					       regul->msrt_mask,
					       regul->msrt_mask);
	case STPMIC2_BYPASS:
		if ((id <= STPMIC2_BUCK7) || (id == STPMIC2_LDO1) ||
		    (id == STPMIC2_LDO4) || (id == STPMIC2_REFDDR)) {
			return RET_ERROR_NOT_SUPPORTED;
		}

		/* clear sink source mode */
		if ((id == STPMIC2_LDO3) && (arg != 0U)) {
			if (stpmic2_update_en_crs(pmic, id, 0, LDO3_SNK_SRC) != 0) {
				return RET_ERROR_GENERIC;
			}
		}

		/* enable bypass mode */
		return stpmic2_update_en_crs(pmic, id,
					     (arg != 0U) ? LDO_BYPASS : 0,
					     LDO_BYPASS);
	case STPMIC2_SINK_SOURCE:
		if (id != STPMIC2_LDO3) {
			return RET_ERROR_NOT_SUPPORTED;
		}

		/* clear bypass mode */
		if (stpmic2_update_en_crs(pmic, id, 0, LDO_BYPASS) != 0) {
			return RET_ERROR_GENERIC;
		}

		return stpmic2_update_en_crs(pmic, id, LDO3_SNK_SRC,
					     LDO3_SNK_SRC);
	case STPMIC2_OCP:
		return stpmic2_register_update(pmic, regul->ocp_reg,
					       regul->ocp_mask,
					       regul->ocp_mask);
	default:
		ERROR("Invalid prop %u\n", prop);
		panic();
	}

	return -EPERM;
}

#if EVENT_LOG_LEVEL == LOG_LEVEL_VERBOSE
void stpmic2_dump_regulators(struct pmic_handle_s *pmic)
{
	size_t i;
	char const *name;

	for (i = 0U; i < ARRAY_SIZE(regul_table); i++) {
		uint16_t val;
		bool state;

		if (!regul_table[i].volt_cr) {
			continue;
		}

		stpmic2_regulator_get_voltage(pmic, i, &val);
		stpmic2_regulator_get_state(pmic, i, &state);

		name = regul_table[i].name;

		VERBOSE("PMIC regul %s: %s, %dmV\n",
			name, state ? "EN" : "DIS", val);
	}
}
#endif

int stpmic2_get_version(struct pmic_handle_s *pmic, uint8_t *val)
{
	return stpmic2_register_read(pmic, VERSION_SR, val);
}

int stpmic2_get_product_id(struct pmic_handle_s *pmic, uint8_t *val)
{
	return stpmic2_register_read(pmic, PRODUCT_ID, val);
}
