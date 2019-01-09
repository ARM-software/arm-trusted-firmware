/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* ZynqMP IPI management enums and defines */

#ifndef PLAT_IPI_H
#define PLAT_IPI_H

#include <stdint.h>
#include <ipi.h>

/*********************************************************************
 * IPI agent IDs macros
 ********************************************************************/
#define IPI_ID_APU	0U
#define IPI_ID_RPU0	1U
#define IPI_ID_RPU1	2U
#define IPI_ID_PMU0	3U
#define IPI_ID_PMU1	4U
#define IPI_ID_PMU2	5U
#define IPI_ID_PMU3	6U
#define IPI_ID_PL0	7U
#define IPI_ID_PL1	8U
#define IPI_ID_PL2	9U
#define IPI_ID_PL3	10U

/*********************************************************************
 * IPI message buffers
 ********************************************************************/
#define IPI_BUFFER_BASEADDR	0xFF990000U

#define IPI_BUFFER_APU_BASE	(IPI_BUFFER_BASEADDR + 0x400U)
#define IPI_BUFFER_PMU_BASE	(IPI_BUFFER_BASEADDR + 0xE00U)

#define IPI_BUFFER_LOCAL_BASE	IPI_BUFFER_APU_BASE
#define IPI_BUFFER_REMOTE_BASE	IPI_BUFFER_PMU_BASE

#define IPI_BUFFER_TARGET_LOCAL_OFFSET	0x80U
#define IPI_BUFFER_TARGET_REMOTE_OFFSET	0x1C0U

#define IPI_BUFFER_MAX_WORDS	8

#define IPI_BUFFER_REQ_OFFSET	0x0U
#define IPI_BUFFER_RESP_OFFSET	0x20U

/*********************************************************************
 * Platform specific IPI API declarations
 ********************************************************************/

/* Configure IPI table for zynqmp */
void zynqmp_ipi_config_table_init(void);

#endif /* PLAT_IPI_H */
