/*
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
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

/*********************************************************************
 * IPI message buffers
 ********************************************************************/
#define IPI_BUFFER_BASEADDR	0xFF3F0000U

#define IPI_LOCAL_ID		IPI_ID_APU
#define IPI_REMOTE_ID		IPI_ID_PMC

#define IPI_BUFFER_LOCAL_BASE	(IPI_BUFFER_BASEADDR + (IPI_LOCAL_ID * 0x200U))
#define IPI_BUFFER_REMOTE_BASE	(IPI_BUFFER_BASEADDR + (IPI_REMOTE_ID * 0x200U))

#define IPI_BUFFER_TARGET_LOCAL_OFFSET	(IPI_LOCAL_ID * 0x40U)
#define IPI_BUFFER_TARGET_REMOTE_OFFSET	(IPI_REMOTE_ID * 0x40U)

#define IPI_BUFFER_MAX_WORDS	8

#define IPI_BUFFER_REQ_OFFSET	0x0U
#define IPI_BUFFER_RESP_OFFSET	0x20U

/*********************************************************************
 * Platform specific IPI API declarations
 ********************************************************************/

/* Configure IPI table for versal */
void versal_ipi_config_table_init(void);

/* IPI registers and bitfields */
#define PMC_REG_BASE		U(0xFF320000)
#define PMC_IPI_TRIG_BIT	(1U << 1U)
#define IPI0_REG_BASE		U(0xFF330000)
#define IPI0_TRIG_BIT		(1U << 2U)
#define IPI1_REG_BASE		U(0xFF340000)
#define IPI1_TRIG_BIT		(1U << 3U)
#define IPI2_REG_BASE		U(0xFF350000)
#define IPI2_TRIG_BIT		(1U << 4U)
#define IPI3_REG_BASE		U(0xFF360000)
#define IPI3_TRIG_BIT		(1U << 5U)
#define IPI4_REG_BASE		U(0xFF370000)
#define IPI4_TRIG_BIT		(1U << 5U)
#define IPI5_REG_BASE		U(0xFF380000)
#define IPI5_TRIG_BIT		(1U << 6U)

#endif /* PLAT_IPI_H */
