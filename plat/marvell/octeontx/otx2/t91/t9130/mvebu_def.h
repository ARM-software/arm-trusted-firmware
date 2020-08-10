/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef __MVEBU_DEF_H__
#define __MVEBU_DEF_H__

#include <a8k_plat_def.h>

/*
 * CN-9130 has single CP0 inside the package and 2 additional one
 * from MoChi interface. In case of db-9130-modular board the MCI interface
 * is routed to:
 * - on-board CP115 (MCI0)
 * - extension board CP115 (MCI1)
 */
#define CP_COUNT		CP_NUM
#define MVEBU_SOC_AP807		1
#define I2C_SPD_ADDR		0x53	/* Access SPD data */
#define I2C_SPD_P0_ADDR		0x36	/* Select SPD data page 0 */

#endif /* __MVEBU_DEF_H__ */
