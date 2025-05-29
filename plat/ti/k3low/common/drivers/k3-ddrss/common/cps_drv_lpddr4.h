/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2026 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef CPS_DRV_H_
#define CPS_DRV_H_

#include <lib/mmio.h>

#include <plat_utils.h>

#define CPS_REG_READ(reg) (mmio_read_32((uintptr_t)(reg)))

#define CPS_REG_WRITE(reg, value) (mmio_write_32((volatile uintptr_t)(reg), (uint32_t)(value)))

#define CPS_FLD_MASK(fld)  (fld ## _MASK)

#define CPS_FLD_READ(fld, reg_value) (FIELD_GET((CPS_FLD_MASK(fld)), (uint32_t)(reg_value)))

#define CPS_FLD_WRITE(fld, reg_value, value) \
	({ \
		(reg_value & ~(CPS_FLD_MASK(fld))) | FIELD_PREP(CPS_FLD_MASK(fld), value); \
	})

#define CPS_FLD_SET(fld, reg_value) \
	({ \
		(reg_value | CPS_FLD_MASK(fld)); \
	})

#ifdef CLR_USED
#define CPS_FLD_CLEAR(reg, fld, reg_value) \
	({ \
		reg_value & ~(CPS_FLD_MASK(fld)); \
	})
#endif

#endif /* CPS_DRV_H_ */
