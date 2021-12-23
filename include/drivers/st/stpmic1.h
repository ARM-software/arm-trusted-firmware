/*
 * Copyright (c) 2016-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STPMIC1_H
#define STPMIC1_H

#include <drivers/st/stm32_i2c.h>
#include <lib/utils_def.h>

#define TURN_ON_REG			0x1U
#define TURN_OFF_REG			0x2U
#define ICC_LDO_TURN_OFF_REG		0x3U
#define ICC_BUCK_TURN_OFF_REG		0x4U
#define RESET_STATUS_REG		0x5U
#define VERSION_STATUS_REG		0x6U
#define MAIN_CONTROL_REG		0x10U
#define PADS_PULL_REG			0x11U
#define BUCK_PULL_DOWN_REG		0x12U
#define LDO14_PULL_DOWN_REG		0x13U
#define LDO56_PULL_DOWN_REG		0x14U
#define VIN_CONTROL_REG			0x15U
#define PONKEY_TIMER_REG		0x16U
#define MASK_RANK_BUCK_REG		0x17U
#define MASK_RESET_BUCK_REG		0x18U
#define MASK_RANK_LDO_REG		0x19U
#define MASK_RESET_LDO_REG		0x1AU
#define WATCHDOG_CONTROL_REG		0x1BU
#define WATCHDOG_TIMER_REG		0x1CU
#define BUCK_ICC_TURNOFF_REG		0x1DU
#define LDO_ICC_TURNOFF_REG		0x1EU
#define BUCK_APM_CONTROL_REG		0x1FU
#define BUCK1_CONTROL_REG		0x20U
#define BUCK2_CONTROL_REG		0x21U
#define BUCK3_CONTROL_REG		0x22U
#define BUCK4_CONTROL_REG		0x23U
#define VREF_DDR_CONTROL_REG		0x24U
#define LDO1_CONTROL_REG		0x25U
#define LDO2_CONTROL_REG		0x26U
#define LDO3_CONTROL_REG		0x27U
#define LDO4_CONTROL_REG		0x28U
#define LDO5_CONTROL_REG		0x29U
#define LDO6_CONTROL_REG		0x2AU
#define BUCK1_PWRCTRL_REG		0x30U
#define BUCK2_PWRCTRL_REG		0x31U
#define BUCK3_PWRCTRL_REG		0x32U
#define BUCK4_PWRCTRL_REG		0x33U
#define VREF_DDR_PWRCTRL_REG		0x34U
#define LDO1_PWRCTRL_REG		0x35U
#define LDO2_PWRCTRL_REG		0x36U
#define LDO3_PWRCTRL_REG		0x37U
#define LDO4_PWRCTRL_REG		0x38U
#define LDO5_PWRCTRL_REG		0x39U
#define LDO6_PWRCTRL_REG		0x3AU
#define FREQUENCY_SPREADING_REG		0x3BU
#define USB_CONTROL_REG			0x40U
#define ITLATCH1_REG			0x50U
#define ITLATCH2_REG			0x51U
#define ITLATCH3_REG			0x52U
#define ITLATCH4_REG			0x53U
#define ITSETLATCH1_REG			0x60U
#define ITSETLATCH2_REG			0x61U
#define ITSETLATCH3_REG			0x62U
#define ITSETLATCH4_REG			0x63U
#define ITCLEARLATCH1_REG		0x70U
#define ITCLEARLATCH2_REG		0x71U
#define ITCLEARLATCH3_REG		0x72U
#define ITCLEARLATCH4_REG		0x73U
#define ITMASK1_REG			0x80U
#define ITMASK2_REG			0x81U
#define ITMASK3_REG			0x82U
#define ITMASK4_REG			0x83U
#define ITSETMASK1_REG			0x90U
#define ITSETMASK2_REG			0x91U
#define ITSETMASK3_REG			0x92U
#define ITSETMASK4_REG			0x93U
#define ITCLEARMASK1_REG		0xA0U
#define ITCLEARMASK2_REG		0xA1U
#define ITCLEARMASK3_REG		0xA2U
#define ITCLEARMASK4_REG		0xA3U
#define ITSOURCE1_REG			0xB0U
#define ITSOURCE2_REG			0xB1U
#define ITSOURCE3_REG			0xB2U
#define ITSOURCE4_REG			0xB3U

/* Registers masks */
#define LDO_VOLTAGE_MASK		GENMASK(6, 2)
#define BUCK_VOLTAGE_MASK		GENMASK(7, 2)
#define LDO_BUCK_VOLTAGE_SHIFT		2
#define LDO_BUCK_ENABLE_MASK		BIT(0)
#define LDO_BUCK_HPLP_ENABLE_MASK	BIT(1)
#define LDO_BUCK_HPLP_SHIFT		1
#define LDO_BUCK_RANK_MASK		BIT(0)
#define LDO_BUCK_RESET_MASK		BIT(0)
#define LDO_BUCK_PULL_DOWN_MASK		GENMASK(1, 0)

/* Pull down register */
#define BUCK1_PULL_DOWN_SHIFT		0
#define BUCK2_PULL_DOWN_SHIFT		2
#define BUCK3_PULL_DOWN_SHIFT		4
#define BUCK4_PULL_DOWN_SHIFT		6
#define VREF_DDR_PULL_DOWN_SHIFT	4

/* ICC register */
#define BUCK1_ICC_SHIFT			0
#define BUCK2_ICC_SHIFT			1
#define BUCK3_ICC_SHIFT			2
#define BUCK4_ICC_SHIFT			3
#define PWR_SW1_ICC_SHIFT		4
#define PWR_SW2_ICC_SHIFT		5
#define BOOST_ICC_SHIFT			6

#define LDO1_ICC_SHIFT			0
#define LDO2_ICC_SHIFT			1
#define LDO3_ICC_SHIFT			2
#define LDO4_ICC_SHIFT			3
#define LDO5_ICC_SHIFT			4
#define LDO6_ICC_SHIFT			5

/* Buck Mask reset register */
#define BUCK1_MASK_RESET		0
#define BUCK2_MASK_RESET		1
#define BUCK3_MASK_RESET		2
#define BUCK4_MASK_RESET		3

/* LDO Mask reset register */
#define LDO1_MASK_RESET			0
#define LDO2_MASK_RESET			1
#define LDO3_MASK_RESET			2
#define LDO4_MASK_RESET			3
#define LDO5_MASK_RESET			4
#define LDO6_MASK_RESET			5
#define VREF_DDR_MASK_RESET		6

/* LDO3 Special modes */
#define LDO3_BYPASS                     BIT(7)
#define LDO3_DDR_SEL                    31U

/* Main PMIC Control Register (MAIN_CONTROL_REG) */
#define ICC_EVENT_ENABLED		BIT(4)
#define PWRCTRL_POLARITY_HIGH		BIT(3)
#define PWRCTRL_PIN_VALID		BIT(2)
#define RESTART_REQUEST_ENABLED		BIT(1)
#define SOFTWARE_SWITCH_OFF_ENABLED	BIT(0)

/* Main PMIC PADS Control Register (PADS_PULL_REG) */
#define WAKEUP_DETECTOR_DISABLED	BIT(4)
#define PWRCTRL_PD_ACTIVE		BIT(3)
#define PWRCTRL_PU_ACTIVE		BIT(2)
#define WAKEUP_PD_ACTIVE		BIT(1)
#define PONKEY_PU_ACTIVE		BIT(0)

/* Main PMIC VINLOW Control Register (VIN_CONTROL_REGC DMSC) */
#define SWIN_DETECTOR_ENABLED		BIT(7)
#define SWOUT_DETECTOR_ENABLED          BIT(6)
#define VINLOW_HYST_MASK		GENMASK(1, 0)
#define VINLOW_HYST_SHIFT		4
#define VINLOW_THRESHOLD_MASK		GENMASK(2, 0)
#define VINLOW_THRESHOLD_SHIFT		1
#define VINLOW_ENABLED			BIT(0)
#define VINLOW_CTRL_REG_MASK		GENMASK(7, 0)

/* USB Control Register */
#define BOOST_OVP_DISABLED		BIT(7)
#define VBUS_OTG_DETECTION_DISABLED	BIT(6)
#define SW_OUT_DISCHARGE		BIT(5)
#define VBUS_OTG_DISCHARGE		BIT(4)
#define OCP_LIMIT_HIGH			BIT(3)
#define SWIN_SWOUT_ENABLED		BIT(2)
#define USBSW_OTG_SWITCH_ENABLED	BIT(1)
#define BOOST_ENABLED			BIT(0)

int stpmic1_powerctrl_on(void);
int stpmic1_switch_off(void);
int stpmic1_register_read(uint8_t register_id, uint8_t *value);
int stpmic1_register_write(uint8_t register_id, uint8_t value);
int stpmic1_register_update(uint8_t register_id, uint8_t value, uint8_t mask);
int stpmic1_regulator_enable(const char *name);
int stpmic1_regulator_disable(const char *name);
bool stpmic1_is_regulator_enabled(const char *name);
int stpmic1_regulator_voltage_set(const char *name, uint16_t millivolts);
int stpmic1_regulator_levels_mv(const char *name, const uint16_t **levels,
				size_t *levels_count);
int stpmic1_regulator_voltage_get(const char *name);
int stpmic1_regulator_pull_down_set(const char *name);
int stpmic1_regulator_mask_reset_set(const char *name);
int stpmic1_regulator_icc_set(const char *name);
int stpmic1_regulator_sink_mode_set(const char *name);
int stpmic1_regulator_bypass_mode_set(const char *name);
int stpmic1_active_discharge_mode_set(const char *name);
void stpmic1_bind_i2c(struct i2c_handle_s *i2c_handle, uint16_t i2c_addr);

int stpmic1_get_version(unsigned long *version);
void stpmic1_dump_regulators(void);

#endif /* STPMIC1_H */
