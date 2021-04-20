/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* DFX sub-FID */
#define MV_SIP_DFX_THERMAL_INIT		1
#define MV_SIP_DFX_THERMAL_READ		2
#define MV_SIP_DFX_THERMAL_IS_VALID	3
#define MV_SIP_DFX_THERMAL_IRQ		4
#define MV_SIP_DFX_THERMAL_THRESH	5
#define MV_SIP_DFX_THERMAL_SEL_CHANNEL	6

#define MV_SIP_DFX_SREAD		20
#define MV_SIP_DFX_SWRITE		21

int mvebu_dfx_thermal_handle(u_register_t func, u_register_t *read,
			     u_register_t x2, u_register_t x3);
int mvebu_dfx_misc_handle(u_register_t func, u_register_t *read,
			  u_register_t addr, u_register_t val);
