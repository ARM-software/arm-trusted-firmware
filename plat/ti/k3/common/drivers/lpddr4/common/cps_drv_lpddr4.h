/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef CPS_DRV_H_
#define CPS_DRV_H_

#ifdef DEMO_TB
#include <cdn_demo.h>
#else
#include <lib/mmio.h>
#endif

#define CPS_REG_READ(reg) (cps_regread((volatile uint32_t *)(reg)))

#define CPS_REG_WRITE(reg, value) (cps_regwrite((volatile uint32_t *)(reg), (uint32_t)(value)))

#define CPS_FLD_MASK(fld)  (fld ## _MASK)
#define CPS_FLD_SHIFT(fld) (fld ## _SHIFT)
#define CPS_FLD_WIDTH(fld) (fld ## _WIDTH)
#define CPS_FLD_WOCLR(fld) (fld ## _WOCLR)
#define CPS_FLD_WOSET(fld) (fld ## _WOSET)

#define CPS_FLD_READ(fld, reg_value) (cps_fldread((uint32_t)(CPS_FLD_MASK(fld)),  \
						  (uint32_t)(CPS_FLD_SHIFT(fld)), \
						  (uint32_t)(reg_value)))

#define CPS_FLD_WRITE(fld, reg_value, value) (cps_fldwrite((uint32_t)(CPS_FLD_MASK(fld)),  \
							   (uint32_t)(CPS_FLD_SHIFT(fld)), \
							   (uint32_t)(reg_value), (uint32_t)(value)))

#define CPS_FLD_SET(fld, reg_value) (cps_fldset((uint32_t)(CPS_FLD_WIDTH(fld)), \
						(uint32_t)(CPS_FLD_MASK(fld)),  \
						(uint32_t)(CPS_FLD_WOCLR(fld)), \
						(uint32_t)(reg_value)))

#ifdef CLR_USED
#define CPS_FLD_CLEAR(reg, fld, reg_value) (cps_fldclear((uint32_t)(CPS_FLD_WIDTH(fld)), \
							 (uint32_t)(CPS_FLD_MASK(fld)),  \
							 (uint32_t)(CPS_FLD_WOSET(fld)), \
							 (uint32_t)(CPS_FLD_WOCLR(fld)), \
							 (uint32_t)(reg_value)))

#endif
static inline uint32_t cps_regread(volatile uint32_t *reg);
static inline uint32_t cps_regread(volatile uint32_t *reg)
{
	return mmio_read_32((uintptr_t)reg);
}

static inline void cps_regwrite(volatile uint32_t *reg, uint32_t value);
static inline void cps_regwrite(volatile uint32_t *reg, uint32_t value)
{
	mmio_write_32((uintptr_t)reg, value);
}

static inline uint32_t cps_fldread(uint32_t mask, uint32_t shift, uint32_t reg_value);
static inline uint32_t cps_fldread(uint32_t mask, uint32_t shift, uint32_t reg_value)
{
	uint32_t result = (reg_value & mask) >> shift;

	return result;
}

static inline uint32_t cps_fldwrite(uint32_t mask, uint32_t shift, uint32_t reg_value, uint32_t value);
static inline uint32_t cps_fldwrite(uint32_t mask, uint32_t shift, uint32_t reg_value, uint32_t value)
{
	uint32_t new_value = (value << shift) & mask;

	new_value = (reg_value & ~mask) | new_value;
	return new_value;
}

static inline uint32_t cps_fldset(uint32_t width, uint32_t mask, uint32_t is_woclr, uint32_t reg_value);
static inline uint32_t cps_fldset(uint32_t width, uint32_t mask, uint32_t is_woclr, uint32_t reg_value)
{
	uint32_t new_value = reg_value;

	if ((width == 1U) && (is_woclr == 0U))
		new_value |= mask;

	return new_value;
}

#ifdef CLR_USED
static inline uint32_t cps_fldclear(uint32_t width, uint32_t mask, uint32_t is_woset, uint32_t is_woclr, uint32_t reg_value);
static inline uint32_t cps_fldclear(uint32_t width, uint32_t mask, uint32_t is_woset, uint32_t is_woclr, uint32_t reg_value)
{
	uint32_t new_value = reg_value;

	if ((width == 1U) && (is_woset == 0U))
		new_value = (new_value & ~mask) | ((is_woclr != 0U) ? mask : 0U);

	return new_value;
}
#endif /* CLR_USED */

#endif /* CPS_DRV_H_ */
