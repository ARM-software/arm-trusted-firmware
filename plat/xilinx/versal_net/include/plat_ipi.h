/*
 * Copyright (c) 2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Versal IPI management enums and defines */

#ifndef PLAT_IPI_H
#define PLAT_IPI_H

#include <stdint.h>

#include <ipi.h>

/*********************************************************************
 * IPI agent IDs macros
 ********************************************************************/
#define IPI_ID_PMC	1U
#define IPI_ID_APU	2U
#define IPI_ID_RPU0	3U
#define IPI_ID_RPU1	4U
#define IPI_ID_3	5U
#define IPI_ID_4	6U
#define IPI_ID_5	7U
#define IPI_ID_MAX	8U

/*********************************************************************
 * IPI message buffers
 ********************************************************************/
#define IPI_BUFFER_BASEADDR	(0xEB3F0000U)

#define IPI_BUFFER_APU_BASE	(IPI_BUFFER_BASEADDR + 0x400U)
#define IPI_BUFFER_PMC_BASE	(IPI_BUFFER_BASEADDR + 0x200U)

#define IPI_BUFFER_TARGET_APU_OFFSET	0x80U
#define IPI_BUFFER_TARGET_PMC_OFFSET	0x40U

#define IPI_BUFFER_LOCAL_BASE	IPI_BUFFER_APU_BASE
#define IPI_BUFFER_REMOTE_BASE	IPI_BUFFER_PMC_BASE

#define IPI_BUFFER_TARGET_LOCAL_OFFSET	IPI_BUFFER_TARGET_APU_OFFSET
#define IPI_BUFFER_TARGET_REMOTE_OFFSET	IPI_BUFFER_TARGET_PMC_OFFSET

#define IPI_BUFFER_MAX_WORDS	8

#define IPI_BUFFER_REQ_OFFSET	0x0U
#define IPI_BUFFER_RESP_OFFSET	0x20U

/*********************************************************************
 * Platform specific IPI API declarations
 ********************************************************************/

/* Configure IPI table for versal_net */
void versal_net_ipi_config_table_init(void);

#endif /* PLAT_IPI_H */
