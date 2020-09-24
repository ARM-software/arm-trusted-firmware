/*
 * Copyright (c) 2016 - 2021, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MDIO_H
#define MDIO_H

#define CMIC_MIIM_PARAM		(PLAT_CMIC_MIIM_BASE + 0x23cU)
#define MDIO_PARAM_MIIM_CYCLE	29U
#define MDIO_PARAM_INTERNAL_SEL	25U
#define MDIO_PARAM_BUSID	22U
#define MDIO_PARAM_BUSID_MASK	0x7U
#define MDIO_PARAM_C45_SEL	21U
#define MDIO_PARAM_PHYID	16U
#define MDIO_PARAM_PHYID_MASK	0x1FU
#define MDIO_PARAM_DATA		0U
#define MDIO_PARAM_DATA_MASK	0xFFFFU
#define CMIC_MIIM_READ_DATA	(PLAT_CMIC_MIIM_BASE + 0x240U)
#define MDIO_READ_DATA_MASK	0xffffU
#define CMIC_MIIM_ADDRESS	(PLAT_CMIC_MIIM_BASE + 0x244U)
#define CMIC_MIIM_CTRL		(PLAT_CMIC_MIIM_BASE + 0x248U)
#define MDIO_CTRL_WRITE_OP	0x1U
#define MDIO_CTRL_READ_OP	0x2U
#define CMIC_MIIM_STAT		(PLAT_CMIC_MIIM_BASE + 0x24cU)
#define MDIO_STAT_DONE		1U

int mdio_write(uint16_t busid, uint16_t phyid, uint32_t reg, uint16_t val);
int mdio_read(uint16_t busid, uint16_t phyid, uint32_t reg);
#endif /* MDIO_H */
