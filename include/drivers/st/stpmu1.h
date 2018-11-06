/*
 * Copyright (c) 2016-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STPMU1_H
#define STPMU1_H

#include <stm32_i2c.h>
#include <utils_def.h>

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
#define LDO_VOLTAGE_MASK		0x7CU
#define BUCK_VOLTAGE_MASK		0xFCU
#define LDO_BUCK_VOLTAGE_SHIFT		2
#define LDO_ENABLE_MASK			0x01U
#define BUCK_ENABLE_MASK		0x01U
#define BUCK_HPLP_ENABLE_MASK		0x02U
#define LDO_HPLP_ENABLE_MASK		0x02U
#define LDO_BUCK_HPLP_SHIFT		1
#define LDO_BUCK_RANK_MASK		0x01U
#define LDO_BUCK_RESET_MASK		0x01U
#define LDO_BUCK_PULL_DOWN_MASK		0x03U

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
#define VINLOW_HYST_MASK		0x3
#define VINLOW_HYST_SHIFT		4
#define VINLOW_THRESHOLD_MASK		0x7
#define VINLOW_THRESHOLD_SHIFT		1
#define VINLOW_ENABLED			0x01
#define VINLOW_CTRL_REG_MASK		0xFF

/* USB Control Register */
#define BOOST_OVP_DISABLED		BIT(7)
#define VBUS_OTG_DETECTION_DISABLED	BIT(6)
#define OCP_LIMIT_HIGH			BIT(3)
#define SWIN_SWOUT_ENABLED		BIT(2)
#define USBSW_OTG_SWITCH_ENABLED	BIT(1)

int stpmu1_switch_off(void);
int stpmu1_register_read(uint8_t register_id, uint8_t *value);
int stpmu1_register_write(uint8_t register_id, uint8_t value);
int stpmu1_register_update(uint8_t register_id, uint8_t value, uint8_t mask);
int stpmu1_regulator_enable(const char *name);
int stpmu1_regulator_disable(const char *name);
uint8_t stpmu1_is_regulator_enabled(const char *name);
int stpmu1_regulator_voltage_set(const char *name, uint16_t millivolts);
void stpmu1_bind_i2c(struct i2c_handle_s *i2c_handle, uint16_t i2c_addr);

#endif /* STPMU1_H */
