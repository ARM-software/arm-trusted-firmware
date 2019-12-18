/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOARD_INFO_H
#define BOARD_INFO_H

#define IHOST_REG_INTEGRATED 0
#define IHOST_REG_EXT_PROGRAMMABLE 1
#define IHOST_REG_EXT_FIXED 2

#if defined(IHOST_REG_TYPE)
	#if ((IHOST_REG_TYPE != IHOST_REG_INTEGRATED) && \
		(IHOST_REG_TYPE != IHOST_REG_EXT_PROGRAMMABLE) && \
		(IHOST_REG_TYPE != IHOST_REG_EXT_FIXED))
		#error "IHOST_REG_TYPE not valid"
	#endif
#else
	#define IHOST_REG_TYPE IHOST_REG_INTEGRATED
#endif

#define VDDC_REG_INTEGRATED 0
#define VDDC_REG_EXT_PROGRAMMABLE 1
#define VDDC_REG_EXT_FIXED 2

#if defined(VDDC_REG_TYPE)
	#if ((VDDC_REG_TYPE != VDDC_REG_INTEGRATED) && \
		(VDDC_REG_TYPE != VDDC_REG_EXT_PROGRAMMABLE) && \
		(VDDC_REG_TYPE != VDDC_REG_EXT_FIXED))
		#error "VDDC_REG_TYPE not valid"
	#endif
#else
	#define VDDC_REG_TYPE VDDC_REG_INTEGRATED
#endif

#endif
