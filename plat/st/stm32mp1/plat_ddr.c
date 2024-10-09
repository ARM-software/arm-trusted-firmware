/*
 * Copyright (C) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <drivers/st/regulator.h>
#include <drivers/st/stm32mp_ddr.h>
#include <drivers/st/stm32mp_pmic.h>

/* configure the STPMIC1 regulators on STMicroelectronics boards */
static int pmic_ddr_power_init(enum ddr_type ddr_type)
{
	int status;
	uint16_t buck3_min_mv __maybe_unused;
	struct rdev *buck2, *buck3 __maybe_unused, *vref;
	struct rdev *ldo3 __maybe_unused;

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
#if STM32MP15
		/*
		 * Set LDO3 to 1.8V according  BUCK3 voltage
		 * => bypass mode if BUCK3 = 1.8V
		 * => normal mode if BUCK3 != 1.8V
		 */
		buck3 = regulator_get_by_name("buck3");
		if (buck3 == NULL) {
			return -ENOENT;
		}

		regulator_get_range(buck3, &buck3_min_mv, NULL);

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

int stm32mp_board_ddr_power_init(enum ddr_type ddr_type)
{
	if (dt_pmic_status() > 0) {
		return pmic_ddr_power_init(ddr_type);
	}

	return 0;
}
