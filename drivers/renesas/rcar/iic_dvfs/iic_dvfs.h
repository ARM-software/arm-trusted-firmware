/*
 * Copyright (c) 2015-2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IIC_DVFS_H
#define IIC_DVFS_H

/* PMIC slave */
#define PMIC			(0x30U)
#define BKUP_MODE_CNT		(0x20U)
#define DVFS_SET_VID		(0x54U)
#define REG_KEEP10		(0x79U)

/* EEPROM slave */
#define EEPROM			(0x50U)
#define BOARD_ID		(0x70U)

int32_t rcar_iic_dvfs_receive(uint8_t slave, uint8_t reg, uint8_t *data);
int32_t rcar_iic_dvfs_send(uint8_t slave, uint8_t regr, uint8_t data);

#endif /* IIC_DVFS_H */
