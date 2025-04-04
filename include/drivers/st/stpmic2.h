/*
 * Copyright (C) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STPMIC2_H
#define STPMIC2_H

#include <drivers/st/stm32_i2c.h>
#include <lib/utils_def.h>

enum {
	STPMIC2_BUCK1 = 0,
	STPMIC2_BUCK2,
	STPMIC2_BUCK3,
	STPMIC2_BUCK4,
	STPMIC2_BUCK5,
	STPMIC2_BUCK6,
	STPMIC2_BUCK7,
	STPMIC2_REFDDR,
	STPMIC2_LDO1,
	STPMIC2_LDO2,
	STPMIC2_LDO3,
	STPMIC2_LDO4,
	STPMIC2_LDO5,
	STPMIC2_LDO6,
	STPMIC2_LDO7,
	STPMIC2_LDO8,
	STPMIC2_NB_REG
};

/* Status Registers */
#define PRODUCT_ID		0x00
#define VERSION_SR		0x01
#define TURN_ON_SR		0x02
#define TURN_OFF_SR		0x03
#define RESTART_SR		0x04
#define OCP_SR1			0x05
#define OCP_SR2			0x06
#define EN_SR1			0x07
#define EN_SR2			0x08
#define FS_CNT_SR1		0x09
#define FS_CNT_SR2		0x0A
#define FS_CNT_SR3		0x0B
#define MODE_SR			0x0C
/* Control Registers */
#define MAIN_CR			0x10
#define VINLOW_CR		0x11
#define PKEY_LKP_CR		0x12
#define WDG_CR			0x13
#define WDG_TMR_CR		0x14
#define WDG_TMR_SR		0x15
#define FS_OCP_CR1		0x16
#define FS_OCP_CR2		0x17
#define PADS_PULL_CR		0x18
#define BUCKS_PD_CR1		0x19
#define BUCKS_PD_CR2		0x1A
#define LDOS_PD_CR1		0x1B
#define LDOS_PD_CR2		0x1C
#define BUCKS_MRST_CR		0x1D
#define LDOS_MRST_CR		0x1E
/* Buck CR */
#define BUCK1_MAIN_CR1		0x20
#define BUCK1_MAIN_CR2		0x21
#define BUCK1_ALT_CR1		0x22
#define BUCK1_ALT_CR2		0x23
#define BUCK1_PWRCTRL_CR	0x24
#define BUCK2_MAIN_CR1		0x25
#define BUCK2_MAIN_CR2		0x26
#define BUCK2_ALT_CR1		0x27
#define BUCK2_ALT_CR2		0x28
#define BUCK2_PWRCTRL_CR	0x29
#define BUCK3_MAIN_CR1		0x2A
#define BUCK3_MAIN_CR2		0x2B
#define BUCK3_ALT_CR1		0x2C
#define BUCK3_ALT_CR2		0x2D
#define BUCK3_PWRCTRL_CR	0x2E
#define BUCK4_MAIN_CR1		0x2F
#define BUCK4_MAIN_CR2		0x30
#define BUCK4_ALT_CR1		0x31
#define BUCK4_ALT_CR2		0x32
#define BUCK4_PWRCTRL_CR	0x33
#define BUCK5_MAIN_CR1		0x34
#define BUCK5_MAIN_CR2		0x35
#define BUCK5_ALT_CR1		0x36
#define BUCK5_ALT_CR2		0x37
#define BUCK5_PWRCTRL_CR	0x38
#define BUCK6_MAIN_CR1		0x39
#define BUCK6_MAIN_CR2		0x3A
#define BUCK6_ALT_CR1		0x3B
#define BUCK6_ALT_CR2		0x3C
#define BUCK6_PWRCTRL_CR	0x3D
#define BUCK7_MAIN_CR1		0x3E
#define BUCK7_MAIN_CR2		0x3F
#define BUCK7_ALT_CR1		0x40
#define BUCK7_ALT_CR2		0x41
#define BUCK7_PWRCTRL_CR	0x42
/* LDO CR */
#define LDO1_MAIN_CR		0x4C
#define LDO1_ALT_CR		0x4D
#define LDO1_PWRCTRL_CR		0x4E
#define LDO2_MAIN_CR		0x4F
#define LDO2_ALT_CR		0x50
#define LDO2_PWRCTRL_CR		0x51
#define LDO3_MAIN_CR		0x52
#define LDO3_ALT_CR		0x53
#define LDO3_PWRCTRL_CR		0x54
#define LDO4_MAIN_CR		0x55
#define LDO4_ALT_CR		0x56
#define LDO4_PWRCTRL_CR		0x57
#define LDO5_MAIN_CR		0x58
#define LDO5_ALT_CR		0x59
#define LDO5_PWRCTRL_CR		0x5A
#define LDO6_MAIN_CR		0x5B
#define LDO6_ALT_CR		0x5C
#define LDO6_PWRCTRL_CR		0x5D
#define LDO7_MAIN_CR		0x5E
#define LDO7_ALT_CR		0x5F
#define LDO7_PWRCTRL_CR		0x60
#define LDO8_MAIN_CR		0x61
#define LDO8_ALT_CR		0x62
#define LDO8_PWRCTRL_CR		0x63
#define REFDDR_MAIN_CR		0x64
#define REFDDR_ALT_CR		0x65
#define REFDDR_PWRCTRL_CR	0x66
/* INTERRUPT CR */
#define INT_PENDING_R1		0x70
#define INT_PENDING_R2		0x71
#define INT_PENDING_R3		0x72
#define INT_PENDING_R4		0x73
#define INT_CLEAR_R1		0x74
#define INT_CLEAR_R2		0x75
#define INT_CLEAR_R3		0x76
#define INT_CLEAR_R4		0x77
#define INT_MASK_R1		0x78
#define INT_MASK_R2		0x79
#define INT_MASK_R3		0x7A
#define INT_MASK_R4		0x7B
#define INT_SRC_R1		0x7C
#define INT_SRC_R2		0x7D
#define INT_SRC_R3		0x7E
#define INT_SRC_R4		0x7F
#define INT_DBG_LATCH_R1	0x80
#define INT_DBG_LATCH_R2	0x81
#define INT_DBG_LATCH_R3	0x82
#define INT_DBG_LATCH_R4	0x83

/* NVM user control registers */
#define NVM_SR			0x8E
#define NVM_CR			0x8F

/* NVM user shadow registers */
#define NVM_MAIN_CTRL_SHR1	0x90
#define NVM_MAIN_CTRL_SHR2	0x91
#define NVM_RANK_SHR1		0x92
#define NVM_RANK_SHR2		0x93
#define NVM_RANK_SHR3		0x94
#define NVM_RANK_SHR4		0x95
#define NVM_RANK_SHR5		0x96
#define NVM_RANK_SHR6		0x97
#define NVM_RANK_SHR7		0x98
#define NVM_RANK_SHR8		0x99
#define NVM_BUCK_MODE_SHR1	0x9A
#define NVM_BUCK_MODE_SHR2	0x9B
#define NVM_BUCK1_VOUT_SHR	0x9C
#define NVM_BUCK2_VOUT_SHR	0x9D
#define NVM_BUCK3_VOUT_SHR	0x9E
#define NVM_BUCK4_VOUT_SHR	0x9F
#define NVM_BUCK5_VOUT_SHR	0xA0
#define NVM_BUCK6_VOUT_SHR	0xA1
#define NVM_BUCK7_VOUT_SHR	0xA2
#define NVM_LDO2_SHR		0xA3
#define NVM_LDO3_SHR		0xA4
#define NVM_LDO5_SHR		0xA5
#define NVM_LDO6_SHR		0xA6
#define NVM_LDO7_SHR		0xA7
#define NVM_LDO8_SHR		0xA8
#define NVM_PD_SHR1		0xA9
#define NVM_PD_SHR2		0xAA
#define NVM_PD_SHR3		0xAB
#define NVM_BUCKS_IOUT_SHR1	0xAC
#define NVM_BUCKS_IOUT_SHR2	0xAD
#define NVM_LDOS_IOUT_SHR	0xAE
#define NVM_FS_OCP_SHR1	0xAF
#define NVM_FS_OCP_SHR2	0xB0
#define NVM_FS_SHR1		0xB1
#define NVM_FS_SHR2		0xB2
#define NVM_FS_SHR3		0xB3
#define NVM_I2C_ADDR_SHR	0xB5
#define NVM_USER_SHR1		0xB6
#define NVM_USER_SHR2		0xB7

/* BUCKS_MRST_CR bits definition */
#define BUCK1_MRST		BIT(0)
#define BUCK2_MRST		BIT(1)
#define BUCK3_MRST		BIT(2)
#define BUCK4_MRST		BIT(3)
#define BUCK5_MRST		BIT(4)
#define BUCK6_MRST		BIT(5)
#define BUCK7_MRST		BIT(6)
#define REFDDR_MRST		BIT(7)

/* LDOS_MRST_CR bits definition */
#define LDO1_MRST		BIT(0)
#define LDO2_MRST		BIT(1)
#define LDO3_MRST		BIT(2)
#define LDO4_MRST		BIT(3)
#define LDO5_MRST		BIT(4)
#define LDO6_MRST		BIT(5)
#define LDO7_MRST		BIT(6)
#define LDO8_MRST		BIT(7)

/* LDOx_MAIN_CR */
#define LDO_VOLT_SHIFT		1
#define LDO_BYPASS		BIT(6)
#define LDO1_INPUT_SRC		BIT(7)
#define LDO3_SNK_SRC		BIT(7)
#define LDO4_INPUT_SRC_SHIFT	6
#define LDO4_INPUT_SRC_MASK	GENMASK_32(7, 6)

/* PWRCTRL register bit definition */
#define PWRCTRL_EN		BIT(0)
#define PWRCTRL_RS		BIT(1)
#define PWRCTRL_SEL_SHIFT	2
#define PWRCTRL_SEL_MASK	GENMASK_32(3, 2)

/* BUCKx_MAIN_CR2 */
#define PREG_MODE_SHIFT		1
#define PREG_MODE_MASK		GENMASK_32(2, 1)

/* BUCKS_PD_CR1 */
#define BUCK1_PD_MASK		GENMASK_32(1, 0)
#define BUCK2_PD_MASK		GENMASK_32(3, 2)
#define BUCK3_PD_MASK		GENMASK_32(5, 4)
#define BUCK4_PD_MASK		GENMASK_32(7, 6)

#define BUCK1_PD_FAST		BIT(1)
#define BUCK2_PD_FAST		BIT(3)
#define BUCK3_PD_FAST		BIT(5)
#define BUCK4_PD_FAST		BIT(7)

/* BUCKS_PD_CR2 */
#define BUCK5_PD_MASK		GENMASK_32(1, 0)
#define BUCK6_PD_MASK		GENMASK_32(3, 2)
#define BUCK7_PD_MASK		GENMASK_32(5, 4)

#define BUCK5_PD_FAST		BIT(1)
#define BUCK6_PD_FAST		BIT(3)
#define BUCK7_PD_FAST		BIT(5)

/* LDOS_PD_CR1 */
#define LDO1_PD			BIT(0)
#define LDO2_PD			BIT(1)
#define LDO3_PD			BIT(2)
#define LDO4_PD			BIT(3)
#define LDO5_PD			BIT(4)
#define LDO6_PD			BIT(5)
#define LDO7_PD			BIT(6)
#define LDO8_PD			BIT(7)

/* LDOS_PD_CR2 */
#define REFDDR_PD		BIT(0)

/* FS_OCP_CR1 */
#define FS_OCP_BUCK1		BIT(0)
#define FS_OCP_BUCK2		BIT(1)
#define FS_OCP_BUCK3		BIT(2)
#define FS_OCP_BUCK4		BIT(3)
#define FS_OCP_BUCK5		BIT(4)
#define FS_OCP_BUCK6		BIT(5)
#define FS_OCP_BUCK7		BIT(6)
#define FS_OCP_REFDDR		BIT(7)

/* FS_OCP_CR2 */
#define FS_OCP_LDO1		BIT(0)
#define FS_OCP_LDO2		BIT(1)
#define FS_OCP_LDO3		BIT(2)
#define FS_OCP_LDO4		BIT(3)
#define FS_OCP_LDO5		BIT(4)
#define FS_OCP_LDO6		BIT(5)
#define FS_OCP_LDO7		BIT(6)
#define FS_OCP_LDO8		BIT(7)

/* NVM_CR */
#define NVM_CMD_MASK		GENMASK_32(1, 0)

#define NVM_CMD_PROGRAM		1
#define NVM_CMD_READ		2

/* NVM_SR */
#define NVM_BUSY		BIT(0)
#define NVM_WRITE_FAIL		BIT(1)

/* IRQ definitions */
#define IT_PONKEY_F	0
#define IT_PONKEY_R	1
#define IT_BUCK1_OCP	16
#define IT_BUCK2_OCP	17
#define IT_BUCK3_OCP	18
#define IT_BUCK4_OCP	19
#define IT_BUCK5_OCP	20
#define IT_BUCK6_OCP	21
#define IT_BUCK7_OCP	22
#define IT_REFDDR_OCP	23
#define IT_LDO1_OCP	24
#define IT_LDO2_OCP	25
#define IT_LDO3_OCP	26
#define IT_LDO4_OCP	27
#define IT_LDO5_OCP	28
#define IT_LDO6_OCP	29
#define IT_LDO7_OCP	30
#define IT_LDO8_OCP	31

enum stpmic2_prop_id {
	STPMIC2_MASK_RESET = 0,
	STPMIC2_PULL_DOWN,
	STPMIC2_BYPASS,		/* arg: 1=set 0=reset */
	STPMIC2_SINK_SOURCE,
	STPMIC2_OCP,
};

struct pmic_handle_s {
	struct i2c_handle_s *i2c_handle;
	uint32_t i2c_addr;
	unsigned int pmic_status;
};

int stpmic2_register_read(struct pmic_handle_s *pmic,
			  uint8_t register_id, uint8_t *value);
int stpmic2_register_write(struct pmic_handle_s *pmic,
			   uint8_t register_id, uint8_t value);
int stpmic2_register_update(struct pmic_handle_s *pmic,
			    uint8_t register_id, uint8_t value, uint8_t mask);

int stpmic2_regulator_set_state(struct pmic_handle_s *pmic,
				uint8_t id, bool enable);
int stpmic2_regulator_get_state(struct pmic_handle_s *pmic,
				uint8_t id, bool *enabled);

int stpmic2_regulator_levels_mv(struct pmic_handle_s *pmic,
				uint8_t id, const uint16_t **levels,
				size_t *levels_count);
int stpmic2_regulator_get_voltage(struct pmic_handle_s *pmic,
				  uint8_t id, uint16_t *val);
int stpmic2_regulator_set_voltage(struct pmic_handle_s *pmic,
				  uint8_t id, uint16_t millivolts);

void stpmic2_dump_regulators(struct pmic_handle_s *pmic);

int stpmic2_get_version(struct pmic_handle_s *pmic, uint8_t *val);
int stpmic2_get_product_id(struct pmic_handle_s *pmic, uint8_t *val);

int stpmic2_regulator_get_prop(struct pmic_handle_s *pmic, uint8_t id,
			       enum stpmic2_prop_id prop);

int stpmic2_regulator_set_prop(struct pmic_handle_s *pmic, uint8_t id,
			       enum stpmic2_prop_id prop, uint32_t arg);

#endif /*STPMIC2_H*/
